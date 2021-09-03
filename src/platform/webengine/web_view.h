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

#ifndef PLATFORM_WEBENGINE_WEB_VIEW_H_
#define PLATFORM_WEBENGINE_WEB_VIEW_H_

#include <string>

#include "webos/webview_base.h"

namespace content {
class WebContents;
}

class WebOSEvent;
class WebPageBlinkDelegate;

class WebView {
 public:
  virtual ~WebView() = default;
  WebView() = default;

  virtual void AddUserScript(const std::string& script) = 0;
  virtual void ClearUserScripts() = 0;
  virtual void ExecuteUserScripts() = 0;
  virtual void SetDelegate(WebPageBlinkDelegate* delegate) = 0;
  virtual WebPageBlinkDelegate* Delegate() = 0;
  virtual int Progress() = 0;

  virtual void Initialize(const std::string& app_id,
                          const std::string& app_path,
                          const std::string& trust_level,
                          const std::string& v8_snapshot_path,
                          const std::string& v8_extra_flags,
                          bool use_native_scroll) = 0;

  virtual void AddUserStyleSheet(const std::string& sheet) = 0;
  virtual std::string DefaultUserAgent() const = 0;
  virtual std::string UserAgent() const = 0;
  virtual void LoadUrl(const std::string& url) = 0;
  virtual void StopLoading() = 0;
  virtual void LoadExtension(const std::string& name) = 0;
  virtual void ClearExtensions() = 0;
  virtual void EnableInspectablePage() = 0;
  virtual void DisableInspectablePage() = 0;
  virtual void SetInspectable(bool enable) = 0;
  virtual void AddAvailablePluginDir(const std::string& directory) = 0;
  virtual void AddCustomPluginDir(const std::string& directory) = 0;
  virtual void SetUserAgent(const std::string& useragent) = 0;
  virtual void SetBackgroundColor(int r, int g, int b, int alpha) = 0;
  virtual void SetShouldSuppressDialogs(bool suppress) = 0;
  virtual void SetUseAccessibility(bool enabled) = 0;
  virtual void SetActiveOnNonBlankPaint(bool active) = 0;
  virtual void SetViewportSize(int width, int height) = 0;
  virtual void NotifyMemoryPressure(
      webos::WebViewBase::MemoryPressureLevel level) = 0;
  virtual void SetVisible(bool visible) = 0;
  virtual void SetPrerenderState();
  virtual void SetVisibilityState(
      webos::WebViewBase::WebPageVisibilityState visibilityState) = 0;
  virtual void DeleteWebStorages(const std::string& identifier) = 0;
  virtual std::string DocumentTitle() const = 0;
  virtual void SuspendWebPageDOM() = 0;
  virtual void ResumeWebPageDOM() = 0;
  virtual void SuspendWebPageMedia() = 0;
  virtual void ResumeWebPageMedia() = 0;
  virtual void SuspendPaintingAndSetVisibilityHidden() = 0;
  virtual void ResumePaintingAndSetVisibilityVisible() = 0;
  virtual void CommitLoadVisually() = 0;
  virtual void RunJavaScript(const std::string& js_code) = 0;
  virtual void RunJavaScriptInAllFrames(const std::string& js_code) = 0;
  virtual void Reload() = 0;
  virtual int RenderProcessPid() const = 0;
  virtual bool IsDrmEncrypted(const std::string& url) = 0;
  virtual std::string DecryptDrm(const std::string& url) = 0;
  virtual void SetFocus(bool focus) = 0;
  virtual double GetZoomFactor() = 0;
  virtual void SetZoomFactor(double factor) = 0;
  virtual void SetDoNotTrack(bool dnt) = 0;
  virtual void ForwardWebOSEvent(WebOSEvent* event) = 0;
  virtual bool CanGoBack() const = 0;
  virtual void GoBack() = 0;
  virtual bool IsInputMethodActive() = 0;
  virtual void SetAdditionalContentsScale(float scale_x, float scale_y) = 0;
  virtual void SetHardwareResolution(int width, int height) = 0;
  virtual void SetEnableHtmlSystemKeyboardAttr(bool enabled) = 0;
  virtual void RequestInjectionLoading(const std::string& injection_name) = 0;
  virtual void DropAllPeerConnections(
      webos::DropPeerConnectionReason reason) = 0;
  virtual void ActivateRendererCompositor() = 0;
  virtual void DeactivateRendererCompositor() = 0;
  virtual const std::string& GetUrl() = 0;
  virtual void UpdatePreferences() = 0;
  virtual void ResetStateToMarkNextPaint() = 0;
  virtual void SetAllowRunningInsecureContent(bool enable) = 0;
  virtual void SetAllowScriptsToCloseWindows(bool enable) = 0;
  virtual void SetAllowUniversalAccessFromFileUrls(bool enable) = 0;
  virtual void SetRequestQuotaEnabled(bool enable) = 0;
  virtual void SetSuppressesIncrementalRendering(bool enable) = 0;
  virtual void SetDisallowScrollbarsInMainFrame(bool enable) = 0;
  virtual void SetDisallowScrollingInMainFrame(bool enable) = 0;
  virtual void SetJavascriptCanOpenWindows(bool enable) = 0;
  virtual void SetSpatialNavigationEnabled(bool enable) = 0;
  virtual void SetSupportsMultipleWindows(bool enable) = 0;
  virtual void SetCSSNavigationEnabled(bool enable) = 0;
  virtual void SetV8DateUseSystemLocaloffset(bool use) = 0;
  virtual void SetAllowLocalResourceLoad(bool enable) = 0;
  virtual void SetLocalStorageEnabled(bool enable) = 0;
  virtual void SetDatabaseIdentifier(const std::string& identifier) = 0;
  virtual void SetWebSecurityEnabled(bool enable) = 0;
  virtual void SetKeepAliveWebApp(bool enable) = 0;
  virtual void SetAdditionalFontFamilyEnabled(bool enable) = 0;
  virtual void SetNetworkStableTimeout(double timeout) = 0;
  virtual void SetAllowFakeBoldText(bool allow) = 0;
  virtual void SetAppId(const std::string& appId) = 0;
  virtual void SetSecurityOrigin(const std::string& identifier) = 0;
  virtual void SetAcceptLanguages(const std::string& lauguages) = 0;
  virtual void SetBoardType(const std::string& board_type) = 0;
  virtual void SetMediaCodecCapability(const std::string& capability) = 0;
  virtual void SetMediaPreferences(const std::string& preferences) = 0;
  virtual void SetSearchKeywordForCustomPlayer(bool enabled) = 0;
  virtual void SetUseUnlimitedMediaPolicy(bool enabled) = 0;
  virtual void SetAudioGuidanceOn(bool on) = 0;
  virtual void SetStandardFontFamily(const std::string& font) = 0;
  virtual void SetFixedFontFamily(const std::string& font) = 0;
  virtual void SetSerifFontFamily(const std::string& font) = 0;
  virtual void SetSansSerifFontFamily(const std::string& font) = 0;
  virtual void SetCursiveFontFamily(const std::string& font) = 0;
  virtual void SetFantasyFontFamily(const std::string& font) = 0;
  virtual void LoadAdditionalFont(const std::string& url,
                                  const std::string& font) = 0;
  virtual void SetUseLaunchOptimization(bool enabled, int delay_ms) = 0;
  virtual void SetUseEnyoOptimization(bool enabled) = 0;
  virtual void SetAppPreloadHint(bool is_preload) = 0;
  virtual void SetTransparentBackground(bool enabled) = 0;
  virtual void SetBackHistoryAPIDisabled(const bool on) = 0;
  virtual content::WebContents* GetWebContents() = 0;
};

#endif  // PLATFORM_WEBENGINE_WEB_VIEW_H_
