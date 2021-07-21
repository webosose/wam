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

#include "PlatformModuleFactoryImpl.h"
#include "Utils.h"
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

    WebViewMock* getView() { return m_webView; }
    WebAppWindowMock* getWindow() { return m_webAppWindow; };

private:
    WebViewMock* createView();
    WebAppWindowMock* createWindow();

    WebViewMock* m_webView = nullptr;
    WebAppWindowMock* m_webAppWindow = nullptr;
    WebPageBlinkDelegate* m_pageDelegate = nullptr;
    WebAppWayland* m_webApp = nullptr;
    std::string m_url = "";
};

WebViewMock* AppTestContext::createView()
{
    WebViewMock* view = new NiceWebViewMock();

    ON_CALL(*view, GetUrl()).WillByDefault(ReturnRef(m_url));
    ON_CALL(*view, setDelegate(_)).WillByDefault(Invoke([&](WebPageBlinkDelegate* delegate) {
        m_pageDelegate = delegate;
    }));
    ON_CALL(*view, LoadUrl(_)).WillByDefault(Invoke([&](const std::string& url) {
        m_url = url;
        if (!m_pageDelegate) {
            return;
        }
        m_pageDelegate->loadStarted();
        m_pageDelegate->loadProgressChanged(100.0);
        m_pageDelegate->loadVisuallyCommitted();
        m_pageDelegate->loadFinished(url);
    }));

    return view;
}

WebAppWindowMock* AppTestContext::createWindow()
{
    WebAppWindowMock* window = new NiceWebAppWindowMock();
    ON_CALL(*window, setWebApp(_)).WillByDefault(Invoke([&](WebAppWayland* webApp) {
        m_webApp = webApp;
    }));
    return window;
}

AppTestContext::AppTestContext()
    : m_webView(createView())
    , m_webAppWindow(createWindow())
{
}

void AttachContext(WebAppFactoryManagerMock* webAppFactory, AppTestContext* context)
{
    if (!webAppFactory || !context)
        return;

    WebAppWindowFactoryMock* webAppWindowFactory = new WebAppWindowFactoryMock();
    WebViewFactoryMock* webViewFactory = new WebViewFactoryMock();
    webAppWindowFactory->setWebAppWindow(context->getWindow());
    webViewFactory->setWebView(context->getView());
    webAppFactory->setWebViewFactory(webViewFactory);
    webAppFactory->setWebAppWindowFactory(webAppWindowFactory);
}

Json::Value launchApp(const char* jsonBody)
{
    if (!jsonBody)
        return Json::Value();

    Json::Value request;
    if (!util::stringToJson(jsonBody, request))
        return Json::Value();

    return WebAppManagerServiceLuna::instance()->launchApp(request);
}

} // namespace

TEST(CloseAllApps, CloseAllApps)
{
    WebAppManager::instance()->setPlatformModules(std::make_unique<PlatformModuleFactoryImpl>());

    WebAppFactoryManagerMock* webAppFactoryManager = new WebAppFactoryManagerMock();

    AppTestContext firstApp;
    AppTestContext secondApp;

    WebAppManager::instance()->setWebAppFactory(std::unique_ptr<WebAppFactoryManagerMock>(webAppFactoryManager));

    {
        AttachContext(webAppFactoryManager, &firstApp);
        const auto& result = launchApp(launchBareAppJsonBody);
        ASSERT_TRUE(result.isObject());
        ASSERT_TRUE(result.isMember("returnValue"));
        EXPECT_TRUE(result["returnValue"].asBool());
    }

    {
        AttachContext(webAppFactoryManager, &secondApp);
        const auto& result = launchApp(launchWebRTCAppJsonBody);
        ASSERT_TRUE(result.isObject());
        ASSERT_TRUE(result.isMember("returnValue"));
        EXPECT_TRUE(result["returnValue"].asBool());
    }

    EXPECT_EQ(WebAppManager::instance()->list().size(), 2);

    {
        Json::Value request;
        ASSERT_TRUE(util::stringToJson("{}", request));
        const auto& result = WebAppManagerServiceLuna::instance()->closeAllApps(request);
        ASSERT_TRUE(result.isObject());
        ASSERT_TRUE(result.isMember("returnValue"));
        EXPECT_TRUE(result["returnValue"].asBool());
    }

    EXPECT_FALSE(WebAppManager::instance()->list().size());
}
