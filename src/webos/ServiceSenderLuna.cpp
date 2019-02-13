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
    activity["name"] = app->appId();
    activity["description"] = "";
    QJsonObject activityType;
    activityType["foreground"] = true;
    activity["type"] = activityType;

    payload["activity"]  = activity;
    payload["subscribe"] = true;
    payload["start"]     = true;
    payload["replace"]   = true;

    bool ret = WebAppManagerServiceLuna::instance()->callPrivate(
                "palm://com.palm.activitymanager/create",
                payload,
                app->page()->getIdentifier().toLatin1().constData());

    if(!ret) {
        LOG_WARNING(MSGID_ACTIVITY_MANAGER_CREATE_FAIL, 0, "Failed to call activitymanager create");
    }
}

#ifndef PRELOADMANAGER_ENABLED
void ServiceSenderLuna::launchContainerApp(const QString& id)
{
    WebAppManagerServiceLuna::instance()->launchContainerApp(id);
}
#endif

void ServiceSenderLuna::postlistRunningApps(std::vector<ApplicationInfo> &apps)
{
    QJsonObject reply;
    QJsonArray runningApps;
    for (auto it = apps.begin(); it != apps.end(); ++it) {
        QJsonObject app;
        app["id"] = it->appId;
        app["processid"] = it->instanceId;
        app["webprocessid"] = QString::number(it->pid);
        runningApps.append(app);
    }
    reply["running"] = runningApps;
    reply["returnValue"] = true;

    WebAppManagerServiceLuna::instance()->postSubscriptionPrivate("listRunningApps", reply);
}

void ServiceSenderLuna::postWebProcessCreated(const QString& appId, uint32_t pid)
{
    QJsonObject reply;
    reply["id"] = appId;
    reply["webprocessid"] = (int)pid;
    reply["returnValue"] = true;

    WebAppManagerServiceLuna::instance()->postSubscriptionPrivate("webProcessCreated", reply);
}

void ServiceSenderLuna::serviceCall(const QString& url, const QString& payload, const QString& appId)
{
    bool ret = WebAppManagerServiceLuna::instance()->callPrivate(
                url.toLatin1().constData(),
                QJsonDocument::fromJson(payload.toStdString().c_str()).object(),
                appId.toLatin1().constData());
    if (!ret) {
        LOG_WARNING(MSGID_SERVICE_CALL_FAIL, 2, PMLOGKS("APP_ID", qPrintable(appId)), PMLOGKS("URL", qPrintable(url)), "ServiceSenderLuna::serviceCall; callPrivate() return false");
    }
}

void ServiceSenderLuna::closeApp(const std::string& id)
{
    WebAppManagerServiceLuna::instance()->closeApp(id);
}


