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

#include "web_page_base.h"

#include <filesystem>
#include <memory>
#include <sstream>

#include <json/value.h>

#include "application_description.h"
#include "log_manager.h"
#include "utils.h"
#include "web_app_manager.h"
#include "web_app_manager_config.h"
#include "web_page_observer.h"
#include "web_process_manager.h"

namespace fs = std::filesystem;

namespace {

const char kIdentifierForNetErrorPage[] = "com.webos.settingsservice.client";

}  // namespace

WebPageBase::WebPageBase() = default;

WebPageBase::WebPageBase(const wam::Url& url,
                         std::shared_ptr<ApplicationDescription> desc,
                         const std::string& params)
    : app_desc_(desc),
      app_id_(desc->Id()),
      default_url_(url),
      launch_params_(params) {
  Json::Value json = util::StringToJson(params);
  if (json.isObject()) {
    instance_id_ = json["instanceId"].asString();
  } else {
    LOG_WARNING(MSGID_TYPE_ERROR, 0,
                "[%s] failed get instanceId from params '%s'", app_id_.c_str(),
                params.c_str());
  }
}

WebPageBase::~WebPageBase() {
  LOG_INFO(MSGID_WEBPAGE_CLOSED, 2, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()), "");
}

std::string WebPageBase::LaunchParams() const {
  return launch_params_;
}

void WebPageBase::SetLaunchParams(const std::string& params) {
  launch_params_ = params;
}

void WebPageBase::SetApplicationDescription(
    std::shared_ptr<ApplicationDescription> desc) {
  app_desc_ = std::move(desc);
  SetPageProperties();
}

std::string WebPageBase::GetIdentifier() const {
  if ((is_load_error_page_finish_ && is_load_error_page_start_) ||
      did_error_page_loaded_from_net_error_helper_) {
    return std::string(kIdentifierForNetErrorPage);
  }
  return app_id_;
}

void WebPageBase::Load() {
  LOG_INFO(MSGID_WEBPAGE_LOAD, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "launch_params_:%s",
           launch_params_.c_str());
  /* this function is main load of WebPage : load default url */
  SetupLaunchEvent();
  if (!DoDeeplinking(launch_params_)) {
    LOG_INFO(MSGID_WEBPAGE_LOAD, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()), "loadDefaultUrl()");
    LoadDefaultUrl();
  }
}

void WebPageBase::SetupLaunchEvent() {
  std::stringstream launch_event;
  std::string params = LaunchParams().empty() ? "{}" : LaunchParams();
  launch_event
      << "(function() {" << "    var dispatchLaunchEvent = function() {"
      << "        var launchEvent = new CustomEvent('webOSLaunch', { detail: "
      << params << " });" << "        setTimeout(function() {"
      << "            document.dispatchEvent(launchEvent);" << "        }, 1);"
      << "    };" << "    if (document.readyState === 'complete') {"
      << "        dispatchLaunchEvent();" << "    } else {"
      << "        document.onreadystatechange = function() {"
      << "            if (document.readyState === 'complete') {"
      << "                dispatchLaunchEvent();" << "            }"
      << "        };" << "    }" << "})();";

  AddUserScript(launch_event.str());
}

void WebPageBase::SendLocaleChangeEvent(const std::string& /*language*/) {
  // TODO(luc2.tran): This should be probably run only when
  // |enable_background_run_| is set to 'true' or the web app is not suspended.
  // The other case (app is suspended), I think we're better not to run the
  // script instead of using a timeout. Using the timeout could make unexpected
  // behavior when the app is unsuspended and then all timers will be executed.
  EvaluateJavaScript(
      "setTimeout(function () {"
      "    var localeEvent=new CustomEvent('webOSLocaleChange');"
      "    document.dispatchEvent(localeEvent);"
      "}, 1);");
}

void WebPageBase::CleanResources() {
  SetCleaningResources(true);
}

bool WebPageBase::Relaunch(const std::string& launch_params,
                           const std::string& /*launching_app_id*/) {
  ResumeWebPagePaintingAndJSExecution();

  // for common webapp relaunch scenario
  // 1. Service worker clients.openWindow case : call LoadUrl with the
  // given url
  // 2. For hosted webapp deeplinking : reload default page
  // 3-1. Check progress; to send webOSRelaunch event, then page loading
  // progress should be 100
  // 3-2. Update launchParams
  // 3-3. Send webOSRelaunch event

  // 1. Handling service worker clients.openWindow case
  Json::Value json_obj = util::StringToJson(launch_params);
  if (json_obj.isMember("sw_clients_openwindow")) {
    const auto& sw_clients_openwindow = json_obj["sw_clients_openwindow"];
    if (sw_clients_openwindow.isString()) {
      std::string target_url = sw_clients_openwindow.asString();
      LOG_DEBUG("[%s] service worker clients.openWindow(%s) relaunch",
                app_id_.c_str(), target_url.c_str());
      LoadUrl(target_url);
      return true;
    }
  }

  if (DoHostedWebAppRelaunch(launch_params)) {
    LOG_DEBUG("[%s] Hosted webapp; handled", app_id_.c_str());
    return true;
  }

  if (!HasBeenShown()) {
    LOG_INFO(MSGID_WEBPAGE_RELAUNCH, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             "In Loading(%d%%), Can not handle relaunch now, return false",
             Progress());
    return false;
  }

  SetLaunchParams(launch_params);

  // WebPageBase::relaunch handles setting the stageArgs for the launch/relaunch
  // events
  SendRelaunchEvent();
  return true;
}

bool WebPageBase::DoHostedWebAppRelaunch(const std::string& launch_params) {
  /* hosted webapp deeplinking spec
  // legacy case
  "deeplinkingParams":"{ \
      \"contentTarget\" : \"https://www.youtube.com/tv?v=$CONTENTID\" \
  }"
  // webOS4.0 spec
  "deeplinkingParams":"{ \
      \"handledBy\" : \"platform\" || \"app\" || \"default\", \
      \"contentTarget\" : \"https://www.youtube.com/tv?v=$CONTENTID\" \
  }"
  To support backward compatibility, should cover the case not having
  "handledBy"
  */
  // check deeplinking relaunch condition
  Json::Value obj = util::StringToJson(launch_params);

  if (Url().Scheme() == "file" || default_url_.Scheme() != "file" ||
      !obj.isObject() /* no launchParams, { }, and this should be check with
                         object().isEmpty()*/
      || obj["contentTarget"].isNull() ||
      (app_desc_ && !app_desc_->HandlesDeeplinking())) {
    LOG_INFO(MSGID_WEBPAGE_RELAUNCH, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             "%s; NOT enough deeplinking condition; return false", __func__);
    return false;
  }

  // Do deeplinking relaunch
  SetLaunchParams(launch_params);
  return DoDeeplinking(launch_params);
}

bool WebPageBase::DoDeeplinking(const std::string& launch_params) {
  Json::Value obj = util::StringToJson(launch_params);
  if (!obj.isObject() || obj["contentTarget"].isNull()) {
    return false;
  }

  std::string handled_by =
      obj["handledBy"].isNull() ? "default" : obj["handledBy"].asString();
  if (handled_by == "platform") {
    std::string target_url = obj["contentTarget"].asString();
    LOG_INFO(MSGID_DEEPLINKING, 4, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             PMLOGKS("handledBy", handled_by.c_str()), "%s; load target URL:%s",
             __func__, target_url.c_str());
    // load the target URL directly
    LoadUrl(target_url);
    return true;
  } else if (handled_by == "app") {
    // If "handledBy" == "app" return false
    // then it will be handled just like common relaunch case, checking progress
    return false;
  } else {
    // handledBy == "default" or "other values"
    LOG_INFO(MSGID_DEEPLINKING, 4, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             PMLOGKS("handledBy", handled_by.c_str()), "%s; loadDefaultUrl",
             __func__);
    LoadDefaultUrl();
    return true;
  }
}

void WebPageBase::SendRelaunchEvent() {
  SetVisible(true);
  LOG_INFO(MSGID_SEND_RELAUNCHEVENT, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()), "");
  // Send the relaunch event on the next tick after javascript is loaded
  // This is a workaround for a problem where WebKit can't free the page
  // if we don't use a timeout here.
  std::stringstream relaunch_event;
  std::string detail = LaunchParams().empty() ? "{}" : LaunchParams();
  relaunch_event
      << "setTimeout(function () {"
      << "    console.log('[WAM] fires webOSRelaunch event');"
      << "    var launchEvent=new CustomEvent('webOSRelaunch', { detail: "
      << detail << " });" << "    document.dispatchEvent(launchEvent);"
      << "}, 1);";
  EvaluateJavaScript(relaunch_event.str().c_str());
}

void WebPageBase::HandleLoadStarted() {
  suspend_at_load_ = true;
  did_error_page_loaded_from_net_error_helper_ = false;
}

void WebPageBase::HandleLoadFinished() {
  LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()),
           "WebPageBase::handleLoadFinished; suspend_at_load_ : %s",
           suspend_at_load_ ? "true; suspend in this time" : "false");

  FOR_EACH_OBSERVER(WebPageObserver, observers_, WebPageLoadFinished());

  // if there was an attempt made to suspend while this page was loading, then
  // we flag suspend_at_load_ = true, and suspend it after it is loaded. This is
  // to prevent application load from failing.
  if (suspend_at_load_) {
    SuspendWebPagePaintingAndJSExecution();
  }
  UpdateIsLoadErrorPageFinish();
}

void WebPageBase::HandleLoadFailed(int error_code) {
  // error_code 204 specifically states that the web browser not relocate
  // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
  // we can't handle unknown protocol like mailto.
  // Client want to not show error page with unknown protocol like chrome.
  if (!is_preload_ && error_code != 204 && error_code != 301) {
    LoadErrorPage(error_code);
  }
}

void WebPageBase::CleanResourcesFinished() {
  WebAppManager::Instance()->PostRunningAppList();
  if (cleaning_resources_) {
    delete this;
  }
}

void WebPageBase::HandleForceDeleteWebPage() {
  delete this;
}

bool WebPageBase::GetSystemLanguage(std::string& value) {
  return WebAppManager::Instance()->GetSystemLanguage(value);
}

bool WebPageBase::GetDeviceInfo(const std::string& name, std::string& value) {
  return WebAppManager::Instance()->GetDeviceInfo(name, value);
}

int WebPageBase::CurrentUiWidth() {
  return WebAppManager::Instance()->CurrentUiWidth();
}

int WebPageBase::CurrentUiHeight() {
  return WebAppManager::Instance()->CurrentUiHeight();
}

WebProcessManager* WebPageBase::GetWebProcessManager() {
  return WebAppManager::Instance()->GetWebProcessManager();
}

WebAppManagerConfig* WebPageBase::GetWebAppManagerConfig() {
  return WebAppManager::Instance()->Config();
}

bool WebPageBase::ProcessCrashed() {
  return WebAppManager::Instance()->ProcessCrashed(AppId(), InstanceId());
}

int WebPageBase::SuspendDelay() {
  return WebAppManager::Instance()->GetSuspendDelay();
}

int WebPageBase::MaxCustomSuspendDelay() {
  return WebAppManager::Instance()->GetMaxCustomSuspendDelay();
}

std::string WebPageBase::TelluriumNubPath() {
  return GetWebAppManagerConfig()->GetTelluriumNubPath();
}

bool WebPageBase::HasLoadErrorPolicy(bool is_http_response_error,
                                     int error_code) {
  if (load_error_policy_ == "event") {
    std::stringstream jss;
    std::string generic_error = is_http_response_error ? "false" : "true";
    jss << "{" << "    console.log('[WAM3] create webOSLoadError event');"
        << "    var launchEvent=new CustomEvent('webOSLoadError',"
        << "        { detail : { genericError : " << generic_error
        << ", errorCode : " << error_code << " }});"
        << "    document.dispatchEvent(launchEvent);" << "}";
    // App has load error policy, do not show platform load error page
    EvaluateJavaScript(jss.str());
    return true;
  }
  return false;
}

void WebPageBase::ApplyPolicyForErrorPage(bool is_main_frame,
                                          const std::string& url,
                                          int error_code) {
  // error_code can be both HTTP Status and Network Error
  // HTTP Status : Positive values
  // Network Error : Negative values
  wam::Url response_url(url);
  static const int http_error_status_code = 400;
  if (response_url.Scheme() != "file" &&
      !HasLoadErrorPolicy(error_code >= http_error_status_code, error_code) &&
      is_main_frame) {
    // If app does not have policy for load error and
    // this error response is from main frame document
    // then before open server error page, reset the body's background color
    // to white
    SetBackgroundColorOfBody("white");
  }
}

void WebPageBase::PostRunningAppList() {
  WebAppManager::Instance()->PostRunningAppList();
}

void WebPageBase::PostWebProcessCreated(uint32_t pid) {
  WebAppManager::Instance()->PostWebProcessCreated(app_id_, instance_id_, pid);
}

void WebPageBase::SetBackgroundColorOfBody(const std::string& color) {
  // for error page only, set default background color to white by executing
  // javascript
  std::stringstream background_color_of_body;
  background_color_of_body
      << "(function() {"
      << "    if(document.readyState === 'complete' || document.readyState === "
         "'interactive') { "
      << "       if(document.body.style.backgroundColor)"
      << "           console.log('[Server Error] Already set "
         "document.body.style.backgroundColor');"
      << "       else {"
      << "           console.log('[Server Error] set background Color of body "
         "to "
      << color << "');" << "           document.body.style.backgroundColor = '"
      << color << "';" << "       }" << "     } else {"
      << "        document.addEventListener('DOMContentLoaded', function() {"
      << "           if(document.body.style.backgroundColor)"
      << "               console.log('[Server Error] Already set "
         "document.body.style.backgroundColor');"
      << "           else {"
      << "               console.log('[Server Error] set background Color of "
         "body to "
      << color << "');"
      << "               document.body.style.backgroundColor = '" << color
      << "';" << "           }" << "        });" << "    }" << "})();";

  EvaluateJavaScript(background_color_of_body.str());
}

std::string WebPageBase::DefaultFont() {
  std::string default_font = "LG Display-Regular";
  std::string language;
  std::string country;
  GetSystemLanguage(language);
  GetDeviceInfo("LocalCountry", country);

  // for the model
  if (country == "JPN") {
    default_font = "LG Display_JP";
  } else if (country == "HKG") {
    default_font = "LG Display GP4_HK";
  }
  // for the locale(language)
  else if (language == "ur-IN") {
    default_font = "LG Display_Urdu";
  }

  LOG_DEBUG("[%s] country : [%s], language : [%s], default font : [%s]",
            AppId().c_str(), country.c_str(), language.c_str(),
            default_font.c_str());
  return default_font;
}

void WebPageBase::UpdateIsLoadErrorPageFinish() {
  // ex)
  // Target error page URL :
  // file:///usr/share/localization/webappmanager2/resources/ko/html/loaderror.html?errorCode=65&webkitErrorCode=65
  // WAM error page :
  // file:///usr/share/localization/webappmanager2/loaderror.html
  is_load_error_page_finish_ = false;

  if (!Url().IsLocalFile()) {
    return;
  }

  fs::path url_path(Url().ToLocalFile());
  std::string url_file_name = url_path.filename();
  std::string url_dir_path = url_path.remove_filename();

  fs::path err_path(
      wam::Url(GetWebAppManagerConfig()->GetErrorPageUrl()).ToLocalFile());
  std::string err_file_name = err_path.filename();
  std::string err_dir_path = err_path.remove_filename();

  if (url_dir_path.starts_with(err_dir_path) &&
      url_file_name == err_file_name) {
    LOG_DEBUG("[%s] This is WAM ErrorPage; URL: %s ", AppId().c_str(),
              Url().ToString().c_str());
    is_load_error_page_finish_ = true;
  }
}

void WebPageBase::SetCustomUserScript() {
  // 1. check app folder has userScripts
  // 2. check userscript.js there is, appfolder/webOSUserScripts/*.js
  auto user_script_file_path = fs::path(app_desc_->FolderPath()) /
                               GetWebAppManagerConfig()->GetUserScriptPath();

  if (!fs::exists(user_script_file_path) ||
      !fs::is_regular_file(fs::status(user_script_file_path))) {
    LOG_WARNING(MSGID_FILE_ERROR, 0,
                "[%s] script not exist on file system '%s'", app_id_.c_str(),
                user_script_file_path.c_str());
    return;
  }

  LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()),
           "User Scripts exists : %s", user_script_file_path.c_str());
  AddUserScriptUrl(wam::Url::FromLocalFile(user_script_file_path.native()));
}

void WebPageBase::AddObserver(WebPageObserver* observer) {
  observers_.AddObserver(observer);
}

void WebPageBase::RemoveObserver(WebPageObserver* observer) {
  observers_.RemoveObserver(observer);
}

bool WebPageBase::IsAccessibilityEnabled() const {
  return WebAppManager::Instance()->IsAccessibilityEnabled();
}

std::string WebPageBase::GetIdentifierForSecurityOrigin() const {
  return WebAppManager::Instance()->IdentifierForSecurityOrigin(
      GetIdentifier());
}
