// Copyright (c) 2014-2021 LG Electronics, Inc.
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

#include "DeviceInfoImpl.h"

#include <string>

#include <glib.h>
#include <json/value.h>
#include <lunaprefs.h>

#include "LogManager.h"
#include "Utils.h"

DeviceInfoImpl::DeviceInfoImpl()
    : m_screenWidth(0)
    , m_screenHeight(0)
    , m_screenDensity(1.0f)
    , m_modelName("webOS.Open")
    , m_platformVersion("00.00.00")
    , m_platformVersionMajor(0)
    , m_platformVersionMinor(0)
    , m_platformVersionDot(0)
    , m_3DSupport(false)
    , m_hardwareVersion("0x00000001")
    , m_firmwareVersion("00.00.01")
{
}

void DeviceInfoImpl::initialize()
{
    const std::string& jsonString = util::readFile("/var/luna/preferences/localeInfo");
    if (jsonString.empty()) {
        return;
    }

    Json::Value localeJson = util::stringToJson(jsonString);
    if (!localeJson.isObject() || localeJson.empty()
       || !localeJson["localeInfo"].isObject()
       || !localeJson["localeInfo"]["locales"].isString()
       || !localeJson["localeInfo"]["country"].isString()
       || !localeJson["localeInfo"]["smartServiceCountryCode3"].isString()) {
        LOG_ERROR(MSGID_LOCALEINFO_READ_FAIL, 1, PMLOGKS("CONTENT", jsonString.c_str()), "");
        return;
    }

    Json::Value localeInfo = localeJson["localeInfo"];

    std::string language(localeInfo["locales"].asString());
    std::string localcountry(localeInfo["country"].asString());
    std::string smartservicecountry(localeInfo["smartServiceCountryCode3"].asString());

    setSystemLanguage(language.c_str());
    setDeviceInfo("LocalCountry", localcountry.c_str());
    setDeviceInfo("SmartServiceCountry", smartservicecountry.c_str());
}

bool DeviceInfoImpl::getInfoFromLunaPrefs(const char* key, std::string& value)
{
    char* str = 0;
    if (LP_ERR_NONE == LPSystemCopyStringValue(key, &str) && str) {
        value = str;
        g_free((gchar*)str);
        return true;
    }

    g_free((gchar*)str);
    value = "Unknown";
    return false;
}

void DeviceInfoImpl::initDisplayInfo()
{
    // Display information --------------------------------------------------------
    float m_screenDensity = 1.0f;
    int hardwareScreenWidth = 0;
    int hardwareScreenHeight = 0;

    std::string hardwareScreenWidthStr;
    std::string hardwareScreenHeightStr;
    if (getDeviceInfo("HardwareScreenWidth", hardwareScreenWidthStr) && getDeviceInfo("HardwareScreenHeight", hardwareScreenHeightStr)) {
        hardwareScreenWidth = util::strToIntWithDefault(hardwareScreenWidthStr, 0);
        hardwareScreenHeight = util::strToIntWithDefault(hardwareScreenHeightStr, 0);
    } else {
        getDisplayWidth(hardwareScreenWidth);
        getDisplayHeight(hardwareScreenHeight);
    }

    m_screenWidth = (int)(hardwareScreenWidth / m_screenDensity);
    m_screenHeight = (int)(hardwareScreenHeight / m_screenDensity);
}

void DeviceInfoImpl::initPlatformInfo()
{
    // normally like this info
    /*
       "modelName": "WEBOS1",
       "platformVersion": "00.00.00",
       "platformVersionDot": 00,
       "platformVersionMajor": 00,
       "platformVersionMinor": 00,
    */

    std::string value;
    if (getDeviceInfo("ModelName", value))
        m_modelName = value;
    if (getDeviceInfo("FirmwareVersion", value))
        m_platformVersion = value;

    std::string platformVersion = m_platformVersion;

    size_t npos1 = 0, npos2 = 0;
    npos1 = platformVersion.find_first_of('.');
    if (npos1 != std::string::npos && npos1 <= platformVersion.size() - 1)
        npos2 = platformVersion.find_first_of('.', npos1 + 1);
    if (npos1 == std::string::npos || npos2 == std::string::npos) {
        m_platformVersionMajor = m_platformVersionMinor = m_platformVersionDot = -1;
    } else {
        m_platformVersionMajor = util::strToIntWithDefault(platformVersion.substr(0, npos1), 0);
        m_platformVersionMinor = util::strToIntWithDefault(platformVersion.substr(npos1 + 1, npos2), 0);
        m_platformVersionDot = util::strToIntWithDefault(platformVersion.substr(npos2 + 1), 0);
    }
}

void DeviceInfoImpl::gatherInfo()
{
    initDisplayInfo();
    initPlatformInfo();
}
