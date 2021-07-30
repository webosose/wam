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

#include <memory>
#include <sstream>

#include <json/value.h>
#include <boost/filesystem.hpp>

#include "application_description.h"
#include "log_manager.h"
#include "utils.h"
#include "web_app_manager.h"
#include "web_app_manager_config.h"
#include "web_page_observer.h"
#include "web_process_manager.h"

namespace fs = boost::filesystem;

namespace {

const char kIdentifierForNetErrorPage[] = "com.webos.settingsservice.client";

}

WebPageBase::WebPageBase()
    : app_desc_(nullptr),
      suspend_at_load_(false),
      is_closing_(false),
      is_load_error_page_finish_(false),
      is_load_error_page_start_(false),
      did_error_page_loaded_from_net_error_helper_(false),
      enable_background_run_(false),
      default_url_(std::string()),
      load_error_policy_("default"),
      cleaning_resources_(false),
      is_preload_(false) {}

WebPageBase::WebPageBase(const wam::Url& url,
                         std::shared_ptr<ApplicationDescription> desc,
                         const std::string& params)
    : app_desc_(desc),
      app_id_(desc->Id()),
      suspend_at_load_(false),
      is_closing_(false),
      is_load_error_page_finish_(false),
      is_load_error_page_start_(false),
      did_error_page_loaded_from_net_error_helper_(false),
      enable_background_run_(false),
      default_url_(url),
      launch_params_(params),
      load_error_policy_("default"),
      cleaning_resources_(false),
      is_preload_(false) {
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
  app_desc_ = desc;
  SetPageProperties();
}

std::string WebPageBase::GetIdentifier() const {
  if ((is_load_error_page_finish_ && is_load_error_page_start_) ||
      did_error_page_loaded_from_net_error_helper_)
    return std::string(kIdentifierForNetErrorPage);
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
  std::stringstream launchEvent;
  std::string params = LaunchParams().empty() ? "{}" : LaunchParams();
  launchEvent
      << "(function() {"
      << "    var dispatchLaunchEvent = function() {"
      << "        var launchEvent = new CustomEvent('webOSLaunch', { detail: "
      << params << " });"
      << "        setTimeout(function() {"
      << "            document.dispatchEvent(launchEvent);"
      << "        }, 1);"
      << "    };"
      << "    if (document.readyState === 'complete') {"
      << "        dispatchLaunchEvent();"
      << "    } else {"
      << "        document.onreadystatechange = function() {"
      << "            if (document.readyState === 'complete') {"
      << "                dispatchLaunchEvent();"
      << "            }"
      << "        };"
      << "    }"
      << "})();";

  AddUserScript(launchEvent.str());
}

void WebPageBase::SendLocaleChangeEvent(const std::string& language) {
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
                           const std::string& launching_app_id) {
  ResumeWebPagePaintingAndJSExecution();

  // for common webapp relaunch scenario
  // 1. For hosted webapp deeplinking : reload default page
  // 2-1. check progress; to send webOSRelaunch event, then page loading
  // progress should be 100 2-2. Update launchParams 2-3. send webOSRelaunch
  // event

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
  if (!obj.isObject() || obj["contentTarget"].isNull())
    return false;

  std::string handledBy =
      obj["handledBy"].isNull() ? "default" : obj["handledBy"].asString();
  if (handledBy == "platform") {
    std::string targetUrl = obj["contentTarget"].asString();
    LOG_INFO(MSGID_DEEPLINKING, 4, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             PMLOGKS("handledBy", handledBy.c_str()), "%s; load target URL:%s",
             __func__, targetUrl.c_str());
    // load the target URL directly
    LoadUrl(targetUrl);
    return true;
  } else if (handledBy == "app") {
    // If "handledBy" == "app" return false
    // then it will be handled just like common relaunch case, checking progress
    return false;
  } else {
    // handledBy == "default" or "other values"
    LOG_INFO(MSGID_DEEPLINKING, 4, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", GetWebProcessPID()),
             PMLOGKS("handledBy", handledBy.c_str()), "%s; loadDefaultUrl",
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
  std::stringstream relaunchEvent;
  std::string detail = LaunchParams().empty() ? "{}" : LaunchParams();
  relaunchEvent
      << "setTimeout(function () {"
      << "    console.log('[WAM] fires webOSRelaunch event');"
      << "    var launchEvent=new CustomEvent('webOSRelaunch', { detail: "
      << detail << " });"
      << "    document.dispatchEvent(launchEvent);"
      << "}, 1);";
  EvaluateJavaScript(relaunchEvent.str().c_str());
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

void WebPageBase::HandleLoadFailed(int errorCode) {
  // errorCode 204 specifically states that the web browser not relocate
  // http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
  // we can't handle unknown protcol like mailto.
  // Client want to not show error page with unknown protocol like chrome.
  if (!is_preload_ && errorCode != 204 && errorCode != 301)
    LoadErrorPage(errorCode);
}

void WebPageBase::CleanResourcesFinished() {
  WebAppManager::Instance()->PostRunningAppList();
  if (cleaning_resources_) {
    WebAppManager::Instance()->RemoveWebAppFromWebProcessInfoMap(AppId());
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

bool WebPageBase::HasLoadErrorPolicy(bool isHttpResponseError, int errorCode) {
  if (load_error_policy_ == "event") {
    std::stringstream jss;
    std::string genericError = isHttpResponseError ? "false" : "true";
    jss << "{"
        << "    console.log('[WAM3] create webOSLoadError event');"
        << "    var launchEvent=new CustomEvent('webOSLoadError',"
        << "        { detail : { genericError : " << genericError
        << ", errorCode : " << errorCode << " }});"
        << "    document.dispatchEvent(launchEvent);"
        << "}";
    // App has load error policy, do not show platform load error page
    EvaluateJavaScript(jss.str());
    return true;
  }
  return false;
}

void WebPageBase::ApplyPolicyForUrlResponse(bool isMainFrame,
                                            const std::string& url,
                                            int status_code) {
  wam::Url response_url(url);
  static const int http_error_status_code = 400;
  if (response_url.Scheme() != "file" &&
      status_code >= http_error_status_code) {
    if (!HasLoadErrorPolicy(true, status_code) && isMainFrame) {
      // If app does not have policy for load error and
      // this error response is from main frame document
      // then before open server error page, reset the body's background color
      // to white
      SetBackgroundColorOfBody("white");
    }
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
      << color << "');"
      << "           document.body.style.backgroundColor = '" << color << "';"
      << "       }"
      << "     } else {"
      << "        document.addEventListener('DOMContentLoaded', function() {"
      << "           if(document.body.style.backgroundColor)"
      << "               console.log('[Server Error] Already set "
         "document.body.style.backgroundColor');"
      << "           else {"
      << "               console.log('[Server Error] set background Color of "
         "body to '"
      << color << "');"
      << "               document.body.style.backgroundColor = '" << color
      << "';"
      << "           }"
      << "        });"
      << "    }"
      << "})();";

  EvaluateJavaScript(background_color_of_body.str());
}

std::string WebPageBase::DefaultFont() {
  std::string default_font = "LG Display-Regular";
  std::string language;
  std::string country;
  GetSystemLanguage(language);
  GetDeviceInfo("LocalCountry", country);

  // for the model
  if (country == "JPN")
    default_font = "LG Display_JP";
  else if (country == "HKG")
    default_font = "LG Display GP4_HK";
  // for the locale(language)
  else if (language == "ur-IN")
    default_font = "LG Display_Urdu";

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

  if (!Url().IsLocalFile())
    return;

  fs::path url_path(Url().ToLocalFile());
  fs::path url_file_name = url_path.filename();

  fs::path url_dir_path = url_path.parent_path();
  fs::path err_path(
      wam::Url(GetWebAppManagerConfig()->GetErrorPageUrl()).ToLocalFile());
  fs::path err_file_name = err_path.filename();
  fs::path err_dir_path = err_path.parent_path();

  if ((url_dir_path.string().find(err_dir_path.string()) ==
       0)  // urlDirPath starts with errDirPath
      && url_file_name == err_file_name) {
    LOG_DEBUG("[%s] This is WAM ErrorPage; URL: %s ", AppId().c_str(),
              Url().ToString().c_str());
    is_load_error_page_finish_ = true;
  }
}

void WebPageBase::SetCustomUserScript() {
  // 1. check app folder has userScripts
  // 2. check userscript.js there is, appfolder/webOSUserScripts/*.js
  auto userScriptFilePath = fs::path(app_desc_->FolderPath()) /
                            GetWebAppManagerConfig()->GetUserScriptPath();

  if (!fs::exists(userScriptFilePath) ||
      !fs::is_regular_file(fs::canonical(userScriptFilePath))) {
    LOG_WARNING(MSGID_FILE_ERROR, 0,
                "[%s] script not exist on file system '%s'", app_id_.c_str(),
                userScriptFilePath.string().c_str());
    return;
  }

  LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", GetWebProcessPID()),
           "User Scripts exists : %s", userScriptFilePath.c_str());
  AddUserScriptUrl(wam::Url::FromLocalFile(userScriptFilePath.string()));
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
