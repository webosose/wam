// Copyright (c) 2013-2021 LG Electronics, Inc.
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

#ifndef CORE_WEB_PAGE_BASE_H_
#define CORE_WEB_PAGE_BASE_H_

#include <memory>
#include <string>

#include "webos/webview_base.h"

#include "observer_list.h"
#include "util/url.h"

class ApplicationDescription;
class WebAppBase;
class WebAppManagerConfig;
class WebPageObserver;
class WebProcessManager;

/**
 * Common base class for UI independent
 * web page functionality
 */
class WebPageBase {
 public:
  // Originally, webview_base.h, WebPageVisibilityState.h, PageVisibilityState.h
  // we can use enum of webview_base.h directly but this is WebPageBase in core
  // shouldn't depend on web engine
  enum WebPageVisibilityState {
    kWebPageVisibilityStateVisible,
    kWebPageVisibilityStateHidden,
    kWebPageVisibilityStateLaunching,
    kWebPageVisibilityStatePrerender,
    kWebPageVisibilityStateLast = kWebPageVisibilityStatePrerender
  };

  WebPageBase();
  WebPageBase(const wam::Url& url,
              std::shared_ptr<ApplicationDescription> desc,
              const std::string& params);
  virtual ~WebPageBase();

  // WebPageBase
  virtual void Init() = 0;
  virtual void* GetWebContents() = 0;
  virtual void SetLaunchParams(const std::string& params);
  virtual void NotifyMemoryPressure(
      webos::WebViewBase::MemoryPressureLevel level) {}

  virtual std::string GetIdentifier() const;
  virtual wam::Url Url() const = 0;
  virtual wam::Url DefaultUrl() const { return default_url_; }
  virtual void SetDefaultUrl(const wam::Url& url) { default_url_ = url; }
  virtual void LoadUrl(const std::string& url) = 0;
  virtual int Progress() const = 0;
  virtual bool HasBeenShown() const = 0;
  virtual void SetPageProperties() = 0;
  virtual void SetPreferredLanguages(const std::string& language) = 0;
  virtual std::string DefaultFont();
  virtual void SetDefaultFont(const std::string& font) = 0;
  virtual void CleanResources();
  virtual void ReloadDefaultPage() = 0;
  virtual void Reload() = 0;
  virtual void SetVisibilityState(WebPageVisibilityState visibility_state) = 0;
  virtual void SetFocus(bool focus) = 0;
  virtual std::string Title() = 0;
  virtual bool CanGoBack() = 0;
  virtual void CloseVkb() = 0;
  virtual void KeyboardVisibilityChanged(bool visible) {}
  virtual void HandleDeviceInfoChanged(const std::string& device_info) = 0;
  virtual bool Relaunch(const std::string& args,
                        const std::string& launching_app_id);
  virtual void EvaluateJavaScript(const std::string& jsCode) = 0;
  virtual void EvaluateJavaScriptInAllFrames(const std::string& js_code,
                                             const char* method = {}) = 0;
  virtual uint32_t GetWebProcessProxyID() = 0;
  virtual uint32_t GetWebProcessPID() const = 0;
  virtual void CreatePalmSystem(WebAppBase* app) = 0;
  virtual void SetUseLaunchOptimization(bool enabled, int delay_ms = {}) {}
  virtual void SetUseSystemAppOptimization(bool enabled) {}
  virtual void SetUseAccessibility(bool enabled) {}
  virtual void SetAppPreloadHint(bool is_preload) {}

  virtual void SuspendWebPageAll() = 0;
  virtual void ResumeWebPageAll() = 0;
  virtual void SuspendWebPageMedia() = 0;
  virtual void ResumeWebPageMedia() = 0;
  virtual void ResumeWebPagePaintingAndJSExecution() = 0;
  virtual bool IsRegisteredCloseCallback() { return false; }
  virtual void ExecuteCloseCallback(bool forced) {}
  virtual void ReloadExtensionData() {}
  virtual bool IsLoadErrorPageFinish() { return is_load_error_page_finish_; }
  virtual bool IsLoadErrorPageStart() { return is_load_error_page_start_; }
  virtual void UpdateIsLoadErrorPageFinish();
  virtual void UpdateDatabaseIdentifier() {}
  virtual void DeleteWebStorages(const std::string& identifier) {}
  virtual void SetInspectorEnable() {}
  virtual void SetKeepAliveWebApp(bool keep_alive) {}
  virtual void SetContentsScale() {}
  virtual void SetCustomUserScript();
  virtual void ForwardEvent(void* event) = 0;
  virtual void SetAudioGuidanceOn(bool on) {}
  virtual bool IsInputMethodActive() const { return false; }

  std::string LaunchParams() const;
  void SetApplicationDescription(std::shared_ptr<ApplicationDescription> desc);
  void Load();
  void SetEnableBackgroundRun(bool enable) { enable_background_run_ = enable; }
  void SendLocaleChangeEvent(const std::string& language);
  void SetCleaningResources(bool cleaning_resources) {
    cleaning_resources_ = cleaning_resources;
  }
  bool CleaningResources() const { return cleaning_resources_; }
  bool DoHostedWebAppRelaunch(const std::string& launch_params);
  void SendRelaunchEvent();
  void SetAppId(const std::string& app_id) { app_id_ = app_id; }
  const std::string& AppId() const { return app_id_; }
  void SetInstanceId(const std::string& instance_id) {
    instance_id_ = instance_id;
  }
  const std::string& InstanceId() const { return instance_id_; }
  ApplicationDescription* GetAppDescription() { return app_desc_.get(); }

  void SetClosing(bool status) { is_closing_ = status; }
  bool IsClosing() { return is_closing_; }

  void SetIsPreload(bool is_preload) { is_preload_ = is_preload; }
  bool IsPreload() const { return is_preload_; }

  void AddObserver(WebPageObserver* observer);
  void RemoveObserver(WebPageObserver* observer);

  virtual std::string GetIdentifierForSecurityOrigin() const;

  virtual void ActivateRendererCompositor() {}
  virtual void DeactivateRendererCompositor() {}

  virtual void SuspendWebPagePaintingAndJSExecution() = 0;

 protected:
  // WebPageBase
  virtual void CleanResourcesFinished();
  virtual void HandleForceDeleteWebPage();
  virtual void LoadDefaultUrl() = 0;
  virtual void AddUserScript(const std::string& script) = 0;
  virtual void AddUserScriptUrl(const wam::Url& url) = 0;
  virtual int SuspendDelay();
  virtual bool HasLoadErrorPolicy(bool is_http_response_error, int error_code);
  virtual void LoadErrorPage(int error_code) = 0;
  virtual void RecreateWebView() = 0;
  virtual void SetVisible(bool visible) {}
  virtual bool DoDeeplinking(const std::string& launch_params);

  void HandleLoadStarted();
  void HandleLoadFinished();
  void HandleLoadFailed(int error_code);
  bool GetDeviceInfo(const std::string& name, std::string& value);
  bool GetSystemLanguage(std::string& value);
  int CurrentUiWidth();
  int CurrentUiHeight();
  WebProcessManager* GetWebProcessManager();
  WebAppManagerConfig* GetWebAppManagerConfig();
  bool ProcessCrashed();

  virtual int MaxCustomSuspendDelay();
  std::string TelluriumNubPath();

  void ApplyPolicyForUrlResponse(bool is_main_frame,
                                 const std::string& url,
                                 int status_code);
  void PostRunningAppList();
  void PostWebProcessCreated(uint32_t pid);
  bool IsAccessibilityEnabled() const;

  std::shared_ptr<ApplicationDescription> app_desc_;
  std::string app_id_;
  std::string instance_id_;
  bool suspend_at_load_;
  bool is_closing_;
  bool is_load_error_page_finish_;
  bool is_load_error_page_start_;
  bool did_error_page_loaded_from_net_error_helper_;
  bool enable_background_run_;
  wam::Url default_url_;
  std::string launch_params_;
  std::string load_error_policy_;
  ObserverList<WebPageObserver> observers_;

 private:
  void SetBackgroundColorOfBody(const std::string& color);
  void SetupLaunchEvent();

  bool cleaning_resources_;
  bool is_preload_;
};

#endif  // CORE_WEB_PAGE_BASE_H_
