// Copyright (c) 2012-2018 LG Electronics, Inc.
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

#include "palm_system_base.h"

#include <json/json.h>

#include "utils.h"
#include "web_app_manager.h"

std::string PalmSystemBase::GetDeviceInfo(const std::string& name) const {
  std::string value;
  WebAppManager::Instance()->GetDeviceInfo(name, value);

  return value;
}

std::string PalmSystemBase::Country() const {
  std::string local_country;
  std::string smart_service_country;

  WebAppManager::Instance()->GetDeviceInfo("LocalCountry", local_country);
  WebAppManager::Instance()->GetDeviceInfo("SmartServiceCountry",
                                           smart_service_country);

  Json::Value obj(Json::objectValue);
  obj["country"] = local_country;
  obj["smartServiceCountry"] = smart_service_country;
  std::string country = util::JsonToString(obj);
  return country;
}

std::string PalmSystemBase::Locale() const {
  std::string system_locale;
  WebAppManager::Instance()->GetSystemLanguage(system_locale);
  return system_locale;
}

std::string PalmSystemBase::LocaleRegion() const {
  return std::string("US");
}

std::string PalmSystemBase::PhoneRegion() const {
  return std::string();
}
