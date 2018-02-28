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
    m_webAppFactoryPluginTypes = QLatin1String(qgetenv("WEBAPPFACTORY"));

    m_webAppFactoryPluginPath = QLatin1String(qgetenv("WEBAPPFACTORY_PLUGIN_PATH"));
    if (m_webAppFactoryPluginPath.isEmpty()) {
        m_webAppFactoryPluginPath = QLatin1String("/usr/lib/webappmanager/plugins");
    }

    QString suspendDelay = QLatin1String(qgetenv("WAM_SUSPEND_DELAY_IN_MS"));
    m_suspendDelayTime = std::max(suspendDelay.toInt(), 1);

    m_webProcessConfigPath = QLatin1String(qgetenv("WEBPROCESS_CONFIGURATION_PATH"));
    if (m_webProcessConfigPath.isEmpty())
        m_webProcessConfigPath = QLatin1String("/etc/wam/com.webos.wam.json");

    m_errorPageUrl = QLatin1String(qgetenv("WAM_ERROR_PAGE"));

    if (qgetenv("DISABLE_CONTAINER") == "1")
        m_containerAppEnabled = false;

    if (qgetenv("LOAD_DYNAMIC_PLUGGABLE") == "1")
        m_dynamicPluggableLoadEnabled = true;

    if (qgetenv("POST_WEBPROCESS_CREATED_DISABLED") == "1")
        m_postWebProcessCreatedDisabled =  true;

    if (qgetenv("LAUNCH_TIME_CHECK") == "1")
        m_checkLaunchTimeEnabled = true;

    if (qgetenv("USE_SYSTEM_APP_OPTIMIZATION") == "1")
        m_useSystemAppOptimization = true;

    if (qgetenv("ENABLE_LAUNCH_OPTIMIZATION") == "1")
        m_launchOptimizationEnabled = true;

    m_userScriptPath = QLatin1String(qgetenv("USER_SCRIPT_PATH"));
    if (m_userScriptPath.isEmpty())
        m_userScriptPath = QLatin1String("webOSUserScripts/userScript.js");

    m_name = qgetenv("WAM_NAME").data();
}

QVariant WebAppManagerConfig::getConfiguration(QString name)
{
    QVariant value(0);

    if (m_configuration.contains(name)) {
        value = m_configuration.value(name);
    }

    return value;
}

void WebAppManagerConfig::setConfiguration(QString name, QVariant value)
{
    m_configuration.insert(name, value);
}

void WebAppManagerConfig::postInitConfiguration()
{
    if (access("/var/luna/preferences/debug_system_apps", F_OK) == 0) {
        m_inspectorEnabled = true;
    }

    if (access("/var/luna/preferences/devmode_enabled", F_OK) == 0) {
        m_devModeEnabled = true;
        m_telluriumNubPath = QLatin1String(qgetenv("TELLURIUM_NUB_PATH"));
    }
}
