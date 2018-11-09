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

#include "NetworkStatus.h"

#include <json/json.h>
#include <time.h>


NetworkStatus::NetworkStatus()
    : m_isInternetConnectionAvailable(false)
    , m_returnValue(false)
{
}

void NetworkStatus::fromJsonObject(const Json::Value& object)
{
    m_returnValue = object["returnValue"].asBool();
    m_isInternetConnectionAvailable = object["isInternetConnectionAvailable"].asBool();
    if (m_returnValue) {
        if (object["wired"].isObject()) {
            m_type = "wired";
            m_information.fromJsonObject(object["wired"]);
        } else if (object["wifi"].isObject()) {
            m_type = "wifi";
            m_information.fromJsonObject(object["wifi"]);
        } else if (object["wifiDirect"].isObject()) {
            m_type = "wifiDirect";
            m_information.fromJsonObject(object["wifiDirect"]);
        }
    }

    time_t raw_time;
    time(&raw_time);
    m_savedDate = QString(ctime(&raw_time));
    m_savedDate = m_savedDate.trimmed();
}

void NetworkStatus::Information::fromJsonObject(const Json::Value& info)
{
    m_netmask = QString::fromStdString(info["netmask"].asString());
    m_dns1 = QString::fromStdString(info["dns1"].asString());
    if (info["dns2"].isString())
        m_dns2 = QString::fromStdString(info["dns2"].asString());
    m_ipAddress = QString::fromStdString(info["ipAddress"].asString());
    m_method = QString::fromStdString(info["method"].asString());
    m_state = QString::fromStdString(info["state"].asString());
    m_gateway = QString::fromStdString(info["gateway"].asString());
    m_interfaceName = QString::fromStdString(info["interfaceName"].asString());
    m_onInternet = QString::fromStdString(info["onInternet"].asString());
}
