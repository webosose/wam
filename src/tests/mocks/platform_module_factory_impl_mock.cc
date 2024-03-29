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

#include "platform_module_factory_impl_mock.h"

#include "blink_web_process_manager_mock.h"
#include "web_app_manager_config.h"
#include "web_app_manager_config_mock.h"

std::map<std::string, std::string>
    PlatformModuleFactoryImplMock::default_config_;

void PlatformModuleFactoryImplMock::SetDefaultConfig(
    const std::map<std::string, std::string>& config) {
  default_config_ = config;
}

std::unique_ptr<WebProcessManager>
PlatformModuleFactoryImplMock::CreateWebProcessManager() {
  return std::make_unique<BlinkWebProcessManagerMock>();
}

std::unique_ptr<WebAppManagerConfig>
PlatformModuleFactoryImplMock::CreateWebAppManagerConfig() {
  return default_config_.empty()
             ? std::make_unique<WebAppManagerConfig>()
             : std::make_unique<WebAppManagerConfigMock>(&default_config_);
}
