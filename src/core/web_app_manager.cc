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

#include "web_app_manager.h"

#include <unistd.h>
#include <algorithm>
#include <sstream>
#include <string>

#include <json/value.h>
#include "webos/application_installation_handler.h"
#include "webos/public/runtime.h"

#include "application_description.h"
#include "base_check.h"
#include "device_info.h"
#include "log_manager.h"
#include "network_status_manager.h"
#include "platform_module_factory.h"
#include "service_sender.h"
#include "util/url.h"
#include "utils.h"
#include "web_app_base.h"
#include "web_app_factory_manager_impl.h"
#include "web_app_manager_config.h"
#include "web_app_manager_service.h"
#include "web_app_manager_tracer.h"
#include "web_page_base.h"
#include "web_process_manager.h"
#include "window_types.h"

static const int kContinuousReloadingLimit = 3;

WebAppManager* WebAppManager::Instance() {
  // not a leak -- static variable initializations are only ever done once
  static WebAppManager* instance = new WebAppManager();
  return instance;
}

WebAppManager::WebAppManager()
    : deleting_pages_(false),
      network_status_manager_(new NetworkStatusManager()),
      suspend_delay_(0),
      max_custom_suspend_delay_(0),
      is_accessibility_enabled_(false) {}

WebAppManager::~WebAppManager() {
  if (device_info_)
    device_info_->Terminate();
}

void WebAppManager::NotifyMemoryPressure(
    webos::WebViewBase::MemoryPressureLevel level) {
  std::list<const WebAppBase*> app_list = RunningApps();
  for (auto it = app_list.begin(); it != app_list.end(); ++it) {
    const WebAppBase* app = *it;
    // Skip memory pressure handling on preloaded apps if chromium pressure is
    // critical (when system is on low or critical) because they will be killed
    // anyway
    if (app->IsActivated() &&
        (!app->Page()->IsPreload() ||
         level != webos::WebViewBase::MEMORY_PRESSURE_CRITICAL))
      app->Page()->NotifyMemoryPressure(level);
    else {
      LOG_DEBUG(
          "Skipping memory pressure handler for"
          " instanceId(%s) appId(%s) isActivated(%d) isPreload(%d) Level(%d)",
          app->InstanceId().c_str(), app->AppId().c_str(), app->IsActivated(),
          app->Page()->IsPreload(), level);
    }
  }
}

void WebAppManager::SetPlatformModules(
    std::unique_ptr<PlatformModuleFactory> factory) {
  web_app_manager_config_ = factory->GetWebAppManagerConfig();
  service_sender_ = factory->GetServiceSender();
  web_process_manager_ = factory->GetWebProcessManager();
  device_info_ = factory->GetDeviceInfo();
  device_info_->Initialize();

  LoadEnvironmentVariable();
}

void WebAppManager::SetWebAppFactory(
    std::unique_ptr<WebAppFactoryManager> factory) {
  web_app_factory_ = std::move(factory);
}

bool WebAppManager::Run() {
  LoadEnvironmentVariable();
  return true;
}

void WebAppManager::Quit() {}

WebAppFactoryManager* WebAppManager::GetWebAppFactory() {
  return web_app_factory_ ? web_app_factory_.get()
                          : WebAppFactoryManagerImpl::Instance();
}

void WebAppManager::LoadEnvironmentVariable() {
  suspend_delay_ = web_app_manager_config_->GetSuspendDelayTime();
  max_custom_suspend_delay_ =
      web_app_manager_config_->GetMaxCustomSuspendDelayTime();
  web_app_manager_config_->PostInitConfiguration();
}

void WebAppManager::SetUiSize(int width, int height) {
  if (device_info_) {
    device_info_->SetDisplayWidth(width);
    device_info_->SetDisplayHeight(height);
  }
}

int WebAppManager::CurrentUiWidth() {
  int width = 0;
  if (device_info_)
    device_info_->GetDisplayWidth(width);
  return width;
}

int WebAppManager::CurrentUiHeight() {
  int height = 0;
  if (device_info_)
    device_info_->GetDisplayHeight(height);
  return height;
}

bool WebAppManager::GetSystemLanguage(std::string& value) {
  if (!device_info_)
    return false;
  return device_info_->GetSystemLanguage(value);
}

bool WebAppManager::GetDeviceInfo(const std::string& name, std::string& value) {
  if (!device_info_)
    return false;
  return device_info_->GetDeviceInfo(name, value);
}

void WebAppManager::OnRelaunchApp(const std::string& instance_id,
                                  const std::string& app_id,
                                  const std::string& args,
                                  const std::string& launching_app_id) {
  WebAppBase* app = FindAppByInstanceId(instance_id);

  if (!app) {
    LOG_WARNING(MSGID_APP_RELAUNCH, 0,
                "Failed to relaunch due to no running app");
    return;
  }

  if (app->AppId() != app_id) {
    LOG_WARNING(MSGID_APP_RELAUNCH, 0,
                "Failed to relaunch due to no running app named %s",
                app_id.c_str());
  }

  // Do not relaunch when preload args is setted
  // luna-send -n 1 luna://com.webos.applicationManager/launch '{"id":<AppId>
  // "preload":<PreloadState> }'
  Json::Value json = util::StringToJson(args);

  if (!json.isObject()) {
    LOG_WARNING(MSGID_APP_RELAUNCH, 0, "Failed to parse json args: '%s'",
                args.c_str());
    return;
  }

  // if this app is KeepAlive and window.close() was once and relaunch now no
  // matter preloaded, fastswitching, launch by launch API need to clear the
  // flag if it needs
  if (app->KeepAlive() && app->ClosePageRequested())
    app->SetClosePageRequested(false);

  if (!json["preload"].isString() && !json["launchedHidden"].asBool()) {
    app->Relaunch(args.c_str(), launching_app_id.c_str());
  } else {
    LOG_INFO(MSGID_WAM_DEBUG, 3, PMLOGKS("APP_ID", app->AppId().c_str()),
             PMLOGKS("INSTANCE_ID", instance_id.c_str()),
             PMLOGKFV("PID", "%d", app->Page()->GetWebProcessPID()),
             "Relaunch with preload option, ignore");
  }
}

bool WebAppManager::PurgeSurfacePool(uint32_t pid) {
  return true;  // Deprecated (2016-04-01)
}

bool WebAppManager::IsDiscardCodeCacheRequired() {
  return false;  // Deprecated (2016-04-01)
}

bool WebAppManager::SetInspectorEnable(const std::string& app_id) {
  // 1. find appId from then running App List,
  for (AppList::const_iterator it = app_list_.begin(); it != app_list_.end();
       ++it) {
    WebAppBase* app = (*it);
    if (app_id == app->Page()->AppId()) {
      LOG_DEBUG("[%s] setInspectorEnable", app_id.c_str());
      app->Page()->SetInspectorEnable();
      return true;
    }
  }
  return false;
}

void WebAppManager::DiscardCodeCache(uint32_t pid) {
  // Deprecated (2016-04-01)
}

void WebAppManager::OnShutdownEvent() {
#if defined(TARGET_DESKTOP)

  for (AppList::const_iterator it = app_list_.begin(); it != app_list_.end();
       ++it) {
    delete (*it);
  }

//		Palm::WebGlobal::garbageCollectNow();
#endif
  return;
}

bool WebAppManager::OnKillApp(const std::string& app_id,
                              const std::string& instance_id,
                              bool force) {
  WebAppBase* app = FindAppByInstanceId(instance_id);
  if (app == nullptr || (app->AppId() != app_id)) {
    LOG_INFO(MSGID_KILL_APP, 2, PMLOGKS("APP_ID", app_id.c_str()),
             PMLOGKS("INSTANCE_ID", instance_id.c_str()),
             "App doesn't exist; return");
    return false;
  }

  if (force)
    ForceCloseAppInternal(app);
  else
    CloseAppInternal(app);
  return true;
}

bool WebAppManager::OnPauseApp(const std::string& instance_id) {
  if (WebAppBase* app = FindAppByInstanceId(instance_id)) {
    // although, name of the handler-function as well as the code it
    // contains are not consistent, according to the "pauseApp" Luna API
    // design, a "paused" application shall be just hidden by WAM
    app->HideWindow();
    return true;
  }

  LOG_INFO(MSGID_PAUSE_APP, 1, PMLOGKS("INSTANCE_ID", instance_id.c_str()),
           "Application not found.");
  return false;
}

std::list<const WebAppBase*> WebAppManager::RunningApps() {
  std::list<const WebAppBase*> apps;

  for (AppList::const_iterator it = app_list_.begin(); it != app_list_.end();
       ++it) {
    apps.push_back(*it);
  }

  return apps;
}

std::list<const WebAppBase*> WebAppManager::RunningApps(uint32_t pid) {
  std::list<const WebAppBase*> apps;

  for (AppList::const_iterator it = app_list_.begin(); it != app_list_.end();
       ++it) {
    WebAppBase* app = (*it);

    if (app->Page()->GetWebProcessPID() == pid)
      apps.push_back(app);
  }

  return apps;
}

WebAppBase* WebAppManager::OnLaunchUrl(
    const std::string& url,
    const std::string& win_type,
    std::shared_ptr<ApplicationDescription> app_desc,
    const std::string& instance_id,
    const std::string& args,
    const std::string& launching_app_id,
    int& err_code,
    std::string& err_msg) {
  WebAppFactoryManager* factory = GetWebAppFactory();
  WebAppBase* app = factory->CreateWebApp(win_type.c_str(), app_desc,
                                          app_desc->SubType().c_str());

  if (!app) {
    err_code = kErrCodeLaunchappUnsupportedType;
    err_msg = kErrUnsupportedType;
    return nullptr;
  }

  WebPageBase* page =
      factory->CreateWebPage(win_type.c_str(), wam::Url(url.c_str()), app_desc,
                             app_desc->SubType().c_str(), args.c_str());

  // set use launching time optimization true while app loading.
  page->SetUseLaunchOptimization(true);

  if (win_type == kWtFloating || win_type == kWtCard)
    page->SetEnableBackgroundRun(app_desc->IsEnableBackgroundRun());

  app->SetAppDescription(app_desc);
  app->SetAppProperties(args);
  app->SetInstanceId(instance_id);
  app->SetLaunchingAppId(launching_app_id);
  if (web_app_manager_config_->IsCheckLaunchTimeEnabled())
    app->StartLaunchTimer();
  app->Attach(page);
  app->SetPreloadState(args);

  page->Load();
  WebPageAdded(page);

  app_list_.push_back(app);

  if (app_version_.find(app_desc->Id()) != app_version_.end()) {
    if (app_version_[app_desc->Id()] != app_desc->Version()) {
      app->SetNeedReload(true);
      app_version_[app_desc->Id()] = app_desc->Version();
    }
  } else {
    app_version_[app_desc->Id()] = app_desc->Version();
  }

  LOG_INFO(MSGID_START_LAUNCHURL, 3, PMLOGKS("APP_ID", app->AppId().c_str()),
           PMLOGKS("INSTANCE_ID", app->InstanceId().c_str()),
           PMLOGKFV("PID", "%d", app->Page()->GetWebProcessPID()), "");

  return app;
}

void WebAppManager::ForceCloseAppInternal(WebAppBase* app) {
  app->SetKeepAlive(false);
  CloseAppInternal(app);
}

void WebAppManager::RemoveClosingAppList(const std::string& instance_id) {
  while (closing_app_list_.find(instance_id) != closing_app_list_.end()) {
    closing_app_list_.erase(instance_id);
  }
}

void WebAppManager::CloseAppInternal(WebAppBase* app,
                                     bool ignore_clean_resource) {
  WebPageBase* page = app->Page();
  UTIL_ASSERT(page);
  if (page->IsClosing()) {
    LOG_INFO(MSGID_CLOSE_APP_INTERNAL, 3,
             PMLOGKS("APP_ID", app->AppId().c_str()),
             PMLOGKS("INSTANCE_ID", app->InstanceId().c_str()),
             PMLOGKFV("PID", "%d", app->Page()->GetWebProcessPID()),
             "In Closing; return");
  }

  LOG_INFO(MSGID_CLOSE_APP_INTERNAL, 3, PMLOGKS("APP_ID", app->AppId().c_str()),
           PMLOGKS("INSTANCE_ID", app->InstanceId().c_str()),
           PMLOGKFV("PID", "%d", app->Page()->GetWebProcessPID()), "");
  if (app->KeepAlive() && app->HideWindow())
    return;

  std::string type = app->GetAppDescription()->DefaultWindowType();
  AppDeleted(app);
  WebPageRemoved(app->Page());
  RemoveWebAppFromWebProcessInfoMap(app->AppId());
  PostRunningAppList();
  last_crashed_app_ids_ = std::unordered_map<std::string, int>();

  // Set m_isClosing flag first, this flag will be checked in web page
  // suspending
  page->SetClosing(true);
  app->DeleteSurfaceGroup();
  // Do suspend WebPage
  if (type == "overlay")
    app->Hide(true);
  else
    app->OnStageDeactivated();

  if (ignore_clean_resource)
    delete app;
  else {
    closing_app_list_.emplace(app->InstanceId(), app);

    if (page->IsRegisteredCloseCallback()) {
      LOG_INFO(MSGID_CLOSE_APP_INTERNAL, 3,
               PMLOGKS("APP_ID", app->AppId().c_str()),
               PMLOGKS("INSTANCE_ID", app->InstanceId().c_str()),
               PMLOGKFV("PID", "%d", app->Page()->GetWebProcessPID()),
               "CloseCallback; execute");
      app->ExecuteCloseCallback();
    } else {
      LOG_INFO(MSGID_CLOSE_APP_INTERNAL, 3,
               PMLOGKS("APP_ID", app->AppId().c_str()),
               PMLOGKS("INSTANCE_ID", app->InstanceId().c_str()),
               PMLOGKFV("PID", "%d", app->Page()->GetWebProcessPID()),
               "NO CloseCallback; load about:blank");
      app->DispatchUnload();
    }
  }
}

bool WebAppManager::CloseAllApps(uint32_t pid) {
  AppList running_apps;

  for (AppList::iterator it = app_list_.begin(); it != app_list_.end(); ++it) {
    WebAppBase* app = (*it);
    if (!pid)
      running_apps.insert(running_apps.end(), app);
    else if (web_process_manager_->GetWebProcessPID(app) == pid)
      running_apps.insert(running_apps.end(), app);
  }

  AppList::iterator it = running_apps.begin();
  while (it != running_apps.end()) {
    WebAppBase* app = (*it);
    ForceCloseAppInternal(app);
    // closeAppInternal will cause the app pointed to to become invalid,
    // so remove it from the list so we don't act upon it after that
    it = running_apps.erase(it);
  }

  return running_apps.empty();
}

void WebAppManager::WebPageAdded(WebPageBase* page) {
  std::string app_id = page->AppId();

  auto found = find_if(app_page_map_.begin(), app_page_map_.end(),
                       [&](const auto& item) {
                         return (item.first == app_id) && (item.second == page);
                       });

  if (found == app_page_map_.end()) {
    app_page_map_.emplace(app_id, page);
  }
}

void WebAppManager::WebPageRemoved(WebPageBase* page) {
  if (!deleting_pages_) {
    // Remove from list of pending delete pages
    PageList::iterator iter = std::find(pages_to_delete_list_.begin(),
                                        pages_to_delete_list_.end(), page);
    if (iter != pages_to_delete_list_.end()) {
      pages_to_delete_list_.erase(iter);
    }
  }

  auto range = app_page_map_.equal_range(page->AppId());
  auto it = range.first;
  while (it != range.second) {
    if (it->second == page) {
      it = app_page_map_.erase(it);
    } else {
      it++;
    }
  }
}

void WebAppManager::RemoveWebAppFromWebProcessInfoMap(
    const std::string& appId) {
  // Deprecated (2016-04-01)
}

WebAppBase* WebAppManager::FindAppById(const std::string& app_id) {
  for (AppList::iterator it = app_list_.begin(); it != app_list_.end(); ++it) {
    WebAppBase* app = (*it);

    if (app->Page() && app->AppId() == app_id)
      return app;
  }

  return 0;
}

std::list<WebAppBase*> WebAppManager::FindAppsById(const std::string& appId) {
  std::list<WebAppBase*> apps;
  for (AppList::iterator it = app_list_.begin(); it != app_list_.end(); ++it) {
    WebAppBase* app = (*it);

    if (app->Page() && app->AppId() == appId)
      apps.push_back(app);
  }

  return apps;
}

WebAppBase* WebAppManager::FindAppByInstanceId(const std::string& instance_id) {
  for (AppList::iterator it = app_list_.begin(); it != app_list_.end(); ++it) {
    WebAppBase* app = (*it);

    if (app->Page() && (app->InstanceId() == instance_id))
      return app;
  }

  return 0;
}

void WebAppManager::AppDeleted(WebAppBase* app) {
  if (!app)
    return;

  std::string app_id;
  if (app->Page())
    app_id = app->AppId();

  app_list_.remove(app);

  if (!app_id.empty())
    shell_page_map_.erase(app_id);
}

void WebAppManager::SetSystemLanguage(const std::string& language) {
  if (!device_info_)
    return;

  device_info_->SetSystemLanguage(language);

  for (AppList::const_iterator it = app_list_.begin(); it != app_list_.end();
       ++it) {
    WebAppBase* app = (*it);
    app->SetPreferredLanguages(language);
  }

  LOG_DEBUG("New system language: %s", language.c_str());
}

void WebAppManager::SetDeviceInfo(const std::string& name,
                                  const std::string& value) {
  if (!device_info_)
    return;

  std::string old_value;
  if (device_info_->GetDeviceInfo(name, old_value) && (old_value == value))
    return;

  device_info_->SetDeviceInfo(name, value);
  BroadcastWebAppMessage(WebAppMessageType::kDeviceInfoChanged, name);
  LOG_DEBUG("SetDeviceInfo %s; %s to %s", name.c_str(), old_value.c_str(),
            value.c_str());
}

void WebAppManager::BroadcastWebAppMessage(WebAppMessageType type,
                                           const std::string& message) {
  for (AppList::const_iterator it = app_list_.begin(); it != app_list_.end();
       ++it) {
    WebAppBase* app = (*it);
    app->HandleWebAppMessage(type, message);
  }
}

bool WebAppManager::ProcessCrashed(const std::string& app_id,
                                   const std::string& instance_id) {
  WebAppBase* app = FindAppByInstanceId(instance_id);
  if (!app)
    return false;

  if (app->IsWindowed()) {
    if (app->IsActivated()) {
      last_crashed_app_ids_[app->AppId()]++;
      int reloading_limit = app->IsNormal() ? kContinuousReloadingLimit - 1
                                            : kContinuousReloadingLimit;

      if (last_crashed_app_ids_[app->AppId()] >= reloading_limit) {
        LOG_INFO(MSGID_WEBPROC_CRASH, 4, PMLOGKS("APP_ID", app_id.c_str()),
                 PMLOGKS("INSTANCE_ID", instance_id.c_str()),
                 PMLOGKS("InForeground", "true"),
                 PMLOGKS("Reloading limit", "Close app"), "");
        CloseAppInternal(app, true);
      } else {
        LOG_INFO(MSGID_WEBPROC_CRASH, 4, PMLOGKS("APP_ID", app_id.c_str()),
                 PMLOGKS("INSTANCE_ID", instance_id.c_str()),
                 PMLOGKS("InForeground", "true"),
                 PMLOGKS("Reloading limit", "OK; Reload default page"), "");
        app->Page()->ReloadDefaultPage();
      }
    } else if (app->IsMinimized()) {
      LOG_INFO(MSGID_WEBPROC_CRASH, 3, PMLOGKS("APP_ID", app_id.c_str()),
               PMLOGKS("INSTANCE_ID", instance_id.c_str()),
               PMLOGKS("InBackground", "Will be Reloaded in Relaunch"), "");
      app->SetCrashState(true);
    }
  }
  return true;
}

const std::string WebAppManager::WindowTypeFromString(const std::string& str) {
  if (str == "overlay")
    return kWtOverlay;
  if (str == "popup")
    return kWtPopup;
  if (str == "minimal")
    return kWtMinimal;
  if (str == "floating")
    return kWtFloating;
  if (str == "system_ui")
    return kWtSystemUi;
  return kWtCard;
}

void WebAppManager::SetForceCloseApp(const std::string& app_id,
                                     const std::string& instance_id) {
  WebAppBase* app = FindAppByInstanceId(instance_id);
  if (!app)
    return;

  if (app->IsWindowed()) {
    if (app->KeepAlive() && app->GetHiddenWindow()) {
      ForceCloseAppInternal(app);
      LOG_INFO(MSGID_FORCE_CLOSE_KEEP_ALIVE_APP, 2,
               PMLOGKS("APP_ID", app_id.c_str()),
               PMLOGKS("INSTANCE_ID", instance_id.c_str()), "");
      return;
    }
  }

  app->SetForceClose();
}

void WebAppManager::RequestKillWebProcess(uint32_t pid) {
  // Deprecated (2016-0401)
}

void WebAppManager::DeleteStorageData(const std::string& identifier) {
  web_process_manager_->DeleteStorageData(
      IdentifierForSecurityOrigin(identifier));
}

void WebAppManager::KillCustomPluginProcess(const std::string& base_path) {
  // Deprecated (2016-04-01)
}

/**
 * Launch an application (webApps only, not native).
 *
 * @param appId The application ID to launch.
 * @param params The call parameters.
 * @param the ID of the application performing the launch (can be NULL).
 * @param errMsg The error message (will be empty if this call was successful).
 *
 * @todo: this should now be moved private and be protected...leaving it for now
 * as to not break stuff and make things slightly faster for intra-sysmgr
 * mainloop launches
 */

std::string WebAppManager::Launch(const std::string& app_desc_string,
                                  const std::string& params,
                                  const std::string& launching_app_id,
                                  int& err_code,
                                  std::string& err_msg) {
  std::shared_ptr<ApplicationDescription> desc(
      ApplicationDescription::FromJsonString(app_desc_string.c_str()));
  if (!desc)
    return std::string();

  std::string url = desc->EntryPoint();
  std::string win_type = WindowTypeFromString(desc->DefaultWindowType());
  err_msg.erase();

  // Set displayAffinity (multi display support)
  Json::Value json = util::StringToJson(params);

  if (!json.isObject()) {
    LOG_WARNING(MSGID_APP_LAUNCH, 0, "Failed to parse params: '%s'",
                params.c_str());
    return std::string();
  }

  Json::Value affinity = json["displayAffinity"];
  if (affinity.isInt())
    desc->SetDisplayAffinity(affinity.asInt());

  std::string instance_id = json["instanceId"].asString();

  // Check if app is already running
  if (IsRunningApp(instance_id)) {
    OnRelaunchApp(instance_id, desc->Id().c_str(), params.c_str(),
                  launching_app_id.c_str());
  } else {
    // Run as a normal app
    if (!OnLaunchUrl(url, win_type, desc, instance_id, params, launching_app_id,
                     err_code, err_msg)) {
      return std::string();
    }
  }

  return instance_id;
}

bool WebAppManager::IsRunningApp(const std::string& id) {
  std::list<const WebAppBase*> running = RunningApps();

  for (auto it = running.begin(); it != running.end(); ++it)
    if ((*it)->InstanceId() == id)
      return true;
  return false;
}

std::vector<ApplicationInfo> WebAppManager::List(bool include_system_apps) {
  std::vector<ApplicationInfo> list;

  std::list<const WebAppBase*> running = RunningApps();
  for (auto it = running.begin(); it != running.end(); ++it) {
    const WebAppBase* web_app_base = *it;
    if (web_app_base->AppId().size() ||
        (!web_app_base->AppId().size() && include_system_apps)) {
      uint32_t pid = web_process_manager_->GetWebProcessPID(web_app_base);
      list.push_back(ApplicationInfo(web_app_base->InstanceId(),
                                     web_app_base->AppId(), pid));
    }
  }

  return list;
}

Json::Value WebAppManager::GetWebProcessProfiling() {
  return web_process_manager_->GetWebProcessProfiling();
}

void WebAppManager::CloseApp(const std::string& app_id) {
  if (service_sender_)
    service_sender_->CloseApp(app_id);
}

void WebAppManager::PostRunningAppList() {
  if (!service_sender_)
    return;

  std::vector<ApplicationInfo> apps = List(true);
  service_sender_->PostlistRunningApps(apps);
}

void WebAppManager::PostWebProcessCreated(const std::string& app_id,
                                          const std::string& instance_id,
                                          uint32_t pid) {
  if (!service_sender_)
    return;

  PostRunningAppList();

  if (!web_app_manager_config_->IsPostWebProcessCreatedDisabled())
    service_sender_->PostWebProcessCreated(app_id, instance_id, pid);
}

uint32_t WebAppManager::GetWebProcessId(const std::string& app_id,
                                        const std::string& instance_id) {
  uint32_t pid = 0;
  WebAppBase* app = FindAppByInstanceId(instance_id);

  if (app && app->AppId() == app_id && web_process_manager_)
    pid = web_process_manager_->GetWebProcessPID(app);

  return pid;
}

std::string WebAppManager::GenerateInstanceId() {
  static int next_process_id = 1000;
  std::ostringstream stream;
  stream << (next_process_id++);

  return stream.str();
}

void WebAppManager::SetAccessibilityEnabled(bool enabled) {
  if (is_accessibility_enabled_ == enabled)
    return;

  for (auto it = app_list_.begin(); it != app_list_.end(); ++it) {
    // set audion guidance on/off on settings app
    if ((*it)->Page())
      (*it)->Page()->SetAudioGuidanceOn(enabled);
    (*it)->SetUseAccessibility(enabled);
  }

  is_accessibility_enabled_ = enabled;
}

void WebAppManager::SendEventToAllAppsAndAllFrames(
    const std::string& jsscript) {
  for (auto it = app_list_.begin(); it != app_list_.end(); ++it) {
    WebAppBase* app = (*it);
    if (app->Page()) {
      LOG_DEBUG("[%s] send event with %s", app->AppId().c_str(),
                jsscript.c_str());
      // to send all subFrame, use this function instead of
      // evaluateJavaScriptInAllFrames()
      app->Page()->EvaluateJavaScriptInAllFrames(jsscript);
    }
  }
}

void WebAppManager::ServiceCall(const std::string& url,
                                const std::string& payload,
                                const std::string& appId) {
  if (service_sender_)
    service_sender_->ServiceCall(url, payload, appId);
}

void WebAppManager::UpdateNetworkStatus(const Json::Value& object) {
  NetworkStatus status;
  status.FromJsonObject(object);

  webos::Runtime::GetInstance()->SetNetworkConnected(
      status.IsInternetConnectionAvailable());
  network_status_manager_->UpdateNetworkStatus(status);
}

bool WebAppManager::IsEnyoApp(const std::string& appId) {
  WebAppBase* app = FindAppById(appId);
  if (app && !app->GetAppDescription()->EnyoVersion().empty())
    return true;

  return false;
}

void WebAppManager::ClearBrowsingData(const int remove_browsing_data_mask) {
  web_process_manager_->ClearBrowsingData(remove_browsing_data_mask);
}

int WebAppManager::MaskForBrowsingDataType(const char* type) {
  return web_process_manager_->MaskForBrowsingDataType(type);
}

void WebAppManager::AppInstalled(const std::string& app_id) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, "App installed; id=%s", app_id.c_str());
  auto p = webos::ApplicationInstallationHandler::GetInstance();
  if (p)
    p->OnAppInstalled(app_id);
}

void WebAppManager::AppRemoved(const std::string& app_id) {
  LOG_INFO(MSGID_WAM_DEBUG, 0, "App removed; id=%s", app_id.c_str());
  auto p = webos::ApplicationInstallationHandler::GetInstance();
  if (p)
    p->OnAppRemoved(app_id);
}

std::string WebAppManager::IdentifierForSecurityOrigin(
    const std::string& identifier) {
  std::string lowcase_identifier = identifier;
  std::transform(lowcase_identifier.begin(), lowcase_identifier.end(),
                 lowcase_identifier.begin(), tolower);

  if (lowcase_identifier != identifier) {
    LOG_WARNING(MSGID_APPID_HAS_UPPERCASE, 0,
                "Application id should not contain capital letters");
  }
  return (lowcase_identifier + webos::WebViewBase::kSecurityOriginPostfix);
}
