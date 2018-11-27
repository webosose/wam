// Copyright (c) 2014-2018 LG Electronics, Inc.
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

#include "WebAppManagerConfig.h"

#include <unistd.h>

#include "WebAppManagerUtils.h"

WebAppManagerConfig::WebAppManagerConfig()
    : m_suspendDelayTime(0)
    , m_devModeEnabled(false)
    , m_inspectorEnabled(false)
    , m_containerAppEnabled(true)
    , m_dynamicPluggableLoadEnabled(false)
    , m_postWebProcessCreatedDisabled(false)
    , m_checkLaunchTimeEnabled(false)
    , m_useSystemAppOptimization(false)
    , m_launchOptimizationEnabled(false)
{
    initConfiguration();
}

void WebAppManagerConfig::initConfiguration()
{
    m_webAppFactoryPluginTypes = WebAppManagerUtils::getEnv("WEBAPPFACTORY");

    m_webAppFactoryPluginPath = WebAppManagerUtils::getEnv("WEBAPPFACTORY_PLUGIN_PATH");
    if (m_webAppFactoryPluginPath.empty()) {
        m_webAppFactoryPluginPath = "/usr/lib/webappmanager/plugins";
    }

    std::string suspendDelay = WebAppManagerUtils::getEnv("WAM_SUSPEND_DELAY_IN_MS");
    m_suspendDelayTime = std::max(std::stoi(suspendDelay), 1);

    m_webProcessConfigPath = WebAppManagerUtils::getEnv("WEBPROCESS_CONFIGURATION_PATH");
    if (m_webProcessConfigPath.empty())
        m_webProcessConfigPath = "/etc/wam/com.webos.wam.json";

    m_errorPageUrl = WebAppManagerUtils::getEnv("WAM_ERROR_PAGE");

    if (WebAppManagerUtils::getEnv("DISABLE_CONTAINER") == "1")
        m_containerAppEnabled = false;

    if (WebAppManagerUtils::getEnv("LOAD_DYNAMIC_PLUGGABLE") == "1")
        m_dynamicPluggableLoadEnabled = true;

    if (WebAppManagerUtils::getEnv("POST_WEBPROCESS_CREATED_DISABLED") == "1")
        m_postWebProcessCreatedDisabled =  true;

    if (WebAppManagerUtils::getEnv("LAUNCH_TIME_CHECK") == "1")
        m_checkLaunchTimeEnabled = true;

    if (WebAppManagerUtils::getEnv("USE_SYSTEM_APP_OPTIMIZATION") == "1")
        m_useSystemAppOptimization = true;

    if (WebAppManagerUtils::getEnv("ENABLE_LAUNCH_OPTIMIZATION") == "1")
        m_launchOptimizationEnabled = true;

    m_userScriptPath = WebAppManagerUtils::getEnv("USER_SCRIPT_PATH");
    if (m_userScriptPath.empty())
        m_userScriptPath = "webOSUserScripts/userScript.js";

    m_name = WebAppManagerUtils::getEnv("WAM_NAME");
}

void WebAppManagerConfig::postInitConfiguration()
{
    if (access("/var/luna/preferences/debug_system_apps", F_OK) == 0) {
        m_inspectorEnabled = true;
    }

    if (access("/var/luna/preferences/devmode_enabled", F_OK) == 0) {
        m_devModeEnabled = true;
        m_telluriumNubPath = WebAppManagerUtils::getEnv("TELLURIUM_NUB_PATH");
    }
}
