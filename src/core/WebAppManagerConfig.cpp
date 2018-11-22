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

#include "StringUtils.h"

static inline std::string getEnvVar(const char *name)
{
    const char *v = getenv("TELLURIUM_NUB_PATH");
    return (v == NULL) ? std::string() : std::string(v);
}

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
    m_webAppFactoryPluginTypes = getEnvVar("WEBAPPFACTORY");

    m_webAppFactoryPluginPath = getEnvVar("WEBAPPFACTORY_PLUGIN_PATH");
    if (m_webAppFactoryPluginPath.empty()) {
        m_webAppFactoryPluginPath = "/usr/lib/webappmanager/plugins";
    }

    std::string suspendDelay = getEnvVar("WAM_SUSPEND_DELAY_IN_MS");
    m_suspendDelayTime = std::max(stringTo<int>(suspendDelay), 1);

    m_webProcessConfigPath = getEnvVar("WEBPROCESS_CONFIGURATION_PATH");
    if (m_webProcessConfigPath.empty())
        m_webProcessConfigPath = "/etc/wam/com.webos.wam.json";

    m_errorPageUrl = getEnvVar("WAM_ERROR_PAGE");

    if (getEnvVar("DISABLE_CONTAINER") == "1")
        m_containerAppEnabled = false;

    if (getEnvVar("LOAD_DYNAMIC_PLUGGABLE") == "1")
        m_dynamicPluggableLoadEnabled = true;

    if (getEnvVar("POST_WEBPROCESS_CREATED_DISABLED") == "1")
        m_postWebProcessCreatedDisabled =  true;

    if (getEnvVar("LAUNCH_TIME_CHECK") == "1")
        m_checkLaunchTimeEnabled = true;

    if (getEnvVar("USE_SYSTEM_APP_OPTIMIZATION") == "1")
        m_useSystemAppOptimization = true;

    if (getEnvVar("ENABLE_LAUNCH_OPTIMIZATION") == "1")
        m_launchOptimizationEnabled = true;

    m_userScriptPath = getEnvVar("USER_SCRIPT_PATH");
    if (m_userScriptPath.empty())
        m_userScriptPath = "webOSUserScripts/userScript.js";

    m_name = getEnvVar("WAM_NAME");
}

QVariant WebAppManagerConfig::getConfiguration(const std::string name)
{
    QVariant value(0);

    auto it = m_configuration.find(name);
    if (it != m_configuration.end()) {
        value = it->second;
    }

    return value;
}

void WebAppManagerConfig::setConfiguration(const std::string& name, QVariant value)
{
    m_configuration.emplace(name, value);
}

void WebAppManagerConfig::postInitConfiguration()
{
    if (access("/var/luna/preferences/debug_system_apps", F_OK) == 0) {
        m_inspectorEnabled = true;
    }

    if (access("/var/luna/preferences/devmode_enabled", F_OK) == 0) {
        m_devModeEnabled = true;
        m_telluriumNubPath = getEnvVar("TELLURIUM_NUB_PATH");
    }
}
