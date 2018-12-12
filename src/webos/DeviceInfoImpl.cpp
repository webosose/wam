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

#include "DeviceInfoImpl.h"
#include "LogManager.h"

#include <lunaprefs.h>
#include <glib.h>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <string>

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
    QFile file("/var/luna/preferences/localeInfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QString jsonStr = file.readAll();
    file.close();

    QJsonDocument localeInfoDoc = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (localeInfoDoc.isNull()) {
        LOG_ERROR(MSGID_LOCALEINFO_READ_FAIL, 1, PMLOGKS("CONTENT", jsonStr.toStdString().c_str()), "");
        return;
    }

    QJsonObject localeInfo = localeInfoDoc.object().value("localeInfo").toObject();

    QString language(localeInfo.value("locales").toObject().value("UI").toString());
    QString localcountry(localeInfoDoc.object().value("country").toString());
    QString smartservicecountry(localeInfoDoc.object().value("smartServiceCountryCode3").toString());

    setSystemLanguage(language);
    setDeviceInfo("LocalCountry", localcountry);
    setDeviceInfo("SmartServiceCountry", smartservicecountry);
}

bool DeviceInfoImpl::getDeviceInfo(QString name, QString &value)
{
    return DeviceInfo::getDeviceInfo(name, value);
}

void DeviceInfoImpl::setDeviceInfo(QString name, QString value)
{
    DeviceInfo::setDeviceInfo(name, value);
}

bool DeviceInfoImpl::getInfoFromLunaPrefs(const char* key, std::string& value)
{
    char* str = 0;
    if (LP_ERR_NONE == LPSystemCopyStringValue(key, &str) && str) {
        value = str;
        g_free((gchar*) str);
        return true;
    }

    g_free((gchar*) str);
    value = "Unknown";
    return false;
}

void DeviceInfoImpl::initDisplayInfo()
{
    // Display information --------------------------------------------------------
    float m_screenDensity = 1.0f;
    int hardwareScreenWidth = 0;
    int hardwareScreenHeight = 0;

    QString hardwareScreenWidthStr;
    QString hardwareScreenHeightStr;
    if (getDeviceInfo("HardwareScreenWidth", hardwareScreenWidthStr) &&
        getDeviceInfo("HardwareScreenHeight", hardwareScreenHeightStr)) {
        hardwareScreenWidth = hardwareScreenWidthStr.toInt();
        hardwareScreenHeight = hardwareScreenHeightStr.toInt();
    } else {
        getDisplayWidth(hardwareScreenWidth);
        getDisplayHeight(hardwareScreenHeight);
    }

    m_screenWidth = (int) (hardwareScreenWidth / m_screenDensity);
    m_screenHeight = (int) (hardwareScreenHeight / m_screenDensity);

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

    QString value;     
     if (getDeviceInfo("ModelName", value))
         m_modelName = value.toStdString();
     if (getDeviceInfo("FirmwareVersion", value))
        m_platformVersion = value.toStdString();

    std::string platformVersion = m_platformVersion;

    size_t npos1 = 0, npos2 = 0;
    npos1 = platformVersion.find_first_of ('.');
    if (npos1 != std::string::npos && npos1 <= platformVersion.size() - 1)
        npos2 = platformVersion.find_first_of ('.', npos1 + 1);
    if (npos1 == std::string::npos || npos2 == std::string::npos)  {
        m_platformVersionMajor = m_platformVersionMinor = m_platformVersionDot = -1;
    }
    else {
        m_platformVersionMajor = atoi ((platformVersion.substr (0, npos1)).c_str());
        m_platformVersionMinor = atoi ((platformVersion.substr (npos1+1, npos2)).c_str());
        m_platformVersionDot = atoi ((platformVersion.substr (npos2+1)).c_str());
    }
}

void DeviceInfoImpl::gatherInfo()
{
    initDisplayInfo();
    initPlatformInfo();
}
