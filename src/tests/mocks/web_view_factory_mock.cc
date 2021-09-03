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

#include "web_view_factory_mock.h"

#include <iostream>

#include "web_view.h"

WebViewFactoryMock::WebViewFactoryMock() : web_view_(nullptr) {}

WebViewFactoryMock::~WebViewFactoryMock() = default;

WebView* WebViewFactoryMock::CreateWebView() {
  if (web_view_)
    return web_view_;
  std::cerr << "Missing WebView pointer. Method setWebView should be called "
               "prior to createWebView"
            << std::endl;
  return nullptr;
}

void WebViewFactoryMock::SetWebView(WebView* web_view) {
  web_view_ = web_view;
}
