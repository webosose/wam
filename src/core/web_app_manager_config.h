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

#ifndef CORE_WEB_APP_MANAGER_CONFIG_H_
#define CORE_WEB_APP_MANAGER_CONFIG_H_

#include <string>

class WebAppManagerConfig {
 public:
  WebAppManagerConfig();
  virtual ~WebAppManagerConfig() {}

  virtual std::string GetWebAppFactoryPluginTypes() const {
    return web_app_factory_plugin_types_;
  }
  virtual std::string GetWebAppFactoryPluginPath() const {
    return web_app_factory_plugin_path_;
  }
  virtual int GetSuspendDelayTime() const { return suspend_delay_time_; }
  virtual int GetMaxCustomSuspendDelayTime() const {
    return max_custom_suspend_delay_time_;
  }
  virtual std::string GetWebProcessConfigPath() const {
    return web_process_config_path_;
  }
  virtual bool IsInspectorEnabled() const { return inspector_enabled_; }
  virtual bool IsDevModeEnabled() const { return dev_mode_enabled_; }
  virtual std::string GetErrorPageUrl() const { return error_page_url_; }
  virtual std::string GetTelluriumNubPath() const {
    return tellurium_nub_path_;
  }
  virtual void PostInitConfiguration();
  virtual bool IsDynamicPluggableLoadEnabled() const {
    return dynamic_pluggable_load_enabled_;
  }
  virtual bool IsPostWebProcessCreatedDisabled() const {
    return post_web_process_created_disabled_;
  }
  virtual bool IsCheckLaunchTimeEnabled() const {
    return check_launch_time_enabled_;
  }
  virtual bool IsUseSystemAppOptimization() const {
    return use_system_app_optimization_;
  }
  virtual std::string GetUserScriptPath() const { return user_script_path_; }
  virtual std::string GetName() const { return name_; }

  virtual bool IsLaunchOptimizationEnabled() const {
    return launch_optimization_enabled_;
  }

 protected:
  virtual std::string WamGetEnv(const char* name);
  void ResetConfiguration();

 private:
  void InitConfiguration();

  std::string web_app_factory_plugin_types_;
  std::string web_app_factory_plugin_path_;
  int suspend_delay_time_;
  int max_custom_suspend_delay_time_;
  std::string web_process_config_path_;
  bool dev_mode_enabled_;
  bool inspector_enabled_;
  std::string error_page_url_;
  std::string tellurium_nub_path_;
  bool dynamic_pluggable_load_enabled_;
  bool post_web_process_created_disabled_;
  bool check_launch_time_enabled_;
  bool use_system_app_optimization_;
  bool launch_optimization_enabled_;
  std::string user_script_path_;
  std::string name_;
};

#endif  // CORE_WEB_APP_MANAGER_CONFIG_H_
