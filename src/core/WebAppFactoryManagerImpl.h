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

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "WebAppFactoryInterface.h"
#include "WebAppFactoryManager.h"

class PluginLoader;
class PluginLibWrapper;

class WebAppFactoryManagerImpl : public WebAppFactoryManager {
public:
    static WebAppFactoryManager* instance();
    // For testing purpose.
    using RemovableManagerPtr = std::unique_ptr<WebAppFactoryManagerImpl, std::function<void(WebAppFactoryManagerImpl*)>>;
    static RemovableManagerPtr testInstance(
            const std::string& plugin_path,
            const std::string& factoryEnv,
            bool load_plugin_on_demand,
            std::unique_ptr<PluginLibWrapper> lib_wrapper);

    WebAppBase* createWebApp(const std::string& winType, std::shared_ptr<ApplicationDescription> desc = nullptr, const std::string& appType = {}) override;
    WebAppBase* createWebApp(const std::string& winType, WebPageBase* page, std::shared_ptr<ApplicationDescription> desc = nullptr, const std::string& appType = {}) override;
    WebPageBase* createWebPage(const std::string& winType, const wam::Url& url, std::shared_ptr<ApplicationDescription> desc, const std::string& appType = {}, const std::string& launchParams = {}) override;
    WebAppFactoryInterface* getPluggable(const std::string& appType);
    WebAppFactoryInterface* loadPluggable(const std::string& appType = {});

private:
    WebAppFactoryManagerImpl();
    // For testing purpose.
    WebAppFactoryManagerImpl(const std::string& plugin_path,
                             const std::string& factoryEnv,
                             bool load_plugin_on_demand,
                             std::unique_ptr<PluginLibWrapper> lib_wrapper);
    ~WebAppFactoryManagerImpl() override;

    static WebAppFactoryManager* m_instance;
    std::unordered_map<std::string, WebAppFactoryInterface*> m_interfaces;
    std::string m_webAppFactoryPluginPath;
    std::unordered_set<std::string> m_factoryEnv;
    bool m_loadPluggableOnDemand;
    std::unique_ptr<PluginLoader> m_pluginLoader;
};

#endif /* WEBAPPFACTORYMANAGERIMPL_H */
