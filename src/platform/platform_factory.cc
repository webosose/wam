// Copyright 2022 LG Electronics, Inc.
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

#include "platform_factory.h"

#include "log_manager.h"

PlatformFactory::PlatformFactory() = default;
PlatformFactory::~PlatformFactory() = default;

std::unique_ptr<neva_app_runtime::NotificationPlatformBridge>
PlatformFactory::CreateNotificationPlatformBridge() {
  LOG_DEBUG("[%s] not implemented", __func__);
  return nullptr;
}

std::unique_ptr<neva_app_runtime::PermissionPrompt>
PlatformFactory::CreatePermissionPrompt(
    neva_app_runtime::PermissionPrompt::Delegate* delegate) {
  LOG_DEBUG("[%s] not implemented", __func__);
  return nullptr;
}
