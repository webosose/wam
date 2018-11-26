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

#include "WebAppBase.h"
#include "WebAppManagerConfig.h"
#include "WebAppManager.h"
#include "WebPageBase.h"

WebAppFactoryManager* WebAppFactoryManager::m_instance = nullptr;

WebAppFactoryManager::~WebAppFactoryManager()
{
}

WebAppFactoryInterface* WebAppFactoryManager::getInterfaceInstance(const std::string& appType)
{
    auto iter = m_interfaces.find(appType);
    if (iter != m_interfaces.end())
        return iter->second;

    return loadInterfaceInstance(appType);
}

WebAppBase* WebAppFactoryManager::createWebApp(const std::string& winType, std::shared_ptr<ApplicationDescription> desc,
                                               const std::string& appType)
{
    WebAppFactoryInterface* interface = getInterfaceInstance(appType);
    if (interface)
        return interface->createWebApp(winType, desc);

    return nullptr;
}

WebAppBase* WebAppFactoryManager::createWebApp(const std::string& winType, WebPageBase* page,
                                               std::shared_ptr<ApplicationDescription> desc, const std::string& appType)
{
    WebAppFactoryInterface* interface = getInterfaceInstance(appType);
    if (interface)
        return interface->createWebApp(winType, page, desc);

    return nullptr;
}

WebPageBase* WebAppFactoryManager::createWebPage(const std::string& winType, const Url& url, std::shared_ptr<ApplicationDescription> desc,
                                                 const std::string& appType, const std::string& launchParams)
{
    WebPageBase *page = nullptr;

    WebAppFactoryInterface* interface = getInterfaceInstance(appType);
    if (interface) {
        page = interface->createWebPage(url, desc, launchParams);
    } else {
        // use default factory if cannot find appType.
        auto it = m_interfaces.find(kDefaultAppType);
        if (it != m_interfaces.end())
            page = it->second->createWebPage(url, desc, launchParams);
    }

    if (page) page->init();
    return page;
}

