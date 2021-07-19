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

#include <set>
#include <unordered_map>

#include <json/json.h>

#include "BlinkWebProcessManager.h"
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
    Json::Value reply;
    Json::Value processArray(Json::arrayValue);
    Json::Value processObject;

    std::set<uint32_t> processIdList;
    std::unordered_multimap<uint32_t, WebAppBase*> runningAppList;

    const std::list<const WebAppBase*>& running = runningApps();

    for (const auto& elem : running) {
        WebAppBase* app = findAppByInstanceId((elem)->instanceId());
        const uint32_t pid = getWebProcessPID(app);
        processIdList.insert(pid);
        runningAppList.emplace(pid, app);
    }

    for (uint32_t pid: processIdList) {
        Json::Value appObject;
        Json::Value appArray(Json::arrayValue);

        processObject["pid"] = std::to_string(pid);
        processObject["webProcessSize"] = getWebProcessMemSize(pid);
        processObject["tileSize"] = 0;
        auto processes = runningAppList.equal_range(pid);
        for (auto app = processes.first; app != processes.second; app++) {
            appObject["id"] = app->second->appId();
            appObject["instanceId"] = app->second->instanceId();
            appArray.append(appObject);
        }
        processObject["runningApps"] = appArray;
        processArray.append(processObject);
    }

    reply["WebProcesses"] = processArray;
    reply["returnValue"] = true;
    return reply;
}

void BlinkWebProcessManager::deleteStorageData(const std::string& identifier)
{
    std::list<const WebAppBase*> runningAppList = runningApps();
    if (!runningAppList.empty()) {
        runningAppList.front()->page()->deleteWebStorages(identifier);
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
