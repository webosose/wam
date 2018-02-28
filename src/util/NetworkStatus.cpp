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
#include "NetworkStatus.h"

NetworkStatus::NetworkStatus()
    : m_isInternetConnectionAvailable(false)
    , m_returnValue(false)
{
}

void NetworkStatus::fromJsonObject(const QJsonObject& object)
{
    m_returnValue = object["returnValue"].toBool();
    m_isInternetConnectionAvailable = object["isInternetConnectionAvailable"].toBool();
    if (m_returnValue) {
        if (!object["wired"].isUndefined()) {
            m_type = "wired";
            m_information.fromJsonObject(object["wired"].toObject());
        } else if (!object["wifi"].isUndefined()) {
            m_type = "wifi";
            m_information.fromJsonObject(object["wifi"].toObject());
        } else {
            m_type = "wifiDirect";
            m_information.fromJsonObject(object["wifiDirect"].toObject());
        }
    }

    time_t raw_time;
    time(&raw_time);
    m_savedDate = QString(ctime(&raw_time));
    m_savedDate = m_savedDate.trimmed();
}

void NetworkStatus::Information::fromJsonObject(const QJsonObject& info)
{
    m_netmask = info["netmask"].toString();
    m_dns1 = info["dns1"].toString();
    if (!info["dns2"].isUndefined())
        m_dns2 = info["dns2"].toString();
    m_ipAddress = info["ipAddress"].toString();
    m_method = info["method"].toString();
    m_state = info["state"].toString();
    m_gateway = info["gateway"].toString();
    m_interfaceName = info["interfaceName"].toString();
    m_onInternet = info["onInternet"].toString();
}
