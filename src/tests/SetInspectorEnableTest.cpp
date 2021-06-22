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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "BaseMockInitializer.h"
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
    constexpr char expectedResponse[] = "{\n    \"returnValue\": true\n}\n";

    BaseMockInitializer<> mockInitializer;

    WebAppManagerServiceLuna* lunaService = WebAppManagerServiceLuna::instance();
    const QJsonObject request {};
    const QJsonObject response = lunaService->setInspectorEnable(request);
    std::string responseStr = QJsonDocument(response).toJson().toStdString();

    ASSERT_STREQ(expectedResponse, responseStr.c_str());
}

TEST(SetInspectorEnableTest, checkCaseNoApplications)
{
    BaseMockInitializer<> mockInitializer;

    QString appId(kApplicationId);
    EXPECT_FALSE(WebAppManager::instance()->setInspectorEnable(appId));
}

TEST(SetInspectorEnableTest, checkCaseApplicationNotExists)
{
    BaseMockInitializer<> mockInitializer;

    QJsonParseError parseError;
    QJsonDocument requestLaunch = QJsonDocument::fromJson(
        QString::fromUtf8(kLaunchAppJsonBody).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);
    WebAppManagerServiceLuna* lunaService = WebAppManagerServiceLuna::instance();
    const QJsonObject responseLaunch = lunaService->launchApp(requestLaunch.object());

    ASSERT_TRUE(responseLaunch.contains("returnValue"));
    ASSERT_TRUE(responseLaunch["returnValue"].toBool());

    QString appId("NotExistingAppId");
    EXPECT_FALSE(WebAppManager::instance()->setInspectorEnable(appId));
}

TEST(SetInspectorEnableTest, checkCaseApplicationExists)
{
    BaseMockInitializer<> mockInitializer;

    QJsonParseError parseError;
    QJsonDocument requestLaunch = QJsonDocument::fromJson(
        QString::fromUtf8(kLaunchAppJsonBody).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);
    WebAppManagerServiceLuna* lunaService = WebAppManagerServiceLuna::instance();
    const QJsonObject responseLaunch = lunaService->launchApp(requestLaunch.object());

    ASSERT_TRUE(responseLaunch.contains("returnValue"));
    ASSERT_TRUE(responseLaunch["returnValue"].toBool());

    QString appId(kApplicationId);
    EXPECT_TRUE(WebAppManager::instance()->setInspectorEnable(appId));
}
