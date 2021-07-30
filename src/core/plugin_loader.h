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

#ifndef CORE_PLUGIN_LOADER_H_
#define CORE_PLUGIN_LOADER_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "plugin_lib_wrapper.h"

class WebAppFactoryInterface;

class PluginLoader {
 public:
  PluginLoader(std::unique_ptr<PluginLibWrapper> lib_wrapper);
  std::string GetAppType(const std::string& file_name);
  bool Load(const std::string& file_name);
  WebAppFactoryInterface* GetWebAppFactoryInstance(
      const std::string& file_name);
  void Unload(const std::string& file_name);
  std::string GetError();
  ~PluginLoader();

 private:
  struct PluginData {
    void* handle;
    WebAppFactoryInterface* interface;
  };
  std::unique_ptr<PluginLibWrapper> lib_wrapper_;
  std::unordered_map<std::string, PluginData> plugins_;
};

#endif  // CORE_PLUGIN_LOADER_H_
