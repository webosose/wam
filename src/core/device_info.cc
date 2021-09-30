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

#include "device_info.h"

#include "utils.h"

bool DeviceInfo::GetDisplayWidth(int& value) const {
  bool ret = false;
  std::string value_str;

  ret = GetDeviceInfo("DisplayWidth", value_str);
  if (ret)
    ret = util::StrToInt(value_str, value);

  return ret;
}

void DeviceInfo::SetDisplayWidth(int value) {
  if(!device_info_.emplace("DisplayWidth", std::to_string(value)).second)
    device_info_["DisplayWidth"] = std::to_string(value);
}

bool DeviceInfo::GetDisplayHeight(int& value) const {
  bool ret = false;
  std::string value_str;

  ret = GetDeviceInfo("DisplayHeight", value_str);
  if (ret)
    ret = util::StrToInt(value_str, value);

  return ret;
}

void DeviceInfo::SetDisplayHeight(int value) {
  if(!device_info_.emplace("DisplayHeight", std::to_string(value)).second)
    device_info_["DisplayHeight"] = std::to_string(value);
}

bool DeviceInfo::GetSystemLanguage(std::string& value) const {
  return GetDeviceInfo("SystemLanguage", value);
}

void DeviceInfo::SetSystemLanguage(const std::string& value) {
  if(!device_info_.emplace("SystemLanguage", value).second)
    device_info_["SystemLanguage"] = value;
}

bool DeviceInfo::GetDeviceInfo(const std::string& name,
                               std::string& value) const {
  auto info = device_info_.find(name);
  if (info != device_info_.end()) {
    value = info->second;
    return true;
  }

  return false;
}

void DeviceInfo::SetDeviceInfo(const std::string& name,
                               const std::string& value) {
  if(!device_info_.emplace(name, value).second)
    device_info_[name] = value;
}
