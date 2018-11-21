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
    std::string valueStr;

    ret = getDeviceInfo("DisplayWidth", valueStr);
    value = std::stoi(valueStr);

    return ret;
}

void DeviceInfo::setDisplayWidth(int value)
{
    m_deviceInfo.emplace("DisplayWidth", std::to_string(value));
}

bool DeviceInfo::getDisplayHeight(int &value)
{
    bool ret = false;
    std::string valueStr;

    ret = getDeviceInfo("DisplayHeight", valueStr);
    value = std::stoi(valueStr);

    return ret;
}

void DeviceInfo::setDisplayHeight(int value)
{
    m_deviceInfo.emplace("DisplayHeight", std::to_string(value));
}

bool DeviceInfo::getSystemLanguage(std::string &value)
{
    return getDeviceInfo("SystemLanguage", value);
}

void DeviceInfo::setSystemLanguage(const std::string &value)
{
    m_deviceInfo.emplace("SystemLanguage", value);
}

bool DeviceInfo::getDeviceInfo(const std::string& name, std::string& value)
{
    auto v = m_deviceInfo.find(name);
    if (v != m_deviceInfo.end()) {
        value = v->second;
        return true;
    }
    return false;
}

void DeviceInfo::setDeviceInfo(const std::string& name, const std::string& value)
{
    m_deviceInfo.emplace(name, value);
}
