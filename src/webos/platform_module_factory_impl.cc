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

#include "platform_module_factory.h"

#include "blink_web_process_manager.h"
#include "device_info_impl.h"
#include "platform_module_factory_impl.h"
#include "service_sender_luna.h"
#include "web_app_manager_config.h"

PlatformModuleFactoryImpl::PlatformModuleFactoryImpl() {
  PrepareRenderingContext();
}

std::unique_ptr<ServiceSender>
PlatformModuleFactoryImpl::CreateServiceSender() {
  return std::unique_ptr<ServiceSenderLuna>(new ServiceSenderLuna());
}

std::unique_ptr<WebProcessManager>
PlatformModuleFactoryImpl::CreateWebProcessManager() {
  return std::unique_ptr<BlinkWebProcessManager>(new BlinkWebProcessManager());
}

std::unique_ptr<DeviceInfo> PlatformModuleFactoryImpl::CreateDeviceInfo() {
  return std::unique_ptr<DeviceInfoImpl>(new DeviceInfoImpl());
}

std::unique_ptr<WebAppManagerConfig>
PlatformModuleFactoryImpl::CreateWebAppManagerConfig() {
  return std::unique_ptr<WebAppManagerConfig>(new WebAppManagerConfig());
}

void PlatformModuleFactoryImpl::PrepareRenderingContext() {}
