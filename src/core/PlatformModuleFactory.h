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

#ifndef PLATFORMMODULEFACTORY_H
#define PLATFORMMODULEFACTORY_H

#include <memory>

#include "ServiceSender.h"
#include "WebProcessManager.h"
#include "DeviceInfo.h"
#include "WebAppManagerConfig.h"

class PlatformModuleFactory {
public:
    std::unique_ptr<ServiceSender> getServiceSender() { return createServiceSender(); }
    std::unique_ptr<WebProcessManager> getWebProcessManager() { return createWebProcessManager(); }
    std::unique_ptr<DeviceInfo> getDeviceInfo() { return createDeviceInfo(); }
    std::unique_ptr<WebAppManagerConfig> getWebAppManagerConfig() { return createWebAppManagerConfig(); }

protected:
    virtual std::unique_ptr<ServiceSender> createServiceSender() = 0;
    virtual std::unique_ptr<WebProcessManager> createWebProcessManager() = 0;
    virtual std::unique_ptr<DeviceInfo> createDeviceInfo() = 0;
    virtual std::unique_ptr<WebAppManagerConfig> createWebAppManagerConfig() = 0;
};

#endif /* PLATFORMMODULEFACTORY_H */
