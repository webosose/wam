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
#include <json/json.h>

#include "platform_module_factory_impl.h"
#include "utils.h"
#include "web_app_factory_manager_mock.h"
#include "web_app_manager.h"
#include "web_app_manager_service_luna.h"
#include "web_app_window_factory_mock.h"
#include "web_app_window_mock.h"
#include "web_page_blink_delegate.h"
#include "web_view_factory_mock.h"
#include "web_view_mock.h"
#include "webos/window_group_configuration.h"

namespace {

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::StrEq;

// TODO: Move it to separate file.
static constexpr char kLaunchBareAppJsonBody[] = R"({
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

static constexpr char kLocaleInfo[] = R"({
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

}  // namespace

class LaunchAppTestSuite : public ::testing::Test {
 public:
  LaunchAppTestSuite() = default;
  ~LaunchAppTestSuite() override = default;

  void SetUp() override;
  void TearDown() override;

  WebAppWindowFactoryMock* web_app_window_factory_;
  WebAppWindowMock* web_app_window_;
  WebPageBlinkDelegate* web_view_delegate_;
  WebViewFactoryMock* web_view_factory_;
  WebViewMock* web_view_;
  std::string view_url_;
};

void LaunchAppTestSuite::SetUp() {
  WebAppManager::Instance()->SetPlatformModules(
      std::make_unique<PlatformModuleFactoryImpl>());
  web_view_factory_ = new WebViewFactoryMock();
  web_app_window_factory_ = new WebAppWindowFactoryMock();
  web_view_ = new NiceWebViewMock();
  web_app_window_ = new NiceWebAppWindowMock();

  auto web_app_factory_manager = std::make_unique<WebAppFactoryManagerMock>();
  web_app_factory_manager->SetWebViewFactory(web_view_factory_);
  web_app_factory_manager->SetWebAppWindowFactory(web_app_window_factory_);

  web_view_factory_->SetWebView(web_view_);
  web_app_window_factory_->SetWebAppWindow(web_app_window_);

  WebAppManager::Instance()->SetWebAppFactory(
      std::move(web_app_factory_manager));

  ON_CALL(*web_view_, GetUrl()).WillByDefault(ReturnRef(view_url_));
  ON_CALL(*web_view_, SetDelegate(_))
      .WillByDefault(Invoke([&](WebPageBlinkDelegate* delegate) {
        web_view_delegate_ = delegate;
      }));
  ON_CALL(*web_view_, LoadUrl(_))
      .WillByDefault(Invoke([&](const std::string& url) {
        view_url_ = url;
        if (!web_view_delegate_) {
          return;
        }
        web_view_delegate_->LoadStarted();
        web_view_delegate_->LoadProgressChanged(100.0);
        web_view_delegate_->LoadVisuallyCommitted();
        web_view_delegate_->LoadFinished(url);
      }));
}

void LaunchAppTestSuite::TearDown() {
  WebAppManager::Instance()->CloseAllApps();
}

TEST_F(LaunchAppTestSuite, LaunchOnPrimaryDisplay) {
  constexpr char instance_id[] = "188f99b7-1e1a-489f-8e3d-56844a7713030";
  constexpr char app_id[] = "bareapp";
  const uint32_t width = 888;
  const uint32_t height = 777;

  Json::Value request;
  ASSERT_TRUE(util::StringToJson(kLaunchBareAppJsonBody, request));

  EXPECT_CALL(*web_app_window_, DisplayWidth()).WillRepeatedly(Return(width));
  EXPECT_CALL(*web_app_window_, DisplayHeight()).WillRepeatedly(Return(height));
  EXPECT_CALL(*web_app_window_, InitWindow(width, height));
  EXPECT_CALL(*web_app_window_, SetWindowProperty(_, _)).Times(AnyNumber());
  EXPECT_CALL(*web_app_window_, SetWindowProperty("appId", app_id));
  EXPECT_CALL(*web_app_window_, SetWindowProperty("displayAffinity", "0"));
  EXPECT_CALL(*web_app_window_, SetWindowProperty("instanceId", instance_id));
  EXPECT_CALL(*web_app_window_,
              SetWindowProperty("launchingAppId", "com.webos.app.home"));
  EXPECT_CALL(*web_app_window_, SetWindowProperty("title", "Bare App"));
  EXPECT_CALL(
      *web_app_window_,
      SetWindowProperty("icon", "/usr/palm/applications/bareapp/icon.png"));

  EXPECT_CALL(*web_view_,
              Initialize("bareapp0", "/usr/palm/applications/bareapp",
                         "default", "", "", false));
  EXPECT_CALL(*web_view_, DefaultUserAgent())
      .WillRepeatedly(Return("Test User-Agent"));
  EXPECT_CALL(*web_view_, SetUserAgent("Test User-Agent "));
  EXPECT_CALL(*web_view_, SetAppId("bareapp0"));
  EXPECT_CALL(*web_view_, SetSecurityOrigin("bareapp-webos"));
  EXPECT_CALL(*web_view_, SetDatabaseIdentifier("bareapp"));
  EXPECT_CALL(*web_view_, LoadUrl(_));
  EXPECT_CALL(
      *web_view_,
      LoadUrl(StrEq("file:///usr/palm/applications/bareapp/index.html")));

  const auto& result = WebAppManagerServiceLuna::Instance()->launchApp(request);
  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result.isMember("instanceId"));
  ASSERT_TRUE(result.isMember("appId"));
  EXPECT_TRUE(result["returnValue"].asBool());
  EXPECT_STREQ(result["instanceId"].asString().c_str(), instance_id);
  EXPECT_STREQ(result["appId"].asString().c_str(), app_id);
}

TEST_F(LaunchAppTestSuite, LaunchOnSecondaryDisplay) {
  constexpr char instance_id[] = "6817be08-1116-415b-9d05-31b0675745a60";
  constexpr char app_id[] = "com.webos.app.test.webrtc";

  Json::Value request;
  ASSERT_TRUE(util::StringToJson(kLaunchWebRTCAppJsonBody, request));

  EXPECT_CALL(*web_app_window_, SetWindowProperty(_, _)).Times(AnyNumber());
  EXPECT_CALL(*web_app_window_, SetWindowProperty("displayAffinity", "1"))
      .Times(1);

  EXPECT_CALL(*web_view_,
              Initialize("com.webos.app.test.webrtc1",
                         "/usr/palm/applications/com.webos.app.test.webrtc",
                         "default", "", "", false));
  EXPECT_CALL(*web_view_, SetAppId("com.webos.app.test.webrtc1"));

  const auto& result = WebAppManagerServiceLuna::Instance()->launchApp(request);
  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result.isMember("instanceId"));
  ASSERT_TRUE(result.isMember("appId"));
  EXPECT_TRUE(result["returnValue"].asBool());
  EXPECT_STREQ(result["instanceId"].asString().c_str(), instance_id);
  EXPECT_STREQ(result["appId"].asString().c_str(), app_id);
}

TEST_F(LaunchAppTestSuite, LaunchAppsWithParams) {
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(kLaunchBareAppJsonBody, request));
  request["parameters"]["testParamField"] = "testParamValue";

  EXPECT_CALL(*web_view_, AddUserScript(::testing::HasSubstr(
                              "\"testParamField\": \"testParamValue\"")));

  const auto& result = WebAppManagerServiceLuna::Instance()->launchApp(request);
  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result.isMember("instanceId"));
  ASSERT_TRUE(result.isMember("appId"));
}

TEST_F(LaunchAppTestSuite, LaunchAppsWithoutParams) {
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(kLaunchBareAppJsonBody, request));
  request.removeMember("parameters");

  const auto& result = WebAppManagerServiceLuna::Instance()->launchApp(request);
  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result.isMember("instanceId"));
  ASSERT_TRUE(result.isMember("appId"));
}

TEST_F(LaunchAppTestSuite, LaunchAppsWithError) {
  constexpr char path[] = "file:///usr/share/localization/wam/loaderror.html";
  constexpr char var_name[] = "WAM_ERROR_PAGE";
  const auto actual_value = getenv(var_name);
  if (!actual_value) {
    int result = setenv(var_name, path, false);
    ASSERT_FALSE(result);
  }
  WebAppManager::Instance()->SetPlatformModules(
      std::make_unique<PlatformModuleFactoryImpl>());

  EXPECT_CALL(*web_view_, LoadUrl(_));
  EXPECT_CALL(*web_view_, LoadUrl(::testing::HasSubstr("index.html")))
      .WillOnce(Invoke([&](const std::string& url) {
        view_url_ = url;
        if (!web_view_delegate_) {
          return;
        }
        web_view_delegate_->LoadStarted();
        web_view_delegate_->LoadProgressChanged(100.0);
        web_view_delegate_->LoadVisuallyCommitted();
        web_view_delegate_->LoadFailed("https://www.youtube.com/", 404,
                                       "SSL_ERROR");
      }));
  EXPECT_CALL(*web_view_, LoadUrl(::testing::HasSubstr("loaderror.html")))
      .WillOnce(Invoke([&](const std::string& url) {
        view_url_ = url;
        EXPECT_THAT(url, ::testing::HasSubstr(
                             "?errorCode=404&hostname=www.youtube.com"));

        if (!web_view_delegate_) {
          return;
        }
        web_view_delegate_->LoadStarted();
        web_view_delegate_->LoadProgressChanged(100.0);
        web_view_delegate_->LoadVisuallyCommitted();
        web_view_delegate_->LoadFinished(url);
      }));

  Json::Value request;
  ASSERT_TRUE(util::StringToJson(kLocaleInfo, request));

  WebAppManagerServiceLuna::Instance()->GetSystemLocalePreferencesCallback(
      request);

  request.clear();
  ASSERT_TRUE(util::StringToJson(kLaunchWebRTCAppJsonBody, request));

  const auto& result = WebAppManagerServiceLuna::Instance()->launchApp(request);
  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result.isMember("instanceId"));
  ASSERT_TRUE(result.isMember("appId"));

  if (!actual_value) {
    int result = unsetenv(var_name);
    ASSERT_FALSE(result);
  }
}
