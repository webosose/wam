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

#include "base_mock_initializer.h"
#include "blink_web_process_manager_mock.h"
#include "platform_module_factory_impl_mock.h"
#include "utils.h"
#include "web_app_manager_service_luna.h"
#include "web_view_mock.h"

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

}  // namespace

TEST(GetWebProcessSizeTest, checkCaseProcessNotExists) {
  BaseMockInitializer<> mock_initializer;

  const Json::Value request_process_size(Json::objectValue);
  WebAppManagerServiceLuna* luna_service = WebAppManagerServiceLuna::Instance();
  const auto response_process_size =
      luna_service->getWebProcessSize(request_process_size);

  ASSERT_TRUE(response_process_size.isObject());
  ASSERT_TRUE(response_process_size.isMember("returnValue"));
  ASSERT_TRUE(response_process_size["returnValue"].asBool());
  ASSERT_TRUE(response_process_size["WebProcesses"].isArray());

  auto processes = response_process_size["WebProcesses"];
  ASSERT_EQ(processes.size(), 0);
}

TEST(GetWebProcessSizeTest, checkCaseProcessExists) {
  BaseMockInitializer<NiceWebViewMock, NiceWebAppWindowMock,
                      PlatformModuleFactoryImplMock>
      mockInitializer;

  Json::Value request_launch;
  ASSERT_TRUE(util::StringToJson(kLaunchAppJsonBody, request_launch));
  WebAppManagerServiceLuna* luna_service = WebAppManagerServiceLuna::Instance();
  const auto response_launch = luna_service->launchApp(request_launch);

  ASSERT_TRUE(response_launch.isObject());
  ASSERT_TRUE(response_launch.isMember("returnValue"));
  ASSERT_TRUE(response_launch["returnValue"].asBool());

  BlinkWebProcessManagerMock* proccess_manager =
      static_cast<BlinkWebProcessManagerMock*>(
          WebAppManager::Instance()->GetWebProcessManager());
  EXPECT_CALL(*proccess_manager, GetWebProcessPIDMock())
      .WillRepeatedly(testing::Return(kProcessId));
  EXPECT_CALL(*proccess_manager, GetWebProcessMemSize(kProcessId))
      .WillRepeatedly(testing::Return(kProcessMemSize));

  const Json::Value request_process_size(Json::objectValue);
  const auto response_process_size =
      luna_service->getWebProcessSize(request_process_size);

  ASSERT_TRUE(response_process_size.isObject());
  ASSERT_TRUE(response_process_size.isMember("returnValue"));
  ASSERT_TRUE(response_process_size["returnValue"].asBool());
  ASSERT_TRUE(response_process_size["WebProcesses"].isArray());

  auto processes = response_process_size["WebProcesses"];
  ASSERT_TRUE(processes.size() > 0);
  int process_position = -1;
  for (unsigned int i = 0; i < processes.size(); i++) {
    ASSERT_TRUE(processes[i].isObject());
    auto process = processes[i];
    ASSERT_TRUE(process.isMember("pid"));
    ASSERT_TRUE(process["pid"].isString());
    int pid = util::StrToIntWithDefault(process["pid"].asString(), 0);
    if (pid == kProcessId) {
      process_position = i;
      ASSERT_TRUE(process.isMember("webProcessSize"));
      ASSERT_STREQ(process["webProcessSize"].asString().c_str(),
                   kProcessMemSize);

      ASSERT_TRUE(process["runningApps"].isArray());

      auto running_apps = process["runningApps"];
      ASSERT_EQ(running_apps.size(), 1);
      ASSERT_TRUE(running_apps[0].isObject());

      auto application = running_apps[0];
      ASSERT_TRUE(application.isMember("id"));
      ASSERT_TRUE(application.isMember("instanceId"));
      ASSERT_STREQ(application["id"].asString().c_str(), kApplicationId);
      ASSERT_STREQ(application["instanceId"].asString().c_str(), kInstanceId);
    }
  }
  ASSERT_TRUE(process_position >= 0);
}
