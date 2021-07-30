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
#include "log_manager.h"

WebViewImpl::WebViewImpl(std::unique_ptr<BlinkWebView> web_view)
    : web_view_(std::move(web_view)) {}

WebViewImpl::~WebViewImpl() = default;

void WebViewImpl::AddUserScript(const std::string& script) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: script:%s", __func__,
           script.c_str());
  if (web_view_)
    web_view_->AddUserScript(script);
}

void WebViewImpl::ClearUserScripts() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->ClearUserScripts();
}

void WebViewImpl::ExecuteUserScripts() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->ExecuteUserScripts();
}

void WebViewImpl::SetDelegate(WebPageBlinkDelegate* delegate) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->SetDelegate(delegate);
}

WebPageBlinkDelegate* WebViewImpl::Delegate() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  return web_view_ ? web_view_->Delegate() : nullptr;
}

int WebViewImpl::Progress() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  return web_view_ ? web_view_->Progress() : 0;
}

void WebViewImpl::Initialize(const std::string& app_id,
                             const std::string& app_path,
                             const std::string& trust_level,
                             const std::string& v8_snapshot_path,
                             const std::string& v8_extra_flags,
                             bool use_native_scroll) {
  LOG_INFO(
      MSGID_WAM_DEBUG, 0,
      ">>>>>>> WebViewImpl::%s: app_id:\"%s\" app_path:\"%s\" "
      "trust_level:\"%s\" "
      "v8_snapshot_path:\"%s\" v8_extra_flags:\"%s\", use_native_scroll:%d",
      __func__, app_id.c_str(), app_path.c_str(), trust_level.c_str(),
      v8_snapshot_path.c_str(), v8_extra_flags.c_str(), use_native_scroll);
  if (web_view_)
    web_view_->Initialize(app_id, app_path, trust_level, v8_snapshot_path,
                          v8_extra_flags, use_native_scroll);
}

void WebViewImpl::AddUserStyleSheet(const std::string& sheet) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: sheet:\"%s\"",
           __func__, sheet.c_str());
  if (web_view_)
    web_view_->AddUserStyleSheet(sheet);
}

std::string WebViewImpl::DefaultUserAgent() const {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  return web_view_ ? web_view_->DefaultUserAgent() : "";
}

std::string WebViewImpl::UserAgent() const {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  return web_view_ ? web_view_->UserAgent() : "";
}

void WebViewImpl::LoadUrl(const std::string& url) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: url:\"%s\"", __func__,
           url.c_str());
  if (web_view_)
    web_view_->LoadUrl(url);
}

void WebViewImpl::StopLoading() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->StopLoading();
}

void WebViewImpl::LoadExtension(const std::string& name) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: name:\"%s\"", __func__,
           name.c_str());
  if (web_view_)
    web_view_->LoadExtension(name);
}

void WebViewImpl::ClearExtensions() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->ClearExtensions();
}

void WebViewImpl::EnableInspectablePage() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->EnableInspectablePage();
}

void WebViewImpl::DisableInspectablePage() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->DisableInspectablePage();
}

void WebViewImpl::SetInspectable(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetInspectable(enable);
}

void WebViewImpl::AddAvailablePluginDir(const std::string& directory) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: directory:\"%s\"",
           __func__, directory.c_str());
  if (web_view_)
    web_view_->AddAvailablePluginDir(directory);
}

void WebViewImpl::AddCustomPluginDir(const std::string& directory) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: directory:\"%s\"",
           __func__, directory.c_str());
  if (web_view_)
    web_view_->AddCustomPluginDir(directory);
}

void WebViewImpl::SetUserAgent(const std::string& useragent) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: useragent:\"%s\"",
           __func__, useragent.c_str());
  if (web_view_)
    web_view_->SetUserAgent(useragent);
}

void WebViewImpl::SetBackgroundColor(int r, int g, int b, int alpha) {
  LOG_INFO(MSGID_WAM_DEBUG, 0,
           ">>>>>>> WebViewImpl::%s: r:\"%d\" g:\"%d\" b:\"%d\" alpha:\"%d\"",
           __func__, r, g, b, alpha);
  if (web_view_)
    web_view_->SetBackgroundColor(r, g, b, alpha);
}

void WebViewImpl::SetShouldSuppressDialogs(bool suppress) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: suppress:\"%d\"",
           __func__, suppress);
  if (web_view_)
    web_view_->SetShouldSuppressDialogs(suppress);
}

void WebViewImpl::SetUseAccessibility(bool enabled) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enabled:\"%d\"",
           __func__, enabled);
  if (web_view_)
    web_view_->SetUseAccessibility(enabled);
}

void WebViewImpl::SetActiveOnNonBlankPaint(bool active) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: active:\"%d\"",
           __func__, active);
  if (web_view_)
    web_view_->SetActiveOnNonBlankPaint(active);
}

void WebViewImpl::SetViewportSize(int width, int height) {
  LOG_INFO(MSGID_WAM_DEBUG, 0,
           ">>>>>>> WebViewImpl::%s: width:\"%d\" height:\"%d\"", __func__,
           width, height);
  if (web_view_)
    web_view_->SetViewportSize(width, height);
}

void WebViewImpl::NotifyMemoryPressure(
    webos::WebViewBase::MemoryPressureLevel level) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: level:\"%d\"",
           __func__, level);
  if (web_view_)
    web_view_->NotifyMemoryPressure(level);
}

void WebViewImpl::SetVisible(bool visible) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: visible:\"%d\"",
           __func__, visible);
  if (web_view_)
    web_view_->SetVisible(visible);
}

void WebViewImpl::SetPrerenderState() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->SetPrerenderState();
}

void WebViewImpl::SetVisibilityState(
    webos::WebViewBase::WebPageVisibilityState visibilityState) {
  LOG_INFO(MSGID_WAM_DEBUG, 0,
           ">>>>>>> WebViewImpl::%s: visibilityState:\"%d\"", __func__,
           visibilityState);
  if (web_view_)
    web_view_->SetVisibilityState(visibilityState);
}

void WebViewImpl::DeleteWebStorages(const std::string& identifier) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: identifier:\"%s\"",
           __func__, identifier.c_str());
  if (web_view_)
    web_view_->DeleteWebStorages(identifier);
}

std::string WebViewImpl::DocumentTitle() const {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  return web_view_ ? web_view_->DocumentTitle() : "";
}

void WebViewImpl::SuspendWebPageDOM() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->SuspendWebPageDOM();
}

void WebViewImpl::ResumeWebPageDOM() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->ResumeWebPageDOM();
}

void WebViewImpl::SuspendWebPageMedia() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->SuspendWebPageMedia();
}

void WebViewImpl::ResumeWebPageMedia() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->ResumeWebPageMedia();
}

void WebViewImpl::SuspendPaintingAndSetVisibilityHidden() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->SuspendPaintingAndSetVisibilityHidden();
}

void WebViewImpl::ResumePaintingAndSetVisibilityVisible() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->ResumePaintingAndSetVisibilityVisible();
}

void WebViewImpl::CommitLoadVisually() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->CommitLoadVisually();
}

void WebViewImpl::RunJavaScript(const std::string& js_code) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: js_code:\"%s\"",
           __func__, js_code.c_str());
  if (web_view_)
    web_view_->RunJavaScript(js_code);
}

void WebViewImpl::RunJavaScriptInAllFrames(const std::string& js_code) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: js_code:\"%s\"",
           __func__, js_code.c_str());
  if (web_view_)
    web_view_->RunJavaScriptInAllFrames(js_code);
}

void WebViewImpl::Reload() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->Reload();
}

int WebViewImpl::RenderProcessPid() const {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  return web_view_ ? web_view_->RenderProcessPid() : 0;
}

bool WebViewImpl::IsDrmEncrypted(const std::string& url) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: url:\"%s\"", __func__,
           url.c_str());
  return web_view_ ? web_view_->IsDrmEncrypted(url) : false;
}

std::string WebViewImpl::DecryptDrm(const std::string& url) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: url:\"%s\"", __func__,
           url.c_str());
  return web_view_ ? web_view_->DecryptDrm(url) : "";
}

void WebViewImpl::SetFocus(bool focus) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: focus:\"%d\"",
           __func__, focus);
  if (web_view_)
    web_view_->SetFocus(focus);
}

double WebViewImpl::GetZoomFactor() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  return web_view_ ? web_view_->GetZoomFactor() : 0;
}

void WebViewImpl::SetZoomFactor(double factor) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: factor:\"%f\"",
           __func__, factor);
  if (web_view_)
    web_view_->SetZoomFactor(factor);
}

void WebViewImpl::SetDoNotTrack(bool dnt) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: dnt:\"%d\"", __func__,
           dnt);
  if (web_view_)
    web_view_->SetDoNotTrack(dnt);
}

void WebViewImpl::ForwardWebOSEvent(WebOSEvent* event) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s(WebOSEvent*)",
           __func__);
  if (web_view_)
    web_view_->ForwardWebOSEvent(event);
}

bool WebViewImpl::CanGoBack() const {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  return web_view_ ? web_view_->CanGoBack() : false;
}

void WebViewImpl::GoBack() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->GoBack();
}

bool WebViewImpl::IsInputMethodActive() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  return web_view_ ? web_view_->IsInputMethodActive() : false;
}

void WebViewImpl::SetAdditionalContentsScale(float scale_x, float scale_y) {
  LOG_INFO(MSGID_WAM_DEBUG, 0,
           ">>>>>>> WebViewImpl::%s: scale_x:\"%f\" scale_x:\"%f\"", __func__,
           scale_x, scale_y);
  if (web_view_)
    web_view_->SetAdditionalContentsScale(scale_x, scale_y);
}

void WebViewImpl::SetHardwareResolution(int width, int height) {
  LOG_INFO(MSGID_WAM_DEBUG, 0,
           ">>>>>>> WebViewImpl::%s: width:\"%d\" height:\"%d\"", __func__,
           width, height);
  if (web_view_)
    web_view_->SetHardwareResolution(width, height);
}

void WebViewImpl::SetEnableHtmlSystemKeyboardAttr(bool enabled) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enabled:\"%d\"",
           __func__, enabled);
  if (web_view_)
    web_view_->SetEnableHtmlSystemKeyboardAttr(enabled);
}

void WebViewImpl::RequestInjectionLoading(const std::string& injection_name) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: injection_name:\"%s\"",
           __func__, injection_name.c_str());
  if (web_view_)
    web_view_->RequestInjectionLoading(injection_name);
}

void WebViewImpl::DropAllPeerConnections(
    webos::DropPeerConnectionReason reason) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: reason:\"%d\"",
           __func__, reason);
  if (web_view_)
    web_view_->DropAllPeerConnections(reason);
}

void WebViewImpl::ActivateRendererCompositor() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->ActivateRendererCompositor();
}

void WebViewImpl::DeactivateRendererCompositor() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->DeactivateRendererCompositor();
}

const std::string& WebViewImpl::GetUrl() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_) {
    return web_view_->GetUrl();
  }
  static const std::string result("");
  return result;
}

void WebViewImpl::UpdatePreferences() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->UpdatePreferences();
}

void WebViewImpl::ResetStateToMarkNextPaint() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  if (web_view_)
    web_view_->ResetStateToMarkNextPaint();
}

void WebViewImpl::SetAllowRunningInsecureContent(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetAllowRunningInsecureContent(enable);
}

void WebViewImpl::SetAllowScriptsToCloseWindows(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetAllowScriptsToCloseWindows(enable);
}

void WebViewImpl::SetAllowUniversalAccessFromFileUrls(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetAllowUniversalAccessFromFileUrls(enable);
}

void WebViewImpl::SetRequestQuotaEnabled(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetRequestQuotaEnabled(enable);
}

void WebViewImpl::SetSuppressesIncrementalRendering(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetSuppressesIncrementalRendering(enable);
}

void WebViewImpl::SetDisallowScrollbarsInMainFrame(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetDisallowScrollbarsInMainFrame(enable);
}

void WebViewImpl::SetDisallowScrollingInMainFrame(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetDisallowScrollingInMainFrame(enable);
}

void WebViewImpl::SetJavascriptCanOpenWindows(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetJavascriptCanOpenWindows(enable);
}

void WebViewImpl::SetSpatialNavigationEnabled(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetSpatialNavigationEnabled(enable);
}

void WebViewImpl::SetSupportsMultipleWindows(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetSupportsMultipleWindows(enable);
}

void WebViewImpl::SetCSSNavigationEnabled(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetCSSNavigationEnabled(enable);
}

void WebViewImpl::SetV8DateUseSystemLocaloffset(bool use) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: use:\"%d\"", __func__,
           use);
  if (web_view_)
    web_view_->SetV8DateUseSystemLocaloffset(use);
}

void WebViewImpl::SetAllowLocalResourceLoad(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetAllowLocalResourceLoad(enable);
}

void WebViewImpl::SetLocalStorageEnabled(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetLocalStorageEnabled(enable);
}

void WebViewImpl::SetDatabaseIdentifier(const std::string& identifier) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%s\"",
           __func__, identifier.c_str());
  if (web_view_)
    web_view_->SetDatabaseIdentifier(identifier);
}

void WebViewImpl::SetWebSecurityEnabled(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetWebSecurityEnabled(enable);
}

void WebViewImpl::SetKeepAliveWebApp(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetKeepAliveWebApp(enable);
}

void WebViewImpl::SetAdditionalFontFamilyEnabled(bool enable) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%d\"",
           __func__, enable);
  if (web_view_)
    web_view_->SetAdditionalFontFamilyEnabled(enable);
}

void WebViewImpl::SetNetworkStableTimeout(double timeout) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enable:\"%f\"",
           __func__, timeout);
  if (web_view_)
    web_view_->SetNetworkStableTimeout(timeout);
}

void WebViewImpl::SetAllowFakeBoldText(bool allow) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: allow:\"%d\"",
           __func__, allow);
  if (web_view_)
    web_view_->SetAllowFakeBoldText(allow);
}

void WebViewImpl::SetAppId(const std::string& appId) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: appId:\"%s\"",
           __func__, appId.c_str());
  if (web_view_)
    web_view_->SetAppId(appId);
}

void WebViewImpl::SetSecurityOrigin(const std::string& identifier) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: identifier:\"%s\"",
           __func__, identifier.c_str());
  if (web_view_)
    web_view_->SetSecurityOrigin(identifier);
}

void WebViewImpl::SetAcceptLanguages(const std::string& lauguages) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: lauguages:\"%s\"",
           __func__, lauguages.c_str());
  if (web_view_)
    web_view_->SetAcceptLanguages(lauguages);
}

void WebViewImpl::SetBoardType(const std::string& board_type) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: board_type:\"%s\"",
           __func__, board_type.c_str());
  if (web_view_)
    web_view_->SetBoardType(board_type);
}

void WebViewImpl::SetMediaCodecCapability(const std::string& capability) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: capability:\"%s\"",
           __func__, capability.c_str());
  if (web_view_)
    web_view_->SetMediaCodecCapability(capability);
}

void WebViewImpl::SetMediaPreferences(const std::string& preferences) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: preferences:\"%s\"",
           __func__, preferences.c_str());
  if (web_view_)
    web_view_->SetMediaPreferences(preferences);
}

void WebViewImpl::SetSearchKeywordForCustomPlayer(bool enabled) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enabled:\"%d\"",
           __func__, enabled);
  if (web_view_)
    web_view_->SetSearchKeywordForCustomPlayer(enabled);
}

void WebViewImpl::SetUseUnlimitedMediaPolicy(bool enabled) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enabled:\"%d\"",
           __func__, enabled);
  if (web_view_)
    web_view_->SetUseUnlimitedMediaPolicy(enabled);
}

void WebViewImpl::SetAudioGuidanceOn(bool on) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: on:\"%d\"", __func__,
           on);
  if (web_view_)
    web_view_->SetAudioGuidanceOn(on);
}

void WebViewImpl::SetStandardFontFamily(const std::string& font) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: font:\"%s\"", __func__,
           font.c_str());
  if (web_view_)
    web_view_->SetStandardFontFamily(font);
}

void WebViewImpl::SetFixedFontFamily(const std::string& font) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: font:\"%s\"", __func__,
           font.c_str());
  if (web_view_)
    web_view_->SetFixedFontFamily(font);
}

void WebViewImpl::SetSerifFontFamily(const std::string& font) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: font:\"%s\"", __func__,
           font.c_str());
  if (web_view_)
    web_view_->SetSerifFontFamily(font);
}

void WebViewImpl::SetSansSerifFontFamily(const std::string& font) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: font:\"%s\"", __func__,
           font.c_str());
  if (web_view_)
    web_view_->SetSansSerifFontFamily(font);
}

void WebViewImpl::SetCursiveFontFamily(const std::string& font) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: font:\"%s\"", __func__,
           font.c_str());
  if (web_view_)
    web_view_->SetCursiveFontFamily(font);
}

void WebViewImpl::SetFantasyFontFamily(const std::string& font) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: font:\"%s\"", __func__,
           font.c_str());
  if (web_view_)
    web_view_->SetFantasyFontFamily(font);
}

void WebViewImpl::LoadAdditionalFont(const std::string& url,
                                     const std::string& font) {
  LOG_INFO(MSGID_WAM_DEBUG, 0,
           ">>>>>>> WebViewImpl::%s: url:\"%s\" font:\"%s\"", __func__,
           url.c_str(), font.c_str());
  if (web_view_)
    web_view_->LoadAdditionalFont(url, font);
}

void WebViewImpl::SetUseLaunchOptimization(bool enabled, int delay_ms) {
  LOG_INFO(MSGID_WAM_DEBUG, 0,
           ">>>>>>> WebViewImpl::%s: enabled:\"%d\" delay_ms:\"%d\"", __func__,
           enabled, delay_ms);
  if (web_view_)
    web_view_->SetUseLaunchOptimization(enabled, delay_ms);
}

void WebViewImpl::SetUseEnyoOptimization(bool enabled) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enabled:\"%d\"",
           __func__, enabled);
  if (web_view_)
    web_view_->SetUseEnyoOptimization(enabled);
}

void WebViewImpl::SetAppPreloadHint(bool is_preload) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: is_preload:\"%d\"",
           __func__, is_preload);
  if (web_view_)
    web_view_->SetAppPreloadHint(is_preload);
}

void WebViewImpl::SetTransparentBackground(bool enabled) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: enabled:\"%d\"",
           __func__, enabled);
  if (web_view_)
    web_view_->SetTransparentBackground(enabled);
}

void WebViewImpl::SetBackHistoryAPIDisabled(const bool on) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s: on:\"%d\"", __func__,
           on);
  if (web_view_)
    web_view_->SetBackHistoryAPIDisabled(on);
}

content::WebContents* WebViewImpl::GetWebContents() {
  LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebViewImpl::%s", __func__);
  return web_view_ ? web_view_->GetWebContents() : nullptr;
}
