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

#ifndef NETWORKSTATUS_H
#define NETWORKSTATUS_H

#include <string>

namespace Json {
class Value;
};

//TODO: remove header when QT less implementation will be completed.
#include <QJsonObject>

class NetworkStatus {
public:
    NetworkStatus();

    class Information {
    public:
        void fromJsonObject(const Json::Value& info);
        std::string netmask() const { return m_netmask; }
        std::string dns1() const { return m_dns1; }
        std::string dns2() const { return m_dns2; }
        std::string ipAddress() const { return m_ipAddress; }
        std::string method() const { return m_method; }
        std::string state() const { return m_state; }
        std::string gateway() const { return m_gateway; }
        std::string interfaceName() const { return m_interfaceName; }
        std::string onInternet() const { return m_onInternet; }

    private:
        std::string m_netmask;
        std::string m_dns1;
        std::string m_dns2;
        std::string m_ipAddress;
        std::string m_method;
        std::string m_state;
        std::string m_gateway;
        std::string m_interfaceName;
        std::string m_onInternet;
    };

    //TODO: remove this method when QT less implementation will be completed.
    void fromJsonObject(const QJsonObject&);
    void fromJsonObject(const Json::Value& object);
    std::string type() const { return m_type; }
    Information information() const { return m_information; }
    std::string savedDate() const { return m_savedDate; }
    bool isInternetConnectionAvailable() { return m_isInternetConnectionAvailable; }

private:
    std::string m_type;
    Information m_information;
    bool m_isInternetConnectionAvailable;
    bool m_returnValue;
    std::string m_savedDate;
};
#endif
