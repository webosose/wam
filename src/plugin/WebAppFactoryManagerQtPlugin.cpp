// Copyright (c) 2008-2018 LG Electronics, Inc.
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

#include "WebAppFactoryManager.h"

#include <QtCore/QDir>
#include <QtCore/QPluginLoader>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

#include "LogManager.h"
#include "WebAppBase.h"
#include "WebAppManagerConfig.h"
#include "WebAppManager.h"
#include "WebPageBase.h"

class WebAppFactoryManagerQtPlugin : public WebAppFactoryManager {
protected:
    virtual WebAppFactoryInterface* loadInterfaceInstance(QString appType);

private:
    friend class WebAppFactoryManager;
    WebAppFactoryManagerQtPlugin();

    QString m_webAppFactoryPluginPath;
    QStringList m_factoryEnv;
    bool m_loadPluggableOnDemand;
};

WebAppFactoryManager* WebAppFactoryManager::instance()
{
    if(!m_instance) {
        m_instance = new WebAppFactoryManagerQtPlugin();
    }
    return m_instance;
}

WebAppFactoryManagerQtPlugin::WebAppFactoryManagerQtPlugin()
    : m_loadPluggableOnDemand(false)
{
    WebAppManagerConfig* webAppManagerConfig = WebAppManager::instance()->config();

    QString factoryEnv = webAppManagerConfig->getWebAppFactoryPluginTypes();
    m_factoryEnv = factoryEnv.split(QLatin1Char(':'));
    m_factoryEnv.append(kDefaultAppType);

    m_webAppFactoryPluginPath = webAppManagerConfig->getWebAppFactoryPluginPath();

    if (webAppManagerConfig->isDynamicPluggableLoadEnabled())
        m_loadPluggableOnDemand = true;

    if (!m_loadPluggableOnDemand)
        loadInterfaceInstance({});
}

WebAppFactoryInterface* WebAppFactoryManagerQtPlugin::loadInterfaceInstance(QString appType)
{
    if (!appType.isEmpty() && !m_factoryEnv.contains(appType))
        return nullptr;

    WebAppFactoryInterface* interface;
    QDir pluginsDir(m_webAppFactoryPluginPath);

    Q_FOREACH (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QString key = pluginLoader.metaData().value("MetaData").toObject().value("Keys").toArray().at(0).toString();

        if (key.contains(appType) || !m_loadPluggableOnDemand) {
            QObject *plugin = pluginLoader.instance();

            if (plugin) {
                interface = qobject_cast<WebAppFactoryInterface*>(plugin);
                if (interface)
                    m_interfaces.insert(key, interface);
                if (!appType.isEmpty())
                    return interface;
            } else {
                LOG_WARNING(MSGID_PLUGIN_LOAD_FAIL, 1, PMLOGKS("ERROR", pluginLoader.errorString().toStdString().c_str()), "");
                if (pluginLoader.isLoaded())
                    pluginLoader.unload();
                if (!appType.isEmpty())
                    return nullptr;
            }
        }
    }
    return nullptr;
}

