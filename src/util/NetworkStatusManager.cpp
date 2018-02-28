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

#include "LogManager.h"
#include "NetworkStatusManager.h"

void NetworkStatusManager::updateNetworkStatus(const NetworkStatus& status)
{
    if (m_current.type() != status.type())
        appendLogList(status.type(), m_current.type(), status.type());

    checkInformationChange(status.information());
    if (m_logList.size() > 0) { // one more information was changed
        appendLogList(QString("date"), m_current.savedDate(), status.savedDate());
        printLog();
        m_current = status;
    }
}

void NetworkStatusManager::checkInformationChange(const NetworkStatus::Information& info)
{
    if (m_current.information().ipAddress() != info.ipAddress())
        appendLogList(QString("ipAddress"), m_current.information().ipAddress(), info.ipAddress());
    if (m_current.information().dns1() != info.dns1())
        appendLogList(QString("dns1"), m_current.information().dns1(), info.dns1());
    if (m_current.information().dns2() != info.dns2())
        appendLogList(QString("dns2"), m_current.information().dns2(), info.dns2());
    if (m_current.information().method() != info.method())
        appendLogList(QString("method"), m_current.information().method(), info.method());
    if (m_current.information().state() != info.state())
        appendLogList(QString("state"), m_current.information().state(), info.state());
    if (m_current.information().gateway() != info.gateway())
        appendLogList(QString("gateway"), m_current.information().gateway(), info.gateway());
    if (m_current.information().interfaceName() != info.interfaceName())
        appendLogList(QString("interfaceName"), m_current.information().interfaceName(), info.interfaceName());
    if (m_current.information().onInternet() != info.onInternet())
        appendLogList(QString("onInternet"), m_current.information().onInternet(), info.onInternet());
}

void NetworkStatusManager::appendLogList(const QString& key, const QString& previous, const QString& current)
{
    QPair<QString, QString> pair = qMakePair(previous, current);
    m_logList[key] = pair;
}

void NetworkStatusManager::printLog()
{
    QMapIterator<QString, QPair<QString, QString> > itr(m_logList);
    while (itr.hasNext()) {
        itr.next();
        LOG_INFO(MSGID_NETWORKSTATUS_INFO, 3, PMLOGKS("CHANGE", qPrintable(itr.key())),
            PMLOGKS("Previous", qPrintable(itr.value().first)),
            PMLOGKS("Current", qPrintable(itr.value().second)), "");
    }
    m_logList.clear();
}

