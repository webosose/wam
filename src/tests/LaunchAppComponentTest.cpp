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

#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QJsonDocument>
#include <QJsonObject>

#include "PlatformModuleFactoryImpl.h"
#include "WebAppFactoryManagerMock.h"
#include "WebAppManager.h"
#include "WebAppManagerServiceLuna.h"
#include "WebAppWindowFactoryMock.h"
#include "WebAppWindowMock.h"
#include "WebPageBlinkDelegate.h"
#include "WebViewFactoryMock.h"
#include "WebViewMock.h"
#include "webos/window_group_configuration.h"

namespace {

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::StrEq;

// TODO: Move it to separate file.
static constexpr char launchBareAppJsonBody[] = R"({
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
  "instanceId": "188f99b7-1e1a-489f-8e3d-56844a7713030"
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
    "id": "com.webos.app.test.webrtc",
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


static constexpr char localeInfo[] = R"({
    "subscribed": false,
    "returnValue": true,
    "method": "getSystemSettings",
    "settings": {
        "localeInfo": {
            "keyboards": [
                "en"
            ],
            "locales": {
                "UI": "en-US",
                "FMT": "en-US",
                "NLP": "en-US",
                "AUD": "en-US",
                "AUD2": "en-US",
                "STT": "en-US"
            },
            "clock": "locale",
            "timezone": ""
        }
    }
})";

} // namespace

class LaunchAppTestSuite : public ::testing::Test {
public:
    LaunchAppTestSuite() = default;
    ~LaunchAppTestSuite() override = default;

    void SetUp() override;
    void TearDown() override;

    WebAppWindowFactoryMock* webAppWindowFactory;
    WebAppWindowMock* webAppWindow;
    WebPageBlinkDelegate* webViewDelegate;
    WebViewFactoryMock* webViewFactory;
    WebViewMock* webView;
    std::string viewUrl;
};

void LaunchAppTestSuite::SetUp()
{
    WebAppManager::instance()->setPlatformModules(std::make_unique<PlatformModuleFactoryImpl>());
    webViewFactory = new WebViewFactoryMock();
    webAppWindowFactory = new WebAppWindowFactoryMock();
    webView = new NiceWebViewMock();
    webAppWindow = new NiceWebAppWindowMock();

    auto webAppFactoryManager = std::make_unique<WebAppFactoryManagerMock>();
    webAppFactoryManager->setWebViewFactory(webViewFactory);
    webAppFactoryManager->setWebAppWindowFactory(webAppWindowFactory);

    webViewFactory->setWebView(webView);
    webAppWindowFactory->setWebAppWindow(webAppWindow);

    WebAppManager::instance()->setWebAppFactory(std::move(webAppFactoryManager));

    ON_CALL(*webView, GetUrl()).WillByDefault(ReturnRef(viewUrl));
    ON_CALL(*webView, setDelegate(_)).WillByDefault(Invoke([&](WebPageBlinkDelegate* delegate) {
        webViewDelegate = delegate;
    }));
    ON_CALL(*webView, LoadUrl(_)).WillByDefault(Invoke([&](const std::string& url) {
        viewUrl = url;
        if (!webViewDelegate) {
            return;
        }
        webViewDelegate->loadStarted();
        webViewDelegate->loadProgressChanged(100.0);
        webViewDelegate->loadVisuallyCommitted();
        webViewDelegate->loadFinished(url);
    }));
}

void LaunchAppTestSuite::TearDown()
{
    WebAppManager::instance()->closeAllApps();
}

TEST_F(LaunchAppTestSuite, LaunchOnPrimaryDisplay)
{
    constexpr char instanceId[] = "188f99b7-1e1a-489f-8e3d-56844a7713030";
    constexpr char appId[] = "bareapp";
    const uint32_t width = 888;
    const uint32_t height = 777;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(launchBareAppJsonBody).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    EXPECT_CALL(*webAppWindow, DisplayWidth()).WillRepeatedly(Return(width));
    EXPECT_CALL(*webAppWindow, DisplayHeight()).WillRepeatedly(Return(height));
    EXPECT_CALL(*webAppWindow, InitWindow(width, height));
    EXPECT_CALL(*webAppWindow, SetWindowProperty(_, _)).Times(AnyNumber());
    EXPECT_CALL(*webAppWindow, SetWindowProperty("appId", appId));
    EXPECT_CALL(*webAppWindow, SetWindowProperty("displayAffinity", "0"));
    EXPECT_CALL(*webAppWindow, SetWindowProperty("instanceId", instanceId));
    EXPECT_CALL(*webAppWindow, SetWindowProperty("launchingAppId", "com.webos.app.home"));
    EXPECT_CALL(*webAppWindow, SetWindowProperty("title", "Bare App"));
    EXPECT_CALL(*webAppWindow, SetWindowProperty("icon", "/usr/palm/applications/bareapp/icon.png"));

    EXPECT_CALL(*webView, Initialize("bareapp0", "/usr/palm/applications/bareapp", "default", "", "", false));
    EXPECT_CALL(*webView, DefaultUserAgent()).WillRepeatedly(Return("Test User-Agent"));
    EXPECT_CALL(*webView, SetUserAgent("Test User-Agent "));
    EXPECT_CALL(*webView, SetAppId("bareapp0"));
    EXPECT_CALL(*webView, SetSecurityOrigin("bareapp-webos"));
    EXPECT_CALL(*webView, SetDatabaseIdentifier("bareapp"));
    EXPECT_CALL(*webView, LoadUrl(_));
    EXPECT_CALL(*webView, LoadUrl(StrEq("file:///usr/palm/applications/bareapp/index.html")));

    const auto& result = WebAppManagerServiceLuna::instance()->launchApp(doc.object());
    ASSERT_TRUE(result.contains("returnValue"));
    ASSERT_TRUE(result.contains("instanceId"));
    ASSERT_TRUE(result.contains("appId"));
    EXPECT_TRUE(result["returnValue"].toBool());
    EXPECT_STREQ(result["instanceId"].toString().toStdString().c_str(), instanceId);
    EXPECT_STREQ(result["appId"].toString().toStdString().c_str(), appId);
}

TEST_F(LaunchAppTestSuite, LaunchOnSecondaryDisplay)
{
    constexpr char instanceId[] = "6817be08-1116-415b-9d05-31b0675745a60";
    constexpr char appId[] = "com.webos.app.test.webrtc";

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(launchWebRTCAppJsonBody).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    EXPECT_CALL(*webAppWindow, SetWindowProperty(_, _)).Times(AnyNumber());
    EXPECT_CALL(*webAppWindow, SetWindowProperty("displayAffinity", "1")).Times(1);

    EXPECT_CALL(*webView, Initialize("com.webos.app.test.webrtc1", "/usr/palm/applications/com.webos.app.test.webrtc", "default", "", "", false));
    EXPECT_CALL(*webView, SetAppId("com.webos.app.test.webrtc1"));

    const auto& result = WebAppManagerServiceLuna::instance()->launchApp(doc.object());
    ASSERT_TRUE(result.contains("returnValue"));
    ASSERT_TRUE(result.contains("instanceId"));
    ASSERT_TRUE(result.contains("appId"));
    EXPECT_TRUE(result["returnValue"].toBool());
    EXPECT_STREQ(result["instanceId"].toString().toStdString().c_str(), instanceId);
    EXPECT_STREQ(result["appId"].toString().toStdString().c_str(), appId);
}

TEST_F(LaunchAppTestSuite, LaunchAppsWithParams)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(launchBareAppJsonBody).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject root = doc.object();
    QJsonObject parameters = root["parameters"].toObject();
    parameters["testParamField"] = "testParamValue";
    root["parameters"] = parameters;
    doc.setObject(root);

    EXPECT_CALL(*webView, addUserScript(::testing::HasSubstr("\"testParamField\": \"testParamValue\"")));

    const auto& result = WebAppManagerServiceLuna::instance()->launchApp(doc.object());
    ASSERT_TRUE(result.contains("returnValue"));
    ASSERT_TRUE(result.contains("instanceId"));
    ASSERT_TRUE(result.contains("appId"));
}

TEST_F(LaunchAppTestSuite, LaunchAppsWithError)
{
    constexpr char path[] = "file:///usr/share/localization/wam/loaderror.html";
    constexpr char varName[] = "WAM_ERROR_PAGE";
    const auto actualValue = getenv(varName);
    if (!actualValue) {
         int result = setenv(varName, path, false);
         ASSERT_FALSE(result);
     }
     WebAppManager::instance()->setPlatformModules(std::make_unique<PlatformModuleFactoryImpl>());

    EXPECT_CALL(*webView, LoadUrl(_));
    EXPECT_CALL(*webView, LoadUrl(::testing::HasSubstr("index.html"))).WillOnce(Invoke([&](const std::string& url) {
        viewUrl = url;
        if (!webViewDelegate) {
            return;
        }
        webViewDelegate->loadStarted();
        webViewDelegate->loadProgressChanged(100.0);
        webViewDelegate->loadVisuallyCommitted();
        webViewDelegate->loadFailed(url, 404, "SSL_ERROR");
    }));
    EXPECT_CALL(*webView, LoadUrl(::testing::HasSubstr("loaderror.html"))).WillOnce(Invoke([&](const std::string& url) {
        viewUrl = url;
        if (!webViewDelegate) {
            return;
        }
        webViewDelegate->loadStarted();
        webViewDelegate->loadProgressChanged(100.0);
        webViewDelegate->loadVisuallyCommitted();
        webViewDelegate->loadFinished(url);
    }));

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(localeInfo).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    WebAppManagerServiceLuna::instance()->getSystemLocalePreferencesCallback(doc.object());

    doc = QJsonDocument::fromJson(QString::fromUtf8(launchWebRTCAppJsonBody).toUtf8(), &parseError);

    const auto& result = WebAppManagerServiceLuna::instance()->launchApp(doc.object());
    ASSERT_TRUE(result.contains("returnValue"));
    ASSERT_TRUE(result.contains("instanceId"));
    ASSERT_TRUE(result.contains("appId"));

    if (!actualValue) {
        int result = unsetenv(varName);
        ASSERT_FALSE(result);
    }
}
