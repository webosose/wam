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

#ifndef SERVICESENDER_H
#define SERVICESENDER_H

#include "WebAppBase.h"
#include "WebAppManager.h"

class ServiceSender {
public:
    virtual ~ServiceSender() {}
    virtual void requestActivity(WebAppBase* app) = 0;
#ifndef PRELOADMANAGER_ENABLED
    virtual void launchContainerApp(const QString& id) = 0;
#endif
    virtual void postlistRunningApps(std::vector<ApplicationInfo>& apps) = 0;
    virtual void postWebProcessCreated(const QString& appId, uint32_t pid) = 0;
    virtual void serviceCall(const QString& url, const QString& payload, const QString& appId) = 0;
    virtual void closeApp(const std::string& id) = 0;
};

#endif //SERVICESENDER_H
