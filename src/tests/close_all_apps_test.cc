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
#include "webos/window_group_configuration.h"

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
    "displayAffinity": 0
  },
  "reason": "com.webos.app.home"
})";

class AppTestContext {
 public:
  AppTestContext();
  ~AppTestContext() = default;

  AppTestContext(const AppTestContext&) = delete;
  AppTestContext& operator=(const AppTestContext&) = delete;

  AppTestContext(const AppTestContext&&) = delete;
  AppTestContext&& operator=(const AppTestContext&&) = delete;

  WebViewMock* GetView() { return web_view_; }
  WebAppWindowMock* GetWindow() { return web_app_window_; };

 private:
  WebViewMock* CreateView();
  WebAppWindowMock* CreateWindow();

  WebViewMock* web_view_ = nullptr;
  WebAppWindowMock* web_app_window_ = nullptr;
  WebPageBlinkDelegate* page_delegate_ = nullptr;
  WebAppWayland* web_app_ = nullptr;
  std::string url_ = "";
};

WebViewMock* AppTestContext::CreateView() {
  WebViewMock* view = new NiceWebViewMock();

  ON_CALL(*view, GetUrl()).WillByDefault(ReturnRef(url_));
  ON_CALL(*view, SetDelegate(_))
      .WillByDefault(Invoke(
          [&](WebPageBlinkDelegate* delegate) { page_delegate_ = delegate; }));
  ON_CALL(*view, LoadUrl(_)).WillByDefault(Invoke([&](const std::string& url) {
    url_ = url;
    if (!page_delegate_) {
      return;
    }
    page_delegate_->LoadStarted();
    page_delegate_->LoadProgressChanged(100.0);
    page_delegate_->LoadVisuallyCommitted();
    page_delegate_->LoadFinished(url);
  }));

  return view;
}

WebAppWindowMock* AppTestContext::CreateWindow() {
  WebAppWindowMock* window = new NiceWebAppWindowMock();
  ON_CALL(*window, SetWebApp(_))
      .WillByDefault(
          Invoke([&](WebAppWayland* web_app) { web_app_ = web_app; }));
  return window;
}

AppTestContext::AppTestContext()
    : web_view_(CreateView()), web_app_window_(CreateWindow()) {}

void AttachContext(WebAppFactoryManagerMock* web_app_factory,
                   AppTestContext* context) {
  if (!web_app_factory || !context)
    return;

  WebAppWindowFactoryMock* web_app_window_factory =
      new WebAppWindowFactoryMock();
  WebViewFactoryMock* web_view_factory = new WebViewFactoryMock();
  web_app_window_factory->SetWebAppWindow(context->GetWindow());
  web_view_factory->SetWebView(context->GetView());
  web_app_factory->SetWebViewFactory(web_view_factory);
  web_app_factory->SetWebAppWindowFactory(web_app_window_factory);
}

Json::Value LaunchApp(const char* json_body) {
  if (!json_body)
    return Json::Value();

  Json::Value request;
  if (!util::StringToJson(json_body, request))
    return Json::Value();

  return WebAppManagerServiceLuna::Instance()->launchApp(request);
}

}  // namespace

TEST(CloseAllApps, CloseAllApps) {
  WebAppManager::Instance()->SetPlatformModules(
      std::make_unique<PlatformModuleFactoryImpl>());

  WebAppFactoryManagerMock* web_app_factory_manager =
      new WebAppFactoryManagerMock();

  AppTestContext first_app;
  AppTestContext second_app;

  WebAppManager::Instance()->SetWebAppFactory(
      std::unique_ptr<WebAppFactoryManagerMock>(web_app_factory_manager));

  {
    AttachContext(web_app_factory_manager, &first_app);
    const auto& result = LaunchApp(kLaunchBareAppJsonBody);
    ASSERT_TRUE(result.isObject());
    ASSERT_TRUE(result.isMember("returnValue"));
    EXPECT_TRUE(result["returnValue"].asBool());
  }

  {
    AttachContext(web_app_factory_manager, &second_app);
    const auto& result = LaunchApp(kLaunchWebRTCAppJsonBody);
    ASSERT_TRUE(result.isObject());
    ASSERT_TRUE(result.isMember("returnValue"));
    EXPECT_TRUE(result["returnValue"].asBool());
  }

  EXPECT_EQ(WebAppManager::Instance()->List().size(), 2);
  {
    Json::Value request;
    ASSERT_TRUE(util::StringToJson("{}", request));
    const auto& result =
        WebAppManagerServiceLuna::Instance()->closeAllApps(request);
    ASSERT_TRUE(result.isObject());
    ASSERT_TRUE(result.isMember("returnValue"));
    EXPECT_TRUE(result["returnValue"].asBool());
  }

  EXPECT_FALSE(WebAppManager::Instance()->List().size());
}
