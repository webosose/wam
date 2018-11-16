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

#ifndef WEBAPPMANAGERCONFIG_H
#define WEBAPPMANAGERCONFIG_H

#include <QMap>
#include <QString>
#include <QVariant>

class WebAppManagerConfig {
public:
    WebAppManagerConfig();
    virtual ~WebAppManagerConfig() {}

    virtual QString getWebAppFactoryPluginTypes() const { return m_webAppFactoryPluginTypes; }
    virtual QString getWebAppFactoryPluginPath() const { return m_webAppFactoryPluginPath; }
    virtual int getSuspendDelayTime() const { return m_suspendDelayTime; }
    virtual QString getWebProcessConfigPath() const { return m_webProcessConfigPath; }
    virtual bool isInspectorEnabled() const { return m_inspectorEnabled; }
    virtual bool isDevModeEnabled() const { return m_devModeEnabled; }
    virtual QString getErrorPageUrl() const { return m_errorPageUrl; }
    virtual QString getTelluriumNubPath() const { return m_telluriumNubPath; }
    virtual bool isContainerAppEnabled() const { return m_containerAppEnabled; }
    virtual void postInitConfiguration();
    virtual bool isDynamicPluggableLoadEnabled() const { return m_dynamicPluggableLoadEnabled; }
    virtual bool isPostWebProcessCreatedDisabled() const { return m_postWebProcessCreatedDisabled; }
    virtual bool isCheckLaunchTimeEnabled() const { return m_checkLaunchTimeEnabled; }
    virtual bool isUseSystemAppOptimization() const { return m_useSystemAppOptimization; }
    virtual QString getUserScriptPath() const { return m_userScriptPath; }
    virtual std::string getName() const { return m_name; }

    virtual bool isLaunchOptimizationEnabled() const { return m_launchOptimizationEnabled; }

protected:
    virtual QVariant getConfiguration(QString name);
    virtual void setConfiguration(QString name, QVariant value);

private:
    void initConfiguration();

    QString m_webAppFactoryPluginTypes;
    QString m_webAppFactoryPluginPath;
    int m_suspendDelayTime;
    QString m_webProcessConfigPath;
    bool m_devModeEnabled;
    bool m_inspectorEnabled;
    QString m_errorPageUrl;
    QString m_telluriumNubPath;
    bool m_containerAppEnabled;
    bool m_dynamicPluggableLoadEnabled;
    bool m_postWebProcessCreatedDisabled;
    bool m_checkLaunchTimeEnabled;
    bool m_useSystemAppOptimization;
    bool m_launchOptimizationEnabled;
    QString m_userScriptPath;
    std::string m_name;

    QMap<QString, QVariant> m_configuration;
};

#endif /* WEBAPPMANAGERCONFIG_H */
