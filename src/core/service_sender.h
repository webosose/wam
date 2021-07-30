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

#ifndef CORE_SERVICE_SENDER_H_
#define CORE_SERVICE_SENDER_H_

#include <string>

#include "web_app_base.h"
#include "web_app_manager.h"

class ServiceSender {
 public:
  virtual ~ServiceSender() = default;
  virtual void PostlistRunningApps(std::vector<ApplicationInfo>& apps) = 0;
  virtual void PostWebProcessCreated(const std::string& app_id,
                                     const std::string& instance_id,
                                     uint32_t pid) = 0;
  virtual void ServiceCall(const std::string& url,
                           const std::string& payload,
                           const std::string& app_id) = 0;
  virtual void CloseApp(const std::string& id) = 0;
};

#endif  // CORE_SERVICE_SENDER_H_
