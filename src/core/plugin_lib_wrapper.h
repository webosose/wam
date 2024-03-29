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

#ifndef CORE_PLUGIN_LIB_WRAPPER_H_
#define CORE_PLUGIN_LIB_WRAPPER_H_

#include <functional>
#include <string>

class WebAppFactoryInterface;

class PluginLibWrapper {
 public:
  using CreateInstanceFunc = std::function<WebAppFactoryInterface*()>;
  using DeleteInstanceFunc = std::function<void(WebAppFactoryInterface*)>;

  PluginLibWrapper() = default;
  virtual void* Load(const std::string& path);
  virtual void Unload(void* handle);
  virtual CreateInstanceFunc GetCreateInstanceFunction(void* handle);
  virtual DeleteInstanceFunc GetDeleteInstanceFunction(void* handle);
  virtual std::string GetAppType(void* handle);
  virtual std::string GetLastError();
  virtual ~PluginLibWrapper() = default;
};

#endif  // CORE_PLUGIN_LIB_WRAPPER_H_
