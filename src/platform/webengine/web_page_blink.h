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

#ifndef PLATFORM_WEBENGINE_WEB_PAGE_BLINK_H_
#define PLATFORM_WEBENGINE_WEB_PAGE_BLINK_H_

#include <memory>
#include <string>
#include <vector>

#include "webos/webview_base.h"

#include "timer.h"
#include "web_page_base.h"
#include "web_page_blink_delegate.h"

namespace wam {
class Url;
}

class WebAppBase;
class WebView;
class WebPageBlinkPrivate;
class WebPageBlinkObserver;
class WebViewFactory;

class WebPageBlink : public WebPageBase, public WebPageBlinkDelegate {
 public:
  WebPageBlink(const wam::Url& url,
               std::shared_ptr<ApplicationDescription> desc,
               const std::string& launch_params,
               std::unique_ptr<WebViewFactory> factory);
  WebPageBlink(const wam::Url& url,
               std::shared_ptr<ApplicationDescription> desc,
               const std::string& launch_params);
  ~WebPageBlink() override;

  void SetObserver(WebPageBlinkObserver* observer);

  // WebPageBase
  void Init() override;
  void* GetWebContents() override;
  void SetLaunchParams(const std::string& params) override;
  void NotifyMemoryPressure(
      webos::WebViewBase::MemoryPressureLevel level) override;
  wam::Url Url() const override;
  void LoadUrl(const std::string& url) override;
  int Progress() const override;
  bool HasBeenShown() const override;
  void SetPageProperties() override;
  void SetPreferredLanguages(const std::string& language) override;
  void SetDefaultFont(const std::string& font) override;
  void ReloadDefaultPage() override;
  void Reload() override;
  void SetVisibilityState(WebPageVisibilityState visibility_state) override;
  void SetFocus(bool focus) override;
  std::string Title() override;
  bool CanGoBack() override;
  void CloseVkb() override;
  bool IsInputMethodActive() const override;
  void KeyboardVisibilityChanged(bool visible) override;
  void HandleDeviceInfoChanged(const std::string& device_info) override;
  void EvaluateJavaScript(const std::string& js_code) override;
  void EvaluateJavaScriptInAllFrames(const std::string& js_code,
                                     const char* method = {}) override;
  uint32_t GetWebProcessProxyID() override;
  uint32_t GetWebProcessPID() const override { return RenderProcessPid(); }
  void CreatePalmSystem(WebAppBase* app) override;
  void SetUseLaunchOptimization(bool enabled, int delayMs = 0);
  void SetUseSystemAppOptimization(bool enabled) override;
  void SetUseAccessibility(bool enabled) override;
  void SetAppPreloadHint(bool is_preload) override;
  void SuspendWebPageAll() override;
  void ResumeWebPageAll() override;
  void SuspendWebPageMedia() override;
  void ResumeWebPageMedia() override;
  void ResumeWebPagePaintingAndJSExecution() override;
  bool IsRegisteredCloseCallback() override { return has_close_callback_; }
  void ReloadExtensionData() override;
  void UpdateIsLoadErrorPageFinish() override;
  void UpdateDatabaseIdentifier() override;
  void DeleteWebStorages(const std::string& identfier) override;
  void SetInspectorEnable() override;
  void SetKeepAliveWebApp(bool keepAlive) override;

  // WebPageBlink
  virtual void LoadExtension();
  virtual void ClearExtensions();
  virtual void SetViewportSize();
  virtual void SetHasOnCloseCallback(bool has_close_callback);
  virtual void ExecuteCloseCallback(bool forced);
  virtual void DidRunCloseCallback();
  virtual void CleanResources();

  // WebPageBlinkDelegate
  void Close() override;
  bool DecidePolicyForResponse(bool is_main_frame,
                               int status_code,
                               const std::string& url,
                               const std::string& status_text) override;
  bool AcceptsVideoCapture() override;
  bool AcceptsAudioCapture() override;
  void DidFirstFrameFocused() override;
  void DidDropAllPeerConnections() override;
  void LoadFinished(const std::string& url) override;
  void LoadFailed(const std::string& url,
                  int err_code,
                  const std::string& err_desc) override;
  void LoadStopped() override;
  void LoadAborted(const std::string& url) override;
  void LoadProgressChanged(double progress) override;
  void DidStartNavigation(const std::string& url,
                          bool is_in_main_frame) override;
  void DidFinishNavigation(const std::string& url,
                           bool is_in_main_frame) override;
  void LoadVisuallyCommitted() override;
  void RenderProcessCreated(int pid) override;
  void RenderProcessCrashed() override;
  void TitleChanged(const std::string& title) override;
  void NavigationHistoryChanged() override;
  void DidErrorPageLoadedFromNetErrorHelper() override;

  void UpdateExtensionData(const std::string& key, const std::string& value);
  void SetLoadErrorPolicy(const std::string& policy);
  void SetTrustLevel(const std::string& trust_level) {
    trust_level_ = trust_level;
  }
  std::string TrustLevel() const { return trust_level_; }
  std::string DefaultTrustLevel() const;
  std::string EscapeData(const std::string& value);
  int RenderProcessPid() const;
  static void SetFileAccessBlocked(bool blocked);
  void UpdateBoardType();
  void UpdateMediaCodecCapability();
  double DevicePixelRatio();
  void SetAdditionalContentsScale(float scale_x, float scale_y);
  void SetSupportDolbyHDRContents();
  void UpdateHardwareResolution();

  void ForwardEvent(void* event) override;
  bool AllowMouseOnOffEvent() const override;
  void DidSwapCompositorFrame();
  void ActivateRendererCompositor() override;
  void DeactivateRendererCompositor() override;

  void DidResumeDOM() override;

  // Timer callback
  void TimeoutCloseCallback();

  void SetAudioGuidanceOn(bool on) override;
  void UpdateBackHistoryAPIDisabled();

 protected:
  WebView* PageView() const;

  // WebPageBase
  virtual void LoadDefaultUrl();
  virtual void LoadErrorPage(int error_code);
  virtual WebView* CreatePageView();
  virtual void SetupStaticUserScripts();
  virtual void AddUserScript(const std::string& script);
  virtual void AddUserScriptUrl(const wam::Url& url);
  virtual void RecreateWebView();
  virtual void SetVisible(bool visible);
  virtual bool ShouldStopJSOnSuspend() const { return true; }

  bool Inspectable();

  // WebPageDelegate
  void HandleBrowserControlCommand(
      const std::string& command,
      const std::vector<std::string>& arguments) override;
  void HandleBrowserControlFunction(const std::string& command,
                                    const std::vector<std::string>& arguments,
                                    std::string* result) override;

  std::string HandleBrowserControlMessage(
      const std::string& command,
      const std::vector<std::string>& arguments);

  virtual void SuspendWebPagePaintingAndJSExecution();

 private:
  void SetCustomPluginIfNeeded();
  void SetDisallowScrolling(bool disallow);
  std::vector<std::string> GetErrorPagePath(const std::string& error_page);

 private:
  WebPageBlinkPrivate* page_private_;

  bool is_paused_;
  bool is_suspended_;
  bool has_custom_policy_for_response_;
  bool has_been_shown_;
  OneShotTimer<WebPageBlink> dom_suspend_timer_;
  std::string custom_plugin_path_;
  double vkb_height_;
  bool vkb_was_overlap_;
  bool has_close_callback_;
  OneShotTimer<WebPageBlink> close_callback_timer_;
  std::string trust_level_;
  std::string load_failed_hostname_;
  std::string loading_url_;
  int custom_suspend_dom_time_;

  WebPageBlinkObserver* observer_;

  std::unique_ptr<WebViewFactory> factory_;
};

#endif  // PLATFORM_WEBENGINE_WEB_PAGE_BLINK_H_
