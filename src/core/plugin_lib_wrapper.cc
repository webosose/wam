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

#include "plugin_lib_wrapper.h"

#include <dlfcn.h>

void* PluginLibWrapper::Load(const std::string& path) {
  return dlopen(path.c_str(), RTLD_LAZY);
}

void PluginLibWrapper::Unload(void* handle) {
  dlclose(handle);
}

PluginLibWrapper::CreateInstanceFunc
PluginLibWrapper::GetCreateInstanceFunction(void* handle) {
  return reinterpret_cast<WebAppFactoryInterface* (*)(void)>(
      dlsym(handle, "CreateInstance"));
}

PluginLibWrapper::DeleteInstanceFunc
PluginLibWrapper::GetDeleteInstanceFunction(void* handle) {
  return reinterpret_cast<void* (*)(WebAppFactoryInterface*)>(
      dlsym(handle, "DeleteInstance"));
}

std::string PluginLibWrapper::GetAppType(void* handle) {
  const char* type = nullptr;
  const char** pointer_to_type =
      reinterpret_cast<const char**>(dlsym(handle, "kPluginApplicationType"));
  if (pointer_to_type)
    type = *pointer_to_type;
  return type == nullptr ? std::string() : std::string(type);
}

std::string PluginLibWrapper::GetLastError() {
  char* error = dlerror();
  return error == nullptr ? std::string() : std::string(error);
}
