// Copyright (c) 2021 LG Electronics, Inc.
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

#include "web_app_factory_manager_impl.h"

#include <dirent.h>
#include <sys/types.h>

#include <string>
#include <vector>

#include "log_manager.h"
#include "plugin_loader.h"
#include "util/url.h"
#include "web_app_base.h"
#include "web_app_manager.h"
#include "web_app_manager_config.h"
#include "web_page_base.h"

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
  DIR* dir = opendir(fixed_path.c_str());
  if (dir != nullptr) {
    struct dirent* entry;
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

WebAppFactoryManager* WebAppFactoryManagerImpl::instance_ = nullptr;

WebAppFactoryManager* WebAppFactoryManagerImpl::Instance() {
  if (!instance_) {
    instance_ = new WebAppFactoryManagerImpl();
  }
  return instance_;
}

WebAppFactoryManagerImpl::RemovableManagerPtr
WebAppFactoryManagerImpl::TestInstance(
    const std::string& plugin_path,
    const std::string& factory_env,
    bool load_plugin_on_demand,
    std::unique_ptr<PluginLibWrapper> lib_wrapper) {
  RemovableManagerPtr unique_instance(
      new WebAppFactoryManagerImpl(plugin_path, factory_env,
                                   load_plugin_on_demand,
                                   std::move(lib_wrapper)),
      [](WebAppFactoryManagerImpl* p) { delete p; });
  return unique_instance;
}

WebAppFactoryManagerImpl::WebAppFactoryManagerImpl()
    : load_pluggable_on_demand_(false),
      plugin_loader_(std::make_unique<PluginLoader>(
          std::make_unique<PluginLibWrapper>())) {
  WebAppManagerConfig* config = WebAppManager::Instance()->Config();

  std::string factory_env = config->GetWebAppFactoryPluginTypes();
  factory_env_ = SplitPluginTypes(factory_env);
  factory_env_.emplace("default");

  web_app_factory_plugin_path_ = config->GetWebAppFactoryPluginPath();

  if (config->IsDynamicPluggableLoadEnabled())
    load_pluggable_on_demand_ = true;

  if (!load_pluggable_on_demand_)
    LoadPluggable();
}

WebAppFactoryManagerImpl::WebAppFactoryManagerImpl(
    const std::string& plugin_path,
    const std::string& factory_env,
    bool load_plugin_on_demand,
    std::unique_ptr<PluginLibWrapper> lib_wrapper)
    : web_app_factory_plugin_path_(plugin_path),
      load_pluggable_on_demand_(load_plugin_on_demand),
      plugin_loader_(std::make_unique<PluginLoader>(std::move(lib_wrapper))) {
  factory_env_ = SplitPluginTypes(factory_env);
  factory_env_.emplace("default");
  if (!load_pluggable_on_demand_)
    LoadPluggable();
}

WebAppFactoryManagerImpl::~WebAppFactoryManagerImpl() = default;

WebAppFactoryInterface* WebAppFactoryManagerImpl::GetPluggable(
    const std::string& app_type) {
  auto iter = interfaces_.find(app_type);
  if (iter != interfaces_.end())
    return iter->second;

  return LoadPluggable(app_type);
}

WebAppFactoryInterface* WebAppFactoryManagerImpl::LoadPluggable(
    const std::string& app_type) {
  if (app_type.size() && factory_env_.find(app_type) == factory_env_.end())
    return nullptr;

  WebAppFactoryInterface* interface;
  for (const auto& file : GetFileList(web_app_factory_plugin_path_)) {
    if (!plugin_loader_->Load(file)) {
      LOG_WARNING(MSGID_PLUGIN_LOAD_FAIL, 1,
                  PMLOGKS("ERROR", plugin_loader_->GetError().c_str()), "");
      continue;
    }
    std::string key = plugin_loader_->GetAppType(file);
    if (key.find(app_type) != std::string::npos || !load_pluggable_on_demand_) {
      interface = plugin_loader_->GetWebAppFactoryInstance(file);
      if (interface) {
        interfaces_.emplace(key, interface);
        if (!app_type.empty())
          return interface;
      } else {
        LOG_WARNING(MSGID_PLUGIN_LOAD_FAIL, 1,
                    PMLOGKS("ERROR", plugin_loader_->GetError().c_str()), "");
        plugin_loader_->Unload(file);
        if (!app_type.empty())
          return nullptr;
      }
    }
  }
  return nullptr;
}

WebAppBase* WebAppFactoryManagerImpl::CreateWebApp(
    const std::string& win_type,
    std::shared_ptr<ApplicationDescription> desc,
    const std::string& app_type) {
  WebAppFactoryInterface* interface = GetPluggable(app_type);
  if (interface)
    return interface->CreateWebApp(win_type, desc);

  return nullptr;
}

WebAppBase* WebAppFactoryManagerImpl::CreateWebApp(
    const std::string& win_type,
    WebPageBase* page,
    std::shared_ptr<ApplicationDescription> desc,
    const std::string& app_type) {
  WebAppFactoryInterface* interface = GetPluggable(app_type);
  if (interface)
    return interface->CreateWebApp(win_type, page, desc);

  return nullptr;
}

WebPageBase* WebAppFactoryManagerImpl::CreateWebPage(
    const std::string& win_type,
    const wam::Url& url,
    std::shared_ptr<ApplicationDescription> desc,
    const std::string& app_type,
    const std::string& launch_params) {
  WebPageBase* page = nullptr;

  WebAppFactoryInterface* interface = GetPluggable(app_type);
  if (interface) {
    page = interface->CreateWebPage(url, desc, launch_params);
  } else {
    auto default_interface = interfaces_.find("default");
    if (default_interface != interfaces_.end()) {
      // use default factory if cannot find app_type.
      page = default_interface->second->CreateWebPage(url, desc, launch_params);
    }
  }

  if (page)
    page->Init();
  return page;
}
