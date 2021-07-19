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

} // namespace

class TouchEventTestSuite : public ::testing::Test {
public:
    TouchEventTestSuite() = default;
    ~TouchEventTestSuite() override = default;

    void SetUp() override;
    void TearDown() override;

    WebAppWindowFactoryMock* webAppWindowFactory = nullptr;
    WebAppWindowMock* webAppWindow = nullptr;

    WebViewFactoryMock* webViewFactory = nullptr;
    WebViewMock* webView = nullptr;

    WebAppWayland* webApp = nullptr;
    WebPageBlinkDelegate* webViewDelegate = nullptr;
    std::string viewUrl;

};

void TouchEventTestSuite::SetUp()
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

    ON_CALL(*webAppWindow, setWebApp(_)).WillByDefault(Invoke([&](WebAppWayland* WebApp) {
        webApp = WebApp;
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

    Json::Value launch_request;
    ASSERT_TRUE(util::JsonValueFromString(launchBareAppJsonBody, launch_request));
    const auto& result = WebAppManagerServiceLuna::instance()->launchApp(launch_request);

    ASSERT_TRUE(result.isObject());
    ASSERT_TRUE(result.isMember("returnValue"));
    ASSERT_TRUE(result.isMember("instanceId"));
    ASSERT_TRUE(result.isMember("appId"));
    EXPECT_TRUE(result["returnValue"].asBool());
}

void TouchEventTestSuite::TearDown()
{
    WebAppManager::instance()->closeAllApps();
}

TEST_F(TouchEventTestSuite, EnterEventTest)
{
    EXPECT_CALL(*webView, RunJavaScript(::testing::HasSubstr("var mouseEvent =new CustomEvent('webOSMouse', { detail: { type : 'Enter' }});")));

    webApp->sendWebOSMouseEvent("Enter");

}

TEST_F(TouchEventTestSuite, LeaveEventTest)
{
    EXPECT_CALL(*webView, RunJavaScript(::testing::HasSubstr("var mouseEvent =new CustomEvent('webOSMouse', { detail: { type : 'Leave' }});")));

    webApp->sendWebOSMouseEvent("Leave");
}

