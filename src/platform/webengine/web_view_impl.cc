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

#include "web_view_impl.h"

#include "blink_web_view.h"

WebViewImpl::WebViewImpl(std::unique_ptr<BlinkWebView> web_view)
    : web_view_(std::move(web_view)) {}

WebViewImpl::~WebViewImpl() = default;

void WebViewImpl::AddUserScript(const std::string& script) {
  if (web_view_)
    web_view_->AddUserScript(script);
}

void WebViewImpl::ClearUserScripts() {
  if (web_view_)
    web_view_->ClearUserScripts();
}

void WebViewImpl::ExecuteUserScripts() {
  if (web_view_)
    web_view_->ExecuteUserScripts();
}

void WebViewImpl::SetDelegate(WebPageBlinkDelegate* delegate) {
  if (web_view_)
    web_view_->SetDelegate(delegate);
}

WebPageBlinkDelegate* WebViewImpl::Delegate() {
  return web_view_ ? web_view_->Delegate() : nullptr;
}

int WebViewImpl::Progress() {
  return web_view_ ? web_view_->Progress() : 0;
}

void WebViewImpl::Initialize(const std::string& app_id,
                             const std::string& app_path,
                             const std::string& trust_level,
                             const std::string& v8_snapshot_path,
                             const std::string& v8_extra_flags,
                             bool use_native_scroll) {
  if (web_view_)
    web_view_->Initialize(app_id, app_path, trust_level, v8_snapshot_path,
                          v8_extra_flags, use_native_scroll);
}

void WebViewImpl::AddUserStyleSheet(const std::string& sheet) {
  if (web_view_)
    web_view_->AddUserStyleSheet(sheet);
}

std::string WebViewImpl::DefaultUserAgent() const {
  return web_view_ ? web_view_->DefaultUserAgent() : "";
}

std::string WebViewImpl::UserAgent() const {
  return web_view_ ? web_view_->UserAgent() : "";
}

void WebViewImpl::LoadUrl(const std::string& url) {
  if (web_view_)
    web_view_->LoadUrl(url);
}

void WebViewImpl::StopLoading() {
  if (web_view_)
    web_view_->StopLoading();
}

void WebViewImpl::LoadExtension(const std::string& name) {
  if (web_view_)
    web_view_->LoadExtension(name);
}

void WebViewImpl::ClearExtensions() {
  if (web_view_)
    web_view_->ClearExtensions();
}

void WebViewImpl::EnableInspectablePage() {
  if (web_view_)
    web_view_->EnableInspectablePage();
}

void WebViewImpl::DisableInspectablePage() {
  if (web_view_)
    web_view_->DisableInspectablePage();
}

void WebViewImpl::SetInspectable(bool enable) {
  if (web_view_)
    web_view_->SetInspectable(enable);
}

void WebViewImpl::AddAvailablePluginDir(const std::string& directory) {
  if (web_view_)
    web_view_->AddAvailablePluginDir(directory);
}

void WebViewImpl::AddCustomPluginDir(const std::string& directory) {
  if (web_view_)
    web_view_->AddCustomPluginDir(directory);
}

void WebViewImpl::SetUserAgent(const std::string& useragent) {
  if (web_view_)
    web_view_->SetUserAgent(useragent);
}

void WebViewImpl::SetBackgroundColor(int r, int g, int b, int alpha) {
  if (web_view_)
    web_view_->SetBackgroundColor(r, g, b, alpha);
}

void WebViewImpl::SetShouldSuppressDialogs(bool suppress) {
  if (web_view_)
    web_view_->SetShouldSuppressDialogs(suppress);
}

void WebViewImpl::SetUseAccessibility(bool enabled) {
  if (web_view_)
    web_view_->SetUseAccessibility(enabled);
}

void WebViewImpl::SetActiveOnNonBlankPaint(bool active) {
  if (web_view_)
    web_view_->SetActiveOnNonBlankPaint(active);
}

void WebViewImpl::SetViewportSize(int width, int height) {
  if (web_view_)
    web_view_->SetViewportSize(width, height);
}

void WebViewImpl::NotifyMemoryPressure(
    webos::WebViewBase::MemoryPressureLevel level) {
  if (web_view_)
    web_view_->NotifyMemoryPressure(level);
}

void WebViewImpl::SetVisible(bool visible) {
  if (web_view_)
    web_view_->SetVisible(visible);
}

void WebViewImpl::SetPrerenderState() {
  if (web_view_)
    web_view_->SetPrerenderState();
}

void WebViewImpl::SetVisibilityState(
    webos::WebViewBase::WebPageVisibilityState visibilityState) {
  if (web_view_)
    web_view_->SetVisibilityState(visibilityState);
}

void WebViewImpl::DeleteWebStorages(const std::string& identifier) {
  if (web_view_)
    web_view_->DeleteWebStorages(identifier);
}

std::string WebViewImpl::DocumentTitle() const {
  return web_view_ ? web_view_->DocumentTitle() : "";
}

void WebViewImpl::SuspendWebPageDOM() {
  if (web_view_)
    web_view_->SuspendWebPageDOM();
}

void WebViewImpl::ResumeWebPageDOM() {
  if (web_view_)
    web_view_->ResumeWebPageDOM();
}

void WebViewImpl::SuspendWebPageMedia() {
  if (web_view_)
    web_view_->SuspendWebPageMedia();
}

void WebViewImpl::ResumeWebPageMedia() {
  if (web_view_)
    web_view_->ResumeWebPageMedia();
}

void WebViewImpl::SuspendPaintingAndSetVisibilityHidden() {
  if (web_view_)
    web_view_->SuspendPaintingAndSetVisibilityHidden();
}

void WebViewImpl::ResumePaintingAndSetVisibilityVisible() {
  if (web_view_)
    web_view_->ResumePaintingAndSetVisibilityVisible();
}

void WebViewImpl::CommitLoadVisually() {
  if (web_view_)
    web_view_->CommitLoadVisually();
}

void WebViewImpl::RunJavaScript(const std::string& js_code) {
  if (web_view_)
    web_view_->RunJavaScript(js_code);
}

void WebViewImpl::RunJavaScriptInAllFrames(const std::string& js_code) {
  if (web_view_)
    web_view_->RunJavaScriptInAllFrames(js_code);
}

void WebViewImpl::Reload() {
  if (web_view_)
    web_view_->Reload();
}

int WebViewImpl::RenderProcessPid() const {
  return web_view_ ? web_view_->RenderProcessPid() : 0;
}

bool WebViewImpl::IsDrmEncrypted(const std::string& url) {
  return web_view_ ? web_view_->IsDrmEncrypted(url) : false;
}

std::string WebViewImpl::DecryptDrm(const std::string& url) {
  return web_view_ ? web_view_->DecryptDrm(url) : "";
}

void WebViewImpl::SetFocus(bool focus) {
  if (web_view_)
    web_view_->SetFocus(focus);
}

double WebViewImpl::GetZoomFactor() {
  return web_view_ ? web_view_->GetZoomFactor() : 0;
}

void WebViewImpl::SetZoomFactor(double factor) {
  if (web_view_)
    web_view_->SetZoomFactor(factor);
}

void WebViewImpl::SetDoNotTrack(bool dnt) {
  if (web_view_)
    web_view_->SetDoNotTrack(dnt);
}

void WebViewImpl::ForwardWebOSEvent(WebOSEvent* event) {
  if (web_view_)
    web_view_->ForwardWebOSEvent(event);
}

bool WebViewImpl::CanGoBack() const {
  return web_view_ ? web_view_->CanGoBack() : false;
}

void WebViewImpl::GoBack() {
  if (web_view_)
    web_view_->GoBack();
}

bool WebViewImpl::IsInputMethodActive() {
  return web_view_ ? web_view_->IsInputMethodActive() : false;
}

void WebViewImpl::SetAdditionalContentsScale(float scale_x, float scale_y) {
  if (web_view_)
    web_view_->SetAdditionalContentsScale(scale_x, scale_y);
}

void WebViewImpl::SetHardwareResolution(int width, int height) {
  if (web_view_)
    web_view_->SetHardwareResolution(width, height);
}

void WebViewImpl::SetEnableHtmlSystemKeyboardAttr(bool enabled) {
  if (web_view_)
    web_view_->SetEnableHtmlSystemKeyboardAttr(enabled);
}

void WebViewImpl::RequestInjectionLoading(const std::string& injection_name) {
  if (web_view_)
    web_view_->RequestInjectionLoading(injection_name);
}

void WebViewImpl::DropAllPeerConnections(
    webos::DropPeerConnectionReason reason) {
  if (web_view_)
    web_view_->DropAllPeerConnections(reason);
}

void WebViewImpl::ActivateRendererCompositor() {
  if (web_view_)
    web_view_->ActivateRendererCompositor();
}

void WebViewImpl::DeactivateRendererCompositor() {
  if (web_view_)
    web_view_->DeactivateRendererCompositor();
}

const std::string& WebViewImpl::GetUrl() {
  if (web_view_) {
    return web_view_->GetUrl();
  }
  static const std::string result("");
  return result;
}

void WebViewImpl::UpdatePreferences() {
  if (web_view_)
    web_view_->UpdatePreferences();
}

void WebViewImpl::ResetStateToMarkNextPaint() {
  if (web_view_)
    web_view_->ResetStateToMarkNextPaint();
}

void WebViewImpl::SetAllowRunningInsecureContent(bool enable) {
  if (web_view_)
    web_view_->SetAllowRunningInsecureContent(enable);
}

void WebViewImpl::SetAllowScriptsToCloseWindows(bool enable) {
  if (web_view_)
    web_view_->SetAllowScriptsToCloseWindows(enable);
}

void WebViewImpl::SetAllowUniversalAccessFromFileUrls(bool enable) {
  if (web_view_)
    web_view_->SetAllowUniversalAccessFromFileUrls(enable);
}

void WebViewImpl::SetRequestQuotaEnabled(bool enable) {
  if (web_view_)
    web_view_->SetRequestQuotaEnabled(enable);
}

void WebViewImpl::SetSuppressesIncrementalRendering(bool enable) {
  if (web_view_)
    web_view_->SetSuppressesIncrementalRendering(enable);
}

void WebViewImpl::SetDisallowScrollbarsInMainFrame(bool enable) {
  if (web_view_)
    web_view_->SetDisallowScrollbarsInMainFrame(enable);
}

void WebViewImpl::SetDisallowScrollingInMainFrame(bool enable) {
  if (web_view_)
    web_view_->SetDisallowScrollingInMainFrame(enable);
}

void WebViewImpl::SetJavascriptCanOpenWindows(bool enable) {
  if (web_view_)
    web_view_->SetJavascriptCanOpenWindows(enable);
}

void WebViewImpl::SetSpatialNavigationEnabled(bool enable) {
  if (web_view_)
    web_view_->SetSpatialNavigationEnabled(enable);
}

void WebViewImpl::SetSupportsMultipleWindows(bool enable) {
  if (web_view_)
    web_view_->SetSupportsMultipleWindows(enable);
}

void WebViewImpl::SetCSSNavigationEnabled(bool enable) {
  if (web_view_)
    web_view_->SetCSSNavigationEnabled(enable);
}

void WebViewImpl::SetV8DateUseSystemLocaloffset(bool use) {
  if (web_view_)
    web_view_->SetV8DateUseSystemLocaloffset(use);
}

void WebViewImpl::SetAllowLocalResourceLoad(bool enable) {
  if (web_view_)
    web_view_->SetAllowLocalResourceLoad(enable);
}

void WebViewImpl::SetLocalStorageEnabled(bool enable) {
  if (web_view_)
    web_view_->SetLocalStorageEnabled(enable);
}

void WebViewImpl::SetDatabaseIdentifier(const std::string& identifier) {
  if (web_view_)
    web_view_->SetDatabaseIdentifier(identifier);
}

void WebViewImpl::SetWebSecurityEnabled(bool enable) {
  if (web_view_)
    web_view_->SetWebSecurityEnabled(enable);
}

void WebViewImpl::SetKeepAliveWebApp(bool enable) {
  if (web_view_)
    web_view_->SetKeepAliveWebApp(enable);
}

void WebViewImpl::SetAdditionalFontFamilyEnabled(bool enable) {
  if (web_view_)
    web_view_->SetAdditionalFontFamilyEnabled(enable);
}

void WebViewImpl::SetNetworkStableTimeout(double timeout) {
  if (web_view_)
    web_view_->SetNetworkStableTimeout(timeout);
}

void WebViewImpl::SetAllowFakeBoldText(bool allow) {
  if (web_view_)
    web_view_->SetAllowFakeBoldText(allow);
}

void WebViewImpl::SetAppId(const std::string& appId) {
  if (web_view_)
    web_view_->SetAppId(appId);
}

void WebViewImpl::SetSecurityOrigin(const std::string& identifier) {
  if (web_view_)
    web_view_->SetSecurityOrigin(identifier);
}

void WebViewImpl::SetAcceptLanguages(const std::string& lauguages) {
  if (web_view_)
    web_view_->SetAcceptLanguages(lauguages);
}

void WebViewImpl::SetBoardType(const std::string& board_type) {
  if (web_view_)
    web_view_->SetBoardType(board_type);
}

void WebViewImpl::SetMediaCodecCapability(const std::string& capability) {
  if (web_view_)
    web_view_->SetMediaCodecCapability(capability);
}

void WebViewImpl::SetMediaPreferences(const std::string& preferences) {
  if (web_view_)
    web_view_->SetMediaPreferences(preferences);
}

void WebViewImpl::SetSearchKeywordForCustomPlayer(bool enabled) {
  if (web_view_)
    web_view_->SetSearchKeywordForCustomPlayer(enabled);
}

void WebViewImpl::SetUseUnlimitedMediaPolicy(bool enabled) {
  if (web_view_)
    web_view_->SetUseUnlimitedMediaPolicy(enabled);
}

void WebViewImpl::SetAudioGuidanceOn(bool on) {
  if (web_view_)
    web_view_->SetAudioGuidanceOn(on);
}

void WebViewImpl::SetStandardFontFamily(const std::string& font) {
  if (web_view_)
    web_view_->SetStandardFontFamily(font);
}

void WebViewImpl::SetFixedFontFamily(const std::string& font) {
  if (web_view_)
    web_view_->SetFixedFontFamily(font);
}

void WebViewImpl::SetSerifFontFamily(const std::string& font) {
  if (web_view_)
    web_view_->SetSerifFontFamily(font);
}

void WebViewImpl::SetSansSerifFontFamily(const std::string& font) {
  if (web_view_)
    web_view_->SetSansSerifFontFamily(font);
}

void WebViewImpl::SetCursiveFontFamily(const std::string& font) {
  if (web_view_)
    web_view_->SetCursiveFontFamily(font);
}

void WebViewImpl::SetFantasyFontFamily(const std::string& font) {
  if (web_view_)
    web_view_->SetFantasyFontFamily(font);
}

void WebViewImpl::LoadAdditionalFont(const std::string& url,
                                     const std::string& font) {
  if (web_view_)
    web_view_->LoadAdditionalFont(url, font);
}

void WebViewImpl::SetUseLaunchOptimization(bool enabled, int delay_ms) {
  if (web_view_)
    web_view_->SetUseLaunchOptimization(enabled, delay_ms);
}

void WebViewImpl::SetUseEnyoOptimization(bool enabled) {
  if (web_view_)
    web_view_->SetUseEnyoOptimization(enabled);
}

void WebViewImpl::SetAppPreloadHint(bool is_preload) {
  if (web_view_)
    web_view_->SetAppPreloadHint(is_preload);
}

void WebViewImpl::SetTransparentBackground(bool enabled) {
  if (web_view_)
    web_view_->SetTransparentBackground(enabled);
}

void WebViewImpl::SetBackHistoryAPIDisabled(const bool on) {
  if (web_view_)
    web_view_->SetBackHistoryAPIDisabled(on);
}

content::WebContents* WebViewImpl::GetWebContents() {
  return web_view_ ? web_view_->GetWebContents() : nullptr;
}
