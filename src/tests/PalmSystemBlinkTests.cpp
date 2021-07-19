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

#include "JsonHelper.h"
#include "PlatformModuleFactoryImpl.h"
#include "WebAppFactoryManagerMock.h"
#include "WebAppManager.h"
#include "WebAppManagerServiceLuna.h"
#include "WebAppWayland.h"
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
using ::testing::Eq;

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

class PalmSystemBlinkTestSuite : public ::testing::Test {
public:
    PalmSystemBlinkTestSuite() = default;
    ~PalmSystemBlinkTestSuite() override = default;

    void SetUp() override;
    void TearDown() override;

    WebAppWindowFactoryMock* webAppWindowFactory = nullptr;
    WebAppWindowMock* webAppWindow = nullptr;
    WebViewFactoryMock* webViewFactory = nullptr;
    WebViewMock* webView = nullptr;
    WebPageBlinkDelegate* webViewDelegate = nullptr;
    WebAppWayland* webApp = nullptr;
    std::string viewUrl;
};

void PalmSystemBlinkTestSuite::SetUp()
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

    ON_CALL(*webAppWindow, setWebApp(_)).WillByDefault(Invoke([&](WebAppWayland* app) {
        webApp = app;
    }));

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

    Json::Value reuest;
    ASSERT_TRUE(util::JsonValueFromString(localeInfo, reuest));
    WebAppManagerServiceLuna::instance()->getSystemLocalePreferencesCallback(reuest);

    reuest.clear();
    ASSERT_TRUE(util::JsonValueFromString(launchBareAppJsonBody, reuest));
    const auto& result = WebAppManagerServiceLuna::instance()->launchApp(reuest);

    ASSERT_TRUE(result.isObject());
    ASSERT_TRUE(result.isMember("instanceId"));
    ASSERT_TRUE(webViewDelegate);
    ASSERT_TRUE(webApp);
}

void PalmSystemBlinkTestSuite::TearDown()
{
    WebAppManager::instance()->closeAllApps();
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_initialize)
{
    std::string returnValue;
    webViewDelegate->handleBrowserControlFunction("initialize", std::vector<std::string>(), &returnValue);

    Json::Value initValue;
    ASSERT_TRUE(util::JsonValueFromString(returnValue, initValue));

    ASSERT_TRUE(initValue.isObject());
    ASSERT_TRUE(initValue.isMember("folderPath"));
    EXPECT_STREQ(initValue["folderPath"].asString().c_str(), "/usr/palm/applications/bareapp");
    ASSERT_TRUE(initValue.isMember("identifier"));
    EXPECT_STREQ(initValue["identifier"].asString().c_str(), "bareapp");
    ASSERT_TRUE(initValue.isMember("trustLevel"));
    EXPECT_STREQ(initValue["trustLevel"].asString().c_str(), "default");
    ASSERT_TRUE(initValue.isMember("launchParams"));
    EXPECT_TRUE(initValue["launchParams"].asString().find("displayAffinity") != std::string::npos);
    ASSERT_TRUE(initValue.isMember("locale"));
    EXPECT_TRUE(initValue["locale"].asString().find("en-US") != std::string::npos);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_isKeyboardVisible)
{
    EXPECT_CALL(*webAppWindow, IsKeyboardVisible()).WillOnce(Return(false));

    std::string returnValue;
    webViewDelegate->handleBrowserControlFunction("isKeyboardVisible", std::vector<std::string>(), &returnValue);

    EXPECT_STREQ(returnValue.c_str(), "false");
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_getIdentifier)
{
    std::string returnValue;
    webViewDelegate->handleBrowserControlFunction("getIdentifier", std::vector<std::string>(), &returnValue);

    EXPECT_STREQ(returnValue.c_str(), "bareapp");

    webViewDelegate->handleBrowserControlFunction("identifier", std::vector<std::string>(), &returnValue);

    EXPECT_STREQ(returnValue.c_str(), "bareapp");
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_isActivated)
{
    std::string returnValue;
    webApp->unfocus();
    webViewDelegate->handleBrowserControlFunction("isActivated", std::vector<std::string>(), &returnValue);

    EXPECT_STREQ(returnValue.c_str(), "false");
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setWindowProperty)
{
    std::string returnValue;
    std::vector<std::string> params;
    params.reserve(2);
    params.emplace_back("TestProperty");
    params.emplace_back("TestValue");

    EXPECT_CALL(*webAppWindow, SetWindowProperty(StrEq("TestProperty"), StrEq("TestValue")));

    webViewDelegate->handleBrowserControlFunction("setWindowProperty", params, &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setCursor)
{
    std::string returnValue;
    std::vector<std::string> params;
    params.reserve(3);
    params.emplace_back("Cursor");
    params.emplace_back(std::to_string(1));
    params.emplace_back(std::to_string(2));

    EXPECT_CALL(*webAppWindow, setCursor(std::string("Cursor"), 1, 2));

    webViewDelegate->handleBrowserControlFunction("setCursor", params, &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_platformBack)
{
    std::string returnValue;
    EXPECT_CALL(*webAppWindow, platformBack());
    webViewDelegate->handleBrowserControlFunction("platformBack", std::vector<std::string>(), &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Home)
{
    std::string returnValue;
    std::vector<std::string> params;
    params.reserve(1);
    params.emplace_back(R"(["KeyMaskHome"])");

    EXPECT_CALL(*webAppWindow, SetKeyMask(webos::WebOSKeyMask::KEY_MASK_HOME));
    webViewDelegate->handleBrowserControlFunction("setKeyMask", params, &returnValue);
}


TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Back)
{
    std::string returnValue;
    std::vector<std::string> params;
    params.reserve(1);
    params.emplace_back(R"(["KeyMaskBack"])");

    EXPECT_CALL(*webAppWindow, SetKeyMask(webos::WebOSKeyMask::KEY_MASK_BACK));
    webViewDelegate->handleBrowserControlFunction("setKeyMask", params, &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Exit)
{
    std::string returnValue;
    std::vector<std::string> params;
    params.reserve(1);
    params.emplace_back(R"(["KeyMaskExit"])");

    EXPECT_CALL(*webAppWindow, SetKeyMask(webos::WebOSKeyMask::KEY_MASK_EXIT));
    webViewDelegate->handleBrowserControlFunction("setKeyMask", params, &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Incorrect)
{
    std::string returnValue;
    std::vector<std::string> params;
    params.reserve(1);
    params.emplace_back(R"(["Incorrect value"])");

    EXPECT_CALL(*webAppWindow, SetKeyMask(static_cast<webos::WebOSKeyMask>(0)));
    webViewDelegate->handleBrowserControlFunction("setKeyMask", params, &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Combination)
{
    std::string returnValue;
    std::vector<std::string> params;
    params.reserve(1);
    params.emplace_back(R"(["KeyMaskExit", "KeyMaskBack"])");

    int keymask = 0;
    keymask |= webos::WebOSKeyMask::KEY_MASK_EXIT;
    keymask |= webos::WebOSKeyMask::KEY_MASK_BACK;

    EXPECT_CALL(*webAppWindow, SetKeyMask(static_cast<webos::WebOSKeyMask>(keymask)));
    webViewDelegate->handleBrowserControlFunction("setKeyMask", params, &returnValue);
}

// NOTE: just ensure that we will not crash on this call
TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_PmLogInfoWithClock)
{
    std::string returnValue;
    std::vector<std::string> params;
    params.reserve(3);
    params.emplace_back("");
    params.emplace_back("");
    params.emplace_back("");

    webViewDelegate->handleBrowserControlFunction("PmLogInfoWithClock", params, &returnValue);
}

// NOTE: just ensure that we will not crash on this call
TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_PmLogString)
{
    std::string returnValue;
    std::vector<std::string> params;
    params.reserve(4);
    params.emplace_back(std::to_string(7));
    params.emplace_back("");
    params.emplace_back("");
    params.emplace_back("");

    webViewDelegate->handleBrowserControlFunction("PmLogString", params, &returnValue);
}
