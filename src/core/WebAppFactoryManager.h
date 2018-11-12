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

#include <memory>

#include <QMap>
#include <QStringList>

#include "WebAppFactoryInterface.h"

static const QString kDefaultAppType = QStringLiteral("default");

class WebAppFactoryManager {
public:
    static WebAppFactoryManager* instance();
    virtual ~WebAppFactoryManager();
    WebAppBase* createWebApp(QString winType, std::shared_ptr<ApplicationDescription> desc = nullptr, QString appType = "");
    WebAppBase* createWebApp(QString winType, WebPageBase* page, std::shared_ptr<ApplicationDescription> desc = nullptr, QString appType = "");
    WebPageBase* createWebPage(QString winType, QUrl url, std::shared_ptr<ApplicationDescription> desc, QString appType = "", QString launchParams = "");
    WebAppFactoryInterface* getInterfaceInstance(QString appType);

protected:
    WebAppFactoryManager() {}
    virtual WebAppFactoryInterface* loadInterfaceInstance(QString appType) = 0;
    QMap<QString, WebAppFactoryInterface*> m_interfaces;

private:
    static WebAppFactoryManager* m_instance;
};

#endif /* WEBAPPFACTORY_H */
