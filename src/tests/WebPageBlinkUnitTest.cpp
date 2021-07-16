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
#include "Utils.h"
#include "WebAppManager.h"
#include "WebPageBlink.h"
#include "WebPageBlinkDelegate.h"
#include "WebView.h"
#include "WebViewFactory.h"
#include "WebViewMock.h"

namespace {

using ::testing::_;
using ::testing::Return;
using ::testing::HasSubstr;

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
    EXPECT_CALL(*factory, createWebView()).Times(1).WillRepeatedly(Return(factory->webView));
}


TEST_F(WebPageBlinkTestSuite, CheckWebViewInitializeCall)
{
    description->setDisplayAffinity(0);

    EXPECT_CALL(*factory->webView, Initialize("com.webos.app.test.webrtc0", "/usr/palm/applications/com.webos.app.test.webrtc", "default", "", "", false));

    WebPageBlink webPage(wam::Url(description->entryPoint()), description, params.c_str(), std::move(factory));
    webPage.init();
}

TEST_F(WebPageBlinkTestSuite, CheckWebViewLoad)
{
    EXPECT_CALL(*factory->webView, LoadUrl("file:///usr/palm/applications/com.webos.app.test.webrtc/index.html"));

    WebPageBlink webPage(wam::Url(description->entryPoint()), description, params.c_str(), std::move(factory));
    webPage.init();
    webPage.load();
}


TEST_F(WebPageBlinkTestSuite, AddCustomPluginDir)
{
    //TODO: Rework this test. Avoid to create some folders on real FS
    constexpr char path[] = "/usr/palm/applications/com.webos.app.test.webrtc/plugins";
    int result = mkdir(path, 0777);
    ASSERT_FALSE(result && result == EEXIST);

    EXPECT_CALL(*factory->webView, AddCustomPluginDir(path));
    EXPECT_CALL(*factory->webView, AddAvailablePluginDir(path));

    WebPageBlink webPage(wam::Url(description->entryPoint()), description, params.c_str(), std::move(factory));
    webPage.init();

    result = remove(path);
    ASSERT_FALSE(result);
}

TEST_F(WebPageBlinkTestSuite, PriviledgetPluginPath)
{
    constexpr char path[] = "/usr/palm/applications/com.webos.app.test.webrtc/";
    constexpr char varName[] = "PRIVILEGED_PLUGIN_PATH";
    auto actualValue = getenv(varName);
    std::string testValue(path);
    if (!actualValue) {
        int result = setenv(varName, path, false);
        ASSERT_FALSE(result);
    } else {
        testValue = actualValue;
    }

    EXPECT_CALL(*factory->webView, AddAvailablePluginDir(testValue));

    WebPageBlink webPage(wam::Url(description->entryPoint()), description, params.c_str(), std::move(factory));
    webPage.init();

    if (!actualValue) {
        int result = unsetenv(varName);
        ASSERT_FALSE(result);
    }
}

TEST_F(WebPageBlinkTestSuite, SetMediaCodecCapability)
{
    // TODO: JSON file path hardcoding should be refactored.
    // Actually this path is hardcoded inside of 
    // WebPageBlink::updateMediaCodecCapability() method.
    // That's a reason why it's also hardcoded here.
    bool skip = false;
    if (util::doesPathExist("/etc/umediaserver/device_codec_capability_config.json")) {
        EXPECT_CALL(*factory->webView, SetMediaCodecCapability(HasSubstr("LICENSE  Copyright (c) 2018-2019 LG Electronics, Inc")));
    } else {
        skip = true;
    }

    WebPageBlink webPage(wam::Url(description->entryPoint()), description, params.c_str(), std::move(factory));
    webPage.init();

    if (skip)
        GTEST_SKIP();
}

TEST_F(WebPageBlinkTestSuite, addUserScript)
{
    constexpr char path[] = "/usr/palm/tellurium/telluriumnub.js";
    constexpr char varName[] = "TELLURIUM_NUB_PATH";
    auto actualValue = getenv(varName);
    if (!actualValue) {
        int result = setenv(varName, path, false);
        ASSERT_FALSE(result);
    }

    WebAppManager::instance()->setPlatformModules(std::make_unique<PlatformModuleFactoryImpl>());
    ASSERT_TRUE(WebAppManager::instance()->config()->isDevModeEnabled()) << "Devmode should be enabled for this test";

    EXPECT_CALL(*factory->webView, addUserScript(HasSubstr("@class Telluriumnub")));
    WebPageBlink webPage(wam::Url(description->entryPoint()), description, params.c_str(), std::move(factory));
    webPage.init();

    if (!actualValue) {
        int result = unsetenv(varName);
        ASSERT_FALSE(result);
    }
}
