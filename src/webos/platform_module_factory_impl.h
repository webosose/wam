// Copyright (c) 2014-2021 LG Electronics, Inc.
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

#ifndef WEBOS_PLATFORM_MODULE_FACTORY_IMPL_H_
#define WEBOS_PLATFORM_MODULE_FACTORY_IMPL_H_

#include "platform_module_factory.h"

class ServiceSender;
class WebProcessManager;
class DeviceInfo;
class WebAppManagerConfig;

class PlatformModuleFactoryImpl : public PlatformModuleFactory {
 public:
  PlatformModuleFactoryImpl();

 protected:
  std::unique_ptr<ServiceSender> CreateServiceSender() override;
  std::unique_ptr<WebProcessManager> CreateWebProcessManager() override;
  std::unique_ptr<DeviceInfo> CreateDeviceInfo() override;
  std::unique_ptr<WebAppManagerConfig> CreateWebAppManagerConfig() override;

 private:
  void PrepareRenderingContext();
};

#endif  // WEBOS_PLATFORM_MODULE_FACTORY_IMPL_H_
