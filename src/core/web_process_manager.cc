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

#include <sys/types.h>

#include <fstream>
#include <list>
#include <string>

#include "utils.h"
#include "web_app_manager.h"

class WebAppBase;

std::list<const WebAppBase*> WebProcessManager::RunningApps() {
  return WebAppManager::Instance()->RunningApps();
}

WebAppBase* WebProcessManager::FindAppByInstanceId(
    const std::string& instance_id) {
  return WebAppManager::Instance()->FindAppByInstanceId(instance_id);
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
