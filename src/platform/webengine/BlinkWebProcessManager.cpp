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

#include <unordered_map>
#include <memory>
#include <set>

#include <json/value.h>

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
    std::set<uint32_t> processIdList;

    std::unordered_multimap<uint32_t, std::string> runningAppsMap;
    std::list<const WebAppBase*> running = runningApps();
    for (auto it = running.begin(); it != running.end(); ++it) {
        std::string appid = (*it)->appId();
        WebAppBase* app = findAppById(appid);
        pid = getWebProcessPID(app);
        processIdList.insert(pid);
        runningAppsMap.emplace(pid, app->appId());
    }

    WebAppBase* containerApp = getContainerApp();
    if (containerApp) {
        pid = getWebProcessPID(containerApp);
        processIdList.insert(pid);

        runningAppsMap.emplace(pid, containerApp->appId());
    }

    for (uint32_t pid : processIdList) {
        Json::Value processObject(Json::objectValue);
        Json::Value appArray(Json::arrayValue);

        processObject["pid"] = std::to_string(pid);
        processObject["webProcessSize"] = getWebProcessMemSize(pid);
        //starfish-surface is note used on Blink
        processObject["tileSize"] = 0;
        auto processes = runningAppsMap.equal_range(pid);
        for (auto p = processes.first; p != processes.second; ++p) {
            Json::Value appObject(Json::objectValue);
            appObject["id"] = p->second;
            appArray.append(appObject);
        }
        processObject["runningApps"] = appArray;
        processArray.append(processObject);
    }

    reply["WebProcesses"] = processArray;
    reply["returnValue"] = true;
    return std::move(reply);
}

void BlinkWebProcessManager::deleteStorageData(const std::string& identifier)
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
        webview->DeleteWebStorages(identifier);
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
