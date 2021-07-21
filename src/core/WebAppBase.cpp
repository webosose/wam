// Copyright (c) 2008-2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "WebAppBase.h"

#include "ApplicationDescription.h"
#include "LogManager.h"
#include "Utils.h"
#include "WebAppManager.h"
#include "WebAppManagerConfig.h"
#include "WebPageBase.h"

class WebAppBasePrivate
{
public:
    WebAppBasePrivate(WebAppBase *d)
    : q(d)
    , m_page(0)
    , m_keepAlive(false)
    , m_forceClose(false)
    , m_appDesc(nullptr)
    {
    }

    ~WebAppBasePrivate()
    {
        delete m_page;

        LOG_DEBUG("Delete webapp base for Instance %s of App ID %s", m_instanceId.c_str(), m_appId.c_str());
    }
public:
    WebAppBase *q;
    WebPageBase* m_page;
    bool m_keepAlive;
    bool m_forceClose;
    std::string m_launchingAppId;
    std::string m_appId;
    std::string m_instanceId;
    std::string m_url;
    std::shared_ptr<ApplicationDescription> m_appDesc;

private:
    WebAppBasePrivate(const WebAppBasePrivate&) = delete;
    WebAppBasePrivate& operator=(const WebAppBasePrivate&) = delete;
};

WebAppBase::WebAppBase()
    : m_preloadState(NONE_PRELOAD)
    , m_addedToWindowMgr(false)
    , m_scaleFactor(1.0f)
    , d(new WebAppBasePrivate(this))
    , m_needReload(false)
    , m_crashed(false)
    , m_hiddenWindow(false)
    , m_closePageRequested(false)
{
}

WebAppBase::~WebAppBase()
{
    LOG_INFO(MSGID_WEBAPP_CLOSED, 3, PMLOGKS("APP_ID", appId().empty() ? "unknown" : appId().c_str()), PMLOGKS("INSTANCE_ID", instanceId().c_str()), PMLOGKFV("PID", "%d", page() ? page()->getWebProcessPID() : 0), "");
    cleanResources();
    delete d;
}

bool WebAppBase::getCrashState()
{
    return m_crashed;
}

void WebAppBase::setCrashState(bool state)
{
    m_crashed = state;
}

void WebAppBase::setHiddenWindow(bool hidden)
{
    m_hiddenWindow = hidden;
}

bool WebAppBase::getHiddenWindow()
{
    return m_hiddenWindow;
}

void WebAppBase::setKeepAlive(bool keepAlive)
{
    d->m_keepAlive = keepAlive;
}

bool WebAppBase::keepAlive() const
{
    return d->m_keepAlive;
}

void WebAppBase::setForceClose()
{
    d->m_forceClose = true;
}

bool WebAppBase::forceClose()
{
    return d->m_forceClose;
}

WebPageBase* WebAppBase::page() const
{
    return d->m_page;
}

bool WebAppBase::isWindowed() const
{
    return false;
}

void WebAppBase::setAppId(const std::string& appId)
{
    d->m_appId = appId;
}

void WebAppBase::setLaunchingAppId(const std::string& appId)
{
    d->m_launchingAppId = appId;
}

std::string WebAppBase::appId() const
{
    return d->m_appId;
}

void WebAppBase::setInstanceId(const std::string& instanceId)
{
    d->m_instanceId = instanceId;
}

std::string WebAppBase::instanceId() const
{
    return d->m_instanceId;
}

std::string WebAppBase::url() const
{
    return d->m_url;
}

std::string WebAppBase::launchingAppId() const
{
    return d->m_launchingAppId;
}

ApplicationDescription* WebAppBase::getAppDescription() const
{
    return d->m_appDesc.get();
}

void WebAppBase::cleanResources()
{
    d->m_appDesc.reset();
}

int WebAppBase::currentUiWidth()
{
    return WebAppManager::instance()->currentUiWidth();
}

int WebAppBase::currentUiHeight()
{
    return WebAppManager::instance()->currentUiHeight();
}

void WebAppBase::setActiveInstanceId(const std::string& id)
{
    WebAppManager::instance()->setActiveInstanceId(id);
}

void WebAppBase::forceCloseAppInternal()
{
    WebAppManager::instance()->forceCloseAppInternal(this);
}

void WebAppBase::closeAppInternal()
{
    WebAppManager::instance()->closeAppInternal(this);
}

void WebAppBase::attach(WebPageBase* page)
{
    // connect to the signals of the WebBridge
    // parse up the ApplicationDescription
    if (d->m_page)
        detach();

    d->m_page = page;
    d->m_page->createPalmSystem(this);

    observe(d->m_page);
}

WebPageBase* WebAppBase::detach(void)
{
    WebPageBase* p = d->m_page;

    unobserve(d->m_page);

    d->m_page = 0;
    return p;
}

void WebAppBase::relaunch(const std::string& args, const std::string& launchingAppId)
{
    LOG_INFO(MSGID_APP_RELAUNCH, 4,
             PMLOGKS("APP_ID", appId().c_str()),
             PMLOGKS("INSTANCE_ID", instanceId().c_str()),
             PMLOGKFV("PID", "%d", page()->getWebProcessPID()),
             PMLOGKS("LAUNCHING_APP_ID", launchingAppId.c_str()), "");
    if (getHiddenWindow()) {
        setHiddenWindow(false);

        clearPreloadState();

        if (WebAppManager::instance()->config()->isCheckLaunchTimeEnabled())
            startLaunchTimer();

        if (keepAlive() && (page()->progress() != 100))
            m_addedToWindowMgr = false;

        // if we're already loaded, then show, else clear the hidden flag, and
        // show as normal when loaded and ready to render
        if(m_addedToWindowMgr ||
            (page()->progress() == 100 && page()->hasBeenShown()))
            showWindow();
    }

    if (getCrashState()) {
        LOG_INFO(MSGID_APP_RELAUNCH, 3,
                 PMLOGKS("APP_ID", appId().c_str()),
                 PMLOGKS("INSTANCE_ID", instanceId().c_str()),
                 PMLOGKFV("PID", "%d", page()->getWebProcessPID()),
                 "Crashed in Background; Reluad Default page");
        page()->reloadDefaultPage();
        setCrashState(false);
    }

    if(d->m_page) {
        WebPageBase* page = d->m_page;
        // try to do relaunch!!
        if(!(page->relaunch(args, launchingAppId))) {
          LOG_INFO(MSGID_APP_RELAUNCH, 3,
                   PMLOGKS("APP_ID", appId().c_str()),
                   PMLOGKS("INSTANCE_ID", instanceId().c_str()),
                   PMLOGKFV("PID", "%d", page->getWebProcessPID()),
                   "Can't handle Relaunch now, backup the args and handle it after page loading finished");
            // if relaunch hasn't beeh executed, then set and wait till currnt page loading is finished
            m_inProgressRelaunchParams = args;
            m_inProgressRelaunchLaunchingAppId = launchingAppId;
            return;
        }

        if(d->m_appDesc && !(d->m_appDesc->handlesRelaunch())) {
            LOG_DEBUG("[%s]:[%s] m_appDesc->handlesRelaunch : false, call raise() to make it full screen", appId().c_str(), instanceId().c_str());
            raise();
        } else {
            LOG_INFO(MSGID_APP_RELAUNCH, 3,
                     PMLOGKS("APP_ID", appId().c_str()),
                     PMLOGKS("INSTANCE_ID", instanceId().c_str()),
                     PMLOGKFV("PID", "%d", page->getWebProcessPID()),
                     "handlesRelanch : true; Do not call raise()");
        }
    }
}

void WebAppBase::webPageLoadFinished()
{
    doPendingRelaunch();
}

void WebAppBase::doPendingRelaunch()
{
    if(m_inProgressRelaunchLaunchingAppId.size() || m_inProgressRelaunchParams.size()) {
      LOG_INFO(MSGID_APP_RELAUNCH, 3,
               PMLOGKS("APP_ID", appId().c_str()),
               PMLOGKS("INSTANCE_ID", instanceId().c_str()),
               PMLOGKFV("PID", "%d", page()->getWebProcessPID()),
               "Page loading --> done; Do pending Relaunch");
        relaunch(m_inProgressRelaunchParams, m_inProgressRelaunchLaunchingAppId);

        m_inProgressRelaunchParams.clear();
        m_inProgressRelaunchLaunchingAppId.clear();
    }
}

void WebAppBase::webPageClosePageRequested()
{
    LOG_INFO(MSGID_WINDOW_CLOSED_JS, 3, PMLOGKS("APP_ID", appId().c_str()), PMLOGKS("INSTANCE_ID", instanceId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "%s%s", m_closePageRequested ? "duplicated window.close();" : "", isClosing() ? "app is closing; drop this window.close()": "");
    if (isClosing() || m_closePageRequested)
        return;

    m_closePageRequested = true;
    WebAppManager::instance()->closeApp(instanceId());
}

void WebAppBase::stagePreparing()
{
    // just has some perf testing
}

void WebAppBase::stageReady()
{
    // NOT IMPLEMENTED
}

void WebAppBase::showWindow()
{
    // Set the accessibility after the application launched
    // because the chromium can generate huge amount of AXEvent during app loading.
    setUseAccessibility(WebAppManager::instance()->isAccessibilityEnabled());
}


void WebAppBase::setAppDescription(std::shared_ptr<ApplicationDescription> appDesc)
{
    d->m_appDesc = appDesc;

    // set appId here from appDesc
   d->m_appId = appDesc->id();
}

void WebAppBase::setAppProperties(const std::string& properties)
{
    Json::Value json = util::stringToJson(properties);
    bool keepAlive = false;

    if (json["keepAlive"].isBool() && json["keepAlive"].asBool()) {
        keepAlive = true;
    }
    setKeepAlive(keepAlive);

    if (json["launchedHidden"].isBool() && json["launchedHidden"].asBool())
        setHiddenWindow(true);
}

void WebAppBase::setPreloadState(const std::string& properties)
{
    Json::Value obj = util::stringToJson(properties);

    std::string preload = obj["preload"].asString();

    if (preload == "full") {
        m_preloadState = FULL_PRELOAD;
    }
    else if (preload == "semi-full") {
        m_preloadState = SEMI_FULL_PRELOAD;
    }
    else if (preload == "partial") {
        m_preloadState = PARTIAL_PRELOAD;
    }
    else if (preload == "minimal") {
        m_preloadState = MINIMAL_PRELOAD;
    }
    else if (obj["launchedHidden"].asBool()) {
        m_preloadState = PARTIAL_PRELOAD;
    }

    if (m_preloadState != NONE_PRELOAD)
        setHiddenWindow(true);

    // set PreloadEnvironment needs attaching WebPageBase.
    if (!d->m_page)
        return;

    switch (m_preloadState) {
        case FULL_PRELOAD :
            // TODO : implement full preload when rule is set.
            break;
        case SEMI_FULL_PRELOAD:
            d->m_page->setAppPreloadHint(true);
            d->m_page->suspendWebPageMedia();
            break;
        case PARTIAL_PRELOAD :
            d->m_page->setAppPreloadHint(true);
            d->m_page->suspendWebPageMedia();
            d->m_page->deactivateRendererCompositor();
            break;
        case MINIMAL_PRELOAD :
            // TODO : implement minimal preloaded when rule is set.
            break;
        default :
            break;
    }
    d->m_page->setIsPreload(m_preloadState != NONE_PRELOAD ? true : false);
}

void WebAppBase::clearPreloadState()
{
   // set PreloadEnvironment needs attaching WebPageBase.
    if (!d->m_page) {
        m_preloadState = NONE_PRELOAD;
        return;
    }

    switch (m_preloadState) {
        case FULL_PRELOAD :
            // TODO : implement full preload when rule is set.
            break;
        case SEMI_FULL_PRELOAD:
            d->m_page->setAppPreloadHint(false);
            d->m_page->resumeWebPageMedia();
            break;
        case PARTIAL_PRELOAD :
            d->m_page->setAppPreloadHint(false);
            d->m_page->resumeWebPageMedia();
            d->m_page->activateRendererCompositor();
            break;
        case MINIMAL_PRELOAD :
            // TODO : implement minimal preloaded when rule is set.
            break;
        default :
            break;
    }
    m_preloadState = NONE_PRELOAD;
    d->m_page->setIsPreload(false);
}

void WebAppBase::setUiSize(int width, int height) {
    WebAppManager::instance()->setUiSize(width, height);
}

void WebAppBase::setPreferredLanguages(const std::string& language)
{
    if (!d->m_page)
        return;
    d->m_page->setPreferredLanguages(language);
    d->m_page->sendLocaleChangeEvent(language);
}

void WebAppBase::handleWebAppMessage(WebAppManager::WebAppMessageType type, const std::string& message)
{
    if (!d->m_page)
        return;

    if (type == WebAppManager::WebAppMessageType::DeviceInfoChanged)
        d->m_page->handleDeviceInfoChanged(message);
}

void WebAppBase::setUseAccessibility(bool enabled)
{
    bool useAccessibility = false;

    LOG_DEBUG("setUseAccessibility : supportsAudioGuidance = %s, LoadErrorPage = %s",
        d->m_appDesc->supportsAudioGuidance()? "true":"false",
        page()->isLoadErrorPageStart()? "true":"false");

    // Check the supportsAudioGuidance attribute to appinfo.json to use this feature or
    // page is loadErrorPage for each app.
    if (enabled && (d->m_appDesc->supportsAudioGuidance() || page()->isLoadErrorPageStart())) {
        useAccessibility = true;
    }

    page()->setUseAccessibility(useAccessibility);
}

void WebAppBase::executeCloseCallback()
{
    page()->executeCloseCallback(forceClose());
    LOG_INFO(MSGID_EXECUTE_CLOSECALLBACK,3, PMLOGKS("APP_ID", appId().c_str()), PMLOGKS("INSTANCE_ID", instanceId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "");
}

void WebAppBase::closeCallbackExecuted()
{
    closeWebApp();
}

void WebAppBase::timeoutExecuteCloseCallback()
{
    closeWebApp();
}

void WebAppBase::closingAppProcessDidCrashed()
{
    closeWebApp();
}

void WebAppBase::didDispatchUnload()
{
    closeWebApp();
}

void WebAppBase::closeWebApp()
{
    LOG_INFO(MSGID_CLEANRESOURCE_COMPLETED, 3, PMLOGKS("APP_ID", appId().c_str()), PMLOGKS("INSTANCE_ID", instanceId().c_str()), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "closeCallback/about:blank is DONE");
    WebAppManager::instance()->removeClosingAppList(appId());
    delete this;
}

void WebAppBase::dispatchUnload()
{
    page()->cleanResources();
}

void WebAppBase::onCursorVisibilityChanged(const std::string& jsscript)
{
    WebAppManager::instance()->sendEventToAllAppsAndAllFrames(jsscript);
}

void WebAppBase::serviceCall(const std::string& url, const std::string& payload, const std::string& appId)
{
    LOG_INFO(MSGID_SERVICE_CALL, 3, PMLOGKS("APP_ID", appId.c_str()), PMLOGKS("INSTANCE_ID", instanceId().c_str()), PMLOGKS("URL", url.c_str()), "");
    WebAppManager::instance()->serviceCall(url, payload, appId);
}

void WebAppBase::keyboardVisibilityChanged(bool visible, int height) {
    if (page())
        page()->keyboardVisibilityChanged(visible);
}

bool WebAppBase::isClosing() const {
    return d->m_page->isClosing();
}

bool WebAppBase::isCheckLaunchTimeEnabled()
{
    return WebAppManager::instance()->config()->isCheckLaunchTimeEnabled();
}
