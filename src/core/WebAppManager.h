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
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

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

namespace Json {
class Value;
};

class ApplicationInfo {
public:
    ApplicationInfo(const std::string& inInstanceId, const std::string& inAppId, const uint32_t& inPid)
        : instanceId(inInstanceId)
        , appId(inAppId)
        , pid(inPid)
    {
    }
    ~ApplicationInfo() {}

    std::string instanceId;
    std::string appId;
    uint32_t pid;
};

class WebAppManager {
public:
    enum WebAppMessageType {
        DeviceInfoChanged = 1
    };

    static WebAppManager* instance();

    bool getSystemLanguage(std::string& value);
    bool getDeviceInfo(const std::string& name, std::string& value);
    void broadcastWebAppMessage(WebAppMessageType type, const std::string& message);

    WebProcessManager* getWebProcessManager() { return m_webProcessManager.get(); }

    virtual ~WebAppManager();

    void setPlatformModules(std::unique_ptr<PlatformModuleFactory> factory);
    bool run();
    void quit();

    std::list<const WebAppBase*> runningApps();
    std::list<const WebAppBase*> runningApps(uint32_t pid);
    WebAppBase* findAppById(const std::string& appId);
    WebAppBase* findAppByInstanceId(const std::string& instanceId);

    std::string launch(const std::string& appDescString,
        const std::string& params,
        const std::string& launchingAppId,
        int& errCode,
        std::string& errMsg);

    std::vector<ApplicationInfo> list(bool includeSystemApps = false);

    Json::Value getWebProcessProfiling();
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
    std::string& getContainerAppId();
    WebAppBase* getContainerApp();
    int currentUiWidth();
    int currentUiHeight();
    void setUiSize(int width, int height);

    void setActiveAppId(std::string id) { m_activeAppId = id; }
    const std::string getActiveAppId() { return m_activeAppId; }

    void onGlobalProperties(int key);
    bool purgeSurfacePool(uint32_t pid);
    bool onKillApp(const std::string& appId);
    bool isDiscardCodeCacheRequired();
    bool setInspectorEnable(const std::string& appId);
    void discardCodeCache(uint32_t pid);

    void setSystemLanguage(const std::string& value);
    void setDeviceInfo(const std::string& name, const std::string& value);
    WebAppManagerConfig* config() { return m_webAppManagerConfig.get(); }

    void requestActivity(WebAppBase* app);
    const std::string windowTypeFromString(const std::string& str);

    bool closeAllApps(uint32_t pid = 0);
    bool closeContainerApp();
    void setForceCloseApp(const std::string& appId);
    void requestKillWebProcess(uint32_t pid);
    bool shouldLaunchContainerAppOnDemand();

    int getSuspendDelay() { return m_suspendDelay; }
    void deleteStorageData(const std::string& identifier);
    void killCustomPluginProcess(const std::string& basePath);
    bool processCrashed(const std::string& appId);

    void closeAppInternal(WebAppBase* app, bool ignoreCleanResource = false);
    void forceCloseAppInternal(WebAppBase* app);

    void webPageAdded(WebPageBase* page);
    void webPageRemoved(WebPageBase* page);
    void removeWebAppFromWebProcessInfoMap(const std::string& appId);

    void appDeleted(WebAppBase* app);
    void postRunningAppList();
    std::string generateInstanceId();
    void removeClosingAppList(const std::string& appId);

    bool isAccessibilityEnabled() { return m_isAccessibilityEnabled; }
    void setAccessibilityEnabled(bool enabled);
    void postWebProcessCreated(const std::string& appId, uint32_t pid);
    uint32_t getWebProcessId(const std::string& appId);
    void sendEventToAllAppsAndAllFrames(const std::string& jsscript);
    void serviceCall(const std::string& url, const std::string& payload, const std::string& appId);
    void updateNetworkStatus(const Json::Value& object);
    void notifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level);

    bool isEnyoApp(const std::string& appId);

    void closeApp(const std::string& appId);

    void clearBrowsingData(const int removeBrowsingDataMask);
    int maskForBrowsingDataType(const char* type);
    void buildWebViewProfile(const std::string& app_id, const std::string& proxy_host, const std::string& proxy_port);
    void deleteWebViewProfile(const std::string& app_id);

protected:
private:
    void loadEnvironmentVariable();

    WebAppBase* onLaunchUrl(const std::string& url, const std::string& winType,
        const std::shared_ptr<ApplicationDescription> appDesc, const std::string& instanceId,
        const std::string& args, const std::string& launchingAppId,
        int& errCode, std::string& errMsg);
    void onLaunchContainerBasedApp(const std::string& url, const std::string& winType,
        const std::shared_ptr<ApplicationDescription> appDesc, const std::string& args, const std::string& launchingAppId);
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

    std::unordered_map<std::string, WebAppBase*> m_closingAppList;

    // Mappings
    std::unordered_map<std::string, WebPageBase*> m_shellPageMap;
    AppList m_appList;
    std::unordered_multimap<std::string, WebPageBase*> m_appPageMap;

    PageList m_pagesToDeleteList;
    bool m_deletingPages;

    std::string m_activeAppId;

    std::unique_ptr<ServiceSender> m_serviceSender;
    std::unique_ptr<ContainerAppManager> m_containerAppManager;
    std::unique_ptr<WebProcessManager> m_webProcessManager;
    std::unique_ptr<DeviceInfo> m_deviceInfo;
    std::unique_ptr<WebAppManagerConfig> m_webAppManagerConfig;
    std::unique_ptr<NetworkStatusManager> m_networkStatusManager;

    std::unordered_map<std::string, int> m_lastCrashedAppIds;

    int m_suspendDelay;

    std::unordered_map<std::string, std::string> m_appVersion;

    bool m_isAccessibilityEnabled;
};

#endif /* WEBAPPMANAGER_H */
