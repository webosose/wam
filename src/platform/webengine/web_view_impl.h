// Copyright (c) 2021 LG Electronics, Inc.
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

#ifndef WEBVIEWIMPL_H
#define WEBVIEWIMPL_H

#include <memory>
#include <string>

#include "web_view.h"

class WebOSEvent;
class WebPageBlinkDelegate;
class BlinkWebView;

class WebViewImpl : public WebView {
 public:
  WebViewImpl(std::unique_ptr<BlinkWebView> webview);
  ~WebViewImpl() override;

  void AddUserScript(const std::string& script) override;
  void ClearUserScripts() override;
  void ExecuteUserScripts() override;
  void SetDelegate(WebPageBlinkDelegate* delegate) override;
  WebPageBlinkDelegate* Delegate() override;
  int Progress() override;

  void Initialize(const std::string& app_id,
                  const std::string& app_path,
                  const std::string& trust_level,
                  const std::string& v8_snapshot_path,
                  const std::string& v8_extra_flags,
                  bool use_native_scroll) override;

  void AddUserStyleSheet(const std::string& sheet) override;
  std::string DefaultUserAgent() const override;
  std::string UserAgent() const override;
  void LoadUrl(const std::string& url) override;
  void StopLoading() override;
  void LoadExtension(const std::string& name) override;
  void ClearExtensions() override;
  void EnableInspectablePage() override;
  void DisableInspectablePage() override;
  void SetInspectable(bool enable) override;
  void AddAvailablePluginDir(const std::string& directory) override;
  void AddCustomPluginDir(const std::string& directory) override;
  void SetUserAgent(const std::string& useragent) override;
  void SetBackgroundColor(int r, int g, int b, int alpha) override;
  void SetShouldSuppressDialogs(bool suppress) override;
  void SetUseAccessibility(bool enabled) override;
  void SetActiveOnNonBlankPaint(bool active) override;
  void SetViewportSize(int width, int height) override;
  void NotifyMemoryPressure(
      webos::WebViewBase::MemoryPressureLevel level) override;
  void SetVisible(bool visible) override;
  void SetPrerenderState();
  void SetVisibilityState(
      webos::WebViewBase::WebPageVisibilityState visibility_state) override;
  void DeleteWebStorages(const std::string& identifier) override;
  std::string DocumentTitle() const override;
  void SuspendWebPageDOM() override;
  void ResumeWebPageDOM() override;
  void SuspendWebPageMedia() override;
  void ResumeWebPageMedia() override;
  void SuspendPaintingAndSetVisibilityHidden() override;
  void ResumePaintingAndSetVisibilityVisible() override;
  void CommitLoadVisually() override;
  void RunJavaScript(const std::string& js_code) override;
  void RunJavaScriptInAllFrames(const std::string& js_code) override;
  void Reload() override;
  int RenderProcessPid() const override;
  bool IsDrmEncrypted(const std::string& url) override;
  std::string DecryptDrm(const std::string& url) override;
  void SetFocus(bool focus) override;
  double GetZoomFactor() override;
  void SetZoomFactor(double factor) override;
  void SetDoNotTrack(bool dnt) override;
  void ForwardWebOSEvent(WebOSEvent* event) override;
  bool CanGoBack() const override;
  void GoBack() override;
  bool IsInputMethodActive() override;
  void SetAdditionalContentsScale(float scale_x, float scale_y) override;
  void SetHardwareResolution(int width, int height) override;
  void SetEnableHtmlSystemKeyboardAttr(bool enabled) override;
  void RequestInjectionLoading(const std::string& injection_name) override;
  void DropAllPeerConnections(webos::DropPeerConnectionReason reason) override;
  void ActivateRendererCompositor() override;
  void DeactivateRendererCompositor() override;
  const std::string& GetUrl() override;
  void UpdatePreferences() override;
  void ResetStateToMarkNextPaint() override;
  void SetAllowRunningInsecureContent(bool enable) override;
  void SetAllowScriptsToCloseWindows(bool enable) override;
  void SetAllowUniversalAccessFromFileUrls(bool enable) override;
  void SetRequestQuotaEnabled(bool enable) override;
  void SetSuppressesIncrementalRendering(bool enable) override;
  void SetDisallowScrollbarsInMainFrame(bool enable) override;
  void SetDisallowScrollingInMainFrame(bool enable) override;
  void SetJavascriptCanOpenWindows(bool enable) override;
  void SetSpatialNavigationEnabled(bool enable) override;
  void SetSupportsMultipleWindows(bool enable) override;
  void SetCSSNavigationEnabled(bool enable) override;
  void SetV8DateUseSystemLocaloffset(bool use) override;
  void SetAllowLocalResourceLoad(bool enable) override;
  void SetLocalStorageEnabled(bool enable) override;
  void SetDatabaseIdentifier(const std::string& identifier) override;
  void SetWebSecurityEnabled(bool enable) override;
  void SetKeepAliveWebApp(bool enable) override;
  void SetAdditionalFontFamilyEnabled(bool enable) override;
  void SetNetworkStableTimeout(double timeout) override;
  void SetAllowFakeBoldText(bool allow) override;
  void SetAppId(const std::string& appId) override;
  void SetSecurityOrigin(const std::string& identifier) override;
  void SetAcceptLanguages(const std::string& lauguages) override;
  void SetBoardType(const std::string& board_type) override;
  void SetMediaCodecCapability(const std::string& capability) override;
  void SetMediaPreferences(const std::string& preferences) override;
  void SetSearchKeywordForCustomPlayer(bool enabled) override;
  void SetUseUnlimitedMediaPolicy(bool enabled) override;
  void SetAudioGuidanceOn(bool on) override;
  void SetStandardFontFamily(const std::string& font) override;
  void SetFixedFontFamily(const std::string& font) override;
  void SetSerifFontFamily(const std::string& font) override;
  void SetSansSerifFontFamily(const std::string& font) override;
  void SetCursiveFontFamily(const std::string& font) override;
  void SetFantasyFontFamily(const std::string& font) override;
  void LoadAdditionalFont(const std::string& url,
                          const std::string& font) override;
  void SetUseLaunchOptimization(bool enabled, int delay_ms) override;
  void SetUseEnyoOptimization(bool enabled) override;
  void SetAppPreloadHint(bool is_preload) override;
  void SetTransparentBackground(bool enabled) override;
  void SetBackHistoryAPIDisabled(const bool on) override;
  content::WebContents* GetWebContents() override;

 private:
  std::unique_ptr<BlinkWebView> web_view_;
};

#endif
