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

#include "service_sender_luna.h"

#include <json/json.h>

#include "log_manager.h"
#include "utils.h"
#include "web_app_manager_service_luna.h"
#include "web_page_base.h"

void ServiceSenderLuna::PostlistRunningApps(
    std::vector<ApplicationInfo>& apps) {
  Json::Value reply;
  Json::Value running_apps;
  for (auto it = apps.begin(); it != apps.end(); ++it) {
    Json::Value app;
    app["id"] = it->app_id_;
    app["instanceid"] = it->instance_id_;
    app["webprocessid"] = std::to_string(it->pid_);
    running_apps.append(app);
  }
  reply["running"] = running_apps;
  reply["returnValue"] = true;

  WebAppManagerServiceLuna::Instance()->PostSubscription("listRunningApps",
                                                         reply);
}

void ServiceSenderLuna::PostWebProcessCreated(const std::string& app_id,
                                              const std::string& instance_id,
                                              uint32_t pid) {
  Json::Value reply;
  reply["id"] = app_id;
  reply["instanceid"] = instance_id;
  reply["webprocessid"] = static_cast<int>(pid);
  reply["returnValue"] = true;

  WebAppManagerServiceLuna::Instance()->PostSubscription("webProcessCreated",
                                                         reply);
}

void ServiceSenderLuna::ServiceCall(const std::string& url,
                                    const std::string& payload,
                                    const std::string& app_id) {
  Json::Value json_payload = util::StringToJson(payload);

  bool ret = WebAppManagerServiceLuna::Instance()->Call(
      url.c_str(), json_payload, app_id.c_str());
  if (!ret) {
    LOG_WARNING(MSGID_SERVICE_CALL_FAIL, 2, PMLOGKS("APP_ID", app_id.c_str()),
                PMLOGKS("URL", url.c_str()),
                "ServiceSenderLuna::serviceCall; callPrivate() return false");
  }
}

void ServiceSenderLuna::CloseApp(const std::string& id) {
  WebAppManagerServiceLuna::Instance()->CloseApp(id);
}
