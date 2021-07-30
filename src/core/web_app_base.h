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

#ifndef CORE_WEB_APP_BASE_H_
#define CORE_WEB_APP_BASE_H_

#include <memory>
#include <string>

#include "web_app_manager.h"
#include "web_page_observer.h"

class ApplicationDescription;
class WebAppBasePrivate;
class WebPageBase;

namespace Json {
class Value;
};

class WebAppBase : public WebPageObserver {
 public:
  enum PreloadState {
    kNonePreload = 0,
    kFullPreload = 1,
    kSemiFullPreload = 2,
    kPartialPreload = 3,
    kMinimalPreload = 4
  };

  WebAppBase();
  ~WebAppBase() override;

  virtual void Attach(WebPageBase*);
  virtual WebPageBase* Detach();
  virtual void SuspendAppRendering() = 0;
  virtual void ResumeAppRendering() = 0;
  virtual bool IsFocused() const = 0;
  virtual void Resize(int width, int height) = 0;
  virtual bool IsActivated() const = 0;
  virtual bool IsMinimized() = 0;
  virtual bool IsNormal() = 0;
  virtual void OnStageActivated() = 0;
  virtual void OnStageDeactivated() = 0;
  virtual void StartLaunchTimer() {}
  virtual void SetHiddenWindow(bool hidden);
  virtual void ConfigureWindow(const std::string& type) = 0;
  virtual void SetKeepAlive(bool keep_alive);
  virtual bool IsWindowed() const;
  virtual void Relaunch(const std::string& args,
                        const std::string& launching_app_id);
  virtual void SetWindowProperty(const std::string& name,
                                 const std::string& value) = 0;
  virtual void PlatformBack() = 0;
  virtual void SetCursor(const std::string& cursor_arg,
                         int hotspot_x,
                         int hotspot_y) = 0;
  virtual void SetInputRegion(const Json::Value& json_doc) = 0;
  virtual void SetKeyMask(const Json::Value& json_doc) = 0;
  virtual void Hide(bool forced_hide = false) = 0;
  virtual void Focus() = 0;
  virtual void Unfocus() = 0;
  virtual void SetOpacity(float opacity) = 0;
  virtual void SetAppDescription(std::shared_ptr<ApplicationDescription>);
  virtual void SetPreferredLanguages(const std::string& language);
  virtual void StagePreparing();
  virtual void StageReady();
  virtual void Raise() = 0;
  virtual void GoBackground() = 0;
  virtual void DoPendingRelaunch();
  virtual void DeleteSurfaceGroup() = 0;
  virtual void KeyboardVisibilityChanged(bool visible, int height);
  virtual void DoClose() = 0;
  virtual void SetUseVirtualKeyboard(const bool enable) = 0;
  virtual bool IsKeyboardVisible() { return false; }
  static void OnCursorVisibilityChanged(const std::string& jsscript);
  virtual bool HideWindow() = 0;

  bool GetCrashState() const;
  void SetCrashState(bool state);
  bool GetHiddenWindow() const;
  bool KeepAlive() const;
  void SetForceClose();
  bool ForceClose();
  WebPageBase* Page() const;
  void HandleWebAppMessage(WebAppManager::WebAppMessageType type,
                           const std::string& message);
  void SetAppId(const std::string& app_id);
  void SetLaunchingAppId(const std::string& app_id);
  std::string AppId() const;
  std::string LaunchingAppId() const;
  void SetInstanceId(const std::string& instance_id);
  std::string InstanceId() const;
  std::string Url() const;

  ApplicationDescription* GetAppDescription() const;

  void SetAppProperties(const std::string& properties);

  void SetNeedReload(bool status) { need_reload_ = status; }
  bool NeedReload() { return need_reload_; }

  static int CurrentUiWidth();
  static int CurrentUiHeight();

  void CleanResources();
  void ExecuteCloseCallback();
  void DispatchUnload();

  void SetUseAccessibility(bool enabled);
  void ServiceCall(const std::string& url,
                   const std::string& payload,
                   const std::string& app_id);

  void SetPreloadState(const std::string& properties);
  void ClearPreloadState();
  PreloadState GetPreloadState() const { return preload_state_; }

  bool IsClosing() const;
  bool IsCheckLaunchTimeEnabled();
  void SetClosePageRequested(bool requested) {
    close_page_requested_ = requested;
  }
  bool ClosePageRequested() const { return close_page_requested_; }

 protected:
  virtual void DoAttach() = 0;
  virtual void ShowWindow();

  void SetUiSize(int width, int height);
  void SetActiveInstanceId(const std::string& id);
  void ForceCloseAppInternal();
  void CloseAppInternal();
  void CloseWebApp();

  // WebPageObserver
  void CloseCallbackExecuted() override;
  void ClosingAppProcessDidCrashed() override;
  void DidDispatchUnload() override;
  void TimeoutExecuteCloseCallback() override;
  void WebPageClosePageRequested() override;
  void WebPageLoadFinished() override;

 protected:
  PreloadState preload_state_;
  bool added_to_window_mgr_;
  std::string in_progress_relaunch_params_;
  std::string in_progress_relaunch_launching_app_id_;
  float scale_factor_;

 private:
  WebAppBasePrivate* app_private_;
  bool need_reload_;
  bool crashed_;
  bool hidden_window_;
  bool close_page_requested_;  // window.close() is called once then have to
                               // drop further requests
};
#endif  // CORE_WEB_APP_BASE_H_
