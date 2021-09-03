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

#ifndef PLATFORM_WEB_APP_WAYLAND_WINDOW_H_
#define PLATFORM_WEB_APP_WAYLAND_WINDOW_H_

#include "webos/webapp_window_base.h"

class WebAppWayland;

class WebAppWaylandWindow : public webos::WebAppWindowBase {
 public:
  WebAppWaylandWindow();
  virtual ~WebAppWaylandWindow() {}
  static WebAppWaylandWindow* Take();
  static void Prepare();

  inline const WebAppWayland* WebApp() const { return web_app_; }
  inline void SetWebApp(WebAppWayland* web_app) { web_app_ = web_app; }

  virtual void HideWindow();
  virtual void ShowWindow();

  void PlatformBack();
  void SetCursor(const std::string& cursor_arg,
                 int hotspot_x = -1,
                 int hotspot_y = -1);

  void AttachWebContentsToWindow(void* web_contents);

  void SendKeyCode(int key_code);

  void DidSwapPageCompositorFrame();

  // webos::WebAppWindowBase
  // TODO Marked as deprecated in webapp_window_delegate.h
  bool event(WebOSEvent* event) override;
  unsigned int CheckKeyFilterTable(unsigned key_code,
                                   unsigned* modifier) override;

 protected:
  bool CursorVisible() const { return cursor_visible_; }
  void SetCursorVisible(bool visible) { cursor_visible_ = visible; }
  bool CursorEnabled() const { return cursor_enabled_; }
  void SetCursorEnabled(bool enabled) { cursor_enabled_ = enabled; }

 private:
  void OnWindowStateChangeEvent();
  bool OnCursorVisibileChangeEvent(WebOSEvent* e);
  static WebAppWaylandWindow* CreateWindow();
  void LogEventDebugging(WebOSEvent* event);

 private:
  static WebAppWaylandWindow* instance_;

  bool cursor_enabled_;

  WebAppWayland* web_app_;
  bool cursor_visible_;
  bool xinput_activated_;

  WebOSMouseEvent last_mouse_event_;
};

#endif  // PLATFORM_WEB_APP_WAYLAND_WINDOW_H_
