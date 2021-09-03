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

#include "web_app_manager_config.h"

#include <unistd.h>

#include "utils.h"

WebAppManagerConfig::WebAppManagerConfig()
    : suspend_delay_time_(0),
      max_custom_suspend_delay_time_(0),
      dev_mode_enabled_(false),
      inspector_enabled_(false),
      dynamic_pluggable_load_enabled_(false),
      post_web_process_created_disabled_(false),
      check_launch_time_enabled_(false),
      use_system_app_optimization_(false),
      launch_optimization_enabled_(false) {
  InitConfiguration();
}

std::string WebAppManagerConfig::WamGetEnv(const char* name) {
  return util::GetEnvVar(name);
}

void WebAppManagerConfig::InitConfiguration() {
  web_app_factory_plugin_types_ = WamGetEnv("WEBAPPFACTORY");

  web_app_factory_plugin_path_ = WamGetEnv("WEBAPPFACTORY_PLUGIN_PATH");
  if (web_app_factory_plugin_path_.empty()) {
    web_app_factory_plugin_path_ = "/usr/lib/webappmanager/plugins";
  }

  std::string suspend_delay = WamGetEnv("WAM_SUSPEND_DELAY_IN_MS");
  int suspend_delay_int = util::StrToIntWithDefault(suspend_delay, 0);
  suspend_delay_time_ = std::max(suspend_delay_int, 1);

  std::string max_custom_suspend_delay =
      WamGetEnv("MAX_CUSTOM_SUSPEND_DELAY_IN_MS");
  int max_custom_suspend_delay_int =
      util::StrToIntWithDefault(max_custom_suspend_delay, 0);
  max_custom_suspend_delay_time_ = std::max(max_custom_suspend_delay_int, 0);

  web_process_config_path_ = WamGetEnv("WEBPROCESS_CONFIGURATION_PATH");
  if (web_process_config_path_.empty())
    web_process_config_path_ = "/etc/wam/com.webos.wam.json";

  error_page_url_ = WamGetEnv("WAM_ERROR_PAGE");

  dynamic_pluggable_load_enabled_ =
      WamGetEnv("LOAD_DYNAMIC_PLUGGABLE").compare("1") == 0;

  post_web_process_created_disabled_ =
      WamGetEnv("POST_WEBPROCESS_CREATED_DISABLED").compare("1") == 0;

  check_launch_time_enabled_ = WamGetEnv("LAUNCH_TIME_CHECK").compare("1") == 0;

  use_system_app_optimization_ =
      WamGetEnv("USE_SYSTEM_APP_OPTIMIZATION").compare("1") == 0;

  launch_optimization_enabled_ =
      WamGetEnv("ENABLE_LAUNCH_OPTIMIZATION").compare("1") == 0;

  user_script_path_ = WamGetEnv("USER_SCRIPT_PATH");
  if (user_script_path_.empty())
    user_script_path_ = "webOSUserScripts/userScript.js";

  name_ = WamGetEnv("WAM_NAME");
}

void WebAppManagerConfig::PostInitConfiguration() {
  if (access("/var/luna/preferences/debug_system_apps", F_OK) == 0) {
    inspector_enabled_ = true;
  }

  if (access("/var/luna/preferences/devmode_enabled", F_OK) == 0) {
    dev_mode_enabled_ = true;
    tellurium_nub_path_ = WamGetEnv("TELLURIUM_NUB_PATH");
  }
}

void WebAppManagerConfig::ResetConfiguration() {
  suspend_delay_time_ = 0;
  max_custom_suspend_delay_time_ = 0;

  dev_mode_enabled_ = false;
  inspector_enabled_ = false;
  dynamic_pluggable_load_enabled_ = false;
  post_web_process_created_disabled_ = false;
  check_launch_time_enabled_ = false;
  use_system_app_optimization_ = false;
  launch_optimization_enabled_ = false;

  web_app_factory_plugin_types_.clear();
  web_app_factory_plugin_path_.clear();
  web_process_config_path_.clear();
  error_page_url_.clear();
  tellurium_nub_path_.clear();
  user_script_path_.clear();
  name_.clear();

  InitConfiguration();
}
