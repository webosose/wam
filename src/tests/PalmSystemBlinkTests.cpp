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

#include <QJsonDocument>
#include <QJsonObject>

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

    QJsonParseError parseError;

    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(localeInfo).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);
    WebAppManagerServiceLuna::instance()->getSystemLocalePreferencesCallback(doc.object());

    doc = QJsonDocument::fromJson(QString::fromUtf8(launchBareAppJsonBody).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);
    const auto& result = WebAppManagerServiceLuna::instance()->launchApp(doc.object());

    ASSERT_TRUE(result.contains("instanceId"));
    ASSERT_TRUE(webViewDelegate);
    ASSERT_TRUE(webApp);
}

void PalmSystemBlinkTestSuite::TearDown()
{
    WebAppManager::instance()->closeAllApps();
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_initialize)
{
    QString returnValue;
    webViewDelegate->handleBrowserControlFunction(QString("initialize"), QStringList(), &returnValue);

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(returnValue.toUtf8(), &parseError);
    QJsonObject initValue = doc.object();
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    ASSERT_TRUE(initValue.contains("folderPath"));
    EXPECT_STREQ(initValue["folderPath"].toString().toStdString().c_str(), "/usr/palm/applications/bareapp");
    ASSERT_TRUE(initValue.contains("identifier"));
    EXPECT_STREQ(initValue["identifier"].toString().toStdString().c_str(), "bareapp");
    ASSERT_TRUE(initValue.contains("trustLevel"));
    EXPECT_STREQ(initValue["trustLevel"].toString().toStdString().c_str(), "default");
    ASSERT_TRUE(initValue.contains("launchParams"));
    EXPECT_TRUE(initValue["launchParams"].toString().contains("displayAffinity"));
    ASSERT_TRUE(initValue.contains("locale"));
    EXPECT_TRUE(initValue["locale"].toString().contains("en-US"));
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_isKeyboardVisible)
{
    EXPECT_CALL(*webAppWindow, IsKeyboardVisible()).WillOnce(Return(false));

    QString returnValue;
    webViewDelegate->handleBrowserControlFunction(QString("isKeyboardVisible"), QStringList(), &returnValue);

    EXPECT_STREQ(returnValue.toStdString().c_str(), "false");
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_getIdentifier)
{
    QString returnValue;
    webViewDelegate->handleBrowserControlFunction(QString("getIdentifier"), QStringList(), &returnValue);

    EXPECT_STREQ(returnValue.toStdString().c_str(), "bareapp");

    webViewDelegate->handleBrowserControlFunction(QString("identifier"), QStringList(), &returnValue);

    EXPECT_STREQ(returnValue.toStdString().c_str(), "bareapp");
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_isActivated)
{
    QString returnValue;
    webApp->unfocus();
    webViewDelegate->handleBrowserControlFunction(QString("isActivated"), QStringList(), &returnValue);

    EXPECT_STREQ(returnValue.toStdString().c_str(), "false");
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setWindowProperty)
{
    QString returnValue;
    QStringList params;
    params << QString("TestProperty");
    params << QString("TestValue");

    EXPECT_CALL(*webAppWindow, SetWindowProperty(StrEq("TestProperty"), StrEq("TestValue")));

    webViewDelegate->handleBrowserControlFunction(QString("setWindowProperty"), params, &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setCursor)
{
    QString returnValue;
    QStringList params;
    params << QString("Cursor");
    params << QString::number(1);
    params << QString::number(2);

    EXPECT_CALL(*webAppWindow, setCursor(Eq(QString("Cursor")), 1, 2));

    webViewDelegate->handleBrowserControlFunction(QString("setCursor"), params, &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_platformBack)
{
    QString returnValue;
    EXPECT_CALL(*webAppWindow, platformBack());
    webViewDelegate->handleBrowserControlFunction(QString("platformBack"), QStringList(), &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Home)
{
    QString returnValue;
    QStringList params;
    params << QString(R"(["KeyMaskHome"])");

    EXPECT_CALL(*webAppWindow, SetKeyMask(webos::WebOSKeyMask::KEY_MASK_HOME));
    webViewDelegate->handleBrowserControlFunction(QString("setKeyMask"), params, &returnValue);
}


TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Back)
{
    QString returnValue;
    QStringList params;
    params << QString(R"(["KeyMaskBack"])");

    EXPECT_CALL(*webAppWindow, SetKeyMask(webos::WebOSKeyMask::KEY_MASK_BACK));
    webViewDelegate->handleBrowserControlFunction(QString("setKeyMask"), params, &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Exit)
{
    QString returnValue;
    QStringList params;
    params << QString(R"(["KeyMaskExit"])");

    EXPECT_CALL(*webAppWindow, SetKeyMask(webos::WebOSKeyMask::KEY_MASK_EXIT));
    webViewDelegate->handleBrowserControlFunction(QString("setKeyMask"), params, &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Incorrect)
{
    QString returnValue;
    QStringList params;
    params << QString(R"(["Incorrect value"])");

    EXPECT_CALL(*webAppWindow, SetKeyMask(static_cast<webos::WebOSKeyMask>(0)));
    webViewDelegate->handleBrowserControlFunction(QString("setKeyMask"), params, &returnValue);
}

TEST_F(PalmSystemBlinkTestSuite, handleBrowserControlMessage_setKeyMask_Combination)
{
    QString returnValue;
    QStringList params;
    params << QString(R"(["KeyMaskExit", "KeyMaskBack"])");

    int keymask = 0;
    keymask |= webos::WebOSKeyMask::KEY_MASK_EXIT;
    keymask |= webos::WebOSKeyMask::KEY_MASK_BACK;

    EXPECT_CALL(*webAppWindow, SetKeyMask(static_cast<webos::WebOSKeyMask>(keymask)));
    webViewDelegate->handleBrowserControlFunction(QString("setKeyMask"), params, &returnValue);
}

