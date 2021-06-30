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

#include "WebAppFactoryManagerImpl.h"

#include <string>
#include <vector>
#include <dirent.h>
#include <sys/types.h>

#include "PluginLoader.h"
#include "LogManager.h"
#include "WebAppBase.h"
#include "WebAppManagerConfig.h"
#include "WebAppManager.h"
#include "WebPageBase.h"

namespace {

std::unordered_set<std::string> SplitPluginTypes(const std::string& types) {
    size_t start = 0;
    size_t end = 0;
    std::string delim = ":";
    std::unordered_set<std::string> result;
    while ((start = types.find_first_not_of(delim, end)) != std::string::npos) {
        end = types.find(delim, start);
        result.emplace(types.substr(start, end - start));
    }
    return result;
}

std::vector<std::string> GetFileList(const std::string& path) {
    std::string fixed_path = path.back() == '/' ? path : path + '/';
    std::vector<std::string> files;
    DIR *dir = opendir(fixed_path.c_str());
    if (dir != nullptr) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string file_name = entry->d_name;
            if (file_name == "." || file_name == "..")
            continue;
            files.emplace_back(fixed_path + file_name);
        }
        closedir(dir);
    }
    return files;
}

}  // namespace

WebAppFactoryManager* WebAppFactoryManagerImpl::m_instance = nullptr;

WebAppFactoryManager* WebAppFactoryManagerImpl::instance()
{
    if(!m_instance) {
        m_instance = new WebAppFactoryManagerImpl();
    }
    return m_instance;
}

WebAppFactoryManagerImpl::RemovableManagerPtr WebAppFactoryManagerImpl::testInstance(
    const std::string& plugin_path,
    const std::string& factoryEnv,
    bool load_plugin_on_demand,
    std::unique_ptr<PluginLibWrapper> lib_wrapper)
{
    RemovableManagerPtr unique_instance(new WebAppFactoryManagerImpl(
        plugin_path, factoryEnv, load_plugin_on_demand, std::move(lib_wrapper)),
        [](WebAppFactoryManagerImpl* p) { delete p; });
    return unique_instance;
}

WebAppFactoryManagerImpl::WebAppFactoryManagerImpl()
    : m_loadPluggableOnDemand(false)
    , m_pluginLoader(std::make_unique<PluginLoader>(
        std::make_unique<PluginLibWrapper>()))
{
    WebAppManagerConfig* webAppManagerConfig = WebAppManager::instance()->config();

    std::string factoryEnv = webAppManagerConfig->getWebAppFactoryPluginTypes().toStdString();
    m_factoryEnv = SplitPluginTypes(factoryEnv);
    m_factoryEnv.emplace("default");

    m_webAppFactoryPluginPath = webAppManagerConfig->getWebAppFactoryPluginPath().toStdString();

    if (webAppManagerConfig->isDynamicPluggableLoadEnabled())
        m_loadPluggableOnDemand = true;

    if (!m_loadPluggableOnDemand)
        loadPluggable();
}

WebAppFactoryManagerImpl::WebAppFactoryManagerImpl(
    const std::string& plugin_path,
    const std::string& factory_env,
    bool load_plugin_on_demand,
    std::unique_ptr<PluginLibWrapper> lib_wrapper)
    : m_webAppFactoryPluginPath(plugin_path)
    , m_loadPluggableOnDemand(load_plugin_on_demand)
    , m_pluginLoader(std::make_unique<PluginLoader>(std::move(lib_wrapper))) {
    m_factoryEnv = SplitPluginTypes(factory_env);
    m_factoryEnv.emplace("default");
    if (!m_loadPluggableOnDemand)
        loadPluggable();
}

WebAppFactoryManagerImpl::~WebAppFactoryManagerImpl() = default;

WebAppFactoryInterface* WebAppFactoryManagerImpl::getPluggable(const std::string& appType)
{
    auto iter = m_interfaces.find(appType);
    if (iter != m_interfaces.end())
        return iter->second;

    return loadPluggable(appType);
}

WebAppFactoryInterface* WebAppFactoryManagerImpl::loadPluggable(const std::string& appType)
{
    if (appType.size() && m_factoryEnv.find(appType) == m_factoryEnv.end())
        return nullptr;

    WebAppFactoryInterface* interface;
    for (const auto& file : GetFileList(m_webAppFactoryPluginPath)) {
        if (!m_pluginLoader->Load(file)) {
          LOG_WARNING(MSGID_PLUGIN_LOAD_FAIL, 1, PMLOGKS("ERROR", m_pluginLoader->GetError().c_str()), "");
          continue;
        }
        std::string key = m_pluginLoader->GetAppType(file);
        if (key.find(appType) != std::string::npos || !m_loadPluggableOnDemand) {
            interface = m_pluginLoader->GetWebAppFactoryInstance(file);
            if (interface) {
                m_interfaces.emplace(key, interface);
                if (!appType.empty())
                    return interface;
            } else {
                LOG_WARNING(MSGID_PLUGIN_LOAD_FAIL, 1, PMLOGKS("ERROR", m_pluginLoader->GetError().c_str()), "");
                m_pluginLoader->Unload(file);
                if (!appType.empty())
                    return nullptr;
            }
        }
    }
    return nullptr;
}

WebAppBase* WebAppFactoryManagerImpl::createWebApp(QString winType, std::shared_ptr<ApplicationDescription> desc, QString appType)
{
    WebAppFactoryInterface* interface = getPluggable(appType.toStdString());
    if (interface)
        return interface->createWebApp(winType, desc);

    return nullptr;
}

WebAppBase* WebAppFactoryManagerImpl::createWebApp(QString winType, WebPageBase* page, std::shared_ptr<ApplicationDescription> desc, QString appType)
{
    WebAppFactoryInterface* interface = getPluggable(appType.toStdString());
    if (interface)
        return interface->createWebApp(winType, page, desc);

    return nullptr;
}

WebPageBase* WebAppFactoryManagerImpl::createWebPage(QString winType, QUrl url, std::shared_ptr<ApplicationDescription> desc, QString appType, QString launchParams)
{
    WebPageBase *page = nullptr;

    WebAppFactoryInterface* interface = getPluggable(appType.toStdString());
    if (interface) {
        page = interface->createWebPage(url, desc, launchParams);
    } else {
        auto default_interface = m_interfaces.find("default");
        if (default_interface != m_interfaces.end()) {
            // use default factory if cannot find appType.
            page = default_interface->second->createWebPage(url, desc, launchParams);
        }
    }

    if (page) page->init();
    return page;
}

