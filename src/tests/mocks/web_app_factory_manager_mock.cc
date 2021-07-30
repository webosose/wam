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

#include "web_app_factory_manager_mock.h"

#include <gtest/gtest.h>

#include "application_description.h"
#include "util/url.h"
#include "web_app_wayland.h"
#include "web_app_window_factory.h"
#include "web_page_blink.h"
#include "web_view_factory.h"

WebAppFactoryManagerMock::WebAppFactoryManagerMock()
    : view_factory_(nullptr), window_factory_(nullptr) {}

WebAppFactoryManagerMock::~WebAppFactoryManagerMock() = default;

WebAppBase* WebAppFactoryManagerMock::CreateWebApp(
    const std::string& win_type,
    std::shared_ptr<ApplicationDescription> desc,
    const std::string& app_type) {
  if (window_factory_)
    return new WebAppWayland(
        win_type, std::unique_ptr<WebAppWindowFactory>(window_factory_),
        desc->WidthOverride(), desc->HeightOverride(),
        desc->GetDisplayAffinity(), desc->LocationHint());

  std::cerr << "Missing WindowFactory pointer. Method setWebAppWindowFactory "
               "should be called prior to createWebApp"
            << std::endl;
  return nullptr;
}

WebAppBase* WebAppFactoryManagerMock::CreateWebApp(
    const std::string& win_type,
    WebPageBase* page,
    std::shared_ptr<ApplicationDescription> desc,
    const std::string& app_type) {
  return CreateWebApp(win_type, desc, app_type);
}

WebPageBase* WebAppFactoryManagerMock::CreateWebPage(
    const std::string& win_type,
    const wam::Url& url,
    std::shared_ptr<ApplicationDescription> desc,
    const std::string& app_type,
    const std::string& launch_params) {
  if (!view_factory_) {
    std::cerr << "Missing ViewFactory pointer. Method setWebViewFactory should "
                 "be called prior to createWebPage"
              << std::endl;
    return nullptr;
  }
  auto page = new WebPageBlink(url, desc, launch_params,
                               std::unique_ptr<WebViewFactory>(view_factory_));
  page->Init();
  return page;
}

void WebAppFactoryManagerMock::SetWebViewFactory(WebViewFactory* view_factory) {
  view_factory_ = view_factory;
}

void WebAppFactoryManagerMock::SetWebAppWindowFactory(
    WebAppWindowFactory* window_factory) {
  window_factory_ = window_factory;
}
