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

#include "PlatformModuleFactory.h"

#include "PlatformModuleFactoryImpl.h"
#include "ServiceSenderLuna.h"
#include "ContainerAppManager.h"
#include "DeviceInfoImpl.h"
#include "WebAppManagerConfig.h"
#include "BlinkWebProcessManager.h"

PlatformModuleFactoryImpl::PlatformModuleFactoryImpl()
{
    prepareRenderingContext();
}

ServiceSender* PlatformModuleFactoryImpl::createServiceSender()
{
    return new ServiceSenderLuna();
}

WebProcessManager* PlatformModuleFactoryImpl::createWebProcessManager()
{
    return new BlinkWebProcessManager();
}

ContainerAppManager* PlatformModuleFactoryImpl::createContainerAppManager()
{
    if(useContainerApp()) {
        return new ContainerAppManager();
    }

    return 0;

}

DeviceInfo* PlatformModuleFactoryImpl::createDeviceInfo()
{
    return new DeviceInfoImpl();
}

WebAppManagerConfig* PlatformModuleFactoryImpl::createWebAppManagerConfig()
{
    return new WebAppManagerConfig();
}

bool PlatformModuleFactoryImpl::useContainerApp()
{
    if (qgetenv("DISABLE_CONTAINER") == "1")
        return false;

    return true;
}

void PlatformModuleFactoryImpl::prepareRenderingContext()
{
}
