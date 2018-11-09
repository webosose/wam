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

// FIXME: Remove once WebAppFactory pluggable system is dropped.
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

#include "LogManager.h"
#include "WebAppBase.h"
#include "WebAppManagerConfig.h"
#include "WebAppManager.h"
#include "WebPageBase.h"

WebAppFactoryManager* WebAppFactoryManager::m_instance = nullptr;

WebAppFactoryManager* WebAppFactoryManager::instance()
{
    if(!m_instance) {
        m_instance = new WebAppFactoryManager();
    }
    return m_instance;
}

WebAppFactoryManager::WebAppFactoryManager()
    : m_loadPluggableOnDemand(false)
{
    WebAppManagerConfig* webAppManagerConfig = WebAppManager::instance()->config();

    QString factoryEnv = webAppManagerConfig->getWebAppFactoryPluginTypes();
    m_factoryEnv = factoryEnv.split(QLatin1Char(':'));
    m_factoryEnv.append(QStringLiteral("default"));

    m_webAppFactoryPluginPath = webAppManagerConfig->getWebAppFactoryPluginPath();

    if (webAppManagerConfig->isDynamicPluggableLoadEnabled())
        m_loadPluggableOnDemand = true;

    if (!m_loadPluggableOnDemand)
        loadPluggable();
}

WebAppFactoryInterface* WebAppFactoryManager::getPluggable(QString appType)
{
    QMap<QString, WebAppFactoryInterface*>::iterator iter = m_interfaces.find(appType);
    if (iter != m_interfaces.end())
        return iter.value();

    return loadPluggable(appType);
}

WebAppFactoryInterface* WebAppFactoryManager::loadPluggable(QString appType)
{
    if (!appType.isEmpty() && !m_factoryEnv.contains(appType))
        return nullptr;

    WebAppFactoryInterface* interface;
    QDir pluginsDir(m_webAppFactoryPluginPath);

    // FIXME: Remove once WebAppFactory pluggable system is dropped.
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

WebAppBase* WebAppFactoryManager::createWebApp(QString winType, std::shared_ptr<ApplicationDescription> desc, QString appType)
{
    WebAppFactoryInterface* interface = getPluggable(appType);
    if (interface)
        return interface->createWebApp(winType, desc);

    return nullptr;
}

WebAppBase* WebAppFactoryManager::createWebApp(QString winType, WebPageBase* page, std::shared_ptr<ApplicationDescription> desc, QString appType)
{
    WebAppFactoryInterface* interface = getPluggable(appType);  
    if (interface)
        return interface->createWebApp(winType, page, desc);

    return nullptr;
}

WebPageBase* WebAppFactoryManager::createWebPage(QString winType, QUrl url, std::shared_ptr<ApplicationDescription> desc, QString appType, QString launchParams)
{
    WebPageBase *page = nullptr;

    WebAppFactoryInterface* interface = getPluggable(appType);
    if (interface) {
        page = interface->createWebPage(url, desc, launchParams);
    } else {
        interface = m_interfaces.value("default");
        if (interface) {
            // use default factory if cannot find appType.
            page = interface->createWebPage(url, desc, launchParams);
        }
    }

    if (page) page->init();
    return page;
}

