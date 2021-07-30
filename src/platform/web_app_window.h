// Copyright (c) 2008-2021 LG Electronics, Inc.
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

#ifndef PLATFORM_WEB_APP_WINDOW_H_
#define PLATFORM_WEB_APP_WINDOW_H_

#include <string>
#include <vector>

#include "webos/webapp_window_base.h"

class WebAppWayland;

class WebAppWindow {
 public:
  WebAppWindow() = default;
  virtual ~WebAppWindow() = default;

  virtual int DisplayWidth() = 0;
  virtual int DisplayHeight() = 0;
  virtual void InitWindow(int width, int height) = 0;
  virtual void SetLocationHint(webos::WebAppWindowBase::LocationHint value) = 0;
  virtual webos::NativeWindowState GetWindowHostState() const = 0;
  virtual void CreateWindowGroup(
      const webos::WindowGroupConfiguration& config) = 0;
  virtual void AttachToWindowGroup(const std::string& name,
                                   const std::string& layer) = 0;
  virtual bool IsKeyboardVisible() = 0;
  virtual void SetKeyMask(webos::WebOSKeyMask key_mask) = 0;
  virtual void SetKeyMask(webos::WebOSKeyMask key_mask, bool set) = 0;
  virtual void SetInputRegion(const std::vector<gfx::Rect>& region) = 0;
  virtual void SetWindowProperty(const std::string& name,
                                 const std::string& value) = 0;
  virtual void Resize(int width, int height) = 0;
  virtual void FocusWindowGroupOwner() = 0;
  virtual void FocusWindowGroupLayer() = 0;
  virtual void SetOpacity(float opacity) = 0;
  virtual void SetWindowHostState(webos::NativeWindowState state) = 0;
  virtual void RecreatedWebContents() = 0;
  virtual void DetachWindowGroup() = 0;
  virtual void SetUseVirtualKeyboard(bool enable) = 0;

  virtual void SetWebApp(WebAppWayland* w) = 0;
  virtual void Hide() = 0;
  virtual void Show() = 0;
  virtual void PlatformBack() = 0;
  virtual void SetCursor(const std::string& cursor_arg,
                         int hotspot_x,
                         int hotspot_y) = 0;
  virtual void AttachWebContents(void* web_contents) = 0;
  virtual bool Event(WebOSEvent* event) = 0;
};

#endif  // PLATFORM_WEB_APP_WINDOW_H_
