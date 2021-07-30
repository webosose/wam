// Copyright (c) 2015-2021 LG Electronics, Inc.
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

#include "web_app_manager_service_luna_impl.h"

#include "json/json.h"

#include "log_manager.h"
#include "utils.h"

#define LS2_CALL(FUNC, SERVICE, PARAMS)                                    \
  Call<WebAppManagerServiceLunaImpl, &WebAppManagerServiceLunaImpl::FUNC>( \
      SERVICE, PARAMS, this)

WebAppManagerServiceLuna* WebAppManagerServiceLuna::Instance() {
  static WebAppManagerServiceLuna* service = new WebAppManagerServiceLunaImpl();
  return service;
}

void WebAppManagerServiceLunaImpl::SystemServiceConnectCallback(
    const Json::Value& reply) {
  WebAppManagerServiceLuna::SystemServiceConnectCallback(reply);

  if (reply.isObject() && reply.isMember("connected")) {
    Json::Value option_params;
    option_params["subscribe"] = true;
    option_params["category"] = "option";
    Json::Value option_list;
    option_list.append("country");
    option_list.append("smartServiceCountryCode3");
    option_list.append("audioGuidance");
    option_list.append("screenRotation");
    option_params["keys"] = option_list;
    LS2_CALL(GetSystemOptionCallback,
             "luna://com.webos.settingsservice/getSystemSettings",
             option_params);
  }
}

Json::Value WebAppManagerServiceLunaImpl::setInspectorEnable(
    const Json::Value& request) {
  return util::StringToJson(R"({"returnValue": true})");
}

void WebAppManagerServiceLunaImpl::GetSystemOptionCallback(
    const Json::Value& reply) {
  Json::Value settings = reply["settings"];
  // The settings is empty when service is crashed
  // The right value will be notified again when service is restarted
  if (!reply.isObject() || !reply["settings"].isObject() ||
      reply["settings"].empty()) {
    LOG_WARNING(MSGID_RECEIVED_INVALID_SETTINGS, 1,
                PMLOGKFV("MSG", "%s", util::JsonToString(reply).c_str()), "");
    return;
  }
  LOG_INFO(MSGID_SETTING_SERVICE, 0,
           "Notified from settingsservice/getSystemSettings");

  std::string country = reply["settings"]["country"].isString()
                            ? settings["country"].asString()
                            : "";
  std::string smart_service_country =
      reply["settings"]["country"].isString()
          ? settings["smartServiceCountryCode3"].asString()
          : "";
  std::string audio_guidance = reply["settings"]["country"].isString()
                                   ? settings["audioGuidance"].asString()
                                   : "";
  std::string screen_rotation = reply["settings"]["country"].isString()
                                    ? settings["screenRotation"].asString()
                                    : "";

  LOG_INFO(
      MSGID_SETTING_SERVICE, 1,
      PMLOGKS("BroadcastCountry", country.empty() ? "Empty" : country.c_str()),
      "");
  LOG_INFO(MSGID_SETTING_SERVICE, 1,
           PMLOGKS("SmartServiceCountry", smart_service_country.empty()
                                              ? "Empty"
                                              : smart_service_country.c_str()),
           "");
  LOG_INFO(MSGID_SETTING_SERVICE, 1,
           PMLOGKS("AudioGuidance",
                   audio_guidance.empty() ? "Empty" : audio_guidance.c_str()),
           "");
  LOG_INFO(MSGID_SETTING_SERVICE, 1,
           PMLOGKS("ScreenRotation",
                   screen_rotation.empty() ? "Empty" : screen_rotation.c_str()),
           "");
  if (!country.empty())
    WebAppManagerService::SetDeviceInfo("LocalCountry", country.c_str());
  if (!smart_service_country.empty())
    WebAppManagerService::SetDeviceInfo("SmartServiceCountry",
                                        smart_service_country.c_str());
  if (!audio_guidance.empty())
    WebAppManagerService::SetAccessibilityEnabled(audio_guidance == "on");
  if (!screen_rotation.empty())
    WebAppManagerService::SetDeviceInfo("ScreenRotation",
                                        screen_rotation.c_str());
}
