// Copyright (c) 2014-2021 LG Electronics, Inc.
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

#ifndef WEBAPPMANAGERSERVICELUNA_H
#define WEBAPPMANAGERSERVICELUNA_H

#include <string>

#include "PalmServiceBase.h"
#include "WebAppManagerService.h"

namespace Json {
class Value;
};

class WebAppManagerServiceLuna : public PalmServiceBase, public WebAppManagerService {
public:
    WebAppManagerServiceLuna();
    ~WebAppManagerServiceLuna() override;

    static WebAppManagerServiceLuna* instance();

    bool startService();

    // WebAppManagerService
    Json::Value launchApp(const Json::Value& request) override;
    Json::Value killApp(const Json::Value& request) override;
    Json::Value logControl(const Json::Value& request) override;
    Json::Value setInspectorEnable(const Json::Value& request) override;
    Json::Value closeAllApps(const Json::Value& request) override;
    Json::Value discardCodeCache(const Json::Value& request) override;
    Json::Value listRunningApps(const Json::Value& request, bool subscribed) override;
    Json::Value getWebProcessSize(const Json::Value& request) override;
    Json::Value pauseApp(const Json::Value& request) override;
    Json::Value clearBrowsingData(const Json::Value& request) override;
    Json::Value webProcessCreated(const Json::Value& request, bool subscribed) override;

    // PlamServiceBase
    void didConnect() override;

    // WebAppManagerServiceLuna
    virtual void systemServiceConnectCallback(const Json::Value& reply);

    void getSystemLocalePreferencesCallback(const Json::Value& reply);

    void memoryManagerConnectCallback(const Json::Value& reply);
    void getCloseAppIdCallback(const Json::Value& reply);
    void thresholdChangedCallback(const Json::Value& reply);

    void applicationManagerConnectCallback(const Json::Value& reply);
    void getAppStatusCallback(const Json::Value& reply);
    void getForegroundAppInfoCallback(const Json::Value& reply);

    void bootdConnectCallback(const Json::Value& reply);
    void getBootStatusCallback(const Json::Value& reply);

    void networkConnectionStatusCallback(const Json::Value& reply);
    void getNetworkConnectionStatusCallback(const Json::Value& reply);

    void closeApp(const std::string& id);
    void closeAppCallback(const Json::Value& reply);

protected:
    //methods implementation of PalmServiceBase
    virtual LSMethod* methods() const { return s_methods; }
    virtual const char* serviceName() const { return "com.palm.webappmanager"; };

    static LSMethod s_methods[];

    bool m_clearedCache;
    bool m_bootDone;
    std::string m_debugLevel;

private:
    bool isValidInstanceId(const std::string& instanceId);
};

#endif // WEBAPPMANAGERSERVICELUNA_H
