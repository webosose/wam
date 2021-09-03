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

#include "blink_web_process_manager.h"
#include "blink_web_view.h"
#include "blink_web_view_profile_helper.h"
#include "log_manager.h"
#include "web_app_base.h"
#include "web_app_manager_utils.h"
#include "web_page_blink.h"
#include "web_process_manager.h"

uint32_t BlinkWebProcessManager::GetWebProcessPID(const WebAppBase* app) const {
  return static_cast<WebPageBlink*>(app->Page())->RenderProcessPid();
}

Json::Value BlinkWebProcessManager::GetWebProcessProfiling() {
  Json::Value reply;
  Json::Value process_array(Json::arrayValue);
  Json::Value process_object;

  std::set<uint32_t> process_id_list;
  std::unordered_multimap<uint32_t, WebAppBase*> running_app_list;

  const std::list<const WebAppBase*>& running = RunningApps();

  for (const auto& elem : running) {
    WebAppBase* app = FindAppByInstanceId((elem)->InstanceId());
    const uint32_t pid = GetWebProcessPID(app);
    process_id_list.insert(pid);
    running_app_list.emplace(pid, app);
  }

  for (uint32_t pid : process_id_list) {
    Json::Value app_object;
    Json::Value app_array(Json::arrayValue);

    process_object["pid"] = std::to_string(pid);
    process_object["webProcessSize"] = GetWebProcessMemSize(pid);
    process_object["tileSize"] = 0;
    auto processes = running_app_list.equal_range(pid);
    for (auto app = processes.first; app != processes.second; app++) {
      app_object["id"] = app->second->AppId();
      app_object["instanceId"] = app->second->InstanceId();
      app_array.append(app_object);
    }
    process_object["runningApps"] = app_array;
    process_array.append(process_object);
  }

  reply["WebProcesses"] = process_array;
  reply["returnValue"] = true;
  return reply;
}

void BlinkWebProcessManager::DeleteStorageData(const std::string& identifier) {
  std::list<const WebAppBase*> running_app_list = RunningApps();
  if (!running_app_list.empty()) {
    running_app_list.front()->Page()->DeleteWebStorages(identifier);
    return;
  }

  BlinkWebView* webview = new BlinkWebView();
  if (webview) {
    webview->DeleteWebStorages(identifier);
    delete webview;
  }
}

uint32_t BlinkWebProcessManager::GetInitialWebViewProxyID() const {
  return 0;
}

void BlinkWebProcessManager::ClearBrowsingData(
    const int remove_browsing_data_mask) {
  BlinkWebViewProfileHelper::ClearBrowsingData(remove_browsing_data_mask);
}

int BlinkWebProcessManager::MaskForBrowsingDataType(const char* type) {
  return BlinkWebViewProfileHelper::MaskForBrowsingDataType(type);
}
