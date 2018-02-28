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

#ifndef WEBAPPMANAGERSERVICELUNA_H
#define WEBAPPMANAGERSERVICELUNA_H

#include <QJsonObject>

#include "PalmServiceBase.h"
#include "WebAppManagerService.h"

class WebAppManagerServiceLuna : public PalmServiceBase, public WebAppManagerService {
public:
    WebAppManagerServiceLuna();
    ~WebAppManagerServiceLuna() override;

    static WebAppManagerServiceLuna* instance();

    bool startService();

    // WebAppManagerService
    QJsonObject launchApp(QJsonObject request) override;
    QJsonObject killApp(QJsonObject request) override;
    QJsonObject logControl(QJsonObject request) override;
    QJsonObject setInspectorEnable(QJsonObject request) override;
    QJsonObject closeAllApps(QJsonObject request) override;
    QJsonObject discardCodeCache(QJsonObject request) override;
    QJsonObject listRunningApps(QJsonObject request, bool subscribed) override;
    QJsonObject closeByProcessId(QJsonObject request) override;
    QJsonObject getWebProcessSize(QJsonObject request) override;
    QJsonObject clearBrowsingData(QJsonObject request) override;
    QJsonObject webProcessCreated(QJsonObject request, bool subscribed) override;

    // PlamServiceBase
    void didConnect() override;

#ifndef PRELOADMANAGER_ENABLED
    //callback methods that return from the bus
    void launchContainerApp(const QString& id);
    void launchContainerAppCallback(QJsonObject reply);
#endif

    // WebAppManagerServiceLuna
    virtual void systemServiceConnectCallback(QJsonObject reply);

    void getSystemLocalePreferencesCallback(QJsonObject reply);

    void memoryManagerConnectCallback(QJsonObject reply);
    void getCloseAppIdCallback(QJsonObject reply);
    void clearContainersCallback(QJsonObject reply);
    void thresholdChangedCallback(QJsonObject reply);

    void applicationManagerConnectCallback(QJsonObject reply);
    void getAppStatusCallback(QJsonObject reply);
    void getForegroundAppInfoCallback(QJsonObject reply);

    void bootdConnectCallback(QJsonObject reply);
    void getBootStatusCallback(QJsonObject reply);

    void networkConnectionStatusCallback(QJsonObject reply);
    void getNetworkConnectionStatusCallback(QJsonObject reply);

    void closeApp(const std::string& id);
    void closeAppCallback(QJsonObject reply);

protected:
    // PlamServiceBase
    LSMethod* privateMethods() const override { return s_privateMethods; }
    LSMethod* publicMethods() const override { return s_publicMethods; }
    const char* serviceName() const override { return "com.palm.webappmanager"; }

    static LSMethod s_privateMethods[];
    static LSMethod s_publicMethods[];

    bool m_clearedCache;
    bool m_bootDone;
    QString m_debugLevel;
};

#endif // WEBAPPMANAGERSERVICELUNA_H
