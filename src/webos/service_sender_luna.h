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

#ifndef WEBOS_SERVICE_SENDER_LUNA_H_
#define WEBOS_SERVICE_SENDER_LUNA_H_

#include <string>

#include "service_sender.h"

class ServiceSenderLuna : public ServiceSender {
 public:
  void PostlistRunningApps(std::vector<ApplicationInfo>& apps) override;
  void PostWebProcessCreated(const std::string& app_id,
                             const std::string& instance_id,
                             uint32_t pid) override;
  void ServiceCall(const std::string& url,
                   const std::string& payload,
                   const std::string& app_id) override;
  void CloseApp(const std::string& id) override;
};

#endif  // WEBOS_SERVICE_SENDER_LUNA_H_
