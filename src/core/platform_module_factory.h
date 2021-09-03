// Copyright (c) 2014-2018 LG Electronics, Inc.
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

#ifndef CORE_PLATFORM_MODULE_FACTORY_H_
#define CORE_PLATFORM_MODULE_FACTORY_H_

#include <memory>

#include "device_info.h"
#include "service_sender.h"
#include "web_app_manager_config.h"
#include "web_process_manager.h"

class PlatformModuleFactory {
 public:
  virtual ~PlatformModuleFactory() = default;
  std::unique_ptr<ServiceSender> GetServiceSender() {
    return CreateServiceSender();
  }
  std::unique_ptr<WebProcessManager> GetWebProcessManager() {
    return CreateWebProcessManager();
  }
  std::unique_ptr<DeviceInfo> GetDeviceInfo() { return CreateDeviceInfo(); }
  std::unique_ptr<WebAppManagerConfig> GetWebAppManagerConfig() {
    return CreateWebAppManagerConfig();
  }

 protected:
  virtual std::unique_ptr<ServiceSender> CreateServiceSender() = 0;
  virtual std::unique_ptr<WebProcessManager> CreateWebProcessManager() = 0;
  virtual std::unique_ptr<DeviceInfo> CreateDeviceInfo() = 0;
  virtual std::unique_ptr<WebAppManagerConfig> CreateWebAppManagerConfig() = 0;
};

#endif  // CORE_PLATFORM_MODULE_FACTORY_H_
