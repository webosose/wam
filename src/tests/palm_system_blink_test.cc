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
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/json.h>

#include "platform_module_factory_impl.h"
#include "utils.h"
#include "web_app_factory_manager_mock.h"
#include "web_app_manager.h"
#include "web_app_manager_service_luna.h"
#include "web_app_wayland.h"
#include "web_app_window_factory_mock.h"
#include "web_app_window_mock.h"
#include "web_page_blink_delegate.h"
#include "web_view_factory_mock.h"
#include "web_view_mock.h"
#include "webos/window_group_configuration.h"

namespace {

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Eq;
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
    "bgImage": "",
    "windowGroup":{
            "name":"Window group name",
            "owner":false,
            "ownerInfo":{
                "allowAnonymous":false,
                "layers":[
                    {"name":"Owner layer name", "z":111}
                ]
            },
            "clientInfo":{
                "layer":"Client layer name",
                "hint":"Client layer hint"
            }
        }
  },
  "appId": "bareapp",
  "parameters": {
    "displayAffinity": 0
  },
  "reason": "com.webos.app.home",
  "launchingProcId": "",
  "instanceId": "188f99b7-1e1a-489f-8e3d-56844a7713030"
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

class PalmSystemBlinkTestSuite : public ::testing::Test {
 public:
  PalmSystemBlinkTestSuite() = default;
  ~PalmSystemBlinkTestSuite() override = default;

  void SetUp() override;
  void TearDown() override;

  WebAppWindowFactoryMock* web_app_window_factory_ = nullptr;
  WebAppWindowMock* web_app_window_ = nullptr;
  WebViewFactoryMock* web_view_factory_ = nullptr;
  WebViewMock* web_view_ = nullptr;
  WebPageBlinkDelegate* web_view_delegate_ = nullptr;
  WebAppWayland* web_app_ = nullptr;
  std::string view_url_;
};

void PalmSystemBlinkTestSuite::SetUp() {
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

  ON_CALL(*web_app_window_, SetWebApp(_))
      .WillByDefault(Invoke([&](WebAppWayland* app) { web_app_ = app; }));

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

  Json::Value reuest;
  ASSERT_TRUE(util::StringToJson(kLocaleInfo, reuest));
  WebAppManagerServiceLuna::Instance()->GetSystemLocalePreferencesCallback(
      reuest);

  reuest.clear();
  ASSERT_TRUE(util::StringToJson(kLaunchBareAppJsonBody, reuest));

  const auto& result = WebAppManagerServiceLuna::Instance()->launchApp(reuest);

  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("instanceId"));
  ASSERT_TRUE(web_view_delegate_);
  ASSERT_TRUE(web_app_);
}

void PalmSystemBlinkTestSuite::TearDown() {
  WebAppManager::Instance()->CloseAllApps();
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_initialize) {
  std::string return_value;
  web_view_delegate_->HandleBrowserControlFunction(
      "initialize", std::vector<std::string>(), &return_value);

  Json::Value init_value;
  ASSERT_TRUE(util::StringToJson(return_value, init_value));

  ASSERT_TRUE(init_value.isObject());
  ASSERT_TRUE(init_value.isMember("folderPath"));
  EXPECT_STREQ(init_value["folderPath"].asString().c_str(),
               "/usr/palm/applications/bareapp");
  ASSERT_TRUE(init_value.isMember("identifier"));
  EXPECT_STREQ(init_value["identifier"].asString().c_str(), "bareapp");
  ASSERT_TRUE(init_value.isMember("trustLevel"));
  EXPECT_STREQ(init_value["trustLevel"].asString().c_str(), "default");
  ASSERT_TRUE(init_value.isMember("launchParams"));
  EXPECT_TRUE(init_value["launchParams"].asString().find("displayAffinity") !=
              std::string::npos);
  ASSERT_TRUE(init_value.isMember("locale"));
  EXPECT_TRUE(init_value["locale"].asString().find("en-US") !=
              std::string::npos);
}

TEST_F(PalmSystemBlinkTestSuite,
       handleBrowserControlMessage_isKeyboardVisible) {
  EXPECT_CALL(*web_app_window_, IsKeyboardVisible()).WillOnce(Return(false));

  std::string return_value;
  web_view_delegate_->HandleBrowserControlFunction(
      "isKeyboardVisible", std::vector<std::string>(), &return_value);

  EXPECT_STREQ(return_value.c_str(), "false");
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_getIdentifier) {
  std::string return_value;
  web_view_delegate_->HandleBrowserControlFunction(
      "getIdentifier", std::vector<std::string>(), &return_value);

  EXPECT_STREQ(return_value.c_str(), "bareapp");

  web_view_delegate_->HandleBrowserControlFunction(
      "identifier", std::vector<std::string>(), &return_value);

  EXPECT_STREQ(return_value.c_str(), "bareapp");
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_isActivated) {
  std::string return_value;
  web_app_->Unfocus();
  web_view_delegate_->HandleBrowserControlFunction(
      "isActivated", std::vector<std::string>(), &return_value);

  EXPECT_STREQ(return_value.c_str(), "false");
}

TEST_F(PalmSystemBlinkTestSuite,
       handleBrowserControlMessage_setWindowProperty) {
  std::string return_value;
  std::vector<std::string> params;
  params.reserve(2);
  params.emplace_back("TestProperty");
  params.emplace_back("TestValue");

  EXPECT_CALL(*web_app_window_,
              SetWindowProperty(StrEq("TestProperty"), StrEq("TestValue")));

  web_view_delegate_->HandleBrowserControlFunction("setWindowProperty", params,
                                                   &return_value);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setCursor) {
  std::string return_value;
  std::vector<std::string> params;
  params.reserve(3);
  params.emplace_back("Cursor");
  params.emplace_back(std::to_string(1));
  params.emplace_back(std::to_string(2));

  EXPECT_CALL(*web_app_window_, SetCursor(std::string("Cursor"), 1, 2));

  web_view_delegate_->HandleBrowserControlFunction("setCursor", params,
                                                   &return_value);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_platformBack) {
  std::string return_value;
  EXPECT_CALL(*web_app_window_, PlatformBack());
  web_view_delegate_->HandleBrowserControlFunction(
      "platformBack", std::vector<std::string>(), &return_value);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Home) {
  std::string return_value;
  std::vector<std::string> params;
  params.reserve(1);
  params.emplace_back(R"(["KeyMaskHome"])");

  EXPECT_CALL(*web_app_window_, SetKeyMask(webos::WebOSKeyMask::KEY_MASK_HOME));
  web_view_delegate_->HandleBrowserControlFunction("setKeyMask", params,
                                                   &return_value);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Back) {
  std::string return_value;
  std::vector<std::string> params;
  params.reserve(1);
  params.emplace_back(R"(["KeyMaskBack"])");

  EXPECT_CALL(*web_app_window_, SetKeyMask(webos::WebOSKeyMask::KEY_MASK_BACK));
  web_view_delegate_->HandleBrowserControlFunction("setKeyMask", params,
                                                   &return_value);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Exit) {
  std::string return_value;
  std::vector<std::string> params;
  params.reserve(1);
  params.emplace_back(R"(["KeyMaskExit"])");

  EXPECT_CALL(*web_app_window_, SetKeyMask(webos::WebOSKeyMask::KEY_MASK_EXIT));
  web_view_delegate_->HandleBrowserControlFunction("setKeyMask", params,
                                                   &return_value);
}

TEST_F(PalmSystemBlinkTestSuite,
       handleBrowserControlMessage_setKeyMask_Incorrect) {
  std::string return_value;
  std::vector<std::string> params;
  params.reserve(1);
  params.emplace_back(R"(["Incorrect value"])");

  EXPECT_CALL(*web_app_window_,
              SetKeyMask(static_cast<webos::WebOSKeyMask>(0)));
  web_view_delegate_->HandleBrowserControlFunction("setKeyMask", params,
                                                   &return_value);
}

TEST_F(PalmSystemBlinkTestSuite,
       handleBrowserControlMessage_setKeyMask_Combination) {
  std::string return_value;
  std::vector<std::string> params;
  params.reserve(1);
  params.emplace_back(R"(["KeyMaskExit", "KeyMaskBack"])");

  int keymask = 0;
  keymask |= webos::WebOSKeyMask::KEY_MASK_EXIT;
  keymask |= webos::WebOSKeyMask::KEY_MASK_BACK;

  EXPECT_CALL(*web_app_window_,
              SetKeyMask(static_cast<webos::WebOSKeyMask>(keymask)));
  web_view_delegate_->HandleBrowserControlFunction("setKeyMask", params,
                                                   &return_value);
}

// NOTE: just ensure that we will not crash on this call
TEST_F(PalmSystemBlinkTestSuite,
       handleBrowserControlMessage_PmLogInfoWithClock) {
  std::string return_value;
  std::vector<std::string> params;
  params.reserve(3);
  params.emplace_back("");
  params.emplace_back("");
  params.emplace_back("");

  web_view_delegate_->HandleBrowserControlFunction("PmLogInfoWithClock", params,
                                                   &return_value);
}

// NOTE: just ensure that we will not crash on this call
TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_PmLogString) {
  std::string return_value;
  std::vector<std::string> params;
  params.reserve(4);
  params.emplace_back(std::to_string(7));
  params.emplace_back("");
  params.emplace_back("");
  params.emplace_back("");

  web_view_delegate_->HandleBrowserControlFunction("PmLogString", params,
                                                   &return_value);
}
