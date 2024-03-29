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

#include "web_process_manager.h"

#include <signal.h>

#include <climits>
#include <cstdio>
#include <fstream>
#include <string>

#include <glib.h>
#include <json/json.h>

#include "application_description.h"
#include "log_manager.h"
#include "utils.h"
#include "web_app_base.h"
#include "web_app_manager.h"
#include "web_app_manager_config.h"
#include "web_app_manager_utils.h"
#include "web_page_base.h"

WebProcessManager::WebProcessManager() {
  ReadWebProcessPolicy();
}

std::list<const WebAppBase*> WebProcessManager::RunningApps() {
  return WebAppManager::Instance()->RunningApps();
}

std::list<const WebAppBase*> WebProcessManager::RunningApps(uint32_t pid) {
  return WebAppManager::Instance()->RunningApps(pid);
}

WebAppBase* WebProcessManager::FindAppById(const std::string& app_id) {
  return WebAppManager::Instance()->FindAppById(app_id);
}

WebAppBase* WebProcessManager::FindAppByInstanceId(
    const std::string& instance_id) {
  return WebAppManager::Instance()->FindAppByInstanceId(instance_id);
}

bool WebProcessManager::WebProcessInfoMapReady() {
  uint32_t count = 0;
  for (const auto& it : web_process_info_map_) {
    if (it.second.proxy_id_ != 0) {
      count++;
    }
  }

  return count == maximum_number_of_processes_;
}

uint32_t WebProcessManager::GetWebProcessProxyID(
    const ApplicationDescription* desc) const {
  if (!desc) {
    return 0;
  }

  std::string key = GetProcessKey(desc);

  auto it = web_process_info_map_.find(key);
  if (it == web_process_info_map_.end() || !it->second.proxy_id_) {
    return GetInitialWebViewProxyID();
  }

  return it->second.proxy_id_;
}

uint32_t WebProcessManager::GetWebProcessProxyID(uint32_t pid) const {
  auto res = find_if(web_process_info_map_.begin(), web_process_info_map_.end(),
                     [pid](const auto& item) {
                       return (item.second.web_process_pid_ == pid);
                     });

  if (res != web_process_info_map_.end()) {
    return res->second.proxy_id_;
  }
  return 0;
}

std::string WebProcessManager::GetWebProcessMemSize(uint32_t pid) const {
  std::string path = "/proc/" + std::to_string(pid) + "/status";
  std::ifstream in(path);

  if (!in.is_open()) {
    return {};
  }

  std::string line;
  while (std::getline(in, line)) {
    if (line.find("VmRSS:", 0, 6) != std::string::npos) {
      return util::TrimString(std::string(line, 6)).c_str();
    }
  }
  return {};
}

void WebProcessManager::ReadWebProcessPolicy() {
  std::string config_path =
      WebAppManager::Instance()->Config()->GetWebProcessConfigPath();
  Json::Value web_process_environment =
      util::StringToJson(util::ReadFile(config_path));

  if (web_process_environment.isNull()) {
    LOG_ERROR(MSGID_WEBPROCESSENV_READ_FAIL, 1,
              PMLOGKS("PATH", config_path.c_str()), "JSON parsing failed");
    return;
  }

  auto create_process_for_each_app =
      web_process_environment["createProcessForEachApp"];
  if (create_process_for_each_app.isBool() &&
      create_process_for_each_app.asBool()) {
    maximum_number_of_processes_ = UINT_MAX;
  } else {
    auto web_process_array = web_process_environment["webProcessList"];
    if (web_process_array.isArray()) {
      for (const auto& value : web_process_array) {
        if (!value.isObject()) {
          continue;
        }
        auto id = value["id"];
        if (id.isString()) {
          web_process_group_app_id_list_.push_back(id.asString());
          SetWebProcessCacheProperty(value, id.asString());
        }
        auto trust_level = value["trustLevel"];
        if (trust_level.isString()) {
          web_process_group_trust_level_list_.push_back(trust_level.asString());
          SetWebProcessCacheProperty(value, trust_level.asString());
        }
      }
    }
    maximum_number_of_processes_ = (web_process_group_trust_level_list_.size() +
                                    web_process_group_app_id_list_.size());
  }

  LOG_INFO(
      MSGID_SET_WEBPROCESS_ENVIRONMENT, 3,
      PMLOGKS("MAXIMUM_WEBPROCESS_NUMBER",
              std::to_string(maximum_number_of_processes_).c_str()),
      PMLOGKS(
          "GROUP_TRUSTLEVELS_COUNT",
          std::to_string(web_process_group_trust_level_list_.size()).c_str()),
      PMLOGKS("GROUP_APP_IDS_COUNT",
              std::to_string(web_process_group_app_id_list_.size()).c_str()),
      "");
}

void WebProcessManager::SetWebProcessCacheProperty(const Json::Value& object,
                                                   const std::string& key) {
  WebProcessInfo info = WebProcessInfo(0, 0);
  auto memory_cache = object["memoryCache"];
  if (memory_cache.isString()) {
    int mem_cache_size = util::StrToIntWithDefault(memory_cache.asString(), 0);
    info.memory_cache_size_ = mem_cache_size;
  }

  auto code_cache = object["codeCache"];
  if (code_cache.isString()) {
    int code_cache_int = util::StrToIntWithDefault(code_cache.asString(), 0);
    info.code_cache_size_ = code_cache_int;
  }

  web_process_info_map_.emplace(key, info);
}

std::string WebProcessManager::GetProcessKey(
    const ApplicationDescription* desc) const {
  if (!desc) {
    return std::string();
  }

  std::string key;
  if (maximum_number_of_processes_ == 1) {
    key = "system";
  } else if (maximum_number_of_processes_ == UINT_MAX) {
    if (desc->TrustLevel() == "default" || desc->TrustLevel() == "trusted") {
      key = "system";
    } else {
      key = desc->Id();
    }
  } else {
    std::vector<std::string> id_list;
    for (const std::string& app_id : web_process_group_app_id_list_) {
      if (app_id.find('*') != std::string::npos) {
        std::string replaced_app_id = app_id;
        util::ReplaceSubstr(replaced_app_id, "*");
        auto l = util::SplitString(replaced_app_id, ',');
        id_list.insert(id_list.end(), l.begin(), l.end());
        for (const auto& id : id_list) {
          if (!desc->Id().compare(0, id.size(), id)) {
            key = app_id;
          }
        }
      } else {
        auto l = util::SplitString(app_id, ',');
        id_list.insert(id_list.end(), l.begin(), l.end());
        for (const auto& id : id_list) {
          if (id == desc->Id()) {
            return app_id;
          }
        }
      }
    }
    if (!key.empty()) {
      return key;
    }

    std::vector<std::string> trust_level_list;
    for (const std::string& trust_level : web_process_group_trust_level_list_) {
      auto l = util::SplitString(trust_level, ',');
      trust_level_list.insert(trust_level_list.end(), l.begin(), l.end());
      for (const auto& trust : trust_level_list) {
        if (trust == desc->TrustLevel()) {
          return trust_level;
        }
      }
    }
    key = "system";
  }
  return key;
}

void WebProcessManager::KillWebProcess(uint32_t pid) {
  for (auto& it : web_process_info_map_) {
    if (it.second.web_process_pid_ == pid) {
      it.second.request_kill_ = false;
      break;
    }
  }

  LOG_INFO(MSGID_KILL_WEBPROCESS, 1, PMLOGKFV("PID", "%u", pid), "");
  int ret = kill(pid, SIGKILL);
  if (ret == -1) {
    LOG_ERROR(MSGID_KILL_WEBPROCESS_FAILED, 1,
              PMLOGKS("ERROR", strerror(errno)), "SystemCall failed");
  }
}

void WebProcessManager::RequestKillWebProcess(uint32_t pid) {
  for (auto& it : web_process_info_map_) {
    if (it.second.web_process_pid_ == pid) {
      LOG_INFO(MSGID_KILL_WEBPROCESS_DELAYED, 1, PMLOGKFV("PID", "%u", pid),
               "");
      it.second.request_kill_ = true;
      return;
    }
  }
}
