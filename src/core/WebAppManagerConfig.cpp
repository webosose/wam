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

#include "TypeConverter.h"

WebAppManagerConfig::WebAppManagerConfig()
    : m_suspendDelayTime(0)
    , m_maxCustomSuspendDelayTime(0)
    , m_devModeEnabled(false)
    , m_inspectorEnabled(false)
    , m_dynamicPluggableLoadEnabled(false)
    , m_postWebProcessCreatedDisabled(false)
    , m_checkLaunchTimeEnabled(false)
    , m_useSystemAppOptimization(false)
    , m_launchOptimizationEnabled(false)
{
    initConfiguration();
}

std::string WebAppManagerConfig::wamGetEnv(const char *name)
{
    return qgetenv(name).toStdString();
}

void WebAppManagerConfig::initConfiguration()
{
    m_webAppFactoryPluginTypes = wamGetEnv("WEBAPPFACTORY");

    m_webAppFactoryPluginPath = wamGetEnv("WEBAPPFACTORY_PLUGIN_PATH");
    if (m_webAppFactoryPluginPath.empty()) {
        m_webAppFactoryPluginPath = "/usr/lib/webappmanager/plugins";
    }

    std::string suspendDelay = wamGetEnv("WAM_SUSPEND_DELAY_IN_MS");
    int suspendDelayInt = 0;
    stringToInt(suspendDelay, suspendDelayInt);
    m_suspendDelayTime = std::max(suspendDelayInt, 1);

    std::string maxCustomSuspendDelay = wamGetEnv("MAX_CUSTOM_SUSPEND_DELAY_IN_MS");
    int maxCustomSuspendDelayInt = 0;
    stringToInt(maxCustomSuspendDelay, maxCustomSuspendDelayInt);
    m_maxCustomSuspendDelayTime = std::max(maxCustomSuspendDelayInt, 0);

    m_webProcessConfigPath = wamGetEnv("WEBPROCESS_CONFIGURATION_PATH");
    if (m_webProcessConfigPath.empty())
        m_webProcessConfigPath = "/etc/wam/com.webos.wam.json";

    m_errorPageUrl = wamGetEnv("WAM_ERROR_PAGE");

    m_dynamicPluggableLoadEnabled =
            wamGetEnv("LOAD_DYNAMIC_PLUGGABLE").compare("1") == 0;

    m_postWebProcessCreatedDisabled =
            wamGetEnv("POST_WEBPROCESS_CREATED_DISABLED").compare("1") == 0;

    m_checkLaunchTimeEnabled =
            wamGetEnv("LAUNCH_TIME_CHECK").compare("1") == 0;

    m_useSystemAppOptimization =
            wamGetEnv("USE_SYSTEM_APP_OPTIMIZATION").compare("1") == 0;

    m_launchOptimizationEnabled =
            wamGetEnv("ENABLE_LAUNCH_OPTIMIZATION").compare("1") == 0;

    m_userScriptPath = wamGetEnv("USER_SCRIPT_PATH");
    if (m_userScriptPath.empty())
        m_userScriptPath = "webOSUserScripts/userScript.js";

    m_name = wamGetEnv("WAM_NAME");
}

void WebAppManagerConfig::postInitConfiguration()
{
    if (access("/var/luna/preferences/debug_system_apps", F_OK) == 0) {
        m_inspectorEnabled = true;
    }

    if (access("/var/luna/preferences/devmode_enabled", F_OK) == 0) {
        m_devModeEnabled = true;
        m_telluriumNubPath = wamGetEnv("TELLURIUM_NUB_PATH");
    }
}

void WebAppManagerConfig::resetConfiguration()
{
    m_suspendDelayTime = 0;
    m_maxCustomSuspendDelayTime = 0;

    m_devModeEnabled = false;
    m_inspectorEnabled = false;
    m_dynamicPluggableLoadEnabled = false;
    m_postWebProcessCreatedDisabled = false;
    m_checkLaunchTimeEnabled = false;
    m_useSystemAppOptimization = false;
    m_launchOptimizationEnabled = false;

    m_webAppFactoryPluginTypes.clear();
    m_webAppFactoryPluginPath.clear();
    m_webProcessConfigPath.clear();
    m_errorPageUrl.clear();
    m_telluriumNubPath.clear();
    m_userScriptPath.clear();
    m_name.clear();

    initConfiguration();
}
