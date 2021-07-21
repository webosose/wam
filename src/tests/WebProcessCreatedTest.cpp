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
#include "Utils.h"
#include "WebAppManagerServiceLuna.h"
#include "WebViewMockImpl.h"

namespace {

static constexpr int kPid = 7236;
static constexpr char kInstanceId[] = "de90e74a-b86b-42c8-8785-3efd927a36430";
static constexpr char kAppId[] = "bareapp";
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

TEST(WebProcessCreatedTest, WebProcessNotExist) {
  Json::Value request;
  request["instanceId"] = kInstanceId;
  request["appId"] = kAppId;
  const auto reply =
      WebAppManagerServiceLuna::instance()->webProcessCreated(request, false);

  ASSERT_TRUE(reply.isObject());
  EXPECT_TRUE(reply.isMember("id"));
  EXPECT_STREQ(reply["id"].asString().c_str(), kAppId);

  EXPECT_TRUE(reply.isMember("instanceId"));
  EXPECT_STREQ(reply["instanceId"].asString().c_str(), kInstanceId);

  EXPECT_TRUE(reply.isMember("returnValue"));
  EXPECT_FALSE(reply["returnValue"].asBool());

  EXPECT_TRUE(reply.isMember("errorText"));
  EXPECT_STREQ(reply["errorText"].asString().c_str(), "process is not running");
}

TEST(WebProcessCreatedTest, BadRequest) {
  Json::Value request(Json::objectValue);
  const auto reply =
      WebAppManagerServiceLuna::instance()->webProcessCreated(request, false);

  ASSERT_TRUE(reply.isObject());
  EXPECT_TRUE(reply["id"].isNull());
  EXPECT_TRUE(reply["instanceId"].isNull());
  EXPECT_TRUE(reply.isMember("returnValue"));
  EXPECT_FALSE(reply["returnValue"].asBool());
  EXPECT_TRUE(reply.isMember("errorText"));
  EXPECT_STREQ(reply["errorText"].asString().c_str(), "parameter error");
}

TEST(WebProcessCreatedTest, CheckSubscribe) {
  const Json::Value request(Json::objectValue);
  const auto reply =
      WebAppManagerServiceLuna::instance()->webProcessCreated(request, true);

  ASSERT_TRUE(reply.isObject());
  EXPECT_TRUE(reply["id"].isNull());
  EXPECT_TRUE(reply["instanceId"].isNull());
  EXPECT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
  EXPECT_TRUE(reply["errorText"].isNull());
}

TEST(WebProcessCreatedTest, WebProcessCreated) {
  BaseMockInitializer<NiceWebViewMockImpl> mock_initializer;
  mock_initializer.GetWebViewMock()->SetOnInitActions();
  mock_initializer.GetWebViewMock()->SetOnLoadURLActions();

  Json::Value launch_request;
  ASSERT_TRUE(util::stringToJson(kLaunchAppJsonBody, launch_request));
  WebAppManagerServiceLuna* luna_service = WebAppManagerServiceLuna::instance();
  const auto result = luna_service->launchApp(launch_request);

  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result["returnValue"].asBool());

  EXPECT_CALL(*mock_initializer.GetWebViewMock(), RenderProcessPid())
      .WillRepeatedly(testing::Return(kPid));

  Json::Value request;
  request["instanceId"] = kInstanceId;
  request["appId"] = kAppId;
  const auto reply = luna_service->webProcessCreated(request, false);

  ASSERT_TRUE(reply.isObject());
  EXPECT_TRUE(reply["errorText"].isNull());

  EXPECT_TRUE(reply.isMember("id"));
  EXPECT_STREQ(reply["id"].asString().c_str(), kAppId);

  EXPECT_TRUE(reply.isMember("instanceId"));
  EXPECT_STREQ(reply["instanceId"].asString().c_str(), kInstanceId);

  EXPECT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());

  EXPECT_TRUE(reply.isMember("webprocessid"));
  EXPECT_EQ(kPid, reply["webprocessid"].asInt());
}
