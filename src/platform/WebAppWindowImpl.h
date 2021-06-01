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

#ifndef WEBAPPWINDOWIMPL_H
#define WEBAPPWINDOWIMPL_H

#include <memory>
#include "WebAppWindow.h"

class WebAppWaylandWindow;

class WebAppWindowImpl : public WebAppWindow {
 public:
  WebAppWindowImpl(std::unique_ptr<WebAppWaylandWindow> window);
  ~WebAppWindowImpl() override;

 private:
  int DisplayWidth() override;
  int DisplayHeight() override;
  void InitWindow(int width, int height) override;
  void SetLocationHint(webos::WebAppWindowBase::LocationHint value) override;
  webos::NativeWindowState GetWindowHostState() const override;
  void CreateWindowGroup(
      const webos::WindowGroupConfiguration& config) override;
  void AttachToWindowGroup(const std::string& name,
                           const std::string& layer) override;
  bool IsKeyboardVisible() override;
  void SetKeyMask(webos::WebOSKeyMask key_mask) override;
  void SetKeyMask(webos::WebOSKeyMask key_mask, bool set) override;
  void SetInputRegion(const std::vector<gfx::Rect>& region) override;
  void SetWindowProperty(const std::string& name,
                         const std::string& value) override;
  void Resize(int width, int height) override;
  void FocusWindowGroupOwner() override;
  void FocusWindowGroupLayer() override;
  void SetOpacity(float opacity) override;
  void SetWindowHostState(webos::NativeWindowState state) override;
  void RecreatedWebContents() override;
  void DetachWindowGroup() override;
  void SetUseVirtualKeyboard(bool enable) override;

  void setWebApp(WebAppWayland* w) override;
  void hide() override;
  void show() override;
  void platformBack() override;
  void setCursor(const QString& cursorArg,
                 int hotspot_x,
                 int hotspot_y) override;
  void attachWebContents(void* webContents) override;
  bool event(WebOSEvent* event) override;

  std::unique_ptr<WebAppWaylandWindow> window_;
};

#endif // WEBAPPWINDOWIMPL_H
