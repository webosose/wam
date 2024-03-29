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

#ifndef TESTS_MOCKS_BASE_MOCK_INITIALIZER_H_
#define TESTS_MOCKS_BASE_MOCK_INITIALIZER_H_

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "platform_module_factory_impl.h"
#include "web_app_factory_manager_mock.h"
#include "web_app_window_factory_mock.h"
#include "web_app_window_mock.h"
#include "web_view_factory_mock.h"
#include "web_view_mock.h"
#include "webos/window_group_configuration.h"

template <typename T = NiceWebViewMock,
          typename U = NiceWebAppWindowMock,
          typename P = PlatformModuleFactoryImpl>
class BaseMockInitializer {
 public:
  BaseMockInitializer() {
    WebAppManager::Instance()->SetPlatformModules(std::make_unique<P>());

    auto web_app_factory_manager = std::make_unique<WebAppFactoryManagerMock>();
    web_app_factory_manager->SetWebViewFactory(web_view_factory_);
    web_app_factory_manager->SetWebAppWindowFactory(web_app_window_factory_);
    web_view_factory_->SetWebView(web_view_);
    web_app_window_factory_->SetWebAppWindow(web_app_window_);
    WebAppManager::Instance()->SetWebAppFactory(
        std::move(web_app_factory_manager));
  }

  BaseMockInitializer(const BaseMockInitializer&) = delete;
  BaseMockInitializer& operator=(const BaseMockInitializer&) = delete;

  ~BaseMockInitializer() { WebAppManager::Instance()->CloseAllApps(); }

  T* GetWebViewMock() { return web_view_; }
  U* GetWebAppWindowMock() { return web_app_window_; }

 private:
  WebViewFactoryMock* web_view_factory_ = new WebViewFactoryMock();
  WebAppWindowFactoryMock* web_app_window_factory_ =
      new WebAppWindowFactoryMock();
  T* web_view_ = new T();
  U* web_app_window_ = new U();
};

#endif  // TESTS_MOCKS_BASE_MOCK_INITIALIZER_H_
