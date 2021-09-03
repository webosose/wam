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

#include "network_status.h"

#include <time.h>

#include <json/json.h>

#include "utils.h"

NetworkStatus::NetworkStatus()
    : is_internet_connection_available_(false), return_value_(false) {}

void NetworkStatus::FromJsonObject(const Json::Value& object) {
  if (!object.isObject())
    return;
  return_value_ = object["returnValue"].asBool();
  is_internet_connection_available_ =
      object["isInternetConnectionAvailable"].asBool();
  if (return_value_) {
    if (object["wired"].isObject()) {
      type_ = "wired";
      information_.FromJsonObject(object["wired"]);
    } else if (object["wifi"].isObject()) {
      type_ = "wifi";
      information_.FromJsonObject(object["wifi"]);
    } else {
      type_ = "wifiDirect";
      information_.FromJsonObject(object["wifiDirect"]);
    }
  }

  time_t raw_time;
  time(&raw_time);
  saved_date_ = util::TrimString(ctime(&raw_time));
}

void NetworkStatus::Information::FromJsonObject(const Json::Value& info) {
  if (!info.isObject())
    return;
  netmask_ = info["netmask"].asString();
  dns1_ = info["dns1"].asString();
  if (info["dns2"].isString())
    dns2_ = info["dns2"].asString();
  ip_address_ = info["ipAddress"].asString();
  method_ = info["method"].asString();
  state_ = info["state"].asString();
  gateway_ = info["gateway"].asString();
  interface_name_ = info["interfaceName"].asString();
  on_internet_ = info["onInternet"].asString();
}
