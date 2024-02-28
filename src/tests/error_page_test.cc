// Copyright (c) 2021 LG Electronics, Inc.
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

#include <glib.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <json/json.h>

#include "base_mock_initializer.h"
#include "platform_module_factory_impl_mock.h"
#include "web_app_manager_service_luna.h"
#include "web_page_base.h"
#include "web_page_blink.h"
#include "web_view_mock_impl.h"

namespace {

constexpr char kLaunchAppJsonBody[] = R"({
  "launchingAppId": "com.webos.app.home",
  "appDesc": {
    "defaultWindowType": "card",
    "uiRevision": "2",
    "version": "1.0.1",
    "vendor": "LG Electronics, Inc.",
    "launchPointId": "bareapp_default",
    "id": "bareapp",
    "trustLevel": "default",
    "title": "Bare App",
    "lptype": "default",
    "folderPath": "/usr/palm/applications/bareapp",
    "main": "index.html"
  },
  "appId": "bareapp",
  "parameters": {
    "displayAffinity": 0
  },
  "reason": "com.webos.app.home",
  "instanceId": "de90e74a-b86b-42c8-8785-3efd927a36430"
})";

const char kIdentifierForNetErrorPage[] = "com.webos.settingsservice.client";
const std::map<std::string, std::string> kEnvironmentVariables = {
    {"WAM_ERROR_PAGE", "file:///usr/share/localization/wam/loaderror.html"}};

}  // namespace

class ErrorPageTestSuite : public ::testing::Test {
 public:
  ErrorPageTestSuite() = default;
  ~ErrorPageTestSuite() override = default;

  void SetUp() override;
  void LaunchApp();
  void SetExpectedLoadUrlRequests();
  void ProcessLoading(const std::string url);
  void TearDown() override;
  static gboolean OnTimeoutFail(gpointer userdata);
  std::unique_ptr<BaseMockInitializer<NiceWebViewMockImpl,
                                      NiceWebAppWindowMock,
                                      PlatformModuleFactoryImplMock>>
      mock_initializer_;
  WebPageBlinkDelegate* web_view_delegate_ = nullptr;
  std::string instance_id_;
  std::string current_url_;
  std::string app_url_;
  static bool timeout_exceeded_;
};

bool ErrorPageTestSuite::timeout_exceeded_ = false;

void ErrorPageTestSuite::SetUp() {
  PlatformModuleFactoryImplMock::SetDefaultConfig(kEnvironmentVariables);
  mock_initializer_ = std::make_unique<
      BaseMockInitializer<NiceWebViewMockImpl, NiceWebAppWindowMock,
                          PlatformModuleFactoryImplMock>>();
  auto view_mock = mock_initializer_->GetWebViewMock();
  view_mock->SetOnInitActions();
  ON_CALL(*view_mock, GetUrl()).WillByDefault(testing::ReturnRef(current_url_));
  ON_CALL(*view_mock, LoadUrl(testing::_))
      .WillByDefault(testing::Invoke([this](const std::string& url) {
        if (!web_view_delegate_) {
          web_view_delegate_ =
              mock_initializer_->GetWebViewMock()->GetWebViewDelegate();
        }
        app_url_ = url;
        ProcessLoading(url);
      }));
  LaunchApp();
}

void ErrorPageTestSuite::LaunchApp() {
  Json::Value launch_request;
  ASSERT_TRUE(util::StringToJson(kLaunchAppJsonBody, launch_request));
  instance_id_ = launch_request["instanceId"].asString();
  const auto result =
      WebAppManagerServiceLuna::Instance()->launchApp(launch_request);
  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.isMember("returnValue"));
  ASSERT_TRUE(result["returnValue"].asBool());
}

void ErrorPageTestSuite::SetExpectedLoadUrlRequests() {
  auto view_mock = mock_initializer_->GetWebViewMock();
  EXPECT_CALL(
      *view_mock,
      LoadUrl(::testing::HasSubstr("loaderror.html?errorCode=404&failedUrl")))
      .WillOnce(testing::Invoke(
          [this](const std::string& url) { ProcessLoading(url); }));
  EXPECT_CALL(*view_mock, LoadUrl(std::string("about:blank")))
      .WillOnce(testing::Invoke(
          [this](const std::string& url) { ProcessLoading(url); }));
}

void ErrorPageTestSuite::ProcessLoading(const std::string url) {
  current_url_ = url;
  ASSERT_NE(web_view_delegate_, nullptr);
  web_view_delegate_->LoadStarted();
  web_view_delegate_->LoadProgressChanged(1.0);
  web_view_delegate_->LoadVisuallyCommitted();
  web_view_delegate_->LoadFinished(url);
}

void ErrorPageTestSuite::TearDown() {
  mock_initializer_.reset(nullptr);
}

gboolean ErrorPageTestSuite::OnTimeoutFail(gpointer userdata) {
  timeout_exceeded_ = true;
  g_main_loop_quit(static_cast<GMainLoop*>(userdata));
  return true;
}

TEST_F(ErrorPageTestSuite, LoadErrorPage) {
  SetExpectedLoadUrlRequests();
  auto app = WebAppManager::Instance()->FindAppByInstanceId(instance_id_);
  ASSERT_NE(app, nullptr);
  auto page = app->Page();
  ASSERT_NE(page, nullptr);
  ASSERT_NE(web_view_delegate_, nullptr);

  EXPECT_FALSE(page->IsLoadErrorPageStart());
  EXPECT_FALSE(page->IsLoadErrorPageFinish());
  EXPECT_EQ(page->FailedUrl(), std::string());
  EXPECT_EQ(page->GetIdentifier(), std::string("bareapp"));
  EXPECT_EQ(static_cast<WebPageBlink*>(page)->TrustLevel(),
            std::string("default"));

  web_view_delegate_->LoadFailed(app_url_, 404);

  EXPECT_TRUE(page->IsLoadErrorPageStart());
  EXPECT_TRUE(page->IsLoadErrorPageFinish());
  EXPECT_EQ(page->FailedUrl(), app_url_);
  EXPECT_EQ(page->GetIdentifier(), std::string(kIdentifierForNetErrorPage));
  EXPECT_EQ(static_cast<WebPageBlink*>(page)->TrustLevel(),
            std::string("trusted"));
}

TEST_F(ErrorPageTestSuite, ReloadOnRelaunch) {
  SetExpectedLoadUrlRequests();
  ASSERT_NE(web_view_delegate_, nullptr);
  auto view_mock = mock_initializer_->GetWebViewMock();
  EXPECT_CALL(*view_mock, RunJavaScript(::testing::HasSubstr("webOSRelaunch")))
      .Times(1);
  web_view_delegate_->LoadFailed(app_url_, 404);
  LaunchApp();
}

TEST_F(ErrorPageTestSuite, ReloadOnNetworkRecovery) {
  SetExpectedLoadUrlRequests();
  ASSERT_NE(web_view_delegate_, nullptr);
  auto view_mock = mock_initializer_->GetWebViewMock();
  EXPECT_CALL(*view_mock, LoadUrl(app_url_))
      .WillOnce(testing::Invoke(
          [this](const std::string& url) { ProcessLoading(url); }));
  web_view_delegate_->LoadFailed(app_url_, 404);
  Json::Value status;
  status["isInternetConnectionAvailable"] = true;
  WebAppManager::Instance()->UpdateNetworkStatus(status);
}

TEST_F(ErrorPageTestSuite, ReloadOnTimeout) {
  SetExpectedLoadUrlRequests();
  ASSERT_NE(web_view_delegate_, nullptr);
  auto view_mock = mock_initializer_->GetWebViewMock();
  GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
  EXPECT_CALL(*view_mock, LoadUrl(app_url_))
      .WillOnce(testing::Invoke([=](const std::string& url) {
        ProcessLoading(url);
        g_main_loop_quit(loop);
      }));
  guint id = g_timeout_add(61000, OnTimeoutFail, loop);
  web_view_delegate_->LoadFailed(app_url_, 404);
  g_main_loop_run(loop);
  if (!timeout_exceeded_) {
    g_source_remove(id);
  }
  EXPECT_FALSE(timeout_exceeded_);
  g_main_loop_unref(loop);
}
