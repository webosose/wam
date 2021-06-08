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

#include "ApplicationDescription.h"
#include "PlatformModuleFactoryImpl.h"
#include "WebAppManager.h"
#include "WebPageBlink.h"
#include "WebView.h"
#include "WebViewFactory.h"
#include "WebViewMock.h"

namespace {
const std::string appDescString = R"({
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
})";

const std::string params = R"({"displayAffinity": 0, "instanceId": ""})";

class WebViewFactoryMock : public WebViewFactory {
public:
    WebViewFactoryMock();
    ~WebViewFactoryMock() override = default;

    MOCK_METHOD(WebView*, createWebView, (), (override));

    WebViewMock* webView;
};

WebViewFactoryMock::WebViewFactoryMock()
    : webView(new NiceWebViewMock())
{
}

}

class WebPageBlinkTestSuite : public ::testing::Test {
public:
    WebPageBlinkTestSuite();
    ~WebPageBlinkTestSuite() override;
    void SetUp() override;

    std::shared_ptr<ApplicationDescription> description;
    std::unique_ptr<WebViewFactoryMock> factory;
};

WebPageBlinkTestSuite::WebPageBlinkTestSuite()
{
    WebAppManager::instance()->setPlatformModules(std::make_unique<PlatformModuleFactoryImpl>());
    description = ApplicationDescription::fromJsonString(appDescString.c_str());
}

WebPageBlinkTestSuite::~WebPageBlinkTestSuite() = default;

void WebPageBlinkTestSuite::SetUp()
{
    ASSERT_TRUE(description);
    factory = std::make_unique<WebViewFactoryMock>();
    EXPECT_CALL(*factory, createWebView()).Times(1).WillRepeatedly(::testing::Return(factory->webView));
}


TEST_F(WebPageBlinkTestSuite, CheckWebViewInitializeCall)
{
    description->setDisplayAffinity(0);

    EXPECT_CALL(*factory->webView, Initialize("com.webos.app.test.webrtc0", "/usr/palm/applications/com.webos.app.test.webrtc", "default", "", "", false));

    WebPageBlink webPage(QUrl(description->entryPoint().c_str()), description, params.c_str(), std::move(factory));
    webPage.init();
}

TEST_F(WebPageBlinkTestSuite, CheckWebViewLoad)
{
    EXPECT_CALL(*factory->webView, LoadUrl("file:///usr/palm/applications/com.webos.app.test.webrtc/index.html"));

    WebPageBlink webPage(QUrl(description->entryPoint().c_str()), description, params.c_str(), std::move(factory));
    webPage.init();
    webPage.load();
}
