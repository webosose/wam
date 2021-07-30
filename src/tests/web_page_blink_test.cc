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

#include "application_description.h"
#include "platform_module_factory_impl.h"
#include "utils.h"
#include "web_app_manager.h"
#include "web_page_blink.h"
#include "web_page_blink_delegate.h"
#include "web_view.h"
#include "web_view_factory.h"
#include "web_view_mock.h"

namespace {

using ::testing::_;
using ::testing::HasSubstr;
using ::testing::Return;

const std::string kAppDescString = R"({
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
  "customPlugin": true,
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
})";

const std::string params = R"({"displayAffinity": 0, "instanceId": ""})";

class WebViewFactoryMock : public WebViewFactory {
 public:
  WebViewFactoryMock();
  ~WebViewFactoryMock() override = default;

  MOCK_METHOD(WebView*, CreateWebView, (), (override));

  WebViewMock* web_view_;
};

WebViewFactoryMock::WebViewFactoryMock() : web_view_(new NiceWebViewMock()) {}

}  // namespace

class WebPageBlinkTestSuite : public ::testing::Test {
 public:
  WebPageBlinkTestSuite();
  ~WebPageBlinkTestSuite() override;
  void SetUp() override;

  std::shared_ptr<ApplicationDescription> description;
  std::unique_ptr<WebViewFactoryMock> factory;
};

WebPageBlinkTestSuite::WebPageBlinkTestSuite() {
  WebAppManager::Instance()->SetPlatformModules(
      std::make_unique<PlatformModuleFactoryImpl>());
  description = ApplicationDescription::FromJsonString(kAppDescString.c_str());
}

WebPageBlinkTestSuite::~WebPageBlinkTestSuite() = default;

void WebPageBlinkTestSuite::SetUp() {
  ASSERT_TRUE(description);
  factory = std::make_unique<WebViewFactoryMock>();
  EXPECT_CALL(*factory, CreateWebView())
      .Times(1)
      .WillRepeatedly(Return(factory->web_view_));
}

TEST_F(WebPageBlinkTestSuite, CheckWebViewInitializeCall) {
  description->SetDisplayAffinity(0);

  EXPECT_CALL(*factory->web_view_,
              Initialize("com.webos.app.test.webrtc0",
                         "/usr/palm/applications/com.webos.app.test.webrtc",
                         "default", "", "", false));

  WebPageBlink web_page(wam::Url(description->EntryPoint()), description,
                        params.c_str(), std::move(factory));
  web_page.Init();
}

TEST_F(WebPageBlinkTestSuite, CheckWebViewLoad) {
  EXPECT_CALL(*factory->web_view_,
              LoadUrl("file:///usr/palm/applications/com.webos.app.test.webrtc/"
                      "index.html"));

  WebPageBlink web_page(wam::Url(description->EntryPoint()), description,
                        params.c_str(), std::move(factory));
  web_page.Init();
  web_page.Load();
}

TEST_F(WebPageBlinkTestSuite, AddCustomPluginDir) {
  // TODO: Rework this test. Avoid to create some folders on real FS
  constexpr char path[] =
      "/usr/palm/applications/com.webos.app.test.webrtc/plugins";
  int result = mkdir(path, 0777);
  ASSERT_FALSE(result && result == EEXIST);

  EXPECT_CALL(*factory->web_view_, AddCustomPluginDir(path));
  EXPECT_CALL(*factory->web_view_, AddAvailablePluginDir(path));

  WebPageBlink web_page(wam::Url(description->EntryPoint()), description,
                        params.c_str(), std::move(factory));
  web_page.Init();

  result = remove(path);
  ASSERT_FALSE(result);
}

TEST_F(WebPageBlinkTestSuite, PriviledgetPluginPath) {
  constexpr char path[] = "/usr/palm/applications/com.webos.app.test.webrtc/";
  constexpr char varName[] = "PRIVILEGED_PLUGIN_PATH";
  auto actual_value = getenv(varName);
  std::string test_value(path);
  if (!actual_value) {
    int result = setenv(varName, path, false);
    ASSERT_FALSE(result);
  } else {
    test_value = actual_value;
  }

  EXPECT_CALL(*factory->web_view_, AddAvailablePluginDir(test_value));

  WebPageBlink webPage(wam::Url(description->EntryPoint()), description,
                       params.c_str(), std::move(factory));
  webPage.Init();

  if (!actual_value) {
    int result = unsetenv(varName);
    ASSERT_FALSE(result);
  }
}

TEST_F(WebPageBlinkTestSuite, SetMediaCodecCapability) {
  // TODO: JSON file path hardcoding should be refactored.
  // Actually this path is hardcoded inside of
  // WebPageBlink::updateMediaCodecCapability() method.
  // That's a reason why it's also hardcoded here.
  bool skip = false;
  if (util::DoesPathExist(
          "/etc/umediaserver/device_codec_capability_config.json")) {
    EXPECT_CALL(*factory->web_view_,
                SetMediaCodecCapability(HasSubstr(
                    "LICENSE  Copyright (c) 2018-2019 LG Electronics, Inc")));
  } else {
    skip = true;
  }

  WebPageBlink webPage(wam::Url(description->EntryPoint()), description,
                       params.c_str(), std::move(factory));
  webPage.Init();

  if (skip)
    GTEST_SKIP();
}

TEST_F(WebPageBlinkTestSuite, addUserScript) {
  constexpr char path[] = "/usr/palm/tellurium/telluriumnub.js";
  constexpr char var_name[] = "TELLURIUM_NUB_PATH";
  auto actual_value = getenv(var_name);
  if (!actual_value) {
    int result = setenv(var_name, path, false);
    ASSERT_FALSE(result);
  }

  WebAppManager::Instance()->SetPlatformModules(
      std::make_unique<PlatformModuleFactoryImpl>());
  ASSERT_TRUE(WebAppManager::Instance()->Config()->IsDevModeEnabled())
      << "Devmode should be enabled for this test";

  EXPECT_CALL(*factory->web_view_,
              AddUserScript(HasSubstr("@class Telluriumnub")));
  WebPageBlink web_page(wam::Url(description->EntryPoint()), description,
                        params.c_str(), std::move(factory));
  web_page.Init();

  if (!actual_value) {
    int result = unsetenv(var_name);
    ASSERT_FALSE(result);
  }
}
