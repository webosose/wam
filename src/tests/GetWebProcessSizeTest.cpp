//  (c) 2021 LG Electronics, Inc.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/json.h>

#include "BaseMockInitializer.h"
#include "BlinkWebProcessManagerMock.h"
#include "JsonHelper.h"
#include "PlatformModuleFactoryImplMock.h"
#include "Utils.h"
#include "WebAppManagerServiceLuna.h"
#include "WebViewMock.h"

namespace {
static constexpr int kProcessId = 7779;
constexpr char kProcessMemSize[] = "320115";
static constexpr char kInstanceId[] = "de90e74a-b86b-42c8-8785-3efd927a36430";
static constexpr char kApplicationId[] = "bareapp";

// TODO: Move it to separate file.
constexpr char kLaunchAppJsonBody[] = R"({
  "launchingAppId": "com.webos.app.home",
  "appDesc": {
    "defaultWindowType": "card",
    "uiRevision": "2",
    "systemApp": true,
    "version": "1.0.1",
    "vendor": "LG Electronics, Inc.",
    "miniicon": "icon.png",
    "hasPromotion": false,
    "tileSize": "normal",
    "icons": [],
    "launchPointId": "bareapp_default",
    "largeIcon": "/usr/palm/applications/bareapp/icon.png",
    "lockable": true,
    "transparent": false,
    "icon": "/usr/palm/applications/bareapp/icon.png",
    "checkUpdateOnLaunch": true,
    "imageForRecents": "",
    "spinnerOnLaunch": true,
    "handlesRelaunch": false,
    "unmovable": false,
    "id": "bareapp",
    "inspectable": false,
    "noSplashOnLaunch": false,
    "privilegedJail": false,
    "trustLevel": "default",
    "title": "Bare App",
    "deeplinkingParams": "",
    "lptype": "default",
    "inAppSetting": false,
    "favicon": "",
    "visible": true,
    "accessibility": {
      "supportsAudioGuidance": false
    },
    "folderPath": "/usr/palm/applications/bareapp",
    "main": "index.html",
    "removable": true,
    "type": "web",
    "disableBackHistoryAPI": false,
    "bgImage": ""
  },
  "appId": "bareapp",
  "parameters": {
    "displayAffinity": 0
  },
  "reason": "com.webos.app.home",
  "launchingProcId": "",
  "instanceId": "de90e74a-b86b-42c8-8785-3efd927a36430"
})";

}

TEST(GetWebProcessSizeTest, checkCaseProcessNotExists)
{
    BaseMockInitializer<> mockInitializer;

    const Json::Value requestProcessSize(Json::objectValue);
    WebAppManagerServiceLuna* lunaService = WebAppManagerServiceLuna::instance();
    const auto responseProcessSize = lunaService->getWebProcessSize(requestProcessSize);

    ASSERT_TRUE(responseProcessSize.isObject());
    ASSERT_TRUE(responseProcessSize.isMember("returnValue"));
    ASSERT_TRUE(responseProcessSize["returnValue"].asBool());
    ASSERT_TRUE(responseProcessSize["WebProcesses"].isArray());

    auto processes = responseProcessSize["WebProcesses"];
    ASSERT_EQ(processes.size(), 0);
}

TEST(GetWebProcessSizeTest, checkCaseProcessExists)
{
    BaseMockInitializer<NiceWebViewMock, NiceWebAppWindowMock,
                        PlatformModuleFactoryImplMock> mockInitializer;

    Json::Value requestLaunch;
    ASSERT_TRUE(util::JsonValueFromString(kLaunchAppJsonBody, requestLaunch));
    WebAppManagerServiceLuna* lunaService = WebAppManagerServiceLuna::instance();
    const auto responseLaunch = lunaService->launchApp(requestLaunch);

    ASSERT_TRUE(responseLaunch.isObject());
    ASSERT_TRUE(responseLaunch.isMember("returnValue"));
    ASSERT_TRUE(responseLaunch["returnValue"].asBool());

    BlinkWebProcessManagerMock* procManager = static_cast<BlinkWebProcessManagerMock*>(
        WebAppManager::instance()->getWebProcessManager());
    EXPECT_CALL(*procManager, getWebProcessPIDMock())
        .WillRepeatedly(testing::Return(kProcessId));
    EXPECT_CALL(*procManager, getWebProcessMemSize(kProcessId))
        .WillRepeatedly(testing::Return(kProcessMemSize));

    const Json::Value requestProcessSize(Json::objectValue);
    const auto responseProcessSize = lunaService->getWebProcessSize(requestProcessSize);

    ASSERT_TRUE(responseProcessSize.isObject());
    ASSERT_TRUE(responseProcessSize.isMember("returnValue"));
    ASSERT_TRUE(responseProcessSize["returnValue"].asBool());
    ASSERT_TRUE(responseProcessSize["WebProcesses"].isArray());

    auto processes = responseProcessSize["WebProcesses"];
    ASSERT_TRUE(processes.size() > 0);
    int processPosition = -1;
    for (unsigned int i = 0; i < processes.size(); i++) {
        ASSERT_TRUE(processes[i].isObject());
        auto process = processes[i];
        ASSERT_TRUE(process.isMember("pid"));
        ASSERT_TRUE(process["pid"].isString());
        int pid = 0;
        strToInt(process["pid"].asString(), pid);
        if (pid == kProcessId) {
            processPosition = i;
            ASSERT_TRUE(process.isMember("webProcessSize"));
            ASSERT_STREQ(process["webProcessSize"].asString().c_str(), kProcessMemSize);

            ASSERT_TRUE(process["runningApps"].isArray());

            auto runningApps = process["runningApps"];
            ASSERT_EQ(runningApps.size(), 1);
            ASSERT_TRUE(runningApps[0].isObject());

            auto application = runningApps[0];
            ASSERT_TRUE(application.isMember("id"));
            ASSERT_TRUE(application.isMember("instanceId"));
            ASSERT_STREQ(application["id"].asString().c_str(), kApplicationId);
            ASSERT_STREQ(application["instanceId"].asString().c_str(), kInstanceId);
        }
    }

    ASSERT_TRUE(processPosition >= 0);
}
