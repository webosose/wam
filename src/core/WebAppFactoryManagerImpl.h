// Copyright (c) 2008-2021 LG Electronics, Inc.
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

#ifndef WEBAPPFACTORYMANAGERIMPL_H
#define WEBAPPFACTORYMANAGERIMPL_H

#include <memory>

#include <QMap>
#include <QStringList>

#include "WebAppFactoryInterface.h"
#include "WebAppFactoryManager.h"

class WebAppFactoryManagerImpl : public WebAppFactoryManager {
public:
    static WebAppFactoryManager* instance();

    WebAppBase* createWebApp(QString winType, std::shared_ptr<ApplicationDescription> desc = nullptr, QString appType = "") override;
    WebAppBase* createWebApp(QString winType, WebPageBase* page, std::shared_ptr<ApplicationDescription> desc = nullptr, QString appType = "") override;
    WebPageBase* createWebPage(QString winType, QUrl url, std::shared_ptr<ApplicationDescription> desc, QString appType = "", QString launchParams = "") override;
    WebAppFactoryInterface* getPluggable(QString appType);
    WebAppFactoryInterface* loadPluggable(QString appType = "");

private:
    WebAppFactoryManagerImpl();
    ~WebAppFactoryManagerImpl() override;

    static WebAppFactoryManager* m_instance;
    QMap<QString, WebAppFactoryInterface*> m_interfaces;
    QString m_webAppFactoryPluginPath;
    QStringList m_factoryEnv;
    bool m_loadPluggableOnDemand;
};

#endif /* WEBAPPFACTORYMANAGERIMPL_H */
