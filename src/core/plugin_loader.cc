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

#include "plugin_loader.h"

#include "plugin_lib_wrapper.h"

PluginLoader::PluginLoader(std::unique_ptr<PluginLibWrapper> lib_wrapper)
    : lib_wrapper_(std::move(lib_wrapper)) {}

PluginLoader::~PluginLoader() {
  for (auto& plugin : plugins_) {
    if (plugin.second.interface) {
      auto delete_instance_func =
          lib_wrapper_->GetDeleteInstanceFunction(plugin.second.handle);
      if (delete_instance_func)
        delete_instance_func(plugin.second.interface);
    }
  }
}

std::string PluginLoader::GetAppType(const std::string& file_name) {
  std::string app_type;
  auto handle = plugins_.find(file_name);
  if (handle != plugins_.end())
    app_type = lib_wrapper_->GetAppType(handle->second.handle);
  return app_type;
}

bool PluginLoader::Load(const std::string& file_name) {
  auto exist_handle = plugins_.find(file_name);
  if (exist_handle != plugins_.end())
    return true;
  void* handle = lib_wrapper_->Load(file_name);
  if (handle) {
    plugins_.emplace(file_name, PluginData{handle, nullptr});
    return true;
  }
  return false;
}

WebAppFactoryInterface* PluginLoader::GetWebAppFactoryInstance(
    const std::string& file_name) {
  auto plugin = plugins_.find(file_name);
  if (plugin != plugins_.end()) {
    if (plugin->second.interface)
      return plugin->second.interface;
    auto create_instance_func =
        lib_wrapper_->GetCreateInstanceFunction(plugin->second.handle);
    if (create_instance_func) {
      plugin->second.interface = create_instance_func();
      return plugin->second.interface;
    }
  }
  return nullptr;
}

void PluginLoader::Unload(const std::string& file_name) {
  auto plugin = plugins_.find(file_name);
  if (plugin != plugins_.end()) {
    if (plugin->second.interface) {
      auto delete_instance_func =
          lib_wrapper_->GetDeleteInstanceFunction(plugin->second.handle);
      if (delete_instance_func)
        delete_instance_func(plugin->second.interface);
    }
    lib_wrapper_->Unload(plugin->second.handle);
    plugins_.erase(plugin);
  }
};

std::string PluginLoader::GetError() {
  return lib_wrapper_->GetLastError();
}
