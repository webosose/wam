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

#include <QJsonObject>
#include <QString>

class NetworkStatus {
public:
    NetworkStatus();

    class Information {
    public:
        void fromJsonObject(const QJsonObject&);
        QString netmask() const { return m_netmask; }
        QString dns1() const { return m_dns1; }
        QString dns2() const { return m_dns2; }
        QString ipAddress() const { return m_ipAddress; }
        QString method() const { return m_method; }
        QString state() const { return m_state; }
        QString gateway() const { return m_gateway; }
        QString interfaceName() const { return m_interfaceName; }
        QString onInternet() const { return m_onInternet; }

    private:
        QString m_netmask;
        QString m_dns1;
        QString m_dns2;
        QString m_ipAddress;
        QString m_method;
        QString m_state;
        QString m_gateway;
        QString m_interfaceName;
        QString m_onInternet;
    };

    void fromJsonObject(const QJsonObject&);
    QString type() const { return m_type; }
    Information information() const { return m_information; }
    QString savedDate() const { return m_savedDate; }
    bool isInternetConnectionAvailable() { return m_isInternetConnectionAvailable; }

private:
    QString m_type;
    Information m_information;
    bool m_isInternetConnectionAvailable;
    bool m_returnValue;
    QString m_savedDate;
};
#endif
