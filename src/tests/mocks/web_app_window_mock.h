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

#ifndef TESTS_MOCKS_WEB_APP_WINDOW_MOCK_H_
#define TESTS_MOCKS_WEB_APP_WINDOW_MOCK_H_

#include <gmock/gmock.h>

#include "web_app_window.h"

class WebAppWindowMock : public WebAppWindow {
 public:
  WebAppWindowMock() = default;
  virtual ~WebAppWindowMock() = default;

  void SetInputRegion(const std::vector<gfx::Rect>&) override {}
  MOCK_METHOD(int, DisplayWidth, (), (override));
  MOCK_METHOD(int, DisplayHeight, (), (override));
  MOCK_METHOD(void, InitWindow, (int, int), (override));
  MOCK_METHOD(void,
              SetLocationHint,
              (webos::WebAppWindowBase::LocationHint),
              (override));
  MOCK_METHOD(webos::NativeWindowState,
              GetWindowHostState,
              (),
              (const, override));
  MOCK_METHOD(void,
              CreateWindowGroup,
              (const webos::WindowGroupConfiguration&),
              (override));
  MOCK_METHOD(void,
              AttachToWindowGroup,
              (const std::string&, const std::string&),
              (override));
  MOCK_METHOD(bool, IsKeyboardVisible, (), (override));
  MOCK_METHOD(void, SetKeyMask, (webos::WebOSKeyMask), (override));
  MOCK_METHOD(void, SetKeyMask, (webos::WebOSKeyMask, bool), (override));
  MOCK_METHOD(void,
              SetWindowProperty,
              (const std::string&, const std::string&),
              (override));
  MOCK_METHOD(void, Resize, (int, int), (override));
  MOCK_METHOD(void, FocusWindowGroupOwner, (), (override));
  MOCK_METHOD(void, FocusWindowGroupLayer, (), (override));
  MOCK_METHOD(void, SetOpacity, (float), (override));
  MOCK_METHOD(void, SetWindowHostState, (webos::NativeWindowState), (override));
  MOCK_METHOD(void, RecreatedWebContents, (), (override));
  MOCK_METHOD(void, DetachWindowGroup, (), (override));
  MOCK_METHOD(void, SetUseVirtualKeyboard, (bool), (override));

  MOCK_METHOD(void, SetWebApp, (WebAppWayland*), (override));
  MOCK_METHOD(void, Hide, (), (override));
  MOCK_METHOD(void, Show, (), (override));
  MOCK_METHOD(void, PlatformBack, (), (override));
  MOCK_METHOD(void, SetCursor, (const std::string&, int, int), (override));
  MOCK_METHOD(void, AttachWebContents, (void*), (override));
  MOCK_METHOD(bool, Event, (WebOSEvent*), (override));
};

using NiceWebAppWindowMock = testing::NiceMock<WebAppWindowMock>;
using StrictWebAppWindowMock = testing::StrictMock<WebAppWindowMock>;

#endif  // TESTS_MOCKS_WEB_APP_WINDOW_MOCK_H_
