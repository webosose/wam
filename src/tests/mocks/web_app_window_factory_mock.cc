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

#include "web_app_window_factory_mock.h"

#include <iostream>

#include "web_app_window.h"

WebAppWindowFactoryMock::WebAppWindowFactoryMock() : web_app_window_(nullptr) {}
WebAppWindowFactoryMock::~WebAppWindowFactoryMock() = default;

WebAppWindow* WebAppWindowFactoryMock::CreateWindow() {
  if (web_app_window_)
    return web_app_window_;

  std::cerr << "Missing WebAppWindow pointer. Method setWebAppWindow should be "
               "called prior to createWindow"
            << std::endl;
  return nullptr;
}

void WebAppWindowFactoryMock::SetWebAppWindow(WebAppWindow* web_app_window) {
  web_app_window_ = web_app_window;
}
