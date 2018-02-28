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

#ifndef WEBAPPMANAGER_H
#define WEBAPPMANAGER_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include <QJsonObject>
#include <QMultiMap>
#include <QString>

#include "webos/webview_base.h"

class ApplicationDescription;
class ContainerAppManager;
class DeviceInfo;
class NetworkStatusManager;
class PlatformModuleFactory;
class ServiceSender;
class WebProcessManager;
class WebAppManagerConfig;
class WebAppBase;
class WebPageBase;

class ApplicationInfo {
public:
    ApplicationInfo(const QString& inInstanceId, const QString& inAppId, const uint32_t& inPid)
        : instanceId(inInstanceId)
        , appId(inAppId)
        , pid(inPid)
    {
    }
    ~ApplicationInfo() {}

    QString instanceId;
    QString appId;
    uint32_t pid;
};

class WebAppManager {
public:
    enum WebAppMessageType {
        DeviceInfoChanged = 1
    };

    static WebAppManager* instance();

    bool getSystemLanguage(QString& value);
    bool getDeviceInfo(QString name, QString& value);
    void broadcastWebAppMessage(WebAppMessageType type, const QString& message);

    WebProcessManager* getWebProcessManager() { return m_webProcessManager; }

    virtual ~WebAppManager();

    void setPlatformModules(PlatformModuleFactory* factory);
    bool run();
    void quit();

    std::list<const WebAppBase*> runningApps();
    std::list<const WebAppBase*> runningApps(uint32_t pid);
    WebAppBase* findAppById(const QString& appId);
    WebAppBase* findAppByInstanceId(const QString& instanceId);

    std::string launch(const std::string& appDescString,
        const std::string& params,
        const std::string& launchingAppId,
        int& errCode,
        std::string& errMsg);

    std::vector<ApplicationInfo> list(bool includeSystemApps = false);

    QJsonObject getWebProcessProfiling();
#ifndef PRELOADMANAGER_ENABLED
    void sendLaunchContainerApp();
    void startContainerTimer();
    void restartContainerApp();
#else
    void insertAppIntoList(WebAppBase* app);
    void deleteAppIntoList(WebAppBase* app);
#endif
    void reloadContainerApp();
    void setContainerAppReady(bool ready);
    void setContainerAppLaunched(bool launched);
    QString& getContainerAppId();
    WebAppBase* getContainerApp();
    int currentUiWidth();
    int currentUiHeight();
    void setUiSize(int width, int height);

    void setActiveAppId(QString id) { m_activeAppId = id; }
    const QString getActiveAppId() { return m_activeAppId; }

    void onGlobalProperties(int key);
    bool purgeSurfacePool(uint32_t pid);
    bool onKillApp(const std::string& appId);
    bool isDiscardCodeCacheRequired();
    bool setInspectorEnable(QString& appId);
    void discardCodeCache(uint32_t pid);

    void setSystemLanguage(QString value);
    void setDeviceInfo(QString name, QString value);
    WebAppManagerConfig* config() { return m_webAppManagerConfig; }

    void requestActivity(WebAppBase* app);
    const QString windowTypeFromString(const std::string& str);

    bool closeAllApps(uint32_t pid = 0);
    bool closeContainerApp();
    void setForceCloseApp(QString appId);
    void requestKillWebProcess(uint32_t pid);
    bool shouldLaunchContainerAppOnDemand();

    int getSuspendDelay() { return m_suspendDelay; }
    void deleteStorageData(const QString& identifier);
    void killCustomPluginProcess(const QString& basePath);
    bool processCrashed(QString appId);

    void closeAppInternal(WebAppBase* app, bool ignoreCleanResource = false);
    void forceCloseAppInternal(WebAppBase* app);

    void webPageAdded(WebPageBase* page);
    void webPageRemoved(WebPageBase* page);
    void removeWebAppFromWebProcessInfoMap(QString appId);

    void appDeleted(WebAppBase* app);
    void postRunningAppList();
    std::string generateInstanceId();
    void removeClosingAppList(const QString& appId);

    bool isAccessibilityEnabled() { return m_isAccessibilityEnabled; }
    void setAccessibilityEnabled(bool enabled);
    void postWebProcessCreated(const QString& appId, uint32_t pid);
    uint32_t getWebProcessId(const QString& appId);
    void sendEventToAllAppsAndAllFrames(const QString& jsscript);
    void serviceCall(const QString& url, const QString& payload, const QString& appId);
    void updateNetworkStatus(const QJsonObject& object);
    void notifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level);

    bool isEnyoApp(const QString& appId);

    void closeApp(const std::string& appId);

    void clearBrowsingData(const int removeBrowsingDataMask);
    int maskForBrowsingDataType(const char* type);

protected:
private:
    void loadEnvironmentVariable();

    WebAppBase* onLaunchUrl(const std::string& url, QString winType,
        const ApplicationDescription* appDesc, const std::string& instanceId,
        const std::string& args, const std::string& launchingAppId,
        int& errCode, std::string& errMsg);
    void onLaunchContainerBasedApp(const std::string& url, QString& winType,
        const ApplicationDescription* appDesc, const std::string& args, const std::string& launchingAppId);
    std::string onLaunchContainerApp(const std::string& appDesc);
    void onRelaunchApp(const std::string& instanceId, const std::string& appId,
        const std::string& args, const std::string& launchingAppId);

    WebAppManager();

    typedef std::list<WebAppBase*> AppList;
    typedef std::list<WebPageBase*> PageList;

    bool isContainerBasedApp(ApplicationDescription* containerBasedAppDesc);
    bool isContainerUsedApp(const ApplicationDescription* containerUsedAppDesc);
    bool isRunningApp(const std::string& id, std::string& instanceId);
    bool isContainerApp(const std::string& url);
    uint32_t getContainerAppProxyID();

    QMap<QString, WebAppBase*> m_closingAppList;

    // Mappings
    QMap<std::string, WebPageBase*> m_shellPageMap;
    AppList m_appList;
    QMultiMap<std::string, WebPageBase*> m_appPageMap;

    PageList m_pagesToDeleteList;
    bool m_deletingPages;

    QString m_activeAppId;

    ServiceSender* m_serviceSender;
    ContainerAppManager* m_containerAppManager;
    WebProcessManager* m_webProcessManager;
    DeviceInfo* m_deviceInfo;
    WebAppManagerConfig* m_webAppManagerConfig;
    NetworkStatusManager* m_networkStatusManager;

    QMap<QString, int> m_lastCrashedAppIds;

    int m_suspendDelay;

    std::map<std::string, std::string> m_appVersion;

    bool m_isAccessibilityEnabled;
};

#endif /* WEBAPPMANAGER_H */
