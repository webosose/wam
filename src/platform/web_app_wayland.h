// Copyright (c) 2014-2021 LG Electronics, Inc.
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

#ifndef PLATFORM_WEB_APP_WAYLAND_H_
#define PLATFORM_WEB_APP_WAYLAND_H_

#include <memory>
#include <optional>
#include <string>

#include "webos/common/webos_constants.h"
#include "webos/common/webos_event.h"
#include "webos/webos_platform.h"

#include "display_id.h"
#include "timer.h"
#include "web_app_base.h"
#include "web_app_window.h"
#include "web_app_window_factory.h"
#include "web_page_blink_observer.h"

namespace Json {
class Value;
}

class ApplicationDescription;
class WebAppWaylandWindow;

class InputManager : public webos::InputPointer {
 public:
  static InputManager* Instance() {
    // InputManager should be globally one.
    if (!webos::WebOSPlatform::GetInstance()->GetInputPointer()) {
      webos::WebOSPlatform::GetInstance()->SetInputPointer(new InputManager());
    }
    return static_cast<InputManager*>(
        webos::WebOSPlatform::GetInstance()->GetInputPointer());
  }

  bool GlobalCursorVisibility() { return IsVisible(); }

  // Overridden from weboswayland::InputPointer:
  void OnCursorVisibilityChanged(bool visible) override;
};

class WebAppWayland : public WebAppBase, WebPageBlinkObserver {
 public:
  explicit WebAppWayland(const std::string& type,
                         std::optional<int> width = std::nullopt,
                         std::optional<int> height = std::nullopt,
                         int display_id = kUndefinedDisplayId,
                         const std::string& location_hint = {});
  WebAppWayland(const std::string& type,
                WebAppWaylandWindow* window,
                std::optional<int> width = std::nullopt,
                std::optional<int> height = std::nullopt,
                int display_id = kUndefinedDisplayId,
                const std::string& location_hint = {});

  WebAppWayland(const std::string& type,
                std::unique_ptr<WebAppWindowFactory> factory,
                std::optional<int> width = std::nullopt,
                std::optional<int> height = std::nullopt,
                int display_id = kUndefinedDisplayId,
                const std::string& location_hint = {});

  ~WebAppWayland() override = default;

  // WebAppBase
  void Attach(WebPageBase*) override;
  WebPageBase* Detach() override;
  void SuspendAppRendering() override;
  void ResumeAppRendering() override;
  bool IsFocused() const override;
  void Resize(int width, int height) override;
  bool IsActivated() const override;
  bool IsMinimized() override;
  bool IsNormal() override;
  void OnStageActivated() override;
  void OnStageDeactivated() override;
  void StartLaunchTimer() override;
  void ConfigureWindow(const std::string& type) override;
  void SetKeepAlive(bool keep_alive) override;
  bool IsWindowed() const override { return true; }
  void SetWindowProperty(const std::string& name,
                         const std::string& value) override;
  void PlatformBack() override;
  void SetCursor(const std::string& cursor_arg,
                 int hotspot_x = -1,
                 int hotspot_y = -1) override;
  void SetInputRegion(const Json::Value& value) override;
  void SetKeyMask(const Json::Value& value) override;
  void SetOpacity(float opacity) override;
  void Hide(bool forced_hide = false) override;
  void Focus() override;
  void Unfocus() override;
  void Raise() override;
  void GoBackground() override;
  void DeleteSurfaceGroup() override;
  void KeyboardVisibilityChanged(bool visible, int height) override;
  void DoClose() override;
  bool IsKeyboardVisible() override;
  bool HideWindow() override;
  void SetUseVirtualKeyboard(const bool enable) override;
  void SetDisplayFirstActivateTimeoutMs(uint32_t timeout) override;

  // WebAppWayland
  virtual void SetKeyMask(webos::WebOSKeyMask key_mask, bool value);
  virtual void SetKeyMask(webos::WebOSKeyMask key_mask);
  virtual void FocusOwner();
  virtual void FocusLayer();

  // WebPageObserver
  void TitleChanged() override;
  void FirstFrameVisuallyCommitted() override;
  void NavigationHistoryChanged() override;

  std::string GetWindowType() const { return window_type_; }
  bool CursorVisibility() {
    return InputManager::Instance()->GlobalCursorVisibility();
  }
  void SendWebOSMouseEvent(const std::string& event_name);

  void PostEvent(WebOSEvent* ev);
  void OnDelegateWindowFrameSwapped();
  void OnLaunchTimeout();

  void ApplyInputRegion();
  void ForwardWebOSEvent(WebOSEvent* event) const;
  void StateAboutToChange(webos::NativeWindowState will_be);
  void StateChanged(webos::NativeWindowState new_state);

  // from WebPageBlinkObserver
  void DidSwapPageCompositorFrame() override;
  void DidResumeDOM() override;

 protected:
  // WebAppBase
  void DoAttach() override;
  void ShowWindow() override;

  void SetupWindowGroup(ApplicationDescription* desc);

  void MoveInputRegion(int height);

  // WebPageObserver
  void WebPageLoadFinished() override;
  void WebPageLoadFailed(int error_code) override;
  void WebViewRecreated() override;

 private:
  void Init(std::optional<int> width, std::optional<int> height);

  std::unique_ptr<WebAppWindow> app_window_;
  std::string window_type_;
  int last_swapped_time_ = 0;
  bool did_activate_stage_ = false;

  std::vector<gfx::Rect> input_region_;
  bool enable_input_region_ = false;

  bool is_focused_ = false;
  float vkb_height_ = 0;

  ElapsedTimer elapsed_launch_timer_;
  OneShotTimer<WebAppWayland> launch_timeout_timer_;

  int display_id_;
  std::string location_hint_;

  std::unique_ptr<WebAppWindowFactory> window_factory_;
};

#endif  // PLATFORM_WEB_APP_WAYLAND_H_
