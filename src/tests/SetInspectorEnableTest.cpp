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

namespace {
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

TEST(SetInspectorEnableTest, checkLunaRequestIsDummy)
{
    BaseMockInitializer<> mockInitializer;

    WebAppManagerServiceLuna* lunaService = WebAppManagerServiceLuna::instance();
    const Json::Value request(Json::objectValue);
    const Json::Value response = lunaService->setInspectorEnable(request);

    ASSERT_TRUE(response.isObject());
    ASSERT_TRUE(response.isMember("returnValue"));
    ASSERT_TRUE(response["returnValue"].asBool());
}

TEST(SetInspectorEnableTest, checkCaseNoApplications)
{
    BaseMockInitializer<> mockInitializer;

    std::string appId(kApplicationId);
    EXPECT_FALSE(WebAppManager::instance()->setInspectorEnable(appId));
}

TEST(SetInspectorEnableTest, checkCaseApplicationNotExists)
{
    BaseMockInitializer<> mockInitializer;

    Json::Value requestLaunch;
    ASSERT_TRUE(util::stringToJson(kLaunchAppJsonBody, requestLaunch));
    WebAppManagerServiceLuna* lunaService = WebAppManagerServiceLuna::instance();
    const auto responseLaunch = lunaService->launchApp(requestLaunch);

    ASSERT_TRUE(responseLaunch.isObject());
    ASSERT_TRUE(responseLaunch.isMember("returnValue"));
    ASSERT_TRUE(responseLaunch["returnValue"].asBool());

    std::string appId("NotExistingAppId");
    EXPECT_FALSE(WebAppManager::instance()->setInspectorEnable(appId));
}

TEST(SetInspectorEnableTest, checkCaseApplicationExists)
{
    BaseMockInitializer<> mockInitializer;

    Json::Value requestLaunch;
    ASSERT_TRUE(util::stringToJson(kLaunchAppJsonBody, requestLaunch));
    WebAppManagerServiceLuna* lunaService = WebAppManagerServiceLuna::instance();
    const auto responseLaunch = lunaService->launchApp(requestLaunch);

    ASSERT_TRUE(responseLaunch.isObject());
    ASSERT_TRUE(responseLaunch.isMember("returnValue"));
    ASSERT_TRUE(responseLaunch["returnValue"].asBool());

    std::string appId(kApplicationId);
    EXPECT_TRUE(WebAppManager::instance()->setInspectorEnable(appId));
}
