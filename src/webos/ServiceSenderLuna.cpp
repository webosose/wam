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
#include "WebAppManagerServiceLuna.h"
#include "WebPageBase.h"
#include "QtLessTemporaryHelpers.h"

void ServiceSenderLuna::postlistRunningApps(std::vector<ApplicationInfo> &apps)
{
    Json::Value reply;
    Json::Value runningApps;
    for (auto it = apps.begin(); it != apps.end(); ++it) {
        Json::Value app;
        app["id"] = it->appId.toStdString();
        app["instanceid"] = it->instanceId.toStdString();
        app["webprocessid"] = qtless::StringHelper::intToStr(it->pid);
        runningApps.append(app);
    }
    reply["running"] = runningApps;
    reply["returnValue"] = true;

    WebAppManagerServiceLuna::instance()->postSubscription("listRunningApps", qtless::JsonHelper::qjsonFromJsonCpp(reply));
}

void ServiceSenderLuna::postWebProcessCreated(const QString& appId, const QString& instanceId, uint32_t pid)
{
    Json::Value reply;
    reply["id"] = appId.toStdString();
    reply["instanceid"] = instanceId.toStdString();
    reply["webprocessid"] = (int)pid;
    reply["returnValue"] = true;

    WebAppManagerServiceLuna::instance()->postSubscription("webProcessCreated", qtless::JsonHelper::qjsonFromJsonCpp(reply));
}

void ServiceSenderLuna::serviceCall(const QString& url, const QString& payload, const QString& appId)
{
    std::string strUrl = url.toStdString();
    std::string strPayload = payload.toStdString();
    std::string strAppId = appId.toStdString();
    Json::Value jsonPayload = qtless::JsonHelper::jsonCppFromString(strPayload);

    bool ret = WebAppManagerServiceLuna::instance()->call(
        strUrl.c_str(),
        qtless::JsonHelper::qjsonFromJsonCpp(jsonPayload),
        strAppId.c_str());
    if (!ret) {
        LOG_WARNING(MSGID_SERVICE_CALL_FAIL, 2, PMLOGKS("APP_ID", strAppId.c_str()), PMLOGKS("URL", strUrl.c_str()), "ServiceSenderLuna::serviceCall; callPrivate() return false");
    }
}

void ServiceSenderLuna::closeApp(const std::string& id)
{
    WebAppManagerServiceLuna::instance()->closeApp(id);
}


