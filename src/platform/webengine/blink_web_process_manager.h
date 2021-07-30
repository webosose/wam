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

#ifndef PLATFORM_WEBENGINE_BLINK_WEB_PROCESS_MANAGER_H_
#define PLATFORM_WEBENGINE_BLINK_WEB_PROCESS_MANAGER_H_

#include <string>

#include "web_process_manager.h"

namespace Json {
class Value;
};

class WebAppBase;

class BlinkWebProcessManager : public WebProcessManager {
 public:
  // WebProcessManager
  Json::Value GetWebProcessProfiling() override;
  uint32_t GetWebProcessPID(const WebAppBase* app) const override;
  void DeleteStorageData(const std::string& identifier) override;
  uint32_t GetInitialWebViewProxyID() const override;
  void ClearBrowsingData(const int remove_browsing_data_mask) override;
  int MaskForBrowsingDataType(const char* type) override;
};

#endif  // BLINKEBPROCESSMANAGER_H
