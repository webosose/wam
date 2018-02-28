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

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include "ApplicationDescription.h"
#include "LogManager.h"
#include "WebAppManagerConfig.h"
#include "WebAppManager.h"
#include "WebPageBase.h"

class WebAppBasePrivate
{
public:
    WebAppBasePrivate(WebAppBase *d)
    : q(d)
    , m_page(0)
    , m_keepAlive(false)
    , m_forceClose(false)
    , m_appDesc(0)
    {
    }

    ~WebAppBasePrivate()
    {
        if(m_page)
            delete m_page;

        LOG_DEBUG("Delete webapp base for App ID %s", qPrintable(m_appId));
    }

    void createActivity()
    {
        if (m_page)
            WebAppManager::instance()->requestActivity(q);
    }

    void destroyActivity() {}

public:
    WebAppBase *q;
    WebPageBase* m_page;
    bool m_keepAlive;
    bool m_forceClose;
    QString m_launchingAppId;
    QString m_appId;
    QString m_instanceId;
    QString m_url;
    ApplicationDescription* m_appDesc;
};

WebAppBase::WebAppBase()
    : m_preloadState(NONE_PRELOAD)
    , m_addedToWindowMgr(false)
    , m_scaleFactor(1.0f)
    , d(new WebAppBasePrivate(this))
    , m_needReload(false)
    , m_crashed(false)
    , m_hiddenWindow(false)
    , m_wasContainerApp(false)
{
}

WebAppBase::~WebAppBase()
{
    LOG_INFO(MSGID_WEBAPP_CLOSED, 2, PMLOGKS("APP_ID", appId().isEmpty() ? "unknown" : qPrintable(appId())), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "");
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

void WebAppBase::setWasContainerApp(bool contained)
{
    m_wasContainerApp = contained;
}

bool WebAppBase::wasContainerApp() const
{
    return m_wasContainerApp;
}

void WebAppBase::setKeepAlive(bool keepAlive)
{
    d->m_keepAlive = keepAlive;
}

bool WebAppBase::keepAlive()
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

void WebAppBase::setAppId(const QString& appId)
{
    d->m_appId = appId;
}

void WebAppBase::setLaunchingAppId(const QString& appId)
{
    d->m_launchingAppId = appId;
}

QString WebAppBase::appId() const
{
    return d->m_appId;
}

void WebAppBase::setInstanceId(const QString& instanceId)
{
    d->m_instanceId = instanceId;
}

QString WebAppBase::instanceId() const
{
    return d->m_instanceId;
}

QString WebAppBase::url() const
{
    return d->m_url;
}

QString WebAppBase::launchingAppId() const
{
    return d->m_launchingAppId;
}

ApplicationDescription* WebAppBase::getAppDescription() const
{
    return d->m_appDesc;
}

void WebAppBase::cleanResources()
{
    // does nothing if m_page has already been deleted and set to 0 by ~WindowedWebApp
    d->destroyActivity();

    if (d->m_appDesc) {
        delete d->m_appDesc;
        d->m_appDesc = 0;
    }
}

int WebAppBase::currentUiWidth()
{
    return WebAppManager::instance()->currentUiWidth();
}

int WebAppBase::currentUiHeight()
{
    return WebAppManager::instance()->currentUiHeight();
}

void WebAppBase::setActiveAppId(QString id)
{
    WebAppManager::instance()->setActiveAppId(id);
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
    connect(d->m_page, SIGNAL(webPageUrlChanged()), this, SLOT(webPageUrlChangedSlot()));
    connect(d->m_page, SIGNAL(webPageLoadFinished()), this, SLOT(webPageLoadFinishedSlot()));
    connect(d->m_page, SIGNAL(webPageLoadFailed(int)), this, SLOT(webPageLoadFailedSlot(int)));
    d->createActivity();
}

WebPageBase* WebAppBase::detach(void)
{
    WebPageBase* p = d->m_page;

    disconnect(d->m_page, 0, this, 0);
    unobserve(d->m_page);

    d->m_page = 0;
    return p;
}

void WebAppBase::relaunch(const QString& args, const QString& launchingAppId)
{
    LOG_INFO(MSGID_APP_RELAUNCH, 3,
             PMLOGKS("APP_ID", qPrintable(appId())),
             PMLOGKFV("PID", "%d", page()->getWebProcessPID()),
             PMLOGKS("LAUNCHING_APP_ID", qPrintable(launchingAppId)), "");
    if (getHiddenWindow()) {
        setHiddenWindow(false);

        clearPreloadState();

        if (WebAppManager::instance()->config()->isCheckLaunchTimeEnabled())
            startLaunchTimer();

        if (keepAlive() && (page()->progress() != 100))
            m_addedToWindowMgr = false;

        // if we're already loaded, then show, else clear the hidden flag, and
        // show as normal when loaded and ready to render
        if(m_addedToWindowMgr || page()->progress() == 100)
            showWindow();
    }

    if (getCrashState()) {
        LOG_INFO(MSGID_APP_RELAUNCH, 2,
                 PMLOGKS("APP_ID", qPrintable(appId())),
                 PMLOGKFV("PID", "%d", page()->getWebProcessPID()),
                 "Crashed in Background; Reluad Default page");
        page()->reloadDefaultPage();
        setCrashState(false);
    }

    if(d->m_page) {
        WebPageBase* page = d->m_page;
        // try to do relaunch!!
        if(!(page->relaunch(args, launchingAppId))) {
          LOG_INFO(MSGID_APP_RELAUNCH, 2,
                   PMLOGKS("APP_ID", qPrintable(appId())),
                   PMLOGKFV("PID", "%d", page->getWebProcessPID()),
                   "Can't handle Relaunch now, backup the args and handle it after page loading finished");
            // if relaunch hasn't beeh executed, then set and wait till currnt page loading is finished
            m_inProgressRelaunchParams = args;
            m_inProgressRelaunchLaunchingAppId = launchingAppId;
            return;
        }

        if(d->m_appDesc && !(d->m_appDesc->handlesRelaunch())) {
            LOG_DEBUG("[%s] m_appDesc->handlesRelaunch : false, call raise() to make it full screen", qPrintable(appId()));
            raise();
        } else {
            LOG_INFO(MSGID_APP_RELAUNCH, 2,
                     PMLOGKS("APP_ID", qPrintable(appId())),
                     PMLOGKFV("PID", "%d", page->getWebProcessPID()),
                     "handlesRelanch : true; Do not call raise()");
        }
    }
}

void WebAppBase::webPageLoadFinishedSlot()
{
    doPendingRelaunch();
}

void WebAppBase::doPendingRelaunch()
{
    if(m_inProgressRelaunchLaunchingAppId.size() || m_inProgressRelaunchParams.size()) {
      LOG_INFO(MSGID_APP_RELAUNCH, 2,
               PMLOGKS("APP_ID", qPrintable(appId())),
               PMLOGKFV("PID", "%d", page()->getWebProcessPID()),
               "Page loading --> done; Do pending Relaunch");
        relaunch(m_inProgressRelaunchParams, m_inProgressRelaunchLaunchingAppId);

        m_inProgressRelaunchParams.clear();
        m_inProgressRelaunchLaunchingAppId.clear();
    }
}

void WebAppBase::webPageClosePageRequestedSlot()
{
    LOG_INFO(MSGID_WINDOW_CLOSED_JS, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "");
    WebAppManager::instance()->closeApp(appId().toStdString());
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
#ifndef PRELOADMANAGER_ENABLED
    if (m_wasContainerApp)
        WebAppManager::instance()->startContainerTimer();
#endif

    // Set the accessibility after the application launched
    // because the chromium can generate huge amount of AXEvent during app loading.
    setUseAccessibility(WebAppManager::instance()->isAccessibilityEnabled());
}

void WebAppBase::showWindowSlot()
{
    showWindow();
}

void WebAppBase::setAppDescription(ApplicationDescription* appDesc)
{
    if (d->m_appDesc) {
        delete d->m_appDesc;
        d->m_appDesc = 0;
    }
    d->m_appDesc = appDesc;

    // set appId here from appDesc
   d->m_appId = QString::fromStdString(appDesc->id());

   if (appDesc->widthOverride() && appDesc->heightOverride()) {
        float scaleX = static_cast<float>(currentUiWidth()) / appDesc->widthOverride();
        float scaleY = static_cast<float>(currentUiHeight()) / appDesc->heightOverride();
        m_scaleFactor = (scaleX < scaleY) ? scaleX : scaleY;
   }
}

void WebAppBase::setAppProperties(QString properties)
{
    QJsonDocument doc = QJsonDocument::fromJson(properties.toStdString().c_str());
    QJsonObject obj = doc.object();

    if (obj["keepAlive"].toBool())
        setKeepAlive(true);
    else
        setKeepAlive(false);

    if (obj["launchedHidden"].toBool())
        setHiddenWindow(true);
}

void WebAppBase::setPreloadState(QString properties)
{
    QJsonDocument doc = QJsonDocument::fromJson(properties.toStdString().c_str());
    QJsonObject obj = doc.object();

    std::string preload = obj["preload"].toString().toStdString().c_str();

    if (preload == "full") {
        m_preloadState = FULL_PRELOAD;
    }
    else if (preload == "partial") {
        m_preloadState = PARTIAL_PRELOAD;
    }
    else if (preload == "minimal") {
        m_preloadState = MINIMAL_PRELOAD;
    }
    else if (obj["launchedHidden"].toBool()) {
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
        case PARTIAL_PRELOAD :
            d->m_page->setBlockWriteDiskcache(true);
            d->m_page->suspendWebPageMedia();
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
        case PARTIAL_PRELOAD :
            d->m_page->setBlockWriteDiskcache(false);
            d->m_page->resumeWebPageMedia();
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

void WebAppBase::webPageUrlChangedSlot()
{
    d->m_url = d->m_page->url().toString();
}

void WebAppBase::setPreferredLanguages(QString language)
{
    if (!d->m_page)
        return;
    d->m_page->setPreferredLanguages(language);
    d->m_page->sendLocaleChangeEvent(language);
}

void WebAppBase::handleWebAppMessage(WebAppManager::WebAppMessageType type, const QString& message)
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
    connect(d->m_page, SIGNAL(closeCallbackExecuted()),this, SLOT(closeWebAppSlot()));
    connect(d->m_page, SIGNAL(timeoutExecuteCloseCallback()),this, SLOT(closeWebAppSlot()));
    connect(d->m_page, SIGNAL(closingAppProcessDidCrashed()),this, SLOT(closeWebAppSlot()));
    page()->executeCloseCallback(forceClose());
    LOG_INFO(MSGID_EXECUTE_CLOSECALLBACK, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "");
}

void WebAppBase::closeWebAppSlot()
{
    LOG_INFO(MSGID_CLEANRESOURCE_COMPLETED, 2, PMLOGKS("APP_ID", qPrintable(appId())), PMLOGKFV("PID", "%d", page()->getWebProcessPID()), "closeCallback/about:blank is DONE");
    WebAppManager::instance()->removeClosingAppList(appId());
#ifdef PRELOADMANAGER_ENABLED
    if (appId() == WebAppManager::instance()->getContainerAppId())
        WebAppManager::instance()->closeContainerApp();
    else
#endif
        delete this;
}

void WebAppBase::dispatchUnload()
{
    connect(d->m_page, SIGNAL(didDispatchUnload()),this, SLOT(closeWebAppSlot()));
    connect(d->m_page, SIGNAL(closingAppProcessDidCrashed()),this, SLOT(closeWebAppSlot()));
    page()->cleanResources();
}

void WebAppBase::onCursorVisibilityChanged(const QString& jsscript)
{
    WebAppManager::instance()->sendEventToAllAppsAndAllFrames(jsscript);
}

void WebAppBase::serviceCall(const QString& url, const QString& payload, const QString& appId)
{
    LOG_INFO(MSGID_SERVICE_CALL, 2, PMLOGKS("APP_ID", qPrintable(appId)), PMLOGKS("URL", qPrintable(url)), "");
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
