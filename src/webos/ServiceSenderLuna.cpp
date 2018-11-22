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

#include "ServiceSenderLuna.h"
#include "WebAppManagerServiceLuna.h"
#include "WebPageBase.h"
#include "LogManager.h"

#include <QJsonObject>
#include <QString>
#include <QJsonArray>

void ServiceSenderLuna::requestActivity(WebAppBase* app)
{
    QJsonObject payload;
    QJsonObject activity;
    activity["name"] = QString::fromStdString(app->appId());
    activity["description"] = "";
    QJsonObject activityType;
    activityType["foreground"] = true;
    activity["type"] = activityType;

    payload["activity"]  = activity;
    payload["subscribe"] = true;
    payload["start"]     = true;
    payload["replace"]   = true;

    bool ret = WebAppManagerServiceLuna::instance()->call(
                  "palm://com.palm.activitymanager/create",
                  payload,
                  app->page()->getIdentifier().c_str())

    if (!ret) {
        LOG_WARNING(MSGID_ACTIVITY_MANAGER_CREATE_FAIL, 0, "Failed to call activitymanager create");
    }
}

#ifndef PRELOADMANAGER_ENABLED
void ServiceSenderLuna::launchContainerApp(const std::string& id)
{
    WebAppManagerServiceLuna::instance()->launchContainerApp(QString::fromStdString(id));
}
#endif

void ServiceSenderLuna::postlistRunningApps(std::vector<ApplicationInfo> &apps)
{
    QJsonObject reply;
    QJsonArray runningApps;
    for (auto it = apps.begin(); it != apps.end(); ++it) {
        QJsonObject app;
        app["id"] = QString::fromStdString(it->appId);
        app["processid"] = QString::fromStdString(it->instanceId);
        app["webprocessid"] = QString::number(it->pid);
        runningApps.append(app);
    }
    reply["running"] = runningApps;
    reply["returnValue"] = true;

    WebAppManagerServiceLuna::instance()->postSubscription("listRunningApps", reply);
}

void ServiceSenderLuna::postWebProcessCreated(const std::string& appId, uint32_t pid)
{
    QJsonObject reply;
    reply["id"] = Qt::fromStdString(appId);
    reply["webprocessid"] = (int)pid;
    reply["returnValue"] = true;

    WebAppManagerServiceLuna::instance()->postSubscription("webProcessCreated", reply);
}

void ServiceSenderLuna::serviceCall(const std::string& url, const std::string& payload, const std::string& appId)
{
    bool ret = WebAppManagerServiceLuna::instance()->call(
        url.c_str(),
        QJsonDocument::fromJson(payload.c_str()).object(),
        appId.c_str());
    if (!ret) {
        LOG_WARNING(MSGID_SERVICE_CALL_FAIL, 2, PMLOGKS("APP_ID", appId.c_str()), PMLOGKS("URL", url.c_str()),
                    "ServiceSenderLuna::serviceCall; callPrivate() return false");
    }
}

void ServiceSenderLuna::closeApp(const std::string& id)
{
    WebAppManagerServiceLuna::instance()->closeApp(id);
}


