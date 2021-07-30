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

#ifndef CORE_WEB_APP_FACTORY_MANAGER_IMPL_H_
#define CORE_WEB_APP_FACTORY_MANAGER_IMPL_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "web_app_factory_interface.h"
#include "web_app_factory_manager.h"

class PluginLoader;
class PluginLibWrapper;

class WebAppFactoryManagerImpl : public WebAppFactoryManager {
 public:
  static WebAppFactoryManager* Instance();
  // For testing purpose.
  using RemovableManagerPtr =
      std::unique_ptr<WebAppFactoryManagerImpl,
                      std::function<void(WebAppFactoryManagerImpl*)>>;
  static RemovableManagerPtr TestInstance(
      const std::string& plugin_path,
      const std::string& factory_env,
      bool load_plugin_on_demand,
      std::unique_ptr<PluginLibWrapper> lib_wrapper);

  WebAppBase* CreateWebApp(
      const std::string& win_type,
      std::shared_ptr<ApplicationDescription> desc = nullptr,
      const std::string& app_type = {}) override;
  WebAppBase* CreateWebApp(
      const std::string& win_type,
      WebPageBase* page,
      std::shared_ptr<ApplicationDescription> desc = nullptr,
      const std::string& app_type = {}) override;
  WebPageBase* CreateWebPage(const std::string& win_type,
                             const wam::Url& url,
                             std::shared_ptr<ApplicationDescription> desc,
                             const std::string& app_type = {},
                             const std::string& launchParams = {}) override;
  WebAppFactoryInterface* GetPluggable(const std::string& app_type);
  WebAppFactoryInterface* LoadPluggable(const std::string& app_type = {});

 private:
  WebAppFactoryManagerImpl();
  // For testing purpose.
  WebAppFactoryManagerImpl(const std::string& plugin_path,
                           const std::string& factory_env,
                           bool load_plugin_on_demand,
                           std::unique_ptr<PluginLibWrapper> lib_wrapper);
  ~WebAppFactoryManagerImpl() override;

  static WebAppFactoryManager* instance_;
  std::unordered_map<std::string, WebAppFactoryInterface*> interfaces_;
  std::string web_app_factory_plugin_path_;
  std::unordered_set<std::string> factory_env_;
  bool load_pluggable_on_demand_;
  std::unique_ptr<PluginLoader> plugin_loader_;
};

#endif  // CORE_WEB_APP_FACTORY_MANAGER_IMPL_H_
