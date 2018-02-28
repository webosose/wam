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

#ifndef PLATFORMMODULEFACTORYIMPL_H
#define PLATFORMMODULEFACTORYIMPL_H

#include "PlatformModuleFactory.h"

class ServiceSender;
class WebProcessManager;
class ContainerAppManager;
class DeviceInfo;
class WebAppManagerConfig;

class PlatformModuleFactoryImpl : public PlatformModuleFactory {
public:
    PlatformModuleFactoryImpl();

protected:
    virtual ServiceSender* createServiceSender();
    virtual WebProcessManager* createWebProcessManager();
    virtual ContainerAppManager* createContainerAppManager();
    virtual DeviceInfo* createDeviceInfo();
    virtual WebAppManagerConfig* createWebAppManagerConfig();

private:
    bool useContainerApp();
    void prepareRenderingContext();
};

#endif //PLATFORMMODULEFACTORYIMPL_H
