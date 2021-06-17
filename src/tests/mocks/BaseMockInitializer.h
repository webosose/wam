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

#ifndef BASEMOCKINITIALIZER_H
#define BASEMOCKINITIALIZER_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "PlatformModuleFactoryImpl.h"
#include "WebAppFactoryManagerMock.h"
#include "WebAppWindowFactoryMock.h"
#include "WebAppWindowMock.h"
#include "WebViewFactoryMock.h"
#include "WebViewMock.h"
#include "webos/window_group_configuration.h"

template <typename T = NiceWebViewMock, typename U = NiceWebAppWindowMock,
          typename P = PlatformModuleFactoryImpl>
class BaseMockInitializer {
 public:
  BaseMockInitializer()
      : web_view_factory_(new WebViewFactoryMock()),
        web_app_window_factory_(new WebAppWindowFactoryMock()),
        web_view_(new T()),
        web_app_window_(new U()) {
    WebAppManager::instance()->setPlatformModules(
        std::make_unique<P>());

    auto web_app_factory_manager = std::make_unique<WebAppFactoryManagerMock>();
    web_app_factory_manager->setWebViewFactory(web_view_factory_);
    web_app_factory_manager->setWebAppWindowFactory(web_app_window_factory_);
    web_view_factory_->setWebView(web_view_);
    web_app_window_factory_->setWebAppWindow(web_app_window_);
    WebAppManager::instance()->setWebAppFactory(
        std::move(web_app_factory_manager));
  }

  ~BaseMockInitializer() { WebAppManager::instance()->closeAllApps(); }

  T* GetWebViewMock() { return web_view_; };
  U* GetWebAppWindowMock() { return web_app_window_; };

 private:
  BaseMockInitializer(const BaseMockInitializer&) = delete;
  BaseMockInitializer& operator=(const BaseMockInitializer&) = delete;

  WebViewFactoryMock* web_view_factory_;
  WebAppWindowFactoryMock* web_app_window_factory_;
  T* web_view_;
  U* web_app_window_;
};

#endif  // BASEMOCKINITIALIZER_H
