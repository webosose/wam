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

#ifndef WEBAPPFACTORYLUNA_H
#define WEBAPPFACTORYLUNA_H

#include "WebAppFactoryInterface.h"

#include <QtCore/QUrl>

class WebAppFactoryLuna : public QObject,
                          public WebAppFactoryInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.WebAppFactoryInterface" FILE "webappfactoryluna-plugin.json")
    Q_INTERFACES(WebAppFactoryInterface)

public:
    virtual WebAppBase* createWebApp(QString winType, ApplicationDescription* desc = 0);
    virtual WebAppBase* createWebApp(QString winType, WebPageBase* page, ApplicationDescription* desc = 0);
    virtual WebPageBase* createWebPage(QUrl url, ApplicationDescription* desc, QString launchParams = "");
};

#endif /* WEBAPPFACTORYLUNA_H */
