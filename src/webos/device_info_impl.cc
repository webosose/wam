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

#include "device_info_impl.h"

#include <string>

#include <glib.h>
#include <json/value.h>
#include <lunaprefs.h>

#include "log_manager.h"
#include "utils.h"

DeviceInfoImpl::DeviceInfoImpl() = default;

void DeviceInfoImpl::Initialize() {
  const std::string& json_string =
      util::ReadFile("/var/luna/preferences/localeInfo");
  if (json_string.empty()) {
    return;
  }

  Json::Value locale_json = util::StringToJson(json_string);
  if (!locale_json.isObject() || locale_json.empty() ||
      !locale_json["localeInfo"].isObject() ||
      !locale_json["localeInfo"]["locales"].isObject() ||
      !locale_json["localeInfo"]["locales"]["UI"].isString() ||
      !locale_json["country"].isString() ||
      !locale_json["smartServiceCountryCode3"].isString()) {
    LOG_ERROR(MSGID_LOCALEINFO_READ_FAIL, 1,
              PMLOGKS("CONTENT", json_string.c_str()), "");
    return;
  }

  Json::Value locale_info = locale_json["localeInfo"];

  std::string language(locale_info["locales"]["UI"].asString());
  std::string localcountry(locale_json["country"].asString());
  std::string smartservicecountry(
      locale_json["smartServiceCountryCode3"].asString());

  SetSystemLanguage(language.c_str());
  SetDeviceInfo("LocalCountry", localcountry.c_str());
  SetDeviceInfo("SmartServiceCountry", smartservicecountry.c_str());
}

bool DeviceInfoImpl::GetInfoFromLunaPrefs(const char* key,
                                          std::string& value) const {
  char* str = nullptr;
  if (LP_ERR_NONE == LPSystemCopyStringValue(key, &str) && str) {
    value = str;
    g_free(static_cast<gchar*>(str));
    return true;
  }

  g_free(static_cast<gchar*>(str));
  value = "Unknown";
  return false;
}

void DeviceInfoImpl::InitDisplayInfo() {
  // Display information
  // --------------------------------------------------------
  int hardware_screen_width = 0;
  int hardware_screen_height = 0;

  std::string hardware_screen_width_str;
  std::string hardware_screen_height_str;
  if (GetDeviceInfo("HardwareScreenWidth", hardware_screen_width_str) &&
      GetDeviceInfo("HardwareScreenHeight", hardware_screen_height_str)) {
    hardware_screen_width =
        util::StrToIntWithDefault(hardware_screen_width_str, 0);
    hardware_screen_height =
        util::StrToIntWithDefault(hardware_screen_height_str, 0);
  } else {
    GetDisplayWidth(hardware_screen_width);
    GetDisplayHeight(hardware_screen_height);
  }

  screen_width_ = static_cast<int>(hardware_screen_width / screen_density_);
  screen_height_ = static_cast<int>(hardware_screen_height / screen_density_);
}

void DeviceInfoImpl::InitPlatformInfo() {
  // normally like this info
  /*
     "modelName": "WEBOS1",
     "platformVersion": "00.00.00",
     "platformVersionDot": 00,
     "platformVersionMajor_pos": 00,
     "platformVersionMinor": 00,
  */

  std::string value;
  if (GetDeviceInfo("ModelName", value)) {
    model_name_ = value;
  }
  if (GetDeviceInfo("FirmwareVersion", value)) {
    platform_version_ = std::move(value);
  }

  size_t major_pos = 0, minor_pos = 0;
  major_pos = platform_version_.find_first_of('.');
  if (major_pos != std::string::npos &&
      major_pos <= platform_version_.size() - 1) {
    minor_pos = platform_version_.find_first_of('.', major_pos + 1);
  }
  if (major_pos == std::string::npos || minor_pos == std::string::npos) {
    version_major_ = version_minor_ = version_dot_ = -1;
  } else {
    version_major_ =
        util::StrToIntWithDefault(platform_version_.substr(0, major_pos), 0);
    version_minor_ = util::StrToIntWithDefault(
        platform_version_.substr(major_pos + 1, minor_pos), 0);
    version_dot_ =
        util::StrToIntWithDefault(platform_version_.substr(minor_pos + 1), 0);
  }
}

void DeviceInfoImpl::GatherInfo() {
  InitDisplayInfo();
  InitPlatformInfo();
}
