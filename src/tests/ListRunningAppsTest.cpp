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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "BaseMockInitializer.h"
#include "WebAppManagerService.h"
#include "WebAppManagerServiceLuna.h"
#include "WebViewMockImpl.h"

namespace {

// TODO: Move it to separate file.
constexpr char launchBareAppJsonBody[] = R"({
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
static constexpr char launchWebRTCAppJsonBody[] = R"({
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

  QJsonParseError parse_error;
  QJsonDocument bare_request = QJsonDocument::fromJson(
      QString::fromUtf8(launchBareAppJsonBody).toUtf8(), &parse_error);
  ASSERT_EQ(parse_error.error, QJsonParseError::NoError);
  WebAppManagerServiceLuna* luna_service = WebAppManagerServiceLuna::instance();
  QJsonObject result = luna_service->launchApp(bare_request.object());

  ASSERT_TRUE(result.contains("returnValue"));
  ASSERT_TRUE(result["returnValue"].toBool());

  QJsonDocument webrtc_request = QJsonDocument::fromJson(
      QString::fromUtf8(launchWebRTCAppJsonBody).toUtf8(), &parse_error);
  ASSERT_EQ(parse_error.error, QJsonParseError::NoError);
  result = luna_service->launchApp(webrtc_request.object());

  ASSERT_TRUE(result.contains("returnValue"));
  ASSERT_TRUE(result["returnValue"].toBool());

  const QJsonObject request{{"includeSysApps", true}};
  const QJsonObject reply = luna_service->listRunningApps(request, true);

  ASSERT_TRUE(reply.contains("returnValue"));
  ASSERT_TRUE(reply["returnValue"].toBool());
  ASSERT_TRUE(reply.contains("running"));
  const auto running_apps = reply["running"].toArray();
  ASSERT_EQ(2, running_apps.size());

  auto running_app = running_apps[0].toObject();
  EXPECT_TRUE(running_app.contains("id"));
  EXPECT_EQ(bare_request["appDesc"].toObject()["id"].toString(),
            running_app["id"].toString());
  EXPECT_TRUE(running_app.contains("instanceId"));
  EXPECT_EQ(bare_request["instanceId"], running_app["instanceId"].toString());
  EXPECT_TRUE(running_app.contains("webprocessid"));
  EXPECT_EQ(QString::number(pid).toStdString(),
            running_app["webprocessid"].toString().toStdString());

  running_app = running_apps[1].toObject();
  EXPECT_TRUE(running_app.contains("id"));
  EXPECT_EQ(webrtc_request["appDesc"].toObject()["id"].toString(),
            running_app["id"].toString());
  EXPECT_TRUE(running_app.contains("instanceId"));
  EXPECT_EQ(webrtc_request["instanceId"], running_app["instanceId"].toString());
  EXPECT_TRUE(running_app.contains("webprocessid"));
  EXPECT_EQ(QString::number(pid).toStdString(),
            running_app["webprocessid"].toString().toStdString());
}

TEST(ListRunningAppsTest, ExcludeSysApps) {
  BaseMockInitializer<NiceWebViewMockImpl> mock_initializer;
  mock_initializer.GetWebViewMock()->SetOnInitActions();
  mock_initializer.GetWebViewMock()->SetOnLoadURLActions();

  int pid = 1 + rand() % 10000;
  EXPECT_CALL(*mock_initializer.GetWebViewMock(), RenderProcessPid())
      .WillRepeatedly(testing::Return(pid));

  QJsonParseError parse_error;
  QJsonDocument bare_request = QJsonDocument::fromJson(
      QString::fromUtf8(launchBareAppJsonBody).toUtf8(), &parse_error);
  ASSERT_EQ(parse_error.error, QJsonParseError::NoError);
  WebAppManagerServiceLuna* luna_service = WebAppManagerServiceLuna::instance();
  QJsonObject result = luna_service->launchApp(bare_request.object());

  ASSERT_TRUE(result.contains("returnValue"));
  ASSERT_TRUE(result["returnValue"].toBool());

  QJsonDocument webrtc_request = QJsonDocument::fromJson(
      QString::fromUtf8(launchWebRTCAppJsonBody).toUtf8(), &parse_error);
  ASSERT_EQ(parse_error.error, QJsonParseError::NoError);
  result = luna_service->launchApp(webrtc_request.object());

  ASSERT_TRUE(result.contains("returnValue"));
  ASSERT_TRUE(result["returnValue"].toBool());

  const QJsonObject request{{"includeSysApps", false}};
  const QJsonObject reply = luna_service->listRunningApps(request, true);

  ASSERT_TRUE(reply.contains("returnValue"));
  ASSERT_TRUE(reply["returnValue"].toBool());
  ASSERT_TRUE(reply.contains("running"));
  const auto running_apps = reply["running"].toArray();
  ASSERT_EQ(1, running_apps.size());

  auto running_app = running_apps[0].toObject();
  EXPECT_TRUE(running_app.contains("id"));
  EXPECT_EQ(bare_request["appDesc"].toObject()["id"].toString(),
            running_app["id"].toString());
  EXPECT_TRUE(running_app.contains("instanceId"));
  EXPECT_EQ(bare_request["instanceId"], running_app["instanceId"].toString());
  EXPECT_TRUE(running_app.contains("webprocessid"));
  EXPECT_EQ(QString::number(pid).toStdString(),
            running_app["webprocessid"].toString().toStdString());
}
