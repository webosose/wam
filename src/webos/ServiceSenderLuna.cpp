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

#include "ServiceSenderLuna.h"

#include <json/json.h>

#include "LogManager.h"
#include "Utils.h"
#include "WebAppManagerServiceLuna.h"
#include "WebPageBase.h"

void ServiceSenderLuna::postlistRunningApps(std::vector<ApplicationInfo> &apps)
{
    Json::Value reply;
    Json::Value runningApps;
    for (auto it = apps.begin(); it != apps.end(); ++it) {
        Json::Value app;
        app["id"] = it->appId;
        app["instanceid"] = it->instanceId;
        app["webprocessid"] = std::to_string(it->pid);
        runningApps.append(app);
    }
    reply["running"] = runningApps;
    reply["returnValue"] = true;

    WebAppManagerServiceLuna::instance()->postSubscription("listRunningApps", reply);
}

void ServiceSenderLuna::postWebProcessCreated(const std::string& appId, const std::string& instanceId, uint32_t pid)
{
    Json::Value reply;
    reply["id"] = appId;
    reply["instanceid"] = instanceId;
    reply["webprocessid"] = (int)pid;
    reply["returnValue"] = true;

    WebAppManagerServiceLuna::instance()->postSubscription("webProcessCreated", reply);
}

void ServiceSenderLuna::serviceCall(const std::string& url, const std::string& payload, const std::string& appId)
{
    Json::Value jsonPayload = util::stringToJson(payload);

    bool ret = WebAppManagerServiceLuna::instance()->call(
        url.c_str(),
        jsonPayload,
        appId.c_str());
    if (!ret) {
        LOG_WARNING(MSGID_SERVICE_CALL_FAIL, 2, PMLOGKS("APP_ID", appId.c_str()), PMLOGKS("URL", url.c_str()), "ServiceSenderLuna::serviceCall; callPrivate() return false");
    }
}

void ServiceSenderLuna::closeApp(const std::string& id)
{
    WebAppManagerServiceLuna::instance()->closeApp(id);
}


