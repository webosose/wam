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

#include <time.h>

#include <json/json.h>

#include "NetworkStatus.h"
#include "Utils.h"

NetworkStatus::NetworkStatus()
    : m_isInternetConnectionAvailable(false)
    , m_returnValue(false)
{
}

void NetworkStatus::fromJsonObject(const Json::Value& object)
{
    if (!object.isObject())
        return;
    m_returnValue = object["returnValue"].asBool();
    m_isInternetConnectionAvailable = object["isInternetConnectionAvailable"].asBool();
    if (m_returnValue) {
        if (object["wired"].isObject()) {
            m_type = "wired";
            m_information.fromJsonObject(object["wired"]);
        } else if (object["wifi"].isObject()) {
            m_type = "wifi";
            m_information.fromJsonObject(object["wifi"]);
        } else {
            m_type = "wifiDirect";
            m_information.fromJsonObject(object["wifiDirect"]);
        }
    }

    time_t raw_time;
    time(&raw_time);
    m_savedDate = util::trimString(ctime(&raw_time));
}

void NetworkStatus::Information::fromJsonObject(const Json::Value& info)
{
    if (!info.isObject())
        return;
    m_netmask = info["netmask"].asString();
    m_dns1 = info["dns1"].asString();
    if (info["dns2"].isString())
        m_dns2 = info["dns2"].asString();
    m_ipAddress = info["ipAddress"].asString();
    m_method = info["method"].asString();
    m_state = info["state"].asString();
    m_gateway = info["gateway"].asString();
    m_interfaceName = info["interfaceName"].asString();
    m_onInternet = info["onInternet"].asString();
}
