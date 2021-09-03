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

#include "web_app_base.h"

#include "application_description.h"
#include "log_manager.h"
#include "utils.h"
#include "web_app_manager.h"
#include "web_app_manager_config.h"
#include "web_page_base.h"

class WebAppBasePrivate {
 public:
  WebAppBasePrivate(WebAppBase* parent)
      : parent_(parent),
        page_(0),
        keep_alive_(false),
        force_close_(false),
        app_desc_(nullptr) {}

  ~WebAppBasePrivate() {
    delete page_;

    LOG_DEBUG("Delete webapp base for Instance %s of App ID %s",
              instance_id_.c_str(), app_id_.c_str());
  }

 public:
  WebAppBase* parent_;
  WebPageBase* page_;
  bool keep_alive_;
  bool force_close_;
  std::string launching_app_id_;
  std::string app_id_;
  std::string instance_id_;
  std::string url_;
  std::shared_ptr<ApplicationDescription> app_desc_;

 private:
  WebAppBasePrivate(const WebAppBasePrivate&) = delete;
  WebAppBasePrivate& operator=(const WebAppBasePrivate&) = delete;
};

WebAppBase::WebAppBase()
    : preload_state_(kNonePreload),
      added_to_window_mgr_(false),
      scale_factor_(1.0f),
      app_private_(new WebAppBasePrivate(this)),
      need_reload_(false),
      crashed_(false),
      hidden_window_(false),
      close_page_requested_(false) {}

WebAppBase::~WebAppBase() {
  LOG_INFO(MSGID_WEBAPP_CLOSED, 3,
           PMLOGKS("APP_ID", AppId().empty() ? "unknown" : AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", Page() ? Page()->GetWebProcessPID() : 0), "");
  CleanResources();
  delete app_private_;
}

bool WebAppBase::GetCrashState() const {
  return crashed_;
}

void WebAppBase::SetCrashState(bool state) {
  crashed_ = state;
}

void WebAppBase::SetHiddenWindow(bool hidden) {
  hidden_window_ = hidden;
}

bool WebAppBase::GetHiddenWindow() const {
  return hidden_window_;
}

void WebAppBase::SetKeepAlive(bool keep_alive) {
  app_private_->keep_alive_ = keep_alive;
}

bool WebAppBase::KeepAlive() const {
  return app_private_->keep_alive_;
}

void WebAppBase::SetForceClose() {
  app_private_->force_close_ = true;
}

bool WebAppBase::ForceClose() {
  return app_private_->force_close_;
}

WebPageBase* WebAppBase::Page() const {
  return app_private_->page_;
}

bool WebAppBase::IsWindowed() const {
  return false;
}

void WebAppBase::SetAppId(const std::string& app_id) {
  app_private_->app_id_ = app_id;
}

void WebAppBase::SetLaunchingAppId(const std::string& app_id) {
  app_private_->launching_app_id_ = app_id;
}

std::string WebAppBase::AppId() const {
  return app_private_->app_id_;
}

void WebAppBase::SetInstanceId(const std::string& instance_id) {
  app_private_->instance_id_ = instance_id;
}

std::string WebAppBase::InstanceId() const {
  return app_private_->instance_id_;
}

std::string WebAppBase::Url() const {
  return app_private_->url_;
}

std::string WebAppBase::LaunchingAppId() const {
  return app_private_->launching_app_id_;
}

ApplicationDescription* WebAppBase::GetAppDescription() const {
  return app_private_->app_desc_.get();
}

void WebAppBase::CleanResources() {
  app_private_->app_desc_.reset();
}

int WebAppBase::CurrentUiWidth() {
  return WebAppManager::Instance()->CurrentUiWidth();
}

int WebAppBase::CurrentUiHeight() {
  return WebAppManager::Instance()->CurrentUiHeight();
}

void WebAppBase::SetActiveInstanceId(const std::string& id) {
  WebAppManager::Instance()->SetActiveInstanceId(id);
}

void WebAppBase::ForceCloseAppInternal() {
  WebAppManager::Instance()->ForceCloseAppInternal(this);
}

void WebAppBase::CloseAppInternal() {
  WebAppManager::Instance()->CloseAppInternal(this);
}

void WebAppBase::Attach(WebPageBase* page) {
  // connect to the signals of the WebBridge
  // parse up the ApplicationDescription
  if (app_private_->page_)
    Detach();

  app_private_->page_ = page;
  app_private_->page_->CreatePalmSystem(this);

  Observe(app_private_->page_);
}

WebPageBase* WebAppBase::Detach(void) {
  WebPageBase* page = app_private_->page_;

  Unobserve(app_private_->page_);

  app_private_->page_ = nullptr;
  return page;
}

void WebAppBase::Relaunch(const std::string& args,
                          const std::string& launching_app_id) {
  LOG_INFO(MSGID_APP_RELAUNCH, 4, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", Page()->GetWebProcessPID()),
           PMLOGKS("LAUNCHING_APP_ID", launching_app_id.c_str()), "");
  if (GetHiddenWindow()) {
    SetHiddenWindow(false);

    ClearPreloadState();

    if (WebAppManager::Instance()->Config()->IsCheckLaunchTimeEnabled())
      StartLaunchTimer();

    if (KeepAlive() && (Page()->Progress() != 100))
      added_to_window_mgr_ = false;

    // if we're already loaded, then show, else clear the hidden flag, and
    // show as normal when loaded and ready to render
    if (added_to_window_mgr_ ||
        (Page()->Progress() == 100 && Page()->HasBeenShown()))
      ShowWindow();
  }

  if (GetCrashState()) {
    LOG_INFO(MSGID_APP_RELAUNCH, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", Page()->GetWebProcessPID()),
             "Crashed in Background; Reluad Default page");
    Page()->ReloadDefaultPage();
    SetCrashState(false);
  }

  if (app_private_->page_) {
    WebPageBase* page = app_private_->page_;
    // try to do relaunch!!
    if (!(page->Relaunch(args, LaunchingAppId()))) {
      LOG_INFO(MSGID_APP_RELAUNCH, 3, PMLOGKS("APP_ID", AppId().c_str()),
               PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
               PMLOGKFV("PID", "%d", page->GetWebProcessPID()),
               "Can't handle Relaunch now, backup the args and handle it after "
               "page loading finished");
      // if relaunch hasn't beeh executed, then set and wait till currnt page
      // loading is finished
      in_progress_relaunch_params_ = args;
      in_progress_relaunch_launching_app_id_ = launching_app_id;
      return;
    }

    if (app_private_->app_desc_ &&
        !(app_private_->app_desc_->HandlesRelaunch())) {
      LOG_DEBUG(
          "[%s]:[%s] m_appDesc->handlesRelaunch : false, call raise() to make "
          "it full screen",
          AppId().c_str(), InstanceId().c_str());
      Raise();
    } else {
      LOG_INFO(MSGID_APP_RELAUNCH, 3, PMLOGKS("APP_ID", AppId().c_str()),
               PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
               PMLOGKFV("PID", "%d", page->GetWebProcessPID()),
               "handlesRelanch : true; Do not call raise()");
    }
  }
}

void WebAppBase::WebPageLoadFinished() {
  DoPendingRelaunch();
}

void WebAppBase::DoPendingRelaunch() {
  if (in_progress_relaunch_launching_app_id_.size() ||
      in_progress_relaunch_params_.size()) {
    LOG_INFO(MSGID_APP_RELAUNCH, 3, PMLOGKS("APP_ID", AppId().c_str()),
             PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
             PMLOGKFV("PID", "%d", Page()->GetWebProcessPID()),
             "Page loading --> done; Do pending Relaunch");
    Relaunch(in_progress_relaunch_params_,
             in_progress_relaunch_launching_app_id_);

    in_progress_relaunch_params_.clear();
    in_progress_relaunch_launching_app_id_.clear();
  }
}

void WebAppBase::WebPageClosePageRequested() {
  LOG_INFO(MSGID_WINDOW_CLOSED_JS, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", Page()->GetWebProcessPID()), "%s%s",
           close_page_requested_ ? "duplicated window.close();" : "",
           IsClosing() ? "app is closing; drop this window.close()" : "");
  if (IsClosing() || close_page_requested_)
    return;

  close_page_requested_ = true;
  WebAppManager::Instance()->CloseApp(InstanceId());
}

void WebAppBase::StagePreparing() {
  // just has some perf testing
}

void WebAppBase::StageReady() {
  // NOT IMPLEMENTED
}

void WebAppBase::ShowWindow() {
  // Set the accessibility after the application launched
  // because the chromium can generate huge amount of AXEvent during app
  // loading.
  SetUseAccessibility(WebAppManager::Instance()->IsAccessibilityEnabled());
}

void WebAppBase::SetAppDescription(
    std::shared_ptr<ApplicationDescription> app_desc) {
  app_private_->app_desc_ = app_desc;

  // set appId here from appDesc
  app_private_->app_id_ = app_desc->Id();
}

void WebAppBase::SetAppProperties(const std::string& properties) {
  Json::Value json = util::StringToJson(properties);
  bool keep_alive = false;

  if (json["keepAlive"].isBool() && json["keepAlive"].asBool()) {
    keep_alive = true;
  }
  SetKeepAlive(keep_alive);

  if (json["launchedHidden"].isBool() && json["launchedHidden"].asBool())
    SetHiddenWindow(true);
}

void WebAppBase::SetPreloadState(const std::string& properties) {
  Json::Value obj = util::StringToJson(properties);

  std::string preload = obj["preload"].asString();

  if (preload == "full") {
    preload_state_ = kFullPreload;
  } else if (preload == "semi-full") {
    preload_state_ = kSemiFullPreload;
  } else if (preload == "partial") {
    preload_state_ = kPartialPreload;
  } else if (preload == "minimal") {
    preload_state_ = kMinimalPreload;
  } else if (obj["launchedHidden"].asBool()) {
    preload_state_ = kPartialPreload;
  }

  if (preload_state_ != kNonePreload)
    SetHiddenWindow(true);

  // set PreloadEnvironment needs attaching WebPageBase.
  if (!app_private_->page_)
    return;

  switch (preload_state_) {
    case kFullPreload:
      // TODO : implement full preload when rule is set.
      break;
    case kSemiFullPreload:
      app_private_->page_->SetAppPreloadHint(true);
      app_private_->page_->SuspendWebPageMedia();
      break;
    case kPartialPreload:
      app_private_->page_->SetAppPreloadHint(true);
      app_private_->page_->SuspendWebPageMedia();
      app_private_->page_->DeactivateRendererCompositor();
      break;
    case kMinimalPreload:
      // TODO : implement minimal preloaded when rule is set.
      break;
    default:
      break;
  }
  app_private_->page_->SetIsPreload(preload_state_ != kNonePreload ? true
                                                                   : false);
}

void WebAppBase::ClearPreloadState() {
  // set PreloadEnvironment needs attaching WebPageBase.
  if (!app_private_->page_) {
    preload_state_ = kNonePreload;
    return;
  }

  switch (preload_state_) {
    case kFullPreload:
      // TODO : implement full preload when rule is set.
      break;
    case kSemiFullPreload:
      app_private_->page_->SetAppPreloadHint(false);
      app_private_->page_->ResumeWebPageMedia();
      break;
    case kPartialPreload:
      app_private_->page_->SetAppPreloadHint(false);
      app_private_->page_->ResumeWebPageMedia();
      app_private_->page_->ActivateRendererCompositor();
      break;
    case kMinimalPreload:
      // TODO : implement minimal preloaded when rule is set.
      break;
    default:
      break;
  }
  preload_state_ = kNonePreload;
  app_private_->page_->SetIsPreload(false);
}

void WebAppBase::SetUiSize(int width, int height) {
  WebAppManager::Instance()->SetUiSize(width, height);
}

void WebAppBase::SetPreferredLanguages(const std::string& language) {
  if (!app_private_->page_)
    return;
  app_private_->page_->SetPreferredLanguages(language);
  app_private_->page_->SendLocaleChangeEvent(language);
}

void WebAppBase::HandleWebAppMessage(WebAppManager::WebAppMessageType type,
                                     const std::string& message) {
  if (!app_private_->page_)
    return;

  if (type == WebAppManager::WebAppMessageType::kDeviceInfoChanged)
    app_private_->page_->HandleDeviceInfoChanged(message);
}

void WebAppBase::SetUseAccessibility(bool enabled) {
  bool use_accessibility = false;

  LOG_DEBUG(
      "setUseAccessibility : supportsAudioGuidance = %s, LoadErrorPage = %s",
      app_private_->app_desc_->SupportsAudioGuidance() ? "true" : "false",
      Page()->IsLoadErrorPageStart() ? "true" : "false");

  // Check the supportsAudioGuidance attribute to appinfo.json to use this
  // feature or page is loadErrorPage for each app.
  if (enabled && (app_private_->app_desc_->SupportsAudioGuidance() ||
                  Page()->IsLoadErrorPageStart())) {
    use_accessibility = true;
  }

  Page()->SetUseAccessibility(use_accessibility);
}

void WebAppBase::ExecuteCloseCallback() {
  Page()->ExecuteCloseCallback(ForceClose());
  LOG_INFO(MSGID_EXECUTE_CLOSECALLBACK, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", Page()->GetWebProcessPID()), "");
}

void WebAppBase::CloseCallbackExecuted() {
  CloseWebApp();
}

void WebAppBase::TimeoutExecuteCloseCallback() {
  CloseWebApp();
}

void WebAppBase::ClosingAppProcessDidCrashed() {
  CloseWebApp();
}

void WebAppBase::DidDispatchUnload() {
  CloseWebApp();
}

void WebAppBase::CloseWebApp() {
  LOG_INFO(MSGID_CLEANRESOURCE_COMPLETED, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKFV("PID", "%d", Page()->GetWebProcessPID()),
           "closeCallback/about:blank is DONE");
  WebAppManager::Instance()->RemoveClosingAppList(AppId());
  delete this;
}

void WebAppBase::DispatchUnload() {
  Page()->CleanResources();
}

void WebAppBase::OnCursorVisibilityChanged(const std::string& jsscript) {
  WebAppManager::Instance()->SendEventToAllAppsAndAllFrames(jsscript);
}

void WebAppBase::ServiceCall(const std::string& url,
                             const std::string& payload,
                             const std::string& app_id) {
  LOG_INFO(MSGID_SERVICE_CALL, 3, PMLOGKS("APP_ID", AppId().c_str()),
           PMLOGKS("INSTANCE_ID", InstanceId().c_str()),
           PMLOGKS("URL", url.c_str()), "");
  WebAppManager::Instance()->ServiceCall(url, payload, app_id);
}

void WebAppBase::KeyboardVisibilityChanged(bool visible, int height) {
  if (Page())
    Page()->KeyboardVisibilityChanged(visible);
}

bool WebAppBase::IsClosing() const {
  return app_private_->page_->IsClosing();
}

bool WebAppBase::IsCheckLaunchTimeEnabled() {
  return WebAppManager::Instance()->Config()->IsCheckLaunchTimeEnabled();
}
