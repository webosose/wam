// Copyright (c) 2008-2018 LG Electronics, Inc.
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

#ifndef UTIL_NETWORK_STATUS_MANAGER_H_
#define UTIL_NETWORK_STATUS_MANAGER_H_

#include "network_status.h"

#include <string>
#include <unordered_map>
#include <utility>

class NetworkStatusManager {
 public:
  void UpdateNetworkStatus(const NetworkStatus& status);
  void CheckInformationChange(const NetworkStatus::Information& information);
  void AppendLogList(const std::string& key,
                     const std::string& previous,
                     const std::string& current);
  void PrintLog();

 private:
  NetworkStatus current_;
  std::unordered_map<std::string, std::pair<std::string, std::string>>
      log_list_;
};

#endif  // UTIL_NETWORK_STATUS_MANAGER_H_
