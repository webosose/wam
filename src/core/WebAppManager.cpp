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

#include "WebAppManager.h"

#include <string>
#include <sstream>
#include <unistd.h>

#include "ApplicationDescription.h"
#include "ContainerAppManager.h"
#include "DeviceInfo.h"
#include "JsonHelper.h"
#include "LogManager.h"
#include "NetworkStatusManager.h"
#include "PlatformModuleFactory.h"
#include "ServiceSender.h"
#include "StringUtils.h"
#include "WebAppBase.h"
#include "WebAppFactoryManager.h"
#include "WebAppManagerConfig.h"
#include "WebAppManagerService.h"
#include "WebAppManagerTracer.h"
#include "WebPageBase.h"
#include "WebProcessManager.h"
#include "WindowTypes.h"

#include "webos/public/runtime.h"

static const int kContinuousReloadingLimit = 3;

WebAppManager* WebAppManager::instance()
{
    // not a leak -- static variable initializations are only ever done once
    static WebAppManager* sInstance = new WebAppManager();
    return sInstance;
}

WebAppManager::WebAppManager()
    : m_deletingPages(false)
    , m_serviceSender(0)
    , m_containerAppManager(0)
    , m_webProcessManager(0)
    , m_deviceInfo(0)
    , m_webAppManagerConfig(0)
    , m_networkStatusManager(new NetworkStatusManager())
    , m_suspendDelay(0)
    , m_isAccessibilityEnabled(false)
{
}

WebAppManager::~WebAppManager()
{
    if (m_containerAppManager)
        delete m_containerAppManager;
    if (m_serviceSender)
        delete m_serviceSender;
    if (m_webProcessManager)
        delete m_webProcessManager;
    if (m_deviceInfo)
        delete m_deviceInfo;
    if (m_networkStatusManager)
        delete m_networkStatusManager;
}

void WebAppManager::notifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level)
{
    std::list<const WebAppBase*> appList = runningApps();
    for (auto it = appList.begin(); it != appList.end(); ++it) {
        const WebAppBase* app = *it;
        if (app->isActivated() && !app->page()->isPreload())
            app->page()->notifyMemoryPressure(level);
    }
}

void WebAppManager::setPlatformModules(PlatformModuleFactory* factory)
{
    m_webAppManagerConfig = factory->getWebAppManagerConfig();
    m_containerAppManager = factory->getContainerAppManager();
    m_serviceSender = factory->getServiceSender();
    m_webProcessManager = factory->getWebProcessManager();
    m_deviceInfo = factory->getDeviceInfo();

    WebAppFactoryManager::instance();
    loadEnvironmentVariable();
}

bool WebAppManager::run()
{
    loadEnvironmentVariable();
    return true;
}

void WebAppManager::quit()
{
}

void WebAppManager::loadEnvironmentVariable()
{
    m_suspendDelay = m_webAppManagerConfig->getSuspendDelayTime();
    m_webAppManagerConfig->postInitConfiguration();

    if (m_containerAppManager)
        m_containerAppManager->setUseContainerAppOptimization(m_webAppManagerConfig->isUseSystemAppOptimization());
}

void WebAppManager::setUiSize(int width, int height)
{
    if (m_deviceInfo) {
        m_deviceInfo->setDisplayWidth(width);
        m_deviceInfo->setDisplayHeight(height);
    }
}

int WebAppManager::currentUiWidth()
{
    int width = 0;
    if (m_deviceInfo)
        m_deviceInfo->getDisplayWidth(width);
    return width;
}

int WebAppManager::currentUiHeight()
{
    int height = 0;
    if (m_deviceInfo)
        m_deviceInfo->getDisplayHeight(height);
    return height;
}

//FIXME: WebAppManager: qstr2stdstr
bool WebAppManager::getSystemLanguage(QString &value)
{
    if (!m_deviceInfo) return false;
    return m_deviceInfo->getSystemLanguage(value);
}

//FIXME: WebAppManager: qstr2stdstr
bool WebAppManager::getDeviceInfo(QString name, QString &value)
{
    if (!m_deviceInfo) return false;
    return m_deviceInfo->getDeviceInfo(name, value);
}

static void buildAppJsTriggers(std::string &eventJS, std::string &versionJS,
                               const ApplicationDescription *app,
                               const std::string &launchDetail)
{
    using std::string;
    using std::stringstream;

    string detail = launchDetail.empty() ? "{}" : launchDetail;
    string title = app->title();
    string folderPath = app->folderPath();
    string containerCSS = app->containerCSS();
    string containerJS = app->containerJS();

    replaceSubstrings(detail, "'", "\\'");
    replaceSubstrings(title, "'", "\\'");
    replaceSubstrings(folderPath, "'", "\\'");
    replaceSubstrings(containerCSS, "'", "\\'");
    replaceSubstrings(containerJS, "'", "\\'");

    stringstream ejs;
    ejs << "setTimeout(function () {"
        << "    var launchEvent=new CustomEvent('webOSContainer', { detail: " << detail << " });"
        << "    launchEvent.containerName='" << title << "';"
        << "    launchEvent.containerDirectory='" << folderPath << "';"
        << "    launchEvent.containerStyle='" << containerCSS << "';"
        << "    launchEvent.containerScript='" << containerJS << "';"
        << "    document.dispatchEvent(launchEvent);"
        << "}, 1);";
    eventJS = ejs.str();


    string version = app->enyoBundleVersion();
    if (!version.empty()) {
        stringstream vjs;
        vjs << "if (container.setVersion != null) {"
            << "    container.setVersion('" << version << "');"
            << "}";
        versionJS = vjs.str();
    }
}

void WebAppManager::onLaunchContainerBasedApp(const std::string& url, const std::string& winType,
                                              const ApplicationDescription* appDesc,
                                              const std::string& args, const std::string& launchingAppId)
{
    if (!m_containerAppManager)
        return;

    std::string appId;
    WebAppBase *app = m_containerAppManager->getContainerApp();
    WebPageBase *page = app->page();

    LOG_DEBUG("[%s] WebAppManager::onLaunchContainerBasedApp(); ", appDesc->id().c_str());

    app->setHiddenWindow(false);
    page->resetStateToMarkNextPaintForContainer();

    // set use launching time optimization true while app loading.
    page->setUseLaunchOptimization(true);

    // set using enyo system app specific optimization if flag is set
    if (m_webAppManagerConfig->isUseSystemAppOptimization())
        page->setUseSystemAppOptimization(true);

    //FIXME: WebPage: qstr2stdstr
    page->setAppId(QString::fromStdString(appDesc->id()));
    page->updateDatabaseIdentifier();

    if (winType == WT_FLOATING)
        page->setEnableBackgroundRun(appDesc->isEnableBackgroundRun());
    page->replaceBaseUrl(QUrl(url.c_str()));
    page->setDefaultUrl(QUrl(url.c_str()));

    app->setAppDescription((ApplicationDescription *)appDesc);
    app->setAppProperties(args);
    app->setPreloadState(args);

    app->setLaunchingAppId(launchingAppId);
    if (m_webAppManagerConfig->isCheckLaunchTimeEnabled())
        app->startLaunchTimer();

    webPageRemoved(page);

    appId = appDesc->id();
    page->setApplicationDescription((ApplicationDescription *)appDesc);
    page->setLaunchParams(args.c_str());

    app->setWasContainerApp(true);

    app->configureWindow(winType);
    page->updatePageSettings();
    page->reloadExtensionData();

    //repost web process status to QoSM
    postWebProcessCreated(appId.c_str(), getWebProcessId(appId));

    std::string eventJS, versionJS;
    buildAppJsTriggers(eventJS, versionJS, appDesc, args);
    page->evaluateJavaScript(eventJS);
    page->evaluateJavaScript(versionJS);

    webPageAdded(page);

    PMTRACE("APP_ATTACHED_TO_CONTAINER");
    LOG_INFO_WITH_CLOCK(MSGID_APP_ATTACHED_TO_CONTAINER, 4,
            PMLOGKS("PerfType", "AppLaunch"), PMLOGKS("PerfGroup", qPrintable(page->appId())),
            PMLOGKS("APP_ID", qPrintable(page->appId())), PMLOGKFV("PID", "%d", page->getWebProcessPID()), "");

    m_containerAppManager->resetContainerAppManager();

    if (m_appVersion.find(appId) != m_appVersion.end()) {
        if (m_appVersion[appId] != appDesc->version()) {
            app->setNeedReload(true);
            m_appVersion[appId] = appDesc->version();
        }
    }
    else {
        m_appVersion[appId] = appDesc->version();
    }
}

void WebAppManager::onRelaunchApp(const std::string& instanceId, const std::string& appId, const std::string& args, const std::string& launchingAppId)
{
    WebAppBase* app = findAppById(appId);

    if (!app) {
        LOG_WARNING(MSGID_APP_RELAUNCH, 0, "Failed to relaunch due to no running app");
        return;
    }

    // Do not relaunch when preload args is setted
    // luna-send -n 1 luna://com.webos.applicationManager/launch '{"id":<AppId> "preload":<PreloadState> }'
    Json::Value obj;
    readJsonFromString(args, obj);

    if (app->instanceId() == instanceId
        && !obj["preload"].isString()
        && obj["launchedHidden"].isBool()
        && !obj["launchedHidden"].asBool()) {
        app->relaunch(args.c_str(), launchingAppId.c_str());
    } else {
        LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", qPrintable(app->appId())), PMLOGKFV("PID", "%d", app->page()->getWebProcessPID()), "Relaunch with preload option, ignore");
    }
}

std::string WebAppManager::onLaunchContainerApp(const std::string& appDesc)
{
    int errorCode = 0;
    std::string instanceId = generateInstanceId();

    WebAppBase* containerApp = m_containerAppManager->launchContainerApp(appDesc, instanceId, errorCode);
    if (containerApp) {
        webPageAdded(containerApp->page());
        return instanceId;
    }

    return "";
}

bool WebAppManager::purgeSurfacePool(uint32_t pid)
{
    return true; // Deprecated (2016-04-01)
}

bool WebAppManager::isDiscardCodeCacheRequired()
{
    return false; // Deprecated (2016-04-01)
}

bool WebAppManager::setInspectorEnable(const std::string& appId)
{
     // 1. find appId from then running App List,
    for (const auto &app : m_appList) {
        if(appId == app->page()->appId().toStdString()) {
            LOG_DEBUG("[%s] setInspectorEnable", appId.c_str());
            app->page()->setInspectorEnable();
            return true;
        }
    }
    return false;
}

void WebAppManager::discardCodeCache(uint32_t pid)
{
    // Deprecated (2016-04-01)
}

bool WebAppManager::onKillApp(const std::string& appId)
{
    WebAppBase* app = findAppById(appId);
    if (!app) {
        LOG_INFO(MSGID_KILL_APP, 1, PMLOGKS("APP_ID", appId.c_str()), "App doesn't exist; return");
        return false;
    }

    closeAppInternal(app);
    return true;
}

std::list<const WebAppBase*> WebAppManager::runningApps()
{
    std::list<const WebAppBase*> apps;

    for (AppList::const_iterator it = m_appList.begin(); it != m_appList.end(); ++it) {
        apps.push_back(*it);
    }

    return apps;
}

std::list<const WebAppBase*> WebAppManager::runningApps(uint32_t pid)
{
    std::list<const WebAppBase*> apps;

    for (AppList::const_iterator it = m_appList.begin(); it != m_appList.end(); ++it) {
        WebAppBase* app = (*it);

        if (app->page()->getWebProcessPID() == pid)
            apps.push_back(app);
    }

    return apps;
}

WebAppBase* WebAppManager::onLaunchUrl(const std::string& url, const std::string& winType,
                                       const ApplicationDescription* appDesc, const std::string& instanceId,
                                       const std::string& args, const std::string& launchingAppId,
                                       int& errCode, std::string& errMsg)
{
    WebAppBase* app = WebAppFactoryManager::instance()->createWebApp(winType, (ApplicationDescription *)appDesc, appDesc->subType());

    if (!app) {
        errCode = ERR_CODE_LAUNCHAPP_UNSUPPORTED_TYPE;
        errMsg = err_unsupportedType;
        return 0;
    }

    WebPageBase* page = WebAppFactoryManager::instance()->createWebPage(winType, QUrl(url.c_str()), (ApplicationDescription *)appDesc, appDesc->subType(), args);

    //set use launching time optimization true while app loading.
    page->setUseLaunchOptimization(true);

    // Set system app optimization - currently turning off inline caching
    // this include the case that container based app is launched
    // not by using container app.
    if (m_webAppManagerConfig->isUseSystemAppOptimization() && isContainerUsedApp(appDesc)) {
      page->setUseSystemAppOptimization(true);
    }

    if (winType == WT_FLOATING)
      page->setEnableBackgroundRun(appDesc->isEnableBackgroundRun());

    app->setAppDescription((ApplicationDescription *)appDesc);
    app->setAppProperties(args);
    app->setInstanceId(instanceId);
    app->setLaunchingAppId(launchingAppId);
    if (m_webAppManagerConfig->isCheckLaunchTimeEnabled())
      app->startLaunchTimer();
    app->attach(page);
    app->setPreloadState(args);

    page->load();
    webPageAdded(page);

    m_appList.push_back(app);

    if (m_appVersion.find(appDesc->id()) != m_appVersion.end()) {
      if (m_appVersion[appDesc->id()] != appDesc->version()) {
        app->setNeedReload(true);
        m_appVersion[appDesc->id()] = appDesc->version();
      }
    }
    else {
      m_appVersion[appDesc->id()] = appDesc->version();
    }

    LOG_INFO(MSGID_START_LAUNCHURL, 2, PMLOGKS("APP_ID", qPrintable(app->appId())), PMLOGKFV("PID", "%d", app->page()->getWebProcessPID()), "");

#ifndef PRELOADMANAGER_ENABLED
    if (m_containerAppManager && m_containerAppManager->getLaunchContainerAppOnDemand() && getContainerAppProxyID() == m_webProcessManager->getWebProcessProxyID(appDesc)) {
        m_containerAppManager->setLaunchContainerAppOnDemand(false);
        m_containerAppManager->startContainerTimer();
    }
#endif

    return app;
}

void WebAppManager::forceCloseAppInternal(WebAppBase* app)
{
    app->setKeepAlive(false);
    closeAppInternal(app);
}

void WebAppManager::removeClosingAppList(const std::string& appId)
{
    const auto &it = m_closingAppList.find(appId);
    if (it == m_closingAppList.end())
        return;

   m_closingAppList.erase(it);
}

void WebAppManager::closeAppInternal(WebAppBase* app, bool ignoreCleanResource)
{
    WebPageBase* page = app->page();
    if (page && page->isClosing()) {
        LOG_INFO(MSGID_CLOSE_APP_INTERNAL, 2, PMLOGKS("APP_ID", qPrintable(app->appId())), PMLOGKFV("PID", "%d", app->page()->getWebProcessPID()), "In Closing; return");
        return;
    }

    LOG_INFO(MSGID_CLOSE_APP_INTERNAL, 2, PMLOGKS("APP_ID", qPrintable(app->appId())), PMLOGKFV("PID", "%d", app->page()->getWebProcessPID()), "");

    std::string type = app->getAppDescription()->defaultWindowType();
    appDeleted(app);
    webPageRemoved(app->page());
    removeWebAppFromWebProcessInfoMap(app->appId().toStdString());
    postRunningAppList();
    m_lastCrashedAppIds = std::unordered_map<std::string, int>();

    // Set m_isClosing flag first, this flag will be checked in web page suspending
    page->setClosing(true);
    app->deleteSurfaceGroup();
    // Do suspend WebPage
    if (type == "overlay")
        app->hide(true);
    else
        app->onStageDeactivated();

    if (ignoreCleanResource)
        delete app;
    else {
        m_closingAppList.emplace(app->appId().toStdString(), app); // FIXME: WebApp: qstr2stdstr

        if (app == getContainerApp())
            m_containerAppManager->closeContainerApp();
        else if (page->isRegisteredCloseCallback()) {
            LOG_INFO(MSGID_CLOSE_APP_INTERNAL, 2, PMLOGKS("APP_ID", qPrintable(app->appId())), PMLOGKFV("PID", "%d", app->page()->getWebProcessPID()), "CloseCallback; execute");
            app->executeCloseCallback();
        }
        else {
            LOG_INFO(MSGID_CLOSE_APP_INTERNAL, 2, PMLOGKS("APP_ID", qPrintable(app->appId())), PMLOGKFV("PID", "%d", app->page()->getWebProcessPID()), "NO CloseCallback; load about:blank");
            app->dispatchUnload();
        }
    }

    deleteWebViewProfile(app->appId().toStdString());
}

bool WebAppManager::closeAllApps(uint32_t pid)
{
    AppList runningApps;

    for (AppList::iterator it = m_appList.begin(); it != m_appList.end(); ++it) {
        WebAppBase* app = (*it);
        if (!pid)
            runningApps.insert(runningApps.end(), app);
        else if (m_webProcessManager->getWebProcessPID(app) == pid)
            runningApps.insert(runningApps.end(), app);
    }

    AppList::iterator it = runningApps.begin();
    while(it != runningApps.end()) {
        WebAppBase* app = (*it);
        forceCloseAppInternal(app);
        // closeAppInternal will cause the app pointed to to become invalid,
        // so remove it from the list so we don't act upon it after that
        it = runningApps.erase(it);
    }

    if (m_containerAppManager) {
        WebAppBase *app = m_containerAppManager->getContainerApp();
        if (!pid || (app && m_webProcessManager->getWebProcessPID(app) == pid))
            m_containerAppManager->closeContainerApp();
    }

    return runningApps.empty();
}

bool WebAppManager::closeContainerApp()
{
    if (!m_containerAppManager)
        return false;
    m_containerAppManager->closeContainerApp();
    postRunningAppList();
    return true;
}

void WebAppManager::webPageAdded(WebPageBase* page)
{
    auto appId = page->appId().toStdString();
    if (m_appPageMap.count(appId) > 0) {
        auto range = m_appPageMap.equal_range(appId);
        for (auto i = range.first; i != range.second; ++i) {
            if (i->second == page) {
                return;
            }
        }
    }

    m_appPageMap.insert(std::make_pair(appId, page));
}

void WebAppManager::webPageRemoved(WebPageBase* page)
{
    if (!m_deletingPages) {
        // Remove from list of pending delete pages
        PageList::iterator iter = std::find(m_pagesToDeleteList.begin(), m_pagesToDeleteList.end(), page);
        if (iter != m_pagesToDeleteList.end()) {
            m_pagesToDeleteList.erase(iter);
        }
    }

    auto appId = page->appId().toStdString();
    if (m_appPageMap.count(appId) > 0) {
        auto range = m_appPageMap.equal_range(appId);
        for (auto i = range.first; i != range.second; ++i) {
            if (i->second == page) {
                m_appPageMap.erase(i);
            }
        }
    }

    m_shellPageMap.erase(appId);
}

void WebAppManager::removeWebAppFromWebProcessInfoMap(const std::string& appId)
{
    // Deprecated (2016-04-01)
}

WebAppBase* WebAppManager::findAppById(const std::string& appId)
{
    for (const auto &app : m_appList) {
        //FIXME: WebApp: qstr2stdstr
        if (app->page() && app->appId().toStdString() == appId)
            return app;
    }
    return 0;
}

WebAppBase* WebAppManager::findAppByInstanceId(const std::string& instanceId)
{
    for (const auto &app : m_appList) {
        if (app->page() && app->instanceId() == instanceId)
            return app;
    }
    return 0;
}

void WebAppManager::appDeleted(WebAppBase* app)
{
    if (!app)
        return;

    std::string appId;
    if (app->page())
        appId = app->appId().toStdString();

    m_appList.remove(app);

    if (!appId.empty())
        m_shellPageMap.erase(appId);
}

// FIXME: WebAppManager: qstr2stdstr
void WebAppManager::setSystemLanguage(QString language)
{
    if (!m_deviceInfo) return;

    m_deviceInfo->setSystemLanguage(language);

    for (AppList::const_iterator it = m_appList.begin(); it != m_appList.end(); ++it)
    {
        WebAppBase* app = (*it);
        app->setPreferredLanguages(language);
    }

    LOG_DEBUG("New system language: %s", language.toStdString().c_str());
}

// FIXME: WebAppManager: qstr2stdstr
void WebAppManager::setDeviceInfo(QString name, QString value)
{
    if (!m_deviceInfo) return;

    QString oldValue;
    if (m_deviceInfo->getDeviceInfo(name, oldValue) && (oldValue == value)) return;

    m_deviceInfo->setDeviceInfo(name, value);
    broadcastWebAppMessage(WebAppMessageType::DeviceInfoChanged, name.toStdString());
    LOG_DEBUG("SetDeviceInfo %s; %s to %s", name.toStdString().c_str(), oldValue.toStdString().c_str(), value.toStdString().c_str());
}

void WebAppManager::broadcastWebAppMessage(WebAppMessageType type, const std::string& message)
{
    for (AppList::const_iterator it = m_appList.begin(); it != m_appList.end(); ++it) {
        WebAppBase* app = (*it);
        app->handleWebAppMessage(type, message);
    }
#ifndef PRELOADMANAGER_ENABLED
    if (m_containerAppManager && m_containerAppManager->getContainerApp()) {
        WebAppBase* container = m_containerAppManager->getContainerApp();
        container->handleWebAppMessage(type, message);
    }
#endif
}

void WebAppManager::requestActivity(WebAppBase* app)
{
    if(m_serviceSender)
        m_serviceSender->requestActivity(app);
}

bool WebAppManager::processCrashed(const std::string& appId) {
    auto containerAppId = m_containerAppManager->getContainerAppId();
    if (m_containerAppManager && (appId == containerAppId)) {
        m_containerAppManager->setContainerAppReady(false);
#ifndef PRELOADMANAGER_ENABLED
        m_containerAppManager->startContainerTimer();
#else
        closeContainerApp();
#endif
        return true;
    }

    WebAppBase* app = findAppById(appId);
    if (!app)
        return false;

    if (app->isWindowed()) {
        if (app->isActivated()) {
            auto id = app->appId().toStdString(); // FIXME: WebApp: qstr2stdstr
            int count = m_lastCrashedAppIds[id];
            m_lastCrashedAppIds[id] = count + 1;

            int reloadingLimit = app->isNormal() ? kContinuousReloadingLimit-1 : kContinuousReloadingLimit;

            if (m_lastCrashedAppIds[id] >= reloadingLimit) {
                LOG_INFO(MSGID_WEBPROC_CRASH, 3, PMLOGKS("APP_ID", appId.c_str()), PMLOGKS("InForeground", "true"), PMLOGKS("Reloading limit", "Close app"),  "");
                closeAppInternal(app, true);
            }
            else {
                LOG_INFO(MSGID_WEBPROC_CRASH, 3, PMLOGKS("APP_ID", appId.c_str()), PMLOGKS("InForeground", "true"), PMLOGKS("Reloading limit", "OK; Reload default page"),  "");
                app->page()->reloadDefaultPage();
            }
        }
        else if (app->isMinimized()) {
            LOG_INFO(MSGID_WEBPROC_CRASH, 2, PMLOGKS("APP_ID", appId.c_str()), PMLOGKS("InBackground", "Will be Reloaded in Relaunch"),  "");
            app->setCrashState(true);
        }
    }
    return true;
}

const std::string WebAppManager::windowTypeFromString(const std::string& str)
{
    if(str == "overlay")
        return WT_OVERLAY;
    if(str == "popup")
        return WT_POPUP;
    if(str == "minimal")
        return WT_MINIMAL;
    if(str == "floating")
        return WT_FLOATING;
    if(str == "system_ui")
        return WT_SYSTEM_UI;
    return WT_CARD;
}

void WebAppManager::setForceCloseApp(const std::string& appId)
{
    WebAppBase* app = findAppById(appId);
    if (!app)
        return;

    if (app->isWindowed()) {
        if (app->keepAlive() && app->getHiddenWindow()) {
            forceCloseAppInternal(app);
            LOG_INFO(MSGID_FORCE_CLOSE_KEEP_ALIVE_APP, 1, PMLOGKS("APP_ID", appId.c_str()), "");
            return;
        }
    }

    app->setForceClose();
}

void WebAppManager::requestKillWebProcess(uint32_t pid)
{
    // Deprecated (2016-0401)
}

bool WebAppManager::shouldLaunchContainerAppOnDemand()
{
    if (m_containerAppManager)
        return  m_containerAppManager->getLaunchContainerAppOnDemand();

    return false;
}

uint32_t WebAppManager::getContainerAppProxyID()
{
    if (!m_containerAppManager || m_containerAppManager->getContainerAppDescription().empty())
        return 0;

    ApplicationDescription* containerDesc = ApplicationDescription::fromJsonString(m_containerAppManager->getContainerAppDescription().c_str());
    uint32_t proxyID = m_webProcessManager->getWebProcessProxyID(containerDesc);
    delete containerDesc;
    return proxyID;
}

void WebAppManager::deleteStorageData(const std::string& identifier)
{
    m_webProcessManager->deleteStorageData(identifier);
}

void WebAppManager::killCustomPluginProcess(const std::string& basePath)
{
    // Deprecated (2016-04-01)
}

/**
 * Launch an application (webApps only, not native).
 *
 * @param appId The application ID to launch.
 * @param params The call parameters.
 * @param the ID of the application performing the launch (can be NULL).
 * @param errMsg The error message (will be empty if this call was successful).
 *
 * @todo: this should now be moved private and be protected...leaving it for now as to not break stuff and make things
 * slightly faster for intra-sysmgr mainloop launches
 */
std::string WebAppManager::launch(const std::string& appDescString, const std::string& params,
        const std::string& launchingAppId, int& errCode, std::string& errMsg)
{
    ApplicationDescription* desc = ApplicationDescription::fromJsonString(appDescString.c_str());
    if (!desc)
        return std::string();

    std::string instanceId = "";
    std::string url = desc->entryPoint();
    std::string winType = windowTypeFromString(desc->defaultWindowType());
    errMsg.erase();

    // Check if app is container itself, it shouldn't be relaunched like normal app
    if (isContainerApp(url)) {
        if (!isRunningApp(desc->id(), instanceId))
            instanceId = onLaunchContainerApp(appDescString);
        else {
            LOG_INFO(MSGID_CONTAINER_APP_RELAUNCHED, 2, PMLOGKS("APP_ID", desc->id().c_str()),
                  PMLOGKS("INSTANCE_ID", instanceId.c_str()), "ContainerApp; Already Running");
        }
        delete desc;
    }
    // Check if app is already running
    else if (isRunningApp(desc->id(), instanceId)) {
        onRelaunchApp(instanceId, desc->id().c_str(), params.c_str(), launchingAppId.c_str());
        delete desc;
    }
    // Check if app is container-based
    else if (isContainerBasedApp(desc)) {
        if (desc->trustLevel() != "default" && desc->trustLevel() != "trusted") {
            delete desc;
            errCode = ERR_CODE_LAUNCHAPP_INVALID_TRUSTLEVEL;
            errMsg = err_invalidTrustLevel;
            return std::string();
        }
        instanceId = m_containerAppManager->getContainerApp()->instanceId();
        onLaunchContainerBasedApp(url.c_str(),
            winType,
            desc,
            params.c_str(), launchingAppId.c_str());
    }
    // Run as a normal app
    else {
        instanceId = generateInstanceId();
        if (!onLaunchUrl(url, winType, desc, instanceId, params, launchingAppId, errCode, errMsg)) {
            delete desc;
            return std::string();
        }
    }

    return instanceId;
}

bool WebAppManager::isContainerApp(const std::string& url)
{
    if (!m_containerAppManager)
        return false;

    if (url.find(m_containerAppManager->getContainerAppId()) != std::string::npos)
        return true;

    return false;
}

bool WebAppManager::isRunningApp(const std::string& id, std::string& instanceId) {
    std::list<const WebAppBase*> running = runningApps();

    QString appIdToFind = QString::fromStdString(id); // FIXME: WebApp: qstr2stdstr
    for (const auto &app : running) {
        if (app->appId() == appIdToFind) {
            instanceId = app->instanceId();
            return true;
        }
    }

    if (m_containerAppManager) {
        WebAppBase* container = m_containerAppManager->getContainerApp();
        if (container && m_containerAppManager->getContainerAppId() == id) {
            instanceId = container->instanceId();
            return true;
        }
    }

    return false;
}

bool WebAppManager::isContainerBasedApp(ApplicationDescription* containerBasedAppDesc) {
    if (!m_containerAppManager || !m_containerAppManager->isContainerAppReady())
        return false;

    if (containerBasedAppDesc->containerJS().size() == 0)
        return false;

    ApplicationDescription* containerAppDesc = m_containerAppManager->getContainerApp()->getAppDescription();

    // check the enyo bundle version
    if (!containerBasedAppDesc->enyoBundleVersion().empty()) {
        std::string enyoBundleVersion = containerBasedAppDesc->enyoBundleVersion();
        auto versions = containerAppDesc->supportedEnyoBundleVersions();
        return versions.find(enyoBundleVersion) != versions.end();
    }

    // check the enyo version
    return containerAppDesc->enyoVersion().compare(containerBasedAppDesc->enyoVersion()) == 0;
}

bool WebAppManager::isContainerUsedApp(const ApplicationDescription* containerUsedAppDesc) {
    return containerUsedAppDesc->containerJS().size()? true : false;

}

std::vector<ApplicationInfo> WebAppManager::list(bool includeSystemApps)
{
    std::vector<ApplicationInfo> list;

    std::list<const WebAppBase*> running = runningApps();
    for (const auto &app : running) {
        if(app->appId().size() || (!app->appId().size() && includeSystemApps)) {
            uint32_t pid = m_webProcessManager->getWebProcessPID(app);
            list.push_back(ApplicationInfo(app->instanceId(), app->appId().toStdString(), pid)); // FIXME: WebApp: qstr2stdstr
        }
    }
    return list;
}

Json::Value WebAppManager::getWebProcessProfiling()
{
    return m_webProcessManager->getWebProcessProfiling();
}

#ifndef PRELOADMANAGER_ENABLED
void WebAppManager::sendLaunchContainerApp()
{
    if (!m_containerAppManager)
        return;

    std::string appId = getContainerAppId();

    if (m_serviceSender)
        m_serviceSender->launchContainerApp(appId);
}

void WebAppManager::startContainerTimer()
{
    if (m_containerAppManager)
        m_containerAppManager->startContainerTimer();
}

void WebAppManager::restartContainerApp()
{
    if (m_containerAppManager)
        m_containerAppManager->restartContainerApp();
}
#else
void WebAppManager::insertAppIntoList(WebAppBase* app)
{
    m_appList.push_back(app);
}

void WebAppManager::deleteAppIntoList(WebAppBase* app)
{
    m_appList.remove(app);
}
#endif

void WebAppManager::closeApp(const std::string& appId)
{
    if (m_serviceSender)
        m_serviceSender->closeApp(appId);
}

void WebAppManager::reloadContainerApp()
{
    if (m_containerAppManager)
        m_containerAppManager->reloadContainerApp();
}

std::string& WebAppManager::getContainerAppId()
{
    static std::string nullStr = "";

    if (m_containerAppManager)
        return m_containerAppManager->getContainerAppId();

    return nullStr;
}

WebAppBase* WebAppManager::getContainerApp()
{
    if (m_containerAppManager)
        return m_containerAppManager->getContainerApp();

    return 0;
}

void WebAppManager::setContainerAppReady(bool ready)
{
    if (m_containerAppManager)
        m_containerAppManager->setContainerAppReady(ready);
}

void WebAppManager::setContainerAppLaunched(bool launched)
{
    if (m_containerAppManager)
        m_containerAppManager->setContainerAppLaunched(launched);
}

void WebAppManager::postRunningAppList()
{
    if (!m_serviceSender)
        return;

    std::vector<ApplicationInfo> apps = list(true);
    m_serviceSender->postlistRunningApps(apps);
}

void WebAppManager::postWebProcessCreated(const std::string& appId, uint32_t pid)
{
    if (!m_serviceSender)
        return;

    postRunningAppList();

    if (!m_webAppManagerConfig->isPostWebProcessCreatedDisabled())
        m_serviceSender->postWebProcessCreated(appId, pid);
}

uint32_t WebAppManager::getWebProcessId(const std::string& appId)
{
    uint32_t pid = 0;
    WebAppBase* app = findAppById(appId);

    if (app && m_webProcessManager)
        pid = m_webProcessManager->getWebProcessPID(app);

    return pid;
}

std::string WebAppManager::generateInstanceId()
{
    static int s_nextProcessId = 1000;
    std::ostringstream stream;
    stream << (s_nextProcessId++);

    return stream.str();
}

void WebAppManager::setAccessibilityEnabled(bool enabled)
{
    if (m_isAccessibilityEnabled == enabled)
        return;

    for (auto it = m_appList.begin(); it != m_appList.end(); ++it) {
        //set audion guidance on/off on settings app
        if ((*it)->page())
            (*it)->page()->setAudioGuidanceOn(enabled);
        (*it)->setUseAccessibility(enabled);
    }

    m_isAccessibilityEnabled = enabled;
}

void WebAppManager::sendEventToAllAppsAndAllFrames(const std::string& jsscript)
{
    for (const auto &app : m_appList) {
        if (app->page()) {
            LOG_DEBUG("[%s] send event with %s", qPrintable(app->appId()), jsscript.c_str());
            // to send all subFrame, use this function instead of evaluateJavaScriptInAllFrames()
            app->page()->evaluateJavaScriptInAllFrames(jsscript);
        }
    }
}

void WebAppManager::serviceCall(const std::string& url, const std::string& payload, const std::string& appId)
{
    if (m_serviceSender)
        m_serviceSender->serviceCall(url, payload, appId);
}

void WebAppManager::updateNetworkStatus(const Json::Value& object)
{
    NetworkStatus status;
    status.fromJsonObject(object);

    webos::Runtime::GetInstance()->SetNetworkConnected(status.isInternetConnectionAvailable());
    m_networkStatusManager->updateNetworkStatus(status);
}

bool WebAppManager::isEnyoApp(const std::string& appId)
{
    WebAppBase* app = findAppById(appId);
    if (app && !app->getAppDescription()->enyoVersion().empty())
        return true;

    return false;
}

void WebAppManager::clearBrowsingData(const int removeBrowsingDataMask)
{
    m_webProcessManager->clearBrowsingData(removeBrowsingDataMask);
}

int WebAppManager::maskForBrowsingDataType(const char* type)
{
    return m_webProcessManager->maskForBrowsingDataType(type);
}

void WebAppManager::buildWebViewProfile(const std::string& app_id, const std::string& proxy_rules)
{
    if (m_webProcessManager)
        m_webProcessManager->buildWebViewProfile(app_id, proxy_rules);
}

void WebAppManager::deleteWebViewProfile(const std::string& app_id)
{
    if (m_webProcessManager)
        m_webProcessManager->deleteWebViewProfile(app_id);
}
