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

#include <unordered_map>
#include <string>

#include "WebAppFactoryInterface.h"

#define kDefaultAppType "default"

class WebAppFactoryManager {
public:
    static WebAppFactoryManager* instance();
    virtual ~WebAppFactoryManager();

    WebAppBase* createWebApp(const std::string& winType, ApplicationDescription* desc = 0,
                             const std::string& appType = "");
    WebAppBase* createWebApp(const std::string& winType, WebPageBase* page, ApplicationDescription* desc = 0,
                             const std::string& appType = "");
    WebPageBase* createWebPage(const std::string& winType, QUrl url, ApplicationDescription* desc,
                               const std::string& appType = "", const std::string& launchParams = "");
    WebAppFactoryInterface* getInterfaceInstance(const std::string& appType);

protected:
    WebAppFactoryManager() {}
    virtual WebAppFactoryInterface* loadInterfaceInstance(const std::string& appType) = 0;
    std::unordered_map<std::string, WebAppFactoryInterface*> m_interfaces;

private:
    static WebAppFactoryManager* m_instance;
};

#endif /* WEBAPPFACTORY_H */
