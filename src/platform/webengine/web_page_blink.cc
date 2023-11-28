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

#include "web_page_blink.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

#include <json/json.h>

#include "application_description.h"
#include "blink_web_process_manager.h"
#include "blink_web_view.h"
#include "log_manager.h"
#include "palm_system_blink.h"
#include "url.h"
#include "utils.h"
#include "web_app_manager_config.h"
#include "web_app_manager_tracer.h"
#include "web_app_manager_utils.h"
#include "web_page_blink_observer.h"
#include "web_page_observer.h"
#include "web_view.h"
#include "web_view_factory.h"
#include "web_view_impl.h"

/**
 * Hide dirty implementation details from
 * public API
 */

static const int kExecuteCloseCallbackTimeOutMs = 10000;
static const int kReloadTimeoutMs = 60000;

class WebPageBlinkPrivate {
 public:
  explicit WebPageBlinkPrivate(WebPageBlink* page) : page_(page) {}

  ~WebPageBlinkPrivate() = default;

  WebPageBlink* page_;
  std::unique_ptr<WebView> page_view_;
  std::unique_ptr<PalmSystemBlink> palm_system_;
};

WebPageBlink::WebPageBlink(const wam::Url& url,
                           std::shared_ptr<ApplicationDescription> desc,
                           const std::string& params,
                           std::unique_ptr<WebViewFactory> factory)
    : WebPageBase(url, desc, params),
      page_private_(std::make_unique<WebPageBlinkPrivate>(this)),
      trust_level_(desc->TrustLevel()),
      factory_(std::move(factory)) {}

WebPageBlink::WebPageBlink(const wam::Url& url,
                           std::shared_ptr<ApplicationDescription> desc,
                           const std::string& params)
    : WebPageBlink(url, std::move(desc), params, nullptr) {}

WebPageBlink::~WebPageBlink() {
  if (dom_suspend_timer_.IsRunning()) {
    dom_suspend_timer_.Stop();
  }
}

void WebPageBlink::Init() {
  page_private_->page_view_ = std::unique_ptr<WebView>(CreatePageView());
  page_private_->page_view_->SetDelegate(this);
  page_private_->page_view_->Initialize(
      app_desc_->Id() + std::to_string(app_desc_->GetDisplayAffinity()),
      app_desc_->FolderPath(), app_desc_->TrustLevel(),
      app_desc_->V8SnapshotPath(), app_desc_->V8ExtraFlags(),
      app_desc_->UseNativeScroll());
  SetViewportSize();

  page_private_->page_view_->SetVisible(false);
  page_private_->page_view_->SetUserAgent(
      page_private_->page_view_->DefaultUserAgent() + " " +
      GetWebAppManagerConfig()->GetName());

  const std::string& privileged_plugin_path =
      util::GetEnvVar("PRIVILEGED_PLUGIN_PATH");
  if (!privileged_plugin_path.empty()) {
    page_private_->page_view_->AddAvailablePluginDir(privileged_plugin_path);
  }

  page_private_->page_view_->SetAllowFakeBoldText(false);

  // FIXME: It should be permitted for backward compatibility for a limited list
  // of legacy applications only.
  page_private_->page_view_->SetAllowRunningInsecureContent(true);
  page_private_->page_view_->SetAllowScriptsToCloseWindows(true);
  page_private_->page_view_->SetAllowUniversalAccessFromFileUrls(true);
  page_private_->page_view_->SetSuppressesIncrementalRendering(true);
  page_private_->page_view_->SetDisallowScrollbarsInMainFrame(true);
  page_private_->page_view_->SetDisallowScrollingInMainFrame(true);
  page_private_->page_view_->SetDoNotTrack(app_desc_->DoNotTrack());
  page_private_->page_view_->SetJavascriptCanOpenWindows(true);
  page_private_->page_view_->SetSupportsMultipleWindows(false);
  page_private_->page_view_->SetCSSNavigationEnabled(true);
  page_private_->page_view_->SetV8DateUseSystemLocaloffset(false);
  page_private_->page_view_->SetLocalStorageEnabled(true);
  page_private_->page_view_->SetShouldSuppressDialogs(true);
  SetDisallowScrolling(app_desc_->DisallowScrollingInMainFrame());

  if (app_desc_->NetworkStableTimeout().has_value() &&
      (app_desc_->NetworkStableTimeout().value() >= 0.0)) {
    page_private_->page_view_->SetNetworkStableTimeout(
        app_desc_->NetworkStableTimeout().value());
  }

  switch (app_desc_->GetThirdPartyCookiesPolicy()) {
    case ApplicationDescription::ThirdPartyCookiesPolicy::kAllow:
      page_private_->page_view_->SetAllowThirdPartyCookies(true);
      break;
    case ApplicationDescription::ThirdPartyCookiesPolicy::kDeny:
      page_private_->page_view_->SetAllowThirdPartyCookies(false);
      break;
    default:
      page_private_->page_view_->SetAllowThirdPartyCookies(
          !(util::GetEnvVar("WAM_DEFAULT_ALLOW_THIRD_PARTY_COOKIES") == "0"));
  }

  if (app_desc_->TrustLevel() == "trusted") {
    LOG_DEBUG("[%s] trustLevel : trusted; allow load local Resources",
              AppId().c_str());
    page_private_->page_view_->SetAllowLocalResourceLoad(true);
  }

  if (app_desc_->CustomSuspendDOMTime().has_value() &&
      app_desc_->CustomSuspendDOMTime().value() > SuspendDelay()) {
    if (app_desc_->CustomSuspendDOMTime().value() > MaxCustomSuspendDelay()) {
      custom_suspend_dom_time_ = MaxCustomSuspendDelay();
    } else {
      custom_suspend_dom_time_ = app_desc_->CustomSuspendDOMTime().value();
    }
    LOG_DEBUG("[%s] set customSuspendDOMTime : %d ms", AppId().c_str(),
              custom_suspend_dom_time_);
  }

  page_private_->page_view_->AddUserStyleSheet(
      "body { -webkit-user-select: none; } :focus { outline: none }");
  page_private_->page_view_->SetBackgroundColor(29, 29, 29, 0xFF);

  SetDefaultFont(DefaultFont());

  std::string language;
  GetSystemLanguage(language);
  SetPreferredLanguages(language);
  page_private_->page_view_->SetAppId(
      AppId() + std::to_string(app_desc_->GetDisplayAffinity()));
  page_private_->page_view_->SetSecurityOrigin(
      GetIdentifierForSecurityOrigin());
  UpdateHardwareResolution();
  UpdateBoardType();
  UpdateDatabaseIdentifier();
  UpdateMediaCodecCapability();
  SetupStaticUserScripts();
  SetCustomPluginIfNeeded();
  SetSupportDolbyHDRContents();
  SetCustomUserScript();
  page_private_->page_view_->SetAudioGuidanceOn(IsAccessibilityEnabled());
  UpdateBackHistoryAPIDisabled();
  page_private_->page_view_->SetUseUnlimitedMediaPolicy(
      app_desc_->UseUnlimitedMediaPolicy());
  page_private_->page_view_->SetEnableBackgroundRun(
      app_desc_->IsEnableBackgroundRun());
  page_private_->page_view_->SetMediaPreferences(app_desc_->MediaPreferences());

  page_private_->page_view_->SetUseVideoDecodeAccelerator(
      app_desc_->UseVideoDecodeAccelerator());

  page_private_->page_view_->UpdatePreferences();

  LoadExtension();
}

void* WebPageBlink::GetWebContents() {
  return static_cast<void*>(page_private_->page_view_->GetWebContents());
}

void WebPageBlink::HandleBrowserControlCommand(
    const std::string& command,
    const std::vector<std::string>& arguments) {
  HandleBrowserControlMessage(command, arguments);
}

void WebPageBlink::HandleBrowserControlFunction(
    const std::string& command,
    const std::vector<std::string>& arguments,
    std::string* result) {
  *result = HandleBrowserControlMessage(command, arguments);
}

std::string WebPageBlink::HandleBrowserControlMessage(
    const std::string& command,
    const std::vector<std::string>& arguments) {
  if (!page_private_->palm_system_) {
    return std::string();
  }
  return page_private_->palm_system_->HandleBrowserControlMessage(command,
                                                                  arguments);
}

bool WebPageBlink::CanGoBack() {
  return page_private_->page_view_->CanGoBack();
}

std::string WebPageBlink::Title() {
  return page_private_->page_view_->DocumentTitle();
}

void WebPageBlink::SetFocus(bool focus) {
  page_private_->page_view_->SetFocus(focus);
}

void WebPageBlink::LoadDefaultUrl() {
  page_private_->page_view_->LoadUrl(DefaultUrl().ToString());
}

int WebPageBlink::Progress() const {
  return page_private_->page_view_->Progress();
}

bool WebPageBlink::HasBeenShown() const {
  return has_been_shown_;
}

wam::Url WebPageBlink::Url() const {
  return wam::Url(page_private_->page_view_->GetUrl());
}

void WebPageBlink::SetPreferredLanguages(const std::string& language) {
  if (page_private_->palm_system_) {
    page_private_->palm_system_->SetLocale(language);
  }

#ifndef TARGET_DESKTOP
  // just set system language for accept-language for http header,
  // navigator.language, navigator.languages even window.languagechange event
  // too
  page_private_->page_view_->SetAcceptLanguages(language);
  page_private_->page_view_->UpdatePreferences();
#endif
}

void WebPageBlink::SetDefaultFont(const std::string& font) {
  page_private_->page_view_->SetStandardFontFamily(font);
  page_private_->page_view_->SetFixedFontFamily(font);
  page_private_->page_view_->SetSerifFontFamily(font);
  page_private_->page_view_->SetSansSerifFontFamily(font);
  page_private_->page_view_->SetCursiveFontFamily(font);
  page_private_->page_view_->SetFantasyFontFamily(font);
}

void WebPageBlink::ReloadDefaultPage() {
  // When WebProcess is crashed
  // not only default page reloading,
  // need to set WebProcess setting (especially the options not using Setting or
  // preference)

  LoadDefaultUrl();
}

std::vector<std::string> WebPageBlink::GetErrorPagePath(
    const std::string& error_page) {
  const std::string& filepath = util::UriToLocal(error_page);
  if (filepath.empty()) {
    return std::vector<std::string>();
  }
  std::string language;
  GetSystemLanguage(language);

  return util::GetErrorPagePaths(filepath, language);
}

void WebPageBlink::ReloadFailedUrl() {
  LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           "ReloadFailedUrl: '%s'", load_failed_url_.c_str());
  page_private_->page_view_->LoadUrl(load_failed_url_);
}

void WebPageBlink::LoadErrorPage(int error_code) {
  const std::string& errorpage = GetWebAppManagerConfig()->GetErrorPageUrl();
  if (!errorpage.empty()) {
    if (HasLoadErrorPolicy(false, error_code)) {
      // has loadErrorPolicy, do not show error page
      LOG_DEBUG(
          "[%s] has own policy for Error Page, do not load Error page; send "
          "webOSLoadError event; return",
          AppId().c_str());
      return;
    }

    // search order:
    // searchPath/resources/<language>/<script>/<region>/html/fileName
    // searchPath/resources/<language>/<region>/html/fileName
    // searchPath/resources/<language>/html/fileName
    // searchPath/resources/html/fileName
    // searchPath/fileName

    // exception :
    // locale : zh-Hant-HK, zh-Hant-TW
    // searchPath/resources/zh/Hant/HK/html/fileName
    // searchPath/resources/zh/Hant/TW/html/fileName
    // es-ES has resources/es/ES/html but QLocale::bcp47Name() returns es not
    // es-ES fr-CA, pt-PT has its own localization folder and
    // QLocale::bcp47Name() returns well

    const auto& paths = GetErrorPagePath(errorpage);
    auto found =
        std::find_if(std::cbegin(paths), std::cend(paths), util::DoesPathExist);

    // finally found something!
    if (found != paths.end()) {
      // re-create it as a proper URL, so WebKit can understand it
      is_load_error_page_start_ = true;
      wam::Url error_url = wam::Url::FromLocalFile(*found);
      if (error_url.ToString().empty()) {
        LOG_ERROR(MSGID_ERROR_ERROR, 1, PMLOGKS("PATH", errorpage.c_str()),
                  "Error during conversion %s to URI", found->c_str());
        return;
      }
      wam::Url::UrlQuery query;
      query.emplace_back("errorCode", std::to_string(error_code));
      if (!load_failed_url_.empty()) {
        query.emplace_back("failedUrl", load_failed_url_);
      }
      error_url.SetQuery(query);
      LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", AppId().c_str()),
               PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
               PMLOGKFV("PID", "%d", GetWebProcessPID()), "LoadErrorPage : %s",
               error_url.ToString().c_str());
      page_private_->page_view_->LoadUrl(error_url.ToString());
    } else {
      LOG_ERROR(MSGID_ERROR_ERROR, 1, PMLOGKS("PATH", errorpage.c_str()),
                "Error loading error page");
    }
  }
}

void WebPageBlink::Reload() {
  page_private_->page_view_->Reload();
}

void WebPageBlink::LoadUrl(const std::string& url) {
  page_private_->page_view_->LoadUrl(url);
}

void WebPageBlink::SetLaunchParams(const std::string& params) {
  WebPageBase::SetLaunchParams(params);
  if (page_private_->palm_system_) {
    page_private_->palm_system_->SetLaunchParams(params);
  }
}

void WebPageBlink::SetUseLaunchOptimization(bool enabled, int delay_ms) {
  if (GetWebAppManagerConfig()->IsLaunchOptimizationEnabled()) {
    page_private_->page_view_->SetUseLaunchOptimization(enabled, delay_ms);
  }
}

void WebPageBlink::SetUseSystemAppOptimization(bool enabled) {
  page_private_->page_view_->SetUseEnyoOptimization(enabled);
}

void WebPageBlink::SetUseAccessibility(bool enabled) {
  page_private_->page_view_->SetUseAccessibility(enabled);
}

void WebPageBlink::SetAppPreloadHint(bool is_preload) {
  page_private_->page_view_->SetAppPreloadHint(is_preload);
}

void WebPageBlink::SuspendWebPageAll() {
  LOG_INFO(MSGID_SUSPEND_WEBPAGE, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "%s", __func__);

  page_private_->page_view_->SetVisible(false);
  if (is_suspended_ || enable_background_run_) {
    return;
  }

  if (!(util::GetEnvVar("WAM_KEEP_RTC_CONNECTIONS_ON_SUSPEND") == "1")) {
    // On sending applications to background, disconnect RTC
    page_private_->page_view_->DropAllPeerConnections(
        webos::DROP_PEER_CONNECTION_REASON_PAGE_HIDDEN);
  }

  SuspendWebPageMedia();

  // suspend painting
  // set visibility : hidden
  // set send to plugin about this visibility change
  // but NOT suspend DOM and JS Execution
  /* actually suspendWebPagePaintingAndJSExecution will do this again,
   * but this visibilitychange event and paint suspend should be done ASAP
   */
  page_private_->page_view_->SuspendPaintingAndSetVisibilityHidden();

  if (IsClosing()) {
    // In app closing scenario, loading about:blank and executing onclose
    // callback should be done For that, WebPage should be resume So, do not
    // suspend here
    LOG_INFO(MSGID_SUSPEND_WEBPAGE, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             "InClosing; Don't start DOMSuspendTimer");
    return;
  }

  is_suspended_ = true;
  if (ShouldStopJSOnSuspend()) {
    dom_suspend_timer_.StartWithReceiver(
        custom_suspend_dom_time_ ? custom_suspend_dom_time_ : SuspendDelay(),
        this, &WebPageBlink::SuspendWebPagePaintingAndJSExecution);
  }
  LOG_INFO(
      MSGID_SUSPEND_WEBPAGE, 3, PMLOGKS("APP_ID", AppId().c_str()),
      PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
      PMLOGKFV("PID", "%d", GetWebProcessPID()),
      "DomSuspendTimer(%dms) Started",
      custom_suspend_dom_time_ ? custom_suspend_dom_time_ : SuspendDelay());
}

void WebPageBlink::ResumeWebPageAll() {
  LOG_INFO(MSGID_RESUME_ALL, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "");
  // resume painting
  // Resume DOM and JS Execution
  // set visibility : visible (dispatch visibilitychange event)
  // set send to plugin about this visibility change
  if (ShouldStopJSOnSuspend()) {
    ResumeWebPagePaintingAndJSExecution();
  }
  ResumeWebPageMedia();
  page_private_->page_view_->SetVisible(true);
}

void WebPageBlink::SuspendWebPageMedia() {
  if (is_paused_ || enable_background_run_) {
    LOG_INFO(MSGID_SUSPEND_MEDIA, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             "%s; Already paused; return", __func__);
    return;
  }

  page_private_->page_view_->SuspendWebPageMedia();
  is_paused_ = true;

  LOG_INFO(MSGID_SUSPEND_MEDIA, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "");
}

void WebPageBlink::ResumeWebPageMedia() {
  if (!is_paused_) {
    LOG_INFO(MSGID_RESUME_MEDIA, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             "%s; Not paused; return", __func__);
    return;
  }

  // If there is a trouble while other app loading(loading fail or other
  // unexpected cases) Set use launching time optimization false. This function
  // call ensure that case.
  SetUseLaunchOptimization(false);

  page_private_->page_view_->ResumeWebPageMedia();
  is_paused_ = false;

  LOG_INFO(MSGID_RESUME_MEDIA, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "");
}

void WebPageBlink::SuspendWebPagePaintingAndJSExecution() {
  LOG_INFO(MSGID_SUSPEND_WEBPAGE, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "%s; is_suspended_ : %s",
           __func__, is_suspended_ ? "true" : "false; will be returned");
  if (dom_suspend_timer_.IsRunning()) {
    LOG_INFO(MSGID_SUSPEND_WEBPAGE_DELAYED, 3,
             PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             "DomSuspendTimer Expired; suspend DOM");
    dom_suspend_timer_.Stop();
  }

  if (enable_background_run_) {
    return;
  }

  if (!is_suspended_) {
    return;
  }

  // if we haven't finished loading the page yet, wait until it is loaded before
  // suspending
  bool is_loading = !HasBeenShown() && Progress() < 100;
  if (is_loading) {
    LOG_INFO(MSGID_SUSPEND_WEBPAGE, 4, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             PMLOGKS("URL", Url().ToString().c_str()),
             "Currently loading, Do not suspend, return");
    suspend_at_load_ = true;
  } else {
    page_private_->page_view_->SuspendPaintingAndSetVisibilityHidden();
    page_private_->page_view_->SuspendWebPageDOM();
    LOG_INFO(MSGID_SUSPEND_WEBPAGE, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()), "DONE");
  }
}

void WebPageBlink::ResumeWebPagePaintingAndJSExecution() {
  LOG_INFO(MSGID_RESUME_WEBPAGE, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "%s; is_suspended_ : %s ",
           __func__, is_suspended_ ? "true" : "false; nothing to resume");
  suspend_at_load_ = false;
  if (is_suspended_) {
    if (dom_suspend_timer_.IsRunning()) {
      LOG_INFO(MSGID_SUSPEND_WEBPAGE, 3, PMLOGKS("APP_ID", AppId().c_str()),
               PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
               PMLOGKFV("PID", "%d", GetWebProcessPID()),
               "DomSuspendTimer canceled by Resume");
      dom_suspend_timer_.Stop();
      page_private_->page_view_->ResumePaintingAndSetVisibilityVisible();
    } else {
      page_private_->page_view_->ResumeWebPageDOM();
      page_private_->page_view_->ResumePaintingAndSetVisibilityVisible();
      LOG_INFO(MSGID_RESUME_WEBPAGE, 3, PMLOGKS("APP_ID", AppId().c_str()),
               PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
               PMLOGKFV("PID", "%d", GetWebProcessPID()), "DONE");
    }
    is_suspended_ = false;
  }
}

std::string WebPageBlink::EscapeData(const std::string& value) {
  std::string escaped_value = value;
  util::ReplaceSubstr(escaped_value, "\\", "\\\\");
  util::ReplaceSubstr(escaped_value, "'", "\\'");
  util::ReplaceSubstr(escaped_value, "\n", "\\n");
  util::ReplaceSubstr(escaped_value, "\r", "\\r");
  return escaped_value;
}

void WebPageBlink::ReloadExtensionData() {
  std::string event_js =
      "if (typeof(webOSSystem) != 'undefined') {"
      "  webOSSystem.reloadInjectionData();"
      "};";

  LOG_INFO(MSGID_PALMSYSTEM, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "Reload");
  EvaluateJavaScript(event_js);
}

void WebPageBlink::UpdateExtensionData(const std::string& key,
                                       const std::string& value) {
  if (!page_private_->palm_system_->IsInitialized()) {
    LOG_WARNING(MSGID_PALMSYSTEM, 3, PMLOGKS("APP_ID", AppId().c_str()),
                PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
                PMLOGKFV("PID", "%d", GetWebProcessPID()),
                "webOSSystem is not initialized. key:%s, value:%s", key.c_str(),
                value.c_str());
    return;
  }
  std::string event_js =
      "if (typeof(webOSSystem) != 'undefined') {"
      "  webOSSystem.updateInjectionData('" +
      EscapeData(key) + "', '" + EscapeData(value) +
      "');"
      "};";
  LOG_INFO(MSGID_PALMSYSTEM, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()),
           "Update; key:%s; value:%s", key.c_str(), value.c_str());
  EvaluateJavaScript(event_js);
}

void WebPageBlink::HandleDeviceInfoChanged(const std::string& device_info) {
  if (!page_private_->palm_system_) {
    return;
  }

  if (device_info == "LocalCountry" || device_info == "SmartServiceCountry") {
    page_private_->palm_system_->SetCountry();
  }
}

void WebPageBlink::EvaluateJavaScript(const std::string& js_code) {
  page_private_->page_view_->RunJavaScript(js_code);
}

void WebPageBlink::EvaluateJavaScriptInAllFrames(const std::string& script,
                                                 const char* /*method*/) {
  page_private_->page_view_->RunJavaScriptInAllFrames(script);
}

void WebPageBlink::CleanResources() {
  WebPageBase::CleanResources();
  LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()),
           "StopLoading and load about:blank");
  page_private_->page_view_->StopLoading();
  page_private_->page_view_->LoadUrl(std::string("about:blank"));
}

void WebPageBlink::Close() {
  FOR_EACH_OBSERVER(WebPageObserver, observers_, WebPageClosePageRequested());
}

void WebPageBlink::DidFirstFrameFocused() {
  LOG_DEBUG("[%s] render process frame focused for the first time",
            AppId().c_str());
  // App load is finished, set use launching time optimization false.
  // If Launch optimization had to be done late, use delayMsForLaunchOptmization
  if (app_desc_->DelayMsForLaunchOptimization().has_value()) {
    int delay_ms = app_desc_->DelayMsForLaunchOptimization().value();
    SetUseLaunchOptimization(false, delay_ms);
  } else {
    SetUseLaunchOptimization(false);
  }
}

void WebPageBlink::DidDropAllPeerConnections() {}

void WebPageBlink::DidSwapCompositorFrame() {
  if (observer_) {
    observer_->DidSwapPageCompositorFrame();
  }
}

void WebPageBlink::DidResumeDOM() {
  if (observer_) {
    observer_->DidResumeDOM();
  }
}

void WebPageBlink::LoadFinished(const std::string& url) {
  LOG_INFO(MSGID_LOAD, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "[FINISH ]%s",
           WebAppManagerUtils::TruncateURL(url).c_str());

  if (CleaningResources()) {
    LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             "cleaningResources():true; (should be about:blank) emit "
             "'didDispatchUnload'");
    FOR_EACH_OBSERVER(WebPageObserver, observers_, DidDispatchUnload());
    return;
  }
  HandleLoadFinished();
}

void WebPageBlink::LoadStopped() {
  loading_url_.clear();
}

void WebPageBlink::DidStartNavigation(const std::string& url,
                                      bool is_in_main_frame) {
  loading_url_ = url;

  // moved from loadStarted
  has_close_callback_ = false;
  HandleLoadStarted();
  LOG_INFO(MSGID_LOAD, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "[START %s]%s",
           is_in_main_frame ? "m" : "s",
           WebAppManagerUtils::TruncateURL(url).c_str());
}

void WebPageBlink::DidFinishNavigation(const std::string& url,
                                       bool /*is_in_main_frame*/) {
  LOG_INFO(MSGID_LOAD, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "[CONNECT]%s",
           WebAppManagerUtils::TruncateURL(url).c_str());
}

void WebPageBlink::LoadProgressChanged(double progress) {
  bool process_ten_percent =
      std::abs(progress - 0.1f) < std::numeric_limits<float>::epsilon();
  if (!(loading_url_.empty() && process_ten_percent)) {
    // loading_url_ is empty then net didStartNavigation yet, default(initial)
    // progress : 0.1 so loading_url_ shouldn't be empty and greater than 0.1
    LOG_INFO(MSGID_LOAD, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()), "[...%3d%%]%s",
             static_cast<int>(progress * 100.0),
             WebAppManagerUtils::TruncateURL(loading_url_).c_str());
  }
}

void WebPageBlink::LoadAborted(const std::string& url) {
  LOG_INFO(MSGID_LOAD, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "[ABORTED]%s",
           WebAppManagerUtils::TruncateURL(url).c_str());
}

void WebPageBlink::LoadFailed(const std::string& url,
                              int err_code,
                              const std::string& err_desc) {
  LOG_INFO(MSGID_LOAD, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "[FAILED ][%d/%s]%s",
           err_code, err_desc.c_str(),
           WebAppManagerUtils::TruncateURL(url).c_str());

  FOR_EACH_OBSERVER(WebPageObserver, observers_, WebPageLoadFailed(err_code));

  if (err_code == -21 /*ERR_NETWORK_CHANGED*/) {
    LoadUrl(page_private_->page_view_->GetUrl());
    return;
  }

  load_failed_url_ = url;
  HandleLoadFailed(err_code);
}

void WebPageBlink::DidErrorPageLoadedFromNetErrorHelper() {
  did_error_page_loaded_from_net_error_helper_ = true;
}

void WebPageBlink::LoadVisuallyCommitted() {
  has_been_shown_ = true;
  FOR_EACH_OBSERVER(WebPageObserver, observers_, FirstFrameVisuallyCommitted());
}

void WebPageBlink::RenderProcessCreated(int pid) {
  PostWebProcessCreated(pid);
}

void WebPageBlink::TitleChanged(const std::string& /*title*/) {
  FOR_EACH_OBSERVER(WebPageObserver, observers_, TitleChanged());
}

void WebPageBlink::NavigationHistoryChanged() {
  FOR_EACH_OBSERVER(WebPageObserver, observers_, NavigationHistoryChanged());
}

void WebPageBlink::ForwardEvent(void* event) {
  page_private_->page_view_->ForwardWebOSEvent(static_cast<WebOSEvent*>(event));
}

void WebPageBlink::RecreateWebView() {
  LOG_INFO(MSGID_WEBPROC_CRASH, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()),
           "recreateWebView; initialize WebPage");
  if (!custom_plugin_path_.empty()) {
    // check setCustomPluginIfNeeded logic
    // not to set duplicated plugin path, it compares custom_plugin_path_ and
    // new one
    custom_plugin_path_.clear();  // just make it init state
  }

  Init();
  FOR_EACH_OBSERVER(WebPageObserver, observers_, WebViewRecreated());

  if (!is_suspended_) {
    // Remove white screen while reloading contents due to the renderer crash
    // 1. Reset state to mark next paint for notification when FMP done.
    //    It will be used to make webview visible later.
    page_private_->page_view_->ResetStateToMarkNextPaint();
    // 2. Set VisibilityState as Launching
    //    It will be used later, WebViewImpl set RenderWidgetCompositor visible,
    //    and make it keep to render the contents.
    SetVisibilityState(
        WebPageBase::WebPageVisibilityState::kWebPageVisibilityStateLaunching);
  }

  if (is_suspended_) {
    is_suspended_ = false;
  }
}

void WebPageBlink::SetVisible(bool visible) {
  page_private_->page_view_->SetVisible(visible);
}

void WebPageBlink::SetViewportSize() {
  if (app_desc_->WidthOverride().has_value() &&
      app_desc_->HeightOverride().has_value()) {
    page_private_->page_view_->SetViewportSize(
        app_desc_->WidthOverride().value(),
        app_desc_->HeightOverride().value());
  }
}

void WebPageBlink::NotifyMemoryPressure(
    webos::WebViewBase::MemoryPressureLevel level) {
  page_private_->page_view_->NotifyMemoryPressure(level);
}

void WebPageBlink::RenderProcessCrashed() {
  LOG_INFO(MSGID_WEBPROC_CRASH, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "is_suspended_ : %s",
           is_suspended_ ? "true" : "false");
  if (IsClosing()) {
    LOG_INFO(MSGID_WEBPROC_CRASH, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()), "In Closing; return");
    if (close_callback_timer_.IsRunning()) {
      close_callback_timer_.Stop();
    }

    FOR_EACH_OBSERVER(WebPageObserver, observers_,
                      ClosingAppProcessDidCrashed());
    return;
  }

  page_private_->palm_system_->ResetInitialized();
  RecreateWebView();
  if (!ProcessCrashed()) {
    HandleForceDeleteWebPage();
  }
}

// functions from webappmanager2
WebView* WebPageBlink::CreatePageView() {
  if (factory_) {
    return factory_->CreateWebView();
  }
  return new WebViewImpl(std::make_unique<BlinkWebView>());
}

WebView* WebPageBlink::PageView() const {
  return page_private_->page_view_.get();
}

bool WebPageBlink::Inspectable() {
  return GetWebAppManagerConfig()->IsInspectorEnabled();
}

// webOSLaunch / webOSRelaunch event:
// webOSLaunch event should be fired after DOMContentLoaded, and contains the
// launch parameters as it's detail. webOSRelaunch event should be fired when an
// app that is already running is triggered from applicationManager/launch, and
// will also contain the launch parameters as it's detail.
// IF we fire webOSLaunch immediately at handleLoadFinished(), the document may
// receive it before it has parsed all of the scripts.

// We cannot setup a generic script at page creation, because we don't know the
// launch parameters at that time. So, at load start, we'll take care of adding
// a user script.  Once that script has been added, it does not need to be added
// again -- triggering a page reload will cause it to fire the event again.

// There are a few caveats here, though:
// 1- We don't want to make a separate HTML file just for this, so we use the C
// API for adding a UserScript 2- The QT API for adding a user script only
// accepts a URL to a file, not absolute code. 3- We can't call
// WKPageGroupAddUserScript with the same argument more than once unless we want
// duplicate code to run

// So, we clear out any userscripts that may have been set, add any userscript
// files (ie Tellurium) via the QT API, then add any other userscripts that we
// might want via the C API, and then proceed.

// IF any further userscripts are desired in the future, they should be added
// here.
void WebPageBlink::AddUserScript(const std::string& script) {
  page_private_->page_view_->AddUserScript(script);
}

void WebPageBlink::AddUserScriptUrl(const wam::Url& url) {
  if (!url.IsLocalFile()) {
    LOG_DEBUG(
        "WebPageBlink: Couldn't open '%s' as user script because only file:/// "
        "URLs are supported.",
        url.ToString().c_str());
    return;
  }

  const std::string& path = url.ToLocalFile();
  const std::string& file_content = util::ReadFile(path);

  if (file_content.empty()) {
    LOG_DEBUG(
        "WebPageBlink: Couldn't open '%s' as user script due to error '%s'.",
        path.c_str(), strerror(errno));
    return;
  }
  page_private_->page_view_->AddUserScript(file_content);
}

void WebPageBlink::SetupStaticUserScripts() {
  page_private_->page_view_->ClearUserScripts();

  // Load Tellurium test framework if available, as a UserScript
  const std::string& tellurium_nub_path = TelluriumNubPath();
  if (!tellurium_nub_path.empty()) {
    LOG_DEBUG("Loading tellurium nub at %s", tellurium_nub_path.c_str());
    AddUserScriptUrl(wam::Url::FromLocalFile(tellurium_nub_path));
  }
}

void WebPageBlink::CloseVkb() {}

bool WebPageBlink::IsInputMethodActive() const {
  return page_private_->page_view_->IsInputMethodActive();
}

void WebPageBlink::SetPageProperties() {
  if (app_desc_->IsTransparent()) {
    page_private_->page_view_->SetTransparentBackground(true);
  }

  // set inspectable
  if (app_desc_->IsInspectable() || Inspectable()) {
    LOG_DEBUG(
        "[%s] inspectable : true or 'debug_system_apps' mode; "
        "setInspectablePage(true)",
        AppId().c_str());
    page_private_->page_view_->SetInspectable(true);
    page_private_->page_view_->EnableInspectablePage();
  }

  SetTrustLevel(DefaultTrustLevel());
  page_private_->page_view_->UpdatePreferences();
}

void WebPageBlink::CreatePalmSystem(WebAppBase* app) {
  page_private_->palm_system_ = std::make_unique<PalmSystemBlink>(app);
  page_private_->palm_system_->SetLaunchParams(launch_params_);
}

std::string WebPageBlink::DefaultTrustLevel() const {
  return app_desc_->TrustLevel();
}

void WebPageBlink::LoadExtension() {
  LOG_DEBUG("WebPageBlink::loadExtension(); Extension : webossystem");
  page_private_->page_view_->LoadExtension("webossystem");
  page_private_->page_view_->LoadExtension("webosservicebridge");
}

void WebPageBlink::ClearExtensions() {
  if (page_private_ && page_private_->page_view_) {
    page_private_->page_view_->ClearExtensions();
  }
}

void WebPageBlink::SetCustomPluginIfNeeded() {
  if (!app_desc_ || !app_desc_->UseCustomPlugin()) {
    return;
  }

  std::string custom_plugin_path = app_desc_->FolderPath();
  custom_plugin_path.append("/plugins");

  if (!util::DoesPathExist(custom_plugin_path.c_str())) {
    return;
  }
  if (custom_plugin_path_ == custom_plugin_path) {
    return;
  }

  custom_plugin_path_ = std::move(custom_plugin_path);
  LOG_INFO(MSGID_WAM_DEBUG, 4, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()),
           PMLOGKS("CUSTOM_PLUGIN_PATH", custom_plugin_path_.c_str()), "%s",
           __func__);

  page_private_->page_view_->AddCustomPluginDir(custom_plugin_path_);
  page_private_->page_view_->AddAvailablePluginDir(custom_plugin_path_);
}

void WebPageBlink::SetDisallowScrolling(bool disallow) {
  page_private_->page_view_->SetDisallowScrollbarsInMainFrame(disallow);
  page_private_->page_view_->SetDisallowScrollingInMainFrame(disallow);
}

int WebPageBlink::RenderProcessPid() const {
  return page_private_->page_view_->RenderProcessPid();
}

void WebPageBlink::DidRunCloseCallback() {
  close_callback_timer_.Stop();
  LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()),
           "WebPageBlink::didRunCloseCallback(); onclose callback done");
  FOR_EACH_OBSERVER(WebPageObserver, observers_, CloseCallbackExecuted());
}

void WebPageBlink::SetHasOnCloseCallback(bool has_close_callback) {
  has_close_callback_ = has_close_callback;
}

void WebPageBlink::ExecuteCloseCallback(bool forced) {
  std::string forced_str = forced ? "forced" : "normal";
  std::string script =
      "window.webOSSystem._onCloseWithNotify_('" + forced_str + "');";

  EvaluateJavaScript(script);

  close_callback_timer_.StartWithReceiver(kExecuteCloseCallbackTimeOutMs, this,
                                          &WebPageBlink::TimeoutCloseCallback);
}

void WebPageBlink::TimeoutCloseCallback() {
  close_callback_timer_.Stop();
  LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()),
           "WebPageBlink::timeoutCloseCallback(); onclose callback Timeout");
  FOR_EACH_OBSERVER(WebPageObserver, observers_, TimeoutExecuteCloseCallback());
}

void WebPageBlink::SetFileAccessBlocked(bool blocked) {
  // TO_DO: Need to verify when shnapshot is ready.
  webos::WebViewBase::SetFileAccessBlocked(blocked);
}

void WebPageBlink::SetAdditionalContentsScale(float scale_x, float scale_y) {
  page_private_->page_view_->SetAdditionalContentsScale(scale_x, scale_y);
}

void WebPageBlink::UpdateHardwareResolution() {
  std::string hardware_width, hardware_height;
  GetDeviceInfo("HardwareScreenWidth", hardware_width);
  GetDeviceInfo("HardwareScreenHeight", hardware_height);
  int width = util::StrToIntWithDefault(hardware_width, 0);
  int height = util::StrToIntWithDefault(hardware_height, 0);
  page_private_->page_view_->SetHardwareResolution(width, height);
}

void WebPageBlink::UpdateBoardType() {
  std::string board_type;
  GetDeviceInfo("boardType", board_type);
  page_private_->page_view_->SetBoardType(board_type);
}

void WebPageBlink::UpdateMediaCodecCapability() {
  const std::string& file_content =
      util::ReadFile("/etc/umediaserver/device_codec_capability_config.json");

  if (!file_content.empty()) {
    page_private_->page_view_->SetMediaCodecCapability(file_content);
  }
}

double WebPageBlink::DevicePixelRatio() {
  float device_pixel_ratio = 1.0;

  int app_width;
  int app_height;
  if (app_desc_->WidthOverride().has_value()) {
    app_width = app_desc_->WidthOverride().value();
  } else {
    app_width = CurrentUiWidth();
  }
  if (app_desc_->HeightOverride().has_value()) {
    app_height = app_desc_->HeightOverride().value();
  } else {
    app_height = CurrentUiHeight();
  }
  if (app_width == 0 || app_height == 0) {
    return device_pixel_ratio;
  }

  int device_width = 0;
  int device_height = 0;
  std::string hardware_width, hardware_height;
  if (GetDeviceInfo("HardwareScreenWidth", hardware_width) &&
      GetDeviceInfo("HardwareScreenHeight", hardware_height)) {
    device_width = util::StrToIntWithDefault(hardware_width, 0);
    device_height = util::StrToIntWithDefault(hardware_height, 0);
  } else {
    device_width = CurrentUiWidth();
    device_height = CurrentUiHeight();
  }

  float ratio_x = static_cast<float>(device_width) / app_width;
  float ratio_y = static_cast<float>(device_height) / app_height;
  bool ratios_are_equal =
      std::abs(ratio_x - ratio_y) < std::numeric_limits<float>::epsilon();
  if (!ratios_are_equal) {
    // device resolution : 5120x2160 (UHD 21:9 - D9)
    // - app resolution : 1280x720 ==> 4:3 (have to take 3)
    // - app resolution : 1920x1080 ==> 2.6:2 (have to take 2)
    device_pixel_ratio = (ratio_x < ratio_y) ? ratio_x : ratio_y;
  } else {
    // device resolution : 1920x1080
    // - app resolution : 1280x720 ==> 1.5:1.5
    // - app resolution : 1920x1080 ==> 1:1
    // device resolution : 3840x2160
    // - app resolution : 1280x720 ==> 3:3
    // - app resolution : 1920x1080 ==> 2:2
    device_pixel_ratio = ratio_x;
  }
  LOG_DEBUG(
      "[%s] WebPageBlink::devicePixelRatio(); devicePixelRatio : %f; "
      "deviceWidth : %d, deviceHeight : %d, appWidth : %d, appHeight : %d",
      AppId().c_str(), device_pixel_ratio, device_width, device_height,
      app_width, app_height);
  return device_pixel_ratio;
}

void WebPageBlink::SetSupportDolbyHDRContents() {
  std::string support_dolby_hdr_contents;
  GetDeviceInfo("supportDolbyHDRContents", support_dolby_hdr_contents);
  LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()),
           "supportDolbyHDRContents:%s", support_dolby_hdr_contents.c_str());

  Json::Value preferences = util::StringToJson(app_desc_->MediaPreferences());
  preferences["supportDolbyHDR"] = support_dolby_hdr_contents == "true";
  app_desc_->SetMediaPreferences(util::JsonToString(preferences));
}

void WebPageBlink::UpdateDatabaseIdentifier() {
  page_private_->page_view_->SetDatabaseIdentifier(app_id_);
}

void WebPageBlink::SetInspectorEnable() {
  LOG_DEBUG("[%s] Inspector enable", AppId().c_str());
  page_private_->page_view_->SetInspectable(true);
  page_private_->page_view_->EnableInspectablePage();
}

void WebPageBlink::SetKeepAliveWebApp(bool keep_alive) {
  LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "setKeepAliveWebApp(%s)",
           keep_alive ? "true" : "false");
  page_private_->page_view_->SetKeepAliveWebApp(keep_alive);
  page_private_->page_view_->UpdatePreferences();
}

void WebPageBlink::SetLoadErrorPolicy(const std::string& policy) {
  load_error_policy_ = policy;
  if (!policy.compare("event")) {
    // policy : event
    has_custom_policy_for_response_ = true;
  } else if (!policy.compare("default")) {
    // policy : default, WAM and blink handle all load errors
    has_custom_policy_for_response_ = false;
  }
}

bool WebPageBlink::DecidePolicyForResponse(bool is_main_frame,
                                           int status_code,
                                           const std::string& url,
                                           const std::string& status_text) {
  LOG_INFO(MSGID_WAM_DEBUG, 8, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()),
           PMLOGKFV("STATUS_CODE", "%d", status_code),
           PMLOGKS("URL", url.c_str()), PMLOGKS("TEXT", status_text.c_str()),
           PMLOGKS("MAIN_FRAME", is_main_frame ? "true" : "false"),
           PMLOGKS("RESPONSE_POLICY",
                   has_custom_policy_for_response_ ? "event" : "default"),
           "");

  // how to WAM3 handle this response
  ApplyPolicyForUrlResponse(is_main_frame, url, status_code);

  // how to blink handle this response
  // ACR requirement : even if received error response from subframe(iframe)ACR
  // app should handle that as a error
  return has_custom_policy_for_response_;
}

bool WebPageBlink::AcceptsVideoCapture() {
  return app_desc_->AllowVideoCapture();
}

bool WebPageBlink::AcceptsAudioCapture() {
  return app_desc_->AllowAudioCapture();
}

void WebPageBlink::KeyboardVisibilityChanged(bool visible) {
  std::string visible_str = visible ? "true" : "false";
  std::string javascript =
      "console.log('[WAM] fires keyboardStateChange event : " + visible_str +
      "');"
      "    var keyboardStateEvent =new CustomEvent('keyboardStateChange', { "
      "detail: { 'visibility' : " +
      visible_str +
      " } });"
      "    keyboardStateEvent.visibility = " +
      visible_str +
      ";"
      "    if(document) document.dispatchEvent(keyboardStateEvent);";
  EvaluateJavaScript(javascript);
}

void WebPageBlink::UpdateIsLoadErrorPageFinish() {
  // If currently loading finished URL is not error page,
  // is_load_error_page_finish_ will be updated
  bool was_error_page = is_load_error_page_finish_;
  WebPageBase::UpdateIsLoadErrorPageFinish();
  if (is_load_error_page_finish_) {
    LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             "Start reload timer");
    net_error_reload_timer_.Stop();
    net_error_reload_timer_.StartWithReceiver(kReloadTimeoutMs, this,
                                              &WebPageBlink::ReloadFailedUrl);
  } else if (was_error_page && !is_load_error_page_finish_) {
    LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()), "Stop reload timer");
    net_error_reload_timer_.Stop();
  }

  if (TrustLevel().compare("trusted") &&
      was_error_page != is_load_error_page_finish_) {
    if (is_load_error_page_finish_) {
      LOG_DEBUG(
          "[%s] WebPageBlink::updateIsLoadErrorPageFinish(); "
          "is_load_error_page_finish_ : %s, set trustLevel : trusted to WAM "
          "and webOSSystem_injection",
          AppId().c_str(), is_load_error_page_finish_ ? "true" : "false");
      SetTrustLevel("trusted");
      UpdateExtensionData("trustLevel", "trusted");
    }
  } else {
    SetTrustLevel(DefaultTrustLevel());
    UpdateExtensionData("trustLevel", TrustLevel());
  }
}

void WebPageBlink::ActivateRendererCompositor() {
  page_private_->page_view_->ActivateRendererCompositor();
}

void WebPageBlink::DeactivateRendererCompositor() {
  page_private_->page_view_->DeactivateRendererCompositor();
}

void WebPageBlink::SetAudioGuidanceOn(bool on) {
  page_private_->page_view_->SetAudioGuidanceOn(on);
  page_private_->page_view_->UpdatePreferences();
}

void WebPageBlink::UpdateBackHistoryAPIDisabled() {
  page_private_->page_view_->SetBackHistoryAPIDisabled(
      app_desc_->BackHistoryAPIDisabled());
}

void WebPageBlink::SetVisibilityState(WebPageVisibilityState visibility_state) {
  page_private_->page_view_->SetVisibilityState(
      static_cast<webos::WebViewBase::WebPageVisibilityState>(
          visibility_state));
}

void WebPageBlink::SetObserver(WebPageBlinkObserver* observer) {
  observer_ = observer;
}
