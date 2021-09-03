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

#ifndef CORE_WEB_APP_MANAGER_H_
#define CORE_WEB_APP_MANAGER_H_

#include <list>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "webos/webview_base.h"

class ApplicationDescription;
class DeviceInfo;
class NetworkStatusManager;
class PlatformModuleFactory;
class ServiceSender;
class WebProcessManager;
class WebAppFactoryManager;
class WebAppManagerConfig;
class WebAppBase;
class WebPageBase;

namespace Json {
class Value;
};

class ApplicationInfo {
 public:
  ApplicationInfo(const std::string& in_instance_id,
                  const std::string& in_app_id,
                  const uint32_t& in_pid)
      : instance_id_(in_instance_id), app_id_(in_app_id), pid_(in_pid) {}
  ~ApplicationInfo() {}

  std::string instance_id_;
  std::string app_id_;
  uint32_t pid_;
};

class WebAppManager {
 public:
  enum WebAppMessageType { kDeviceInfoChanged = 1 };

  static WebAppManager* Instance();

  bool GetSystemLanguage(std::string& value);
  bool GetDeviceInfo(const std::string& name, std::string& value);
  void BroadcastWebAppMessage(WebAppMessageType type,
                              const std::string& message);

  WebProcessManager* GetWebProcessManager() {
    return web_process_manager_.get();
  }

  virtual ~WebAppManager();

  void SetPlatformModules(std::unique_ptr<PlatformModuleFactory> factory);
  void SetWebAppFactory(std::unique_ptr<WebAppFactoryManager> factory);
  bool Run();
  void Quit();

  std::list<const WebAppBase*> RunningApps();
  std::list<const WebAppBase*> RunningApps(uint32_t pid);
  WebAppBase* FindAppById(const std::string& app_id);
  std::list<WebAppBase*> FindAppsById(const std::string& app_id);
  WebAppBase* FindAppByInstanceId(const std::string& instance_id);

  std::string Launch(const std::string& app_desc_string,
                     const std::string& params,
                     const std::string& launching_app_id,
                     int& err_code,
                     std::string& err_msg);

  std::vector<ApplicationInfo> List(bool include_system_apps = false);

  Json::Value GetWebProcessProfiling();
  int CurrentUiWidth();
  int CurrentUiHeight();
  void SetUiSize(int width, int height);

  void SetActiveInstanceId(const std::string& id) { active_instance_id_ = id; }
  const std::string GetActiveInstanceId() const { return active_instance_id_; }

  void OnGlobalProperties(int key);
  bool PurgeSurfacePool(uint32_t pid);
  void OnShutdownEvent();
  bool OnKillApp(const std::string& app_id,
                 const std::string& instance_id,
                 bool force = false);
  bool OnPauseApp(const std::string& instance_id);
  bool IsDiscardCodeCacheRequired();
  bool SetInspectorEnable(const std::string& app_id);
  void DiscardCodeCache(uint32_t pid);

  void SetSystemLanguage(const std::string& value);
  void SetDeviceInfo(const std::string& name, const std::string& value);
  WebAppManagerConfig* Config() { return web_app_manager_config_.get(); }

  const std::string WindowTypeFromString(const std::string& str);

  bool CloseAllApps(uint32_t pid = 0);
  void SetForceCloseApp(const std::string& app_id,
                        const std::string& instance_id);
  void RequestKillWebProcess(uint32_t pid);

  int GetSuspendDelay() { return suspend_delay_; }
  int GetMaxCustomSuspendDelay() const { return max_custom_suspend_delay_; }
  void DeleteStorageData(const std::string& identifier);
  void KillCustomPluginProcess(const std::string& base_path);
  bool ProcessCrashed(const std::string& app_id,
                      const std::string& instance_id);

  void CloseAppInternal(WebAppBase* app, bool ignore_clean_resource = false);
  void ForceCloseAppInternal(WebAppBase* app);

  void WebPageAdded(WebPageBase* page);
  void WebPageRemoved(WebPageBase* page);
  void RemoveWebAppFromWebProcessInfoMap(const std::string& app_id);

  void AppDeleted(WebAppBase* app);
  void PostRunningAppList();
  std::string GenerateInstanceId();
  void RemoveClosingAppList(const std::string& instance_id);

  bool IsAccessibilityEnabled() const { return is_accessibility_enabled_; }
  void SetAccessibilityEnabled(bool enabled);
  void PostWebProcessCreated(const std::string& app_id,
                             const std::string& instance_id,
                             uint32_t pid);
  uint32_t GetWebProcessId(const std::string& app_id,
                           const std::string& instance_id);
  void SendEventToAllAppsAndAllFrames(const std::string& jsscript);
  void ServiceCall(const std::string& url,
                   const std::string& payload,
                   const std::string& app_id);
  void UpdateNetworkStatus(const Json::Value& object);
  void NotifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level);

  bool IsEnyoApp(const std::string& app_id);

  void CloseApp(const std::string& app_id);

  void ClearBrowsingData(const int removeBrowsingDataMask);
  int MaskForBrowsingDataType(const char* type);

  void AppInstalled(const std::string& app_id);
  void AppRemoved(const std::string& app_id);

  std::string IdentifierForSecurityOrigin(const std::string& identifier);

 protected:
 private:
  WebAppFactoryManager* GetWebAppFactory();
  void LoadEnvironmentVariable();

  WebAppBase* OnLaunchUrl(const std::string& url,
                          const std::string& win_type,
                          std::shared_ptr<ApplicationDescription> app_desc,
                          const std::string& instance_id,
                          const std::string& args,
                          const std::string& launching_app_id,
                          int& errCode,
                          std::string& err_msg);
  void OnRelaunchApp(const std::string& instance_id,
                     const std::string& app_id,
                     const std::string& args,
                     const std::string& launching_app_id);

  WebAppManager();

  typedef std::list<WebAppBase*> AppList;
  typedef std::list<WebPageBase*> PageList;

  bool IsRunningApp(const std::string& id);
  std::unordered_map<std::string, WebAppBase*> closing_app_list_;

  // Mappings
  std::unordered_map<std::string, WebPageBase*> shell_page_map_;
  AppList app_list_;
  std::unordered_multimap<std::string, WebPageBase*> app_page_map_;

  PageList pages_to_delete_list_;
  bool deleting_pages_;

  std::string active_instance_id_;

  std::unique_ptr<ServiceSender> service_sender_;
  std::unique_ptr<WebProcessManager> web_process_manager_;
  std::unique_ptr<DeviceInfo> device_info_;
  std::unique_ptr<WebAppManagerConfig> web_app_manager_config_;
  std::unique_ptr<NetworkStatusManager> network_status_manager_;
  std::unique_ptr<WebAppFactoryManager> web_app_factory_;

  std::unordered_map<std::string, int> last_crashed_app_ids_;

  int suspend_delay_;
  int max_custom_suspend_delay_;

  std::map<std::string, std::string> app_version_;

  bool is_accessibility_enabled_;
};

#endif  // CORE_WEB_APP_MANAGER_H_
