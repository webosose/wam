// Copyright (c) 2014-2018 LG Electronics, Inc.
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

#ifndef WEBAPPMANAGERSERVICE_H
#define WEBAPPMANAGERSERVICE_H

#include <QJsonObject>

#include "WebAppManager.h"

#include "webos/webview_base.h"

enum ErrorCode {
    ERR_CODE_LAUNCHAPP_MISS_PARAM = 1000,
    ERR_CODE_LAUNCHAPP_UNSUPPORTED_TYPE = 1001,
    ERR_CODE_LAUNCHAPP_INVALID_TRUSTLEVEL = 1002,
    ERR_CODE_KILLAPP_NO_APP = 2000,
    ERR_CODE_CLEAR_DATA_BRAWSING_EMPTY_ARRAY = 3000,
    ERR_CODE_CLEAR_DATA_BRAWSING_INVALID_VALUE = 3001,
    ERR_CODE_CLEAR_DATA_BRAWSING_UNKNOWN_DATA = 3002
};

const std::string err_missParam = "Miss launch parameter(s)";
const std::string err_unsupportedType = "Unsupported app type (Check subType)";
const std::string err_invalidTrustLevel = "Invalid trust level (Check trustLevel)";

const std::string err_noRunningApp = "App is not running";

const std::string err_emptyArray = "Empty array is not allowed.";
const std::string err_invalidValue = "Invalid value";
const std::string err_unknownData = "Unknown data";
const std::string err_onlyAllowedForString = "Only allowed for string type";

class WebAppBase;

class WebAppManagerService {
public:
    WebAppManagerService();

    virtual bool startService() = 0;
    // methods published to the bus
    virtual QJsonObject launchApp(QJsonObject request) = 0;
    virtual QJsonObject killApp(QJsonObject request) = 0;
    virtual QJsonObject logControl(QJsonObject request) = 0;
    virtual QJsonObject setInspectorEnable(QJsonObject request) = 0;
    virtual QJsonObject closeAllApps(QJsonObject request) = 0;
    virtual QJsonObject discardCodeCache(QJsonObject request) = 0;
    virtual QJsonObject listRunningApps(QJsonObject request, bool subscribed) = 0;
    virtual QJsonObject closeByProcessId(QJsonObject request) = 0;
    virtual QJsonObject getWebProcessSize(QJsonObject request) = 0;
    virtual QJsonObject clearBrowsingData(QJsonObject request) = 0;
    virtual QJsonObject webProcessCreated(QJsonObject request, bool subscribed) = 0;

protected:
    std::string onLaunch(const std::string& appDescString,
        const std::string& params,
        const std::string& launchingAppId,
        int& errCode,
        std::string& errMsg);

    bool onKillApp(const std::string& appId);
    QJsonObject onLogControl(const std::string& keys, const std::string& value);
    bool onCloseAllApps(uint32_t pid = 0);
    bool closeContainerApp();
    bool isDiscardCodeCacheRequired();
    void onDiscardCodeCache(uint32_t pid);
    bool onPurgeSurfacePool(uint32_t pid);
    QJsonObject getWebProcessProfiling();
    QJsonObject closeByInstanceId(QString instanceId);
    int maskForBrowsingDataType(const char* type);
    void onClearBrowsingData(const int removeBrowsingDataMask);

    WebAppBase* getContainerApp();
#ifndef PRELOADMANAGER_ENABLED
    void reloadContainerApp();
    void startContainerTimer();
    void restartContainerApp();
#endif
    void setDeviceInfo(const QString& name, const QString& value);
    void setUiSize(int width, int height);
    void setSystemLanguage(const QString& language);
    QString getSystemLanguage();
    void setForceCloseApp(const QString& appId);
    void deleteStorageData(const QString& identifier);
    void killCustomPluginProcess(const QString& appBasePath);
    void requestKillWebProcess(uint32_t pid);
    bool shouldLaunchContainerAppOnDemand();
    void updateNetworkStatus(const QJsonObject& object);
    void notifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level);
    void setAccessibilityEnabled(bool enable);
    uint32_t getWebProcessId(const QString& appId);

    std::list<const WebAppBase*> runningApps();
    std::list<const WebAppBase*> runningApps(uint32_t pid);
    std::vector<ApplicationInfo> list(bool includeSystemApps = false);

    bool isEnyoApp(const QString& apppId);
};

#endif // WEBAPPMANAGERSERVICE_H
