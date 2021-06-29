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

#ifndef WEBAPPFACTORYINTERFACE_H
#define WEBAPPFACTORYINTERFACE_H

#include <memory>

#include <QString>
#include <QUrl>
#include <QtPlugin>

#include "ApplicationDescription.h"

class WebAppBase;
class WebPageBase;

class WebAppFactoryInterface {
public:
    virtual WebAppBase* createWebApp(QString winType, std::shared_ptr<ApplicationDescription> desc = nullptr) = 0;
    virtual WebAppBase* createWebApp(QString winType, WebPageBase* page, std::shared_ptr<ApplicationDescription> desc = nullptr) = 0;
    virtual WebPageBase* createWebPage(QUrl url, std::shared_ptr<ApplicationDescription> desc, QString launchParams = "") = 0;
    virtual ~WebAppFactoryInterface() {};
};

#endif // WEBAPPFACTORYINTERFACE_H
