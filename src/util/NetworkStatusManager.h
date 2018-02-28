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

#ifndef NETWORKSTATUS_MANAGER_H
#define NETWORKSTATUS_MANAGER_H

#include "NetworkStatus.h"

#include <QMap>
#include <QPair>
#include <QString>

class NetworkStatusManager {
public:
    void updateNetworkStatus(const NetworkStatus& status);
    void checkInformationChange(const NetworkStatus::Information& information);
    void appendLogList(const QString& key, const QString& previous, const QString& current);
    void printLog();

private:
    NetworkStatus m_current;
    QMap<QString, QPair<QString, QString>> m_logList;
};

#endif //#ifndef NETWORKSTATUS_MANAGER_H
