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

#ifndef CONTAINERAPPMANAGER_H
#define CONTAINERAPPMANAGER_H

#include "Timer.h"

#include <QString>
#include <string>

class WebAppBase;

class ContainerAppManager {
public:
    ContainerAppManager();
    ~ContainerAppManager();

    void startContainerTimer();
    void stopContainerTimer();
    QString& getContainerAppId();
    WebAppBase* launchContainerApp(const std::string& appDesc, const std::string& instanceId, int& errorCode);
    void closeContainerApp();
    void reloadContainerApp();
    void restartContainerApp();
    WebAppBase* getContainerApp() { return m_containerApp; }
    void setContainerAppLaunched(bool launched) { m_containerAppIsLaunched = launched; }
    bool isContainerAppLaunched() const { return m_containerAppIsLaunched; }
    void setContainerAppReady(bool ready) { m_containerAppIsReady = ready; }
    bool isContainerAppReady();
    void resetContainerAppManager();
    bool isContainerApp(WebAppBase* app) { return app == m_containerApp ? true : false; }
    std::string getContainerAppDescription() { return m_containerDesc; }
    bool getLaunchContainerAppOnDemand() { return m_launchContainerAppOnDemand; }
    void setLaunchContainerAppOnDemand(bool demand) { m_launchContainerAppOnDemand = demand; }
    void setUseContainerAppOptimization(bool enabled) { m_useContainerAppOptimization = enabled; }
    void containerAppLaunch();

private:
    void loadContainerInfo();
    WebAppBase* launchContainerAppInternal(const std::string& instanceId, int& errorCode);

    WebAppBase* m_containerApp;
    OneShotTimer<ContainerAppManager> m_containerAppLaunchTimer;
    std::string m_containerDesc;
    int m_containerAppRelaunchCounter;
    bool m_containerAppIsLaunched;
    bool m_containerAppIsReady;
    bool m_launchContainerAppOnDemand;
    bool m_useContainerAppOptimization;
};

#endif /* CONTAINERAPPMANAGER_H */
