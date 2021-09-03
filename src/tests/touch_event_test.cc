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

}  // namespace

class TouchEventTestSuite : public ::testing::Test {
 public:
  TouchEventTestSuite() = default;
  ~TouchEventTestSuite() override = default;

  void SetUp() override;
  void TearDown() override;

  WebAppWindowFactoryMock* web_app_window_factory_ = nullptr;
  WebAppWindowMock* web_app_window_ = nullptr;

  WebViewFactoryMock* web_view_factory_ = nullptr;
  WebViewMock* web_view_ = nullptr;

  WebAppWayland* web_app_ = nullptr;
  WebPageBlinkDelegate* web_view_delegate_ = nullptr;
  std::string view_url_;
};

void TouchEventTestSuite::SetUp() {
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
      .WillByDefault(Invoke([&](WebAppWayland* WebApp) { web_app_ = WebApp; }));

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

  Json::Value launch_request;
  ASSERT_TRUE(util::StringToJson(launchBareAppJsonBody, launch_request));
  const auto& result =
      WebAppManagerServiceLuna::Instance()->launchApp(launch_request);

  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result.isMember("instanceId"));
  ASSERT_TRUE(result.isMember("appId"));
  EXPECT_TRUE(result["returnValue"].asBool());
}

void TouchEventTestSuite::TearDown() {
  WebAppManager::Instance()->CloseAllApps();
}

TEST_F(TouchEventTestSuite, EnterEventTest) {
  EXPECT_CALL(*web_view_, RunJavaScript(::testing::HasSubstr(
                              "var mouseEvent =new CustomEvent('webOSMouse', { "
                              "detail: { type : 'Enter' }});")));

  web_app_->SendWebOSMouseEvent("Enter");
}

TEST_F(TouchEventTestSuite, LeaveEventTest) {
  EXPECT_CALL(*web_view_, RunJavaScript(::testing::HasSubstr(
                              "var mouseEvent =new CustomEvent('webOSMouse', { "
                              "detail: { type : 'Leave' }});")));

  web_app_->SendWebOSMouseEvent("Leave");
}
