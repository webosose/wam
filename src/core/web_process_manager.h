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

#ifndef CORE_WEB_PROCESS_MANAGER_H_
#define CORE_WEB_PROCESS_MANAGER_H_

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

namespace Json {
class Value;
};

class ApplicationDescription;
class WebPageBase;
class WebAppBase;

class WebProcessManager {
 public:
  WebProcessManager();
  virtual ~WebProcessManager() {}

  uint32_t GetWebProcessProxyID(const ApplicationDescription* desc) const;
  uint32_t GetWebProcessProxyID(uint32_t pid) const;
  virtual std::string GetWebProcessMemSize(uint32_t pid) const;
  void KillWebProcess(uint32_t pid);
  void RequestKillWebProcess(uint32_t pid);
  bool WebProcessInfoMapReady();
  void SetWebProcessCacheProperty(const Json::Value& object,
                                  const std::string& key);
  void ReadWebProcessPolicy();
  std::string GetProcessKey(const ApplicationDescription* desc) const;

  virtual Json::Value GetWebProcessProfiling() = 0;
  virtual uint32_t GetWebProcessPID(const WebAppBase* app) const = 0;
  virtual void DeleteStorageData(const std::string& identifier) = 0;
  virtual uint32_t GetInitialWebViewProxyID() const = 0;
  virtual void ClearBrowsingData(const int remove_browsing_data_mask) = 0;
  virtual int MaskForBrowsingDataType(const char* type) = 0;

 protected:
  std::list<const WebAppBase*> RunningApps();
  std::list<const WebAppBase*> RunningApps(uint32_t pid);
  WebAppBase* FindAppById(const std::string& app_id);
  WebAppBase* FindAppByInstanceId(const std::string& instance_id);

 protected:
  class WebProcessInfo {
   public:
    // FIXME: Fix default cache values when WebKit defaults change.
    static const uint32_t kDefaultMemoryCache = 32;
    static const uint32_t kDefaultCodeCache = 8;

    WebProcessInfo(uint32_t id,
                   uint32_t pid,
                   uint32_t memory_cache = kDefaultMemoryCache,
                   uint32_t code_cache = kDefaultCodeCache)
        : proxy_id_(id),
          web_process_pid_(pid),
          number_of_apps_(1),
          memory_cache_size_(memory_cache),
          code_cache_size_(code_cache),
          request_kill_(false) {}

    uint32_t proxy_id_;
    uint32_t web_process_pid_;
    uint32_t number_of_apps_;
    uint32_t memory_cache_size_;
    uint32_t code_cache_size_;
    bool request_kill_;
  };
  std::unordered_map<std::string, WebProcessInfo> web_process_info_map_;

  uint32_t maximum_number_of_processes_;
  std::vector<std::string> web_process_group_app_id_list_;
  std::vector<std::string> web_process_group_trust_level_list_;
};

#endif  // CORE_WEB_PROCESS_MANAGER_H_
