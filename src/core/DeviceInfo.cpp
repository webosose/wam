// Copyright (c) 2014-2018 LG Electronics, Inc.
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

#include "DeviceInfo.h"

bool DeviceInfo::getDisplayWidth(int &value)
{
    bool ret = false;
    QString valueStr;

    ret = getDeviceInfo("DisplayWidth", valueStr);
    value = valueStr.toInt();

    return ret;
}

void DeviceInfo::setDisplayWidth(int value)
{
    m_deviceInfo.insert("DisplayWidth", QString::number(value));
}

bool DeviceInfo::getDisplayHeight(int &value)
{
    bool ret = false;
    QString valueStr;

    ret = getDeviceInfo("DisplayHeight", valueStr);
    value = valueStr.toInt();

    return ret;
}

void DeviceInfo::setDisplayHeight(int value)
{
    m_deviceInfo.insert("DisplayHeight", QString::number(value));
}

bool DeviceInfo::getSystemLanguage(QString &value)
{
    return getDeviceInfo("SystemLanguage", value);
}

void DeviceInfo::setSystemLanguage(QString value)
{
    m_deviceInfo.insert("SystemLanguage", value);
}

bool DeviceInfo::getDeviceInfo(QString name, QString &value)
{
    if (m_deviceInfo.contains(name)) {
        value = m_deviceInfo.value(name);
        return true;
    }

    return false;
}

void DeviceInfo::setDeviceInfo(QString name, QString value)
{
    m_deviceInfo.insert(name, value);
}
