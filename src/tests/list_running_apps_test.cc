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

#include <cstdlib>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/json.h>

#include "base_mock_initializer.h"
#include "utils.h"
#include "web_app_manager_service.h"
#include "web_app_manager_service_luna.h"
#include "web_view_mock_impl.h"

namespace {

// TODO: Move it to separate file.
constexpr char kLaunchBareAppJsonBody[] = R"({
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

// TODO: Move it to separate file.
static constexpr char kLaunchWebRTCAppJsonBody[] = R"({
  "appDesc": {
    "accessibility": {
      "supportsAudioGuidance": false
    },
    "allowAudioCapture": true,
    "allowVideoCapture": true,
    "bgImage": "",
    "checkUpdateOnLaunch": true,
    "deeplinkingParams": "",
    "defaultWindowType": "card",
    "disableBackHistoryAPI": false,
    "favicon": "",
    "folderPath": "/usr/palm/applications/com.webos.app.test.webrtc",
    "handlesRelaunch": false,
    "hasPromotion": false,
    "icon": "/usr/palm/applications/com.webos.app.test.webrtc/icon.png",
    "icons": [],
    "id": "",
    "imageForRecents": "",
    "inAppSetting": false,
    "inspectable": false,
    "largeIcon": "",
    "launchPointId": "com.webos.app.test.webrtc_default",
    "lockable": true,
    "lptype": "default",
    "main": "index.html",
    "noSplashOnLaunch": false,
    "privilegedJail": false,
    "removable": false,
    "requiredPermissions": [
      "audio.operation",
      "media.operation",
      "mediapipeline.resourcemanagement",
      "mediapipeline.operation"
    ],
    "spinnerOnLaunch": true,
    "systemApp": true,
    "tileSize": "normal",
    "title": "Test WebRTC",
    "transparent": false,
    "trustLevel": "default",
    "type": "web",
    "uiRevision": 2,
    "unmovable": false,
    "vendor": "LG Silicon Valley Labs",
    "version": "1.0.0",
    "visible": true
  },
  "appId": "com.webos.app.test.webrtc",
  "instanceId": "6817be08-1116-415b-9d05-31b0675745a60",
  "launchingAppId": "com.webos.app.home",
  "launchingProcId": "",
  "parameters": {
    "displayAffinity": 1
  },
  "reason": "com.webos.app.home"
})";

}  // namespace

TEST(ListRunningAppsTest, IncludeSysApps) {
  BaseMockInitializer<NiceWebViewMockImpl> mock_initializer;
  mock_initializer.GetWebViewMock()->SetOnInitActions();
  mock_initializer.GetWebViewMock()->SetOnLoadURLActions();

  int pid = 1 + rand() % 10000;
  EXPECT_CALL(*mock_initializer.GetWebViewMock(), RenderProcessPid())
      .WillRepeatedly(testing::Return(pid));

  Json::Value bare_request;
  ASSERT_TRUE(util::StringToJson(kLaunchBareAppJsonBody, bare_request));
  WebAppManagerServiceLuna* luna_service = WebAppManagerServiceLuna::Instance();
  auto result = luna_service->launchApp(bare_request);

  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result["returnValue"].asBool());

  Json::Value webrtc_request;
  ASSERT_TRUE(util::StringToJson(kLaunchWebRTCAppJsonBody, webrtc_request));
  result = luna_service->launchApp(webrtc_request);

  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result["returnValue"].asBool());

  Json::Value request;
  request["includeSysApps"] = true;
  const auto reply = luna_service->listRunningApps(request, true);

  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(reply["returnValue"].asBool());
  ASSERT_TRUE(reply["running"].isArray());
  const auto running_apps = reply["running"];
  ASSERT_EQ(2, running_apps.size());

  ASSERT_TRUE(running_apps[0].isObject());
  auto running_app = running_apps[0];
  EXPECT_TRUE(running_app.isMember("id"));
  EXPECT_EQ(bare_request["appDesc"]["id"].asString(),
            running_app["id"].asString());
  EXPECT_TRUE(running_app.isMember("instanceId"));
  EXPECT_EQ(bare_request["instanceId"].asString(),
            running_app["instanceId"].asString());
  EXPECT_TRUE(running_app.isMember("webprocessid"));
  EXPECT_EQ(std::to_string(pid), running_app["webprocessid"].asString());

  running_app = running_apps[1];
  EXPECT_TRUE(running_app.isMember("id"));
  EXPECT_EQ(webrtc_request["appDesc"]["id"].asString(),
            running_app["id"].asString());
  EXPECT_TRUE(running_app.isMember("instanceId"));
  EXPECT_EQ(webrtc_request["instanceId"].asString(),
            running_app["instanceId"].asString());
  EXPECT_TRUE(running_app.isMember("webprocessid"));
  EXPECT_EQ(std::to_string(pid), running_app["webprocessid"].asString());
}

TEST(ListRunningAppsTest, ExcludeSysApps) {
  BaseMockInitializer<NiceWebViewMockImpl> mock_initializer;
  mock_initializer.GetWebViewMock()->SetOnInitActions();
  mock_initializer.GetWebViewMock()->SetOnLoadURLActions();

  int pid = 1 + rand() % 10000;
  EXPECT_CALL(*mock_initializer.GetWebViewMock(), RenderProcessPid())
      .WillRepeatedly(testing::Return(pid));

  Json::Value bare_request;
  ASSERT_TRUE(util::StringToJson(kLaunchBareAppJsonBody, bare_request));
  WebAppManagerServiceLuna* luna_service = WebAppManagerServiceLuna::Instance();
  auto result = luna_service->launchApp(bare_request);

  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result["returnValue"].asBool());

  Json::Value webrtc_request;
  ASSERT_TRUE(util::StringToJson(kLaunchWebRTCAppJsonBody, webrtc_request));
  result = luna_service->launchApp(webrtc_request);

  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result["returnValue"].asBool());

  Json::Value request;
  request["includeSysApps"] = false;
  const auto reply = luna_service->listRunningApps(request, true);

  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  ASSERT_TRUE(reply["returnValue"].asBool());
  ASSERT_TRUE(reply["running"].isArray());
  const auto running_apps = reply["running"];
  ASSERT_EQ(1, running_apps.size());

  auto running_app = running_apps[0];
  EXPECT_TRUE(running_app.isMember("id"));
  EXPECT_EQ(bare_request["appDesc"]["id"].asString(),
            running_app["id"].asString());
  EXPECT_TRUE(running_app.isMember("instanceId"));
  EXPECT_EQ(bare_request["instanceId"].asString(),
            running_app["instanceId"].asString());
  EXPECT_TRUE(running_app.isMember("webprocessid"));
  EXPECT_EQ(std::to_string(pid), running_app["webprocessid"].asString());
}
