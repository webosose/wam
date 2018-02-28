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

#ifndef WEBAPPFACTORYMANAGER_H
#define WEBAPPFACTORYMANAGER_H

#include <QMap>
#include <QStringList>

#include "WebAppFactoryInterface.h"

class WebAppFactoryManager {
public:
    static WebAppFactoryManager* instance();
    WebAppBase* createWebApp(QString winType, ApplicationDescription* desc = 0, QString appType = "");
    WebAppBase* createWebApp(QString winType, WebPageBase* page, ApplicationDescription* desc = 0, QString appType = "");
    WebPageBase* createWebPage(QString winType, QUrl url, ApplicationDescription* desc, QString appType = "", QString launchParams = "");
    WebAppFactoryInterface* getPluggable(QString appType);
    WebAppFactoryInterface* loadPluggable(QString appType = "");

private:
    static WebAppFactoryManager* m_instance;
    WebAppFactoryManager();
    QMap<QString, WebAppFactoryInterface*> m_interfaces;
    QString m_webAppFactoryPluginPath;
    QStringList m_factoryEnv;
    bool m_loadPluggableOnDemand;
};

#endif /* WEBAPPFACTORY_H */
