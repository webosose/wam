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

#include "BlinkWebProcessManager.h"

#include <memory>
#include <json/value.h>
#include <QtCore/QList>
#include <QString>

#include "WebPageBlink.h"
#include "WebAppBase.h"
#include "WebAppManagerUtils.h"
#include "LogManager.h"
#include "BlinkWebView.h"
#include "BlinkWebViewProfileHelper.h"
#include "WebProcessManager.h"

uint32_t BlinkWebProcessManager::getWebProcessPID(const WebAppBase* app) const
{
    return static_cast<WebPageBlink*>(app->page())->renderProcessPid();
}

Json::Value BlinkWebProcessManager::getWebProcessProfiling()
{
    Json::Value reply(Json::objectValue);
    Json::Value processArray(Json::arrayValue);
    uint32_t pid;
    QList<uint32_t> processIdList;

    QMap<uint32_t, QString> runningAppList;
    std::list<const WebAppBase*> running = runningApps();
    for (auto it = running.begin(); it != running.end(); ++it) {
        WebAppBase* app = findAppById((*it)->appId());
        pid = getWebProcessPID(app);
        if (!processIdList.contains(pid))
            processIdList.append(pid);

        runningAppList.insertMulti(pid, app->appId());
    }

    WebAppBase* containerApp = getContainerApp();
    if (containerApp) {
        pid = getWebProcessPID(containerApp);
        if (!processIdList.contains(pid))
            processIdList.append(pid);

        runningAppList.insertMulti(pid, containerApp->appId());
    }

    for (int id = 0; id < processIdList.size(); id++) {
        Json::Value processObject(Json::objectValue);
        Json::Value appArray(Json::arrayValue);
        pid = processIdList.at(id);

        processObject["pid"] = QString::number(pid).toStdString();
        processObject["webProcessSize"] = getWebProcessMemSize(pid).toStdString();
        //starfish-surface is note used on Blink
        processObject["tileSize"] = 0;
        QList<QString> processApp = runningAppList.values(pid);
        for (int app = 0; app < processApp.size(); app++) {
            Json::Value appObject(Json::objectValue);
            appObject["id"] = processApp.at(app).toStdString();
            appArray.append(appObject);
        }
        processObject["runningApps"] = appArray;
        processArray.append(processObject);
    }

    reply["WebProcesses"] = processArray;
    reply["returnValue"] = true;
    return std::move(reply);
}

void BlinkWebProcessManager::deleteStorageData(const QString& identifier)
{
    std::list<const WebAppBase*> runningAppList = runningApps();
    if (!runningAppList.empty()) {
        runningAppList.front()->page()->deleteWebStorages(identifier);
        return;
    }

    WebAppBase* containerApp = getContainerApp();
    if (containerApp) {
        containerApp->page()->deleteWebStorages(identifier);
        return;
    }

    BlinkWebView* webview = new BlinkWebView();
    if (webview) {
        webview->DeleteWebStorages(identifier.toStdString());
        delete webview;
    }
}

uint32_t BlinkWebProcessManager::getInitialWebViewProxyID() const
{
    return 0;
}

void BlinkWebProcessManager::clearBrowsingData(const int removeBrowsingDataMask)
{
    BlinkWebViewProfileHelper::clearBrowsingData(removeBrowsingDataMask);
}

int BlinkWebProcessManager::maskForBrowsingDataType(const char* type)
{
    return BlinkWebViewProfileHelper::maskForBrowsingDataType(type);
}

void BlinkWebProcessManager::setProxyServer(const std::string& ip, const std::string& port) {
    BlinkWebViewProfileHelper::setProxyServer(ip, port);
}
