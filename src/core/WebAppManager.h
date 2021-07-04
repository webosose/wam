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
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <QJsonObject>
#include <QMultiMap>
#include <QString>

#include "webos/webview_base.h"

class ApplicationDescription;
class DeviceInfo;
class NetworkStatusManager;
class PlatformModuleFactory;
class ServiceSender;
class WebProcessManager;
class WebAppFactoryManager;
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

    WebProcessManager* getWebProcessManager() { return m_webProcessManager.get(); }

    virtual ~WebAppManager();

    void setPlatformModules(std::unique_ptr<PlatformModuleFactory> factory);
    void setWebAppFactory(std::unique_ptr<WebAppFactoryManager> factory);
    bool run();
    void quit();

    std::list<const WebAppBase*> runningApps();
    std::list<const WebAppBase*> runningApps(uint32_t pid);
    WebAppBase* findAppById(const QString& appId);
    std::list<WebAppBase*> findAppsById(const QString& appId);
    WebAppBase* findAppByInstanceId(const QString& instanceId);

    std::string launch(const std::string& appDescString,
        const std::string& params,
        const std::string& launchingAppId,
        int& errCode,
        std::string& errMsg);

    std::vector<ApplicationInfo> list(bool includeSystemApps = false);

    QJsonObject getWebProcessProfiling();
    int currentUiWidth();
    int currentUiHeight();
    void setUiSize(int width, int height);

    void setActiveInstanceId(QString id) { m_activeInstanceId = id.toStdString(); }
    const std::string getActiveInstanceId() { return m_activeInstanceId; }

    void onGlobalProperties(int key);
    bool purgeSurfacePool(uint32_t pid);
    void onShutdownEvent();
    bool onKillApp(const std::string& appId, const std::string& instanceId, bool force = false);
    bool onPauseApp(const std::string& instanceId);
    bool isDiscardCodeCacheRequired();
    bool setInspectorEnable(QString& appId);
    void discardCodeCache(uint32_t pid);

    void setSystemLanguage(QString value);
    void setDeviceInfo(QString name, QString value);
    WebAppManagerConfig* config() { return m_webAppManagerConfig.get(); }

    const std::string windowTypeFromString(const std::string& str);

    bool closeAllApps(uint32_t pid = 0);
    void setForceCloseApp(const QString& appId, const QString& instanceId);
    void requestKillWebProcess(uint32_t pid);

    int getSuspendDelay() { return m_suspendDelay; }
    int getMaxCustomSuspendDelay() const { return m_maxCustomSuspendDelay; }
    void deleteStorageData(const QString& identifier);
    void killCustomPluginProcess(const QString& basePath);
    bool processCrashed(QString appId, QString instanceId);

    void closeAppInternal(WebAppBase* app, bool ignoreCleanResource = false);
    void forceCloseAppInternal(WebAppBase* app);

    void webPageAdded(WebPageBase* page);
    void webPageRemoved(WebPageBase* page);
    void removeWebAppFromWebProcessInfoMap(QString appId);

    void appDeleted(WebAppBase* app);
    void postRunningAppList();
    std::string generateInstanceId();
    void removeClosingAppList(const QString& instanceId);

    bool isAccessibilityEnabled() { return m_isAccessibilityEnabled; }
    void setAccessibilityEnabled(bool enabled);
    void postWebProcessCreated(const QString& appId, const QString& instanceId, uint32_t pid);
    uint32_t getWebProcessId(const QString& appId, const QString& instanceId);
    void sendEventToAllAppsAndAllFrames(const QString& jsscript);
    void serviceCall(const QString& url, const QString& payload, const QString& appId);
    void updateNetworkStatus(const QJsonObject& object);
    void notifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level);

    bool isEnyoApp(const QString& appId);

    void closeApp(const std::string& appId);

    void clearBrowsingData(const int removeBrowsingDataMask);
    int maskForBrowsingDataType(const char* type);

    void appInstalled(const std::string& app_id);
    void appRemoved(const std::string& app_id);

    std::string identifierForSecurityOrigin(const std::string& identifier);

protected:
private:
    WebAppFactoryManager* getWebAppFactory();
    void loadEnvironmentVariable();

    WebAppBase* onLaunchUrl(const std::string& url, const std::string& winType,
        std::shared_ptr<ApplicationDescription> appDesc, const std::string& instanceId,
        const std::string& args, const std::string& launchingAppId,
        int& errCode, std::string& errMsg);
    void onRelaunchApp(const std::string& instanceId, const std::string& appId,
        const std::string& args, const std::string& launchingAppId);

    WebAppManager();

    typedef std::list<WebAppBase*> AppList;
    typedef std::list<WebPageBase*> PageList;

    bool isRunningApp(const std::string& id);
    std::unordered_map<std::string, WebAppBase*> m_closingAppList;

    // Mappings
    std::unordered_map<std::string, WebPageBase*> m_shellPageMap;
    AppList m_appList;
    std::unordered_multimap<std::string, WebPageBase*> m_appPageMap;

    PageList m_pagesToDeleteList;
    bool m_deletingPages;

    std::string m_activeInstanceId;

    std::unique_ptr<ServiceSender> m_serviceSender;
    std::unique_ptr<WebProcessManager> m_webProcessManager;
    std::unique_ptr<DeviceInfo> m_deviceInfo;
    std::unique_ptr<WebAppManagerConfig> m_webAppManagerConfig;
    std::unique_ptr<NetworkStatusManager> m_networkStatusManager;
    std::unique_ptr<WebAppFactoryManager> m_webAppFactory;

    std::unordered_map<std::string, int> m_lastCrashedAppIds;

    int m_suspendDelay;
    int m_maxCustomSuspendDelay;

    std::map<std::string, std::string> m_appVersion;

    bool m_isAccessibilityEnabled;
};

#endif /* WEBAPPMANAGER_H */
