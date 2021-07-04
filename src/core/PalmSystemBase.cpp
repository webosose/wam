// Copyright (c) 2012-2018 LG Electronics, Inc.
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

#include "PalmSystemBase.h"

#include <string>

#include <QByteArray>

#include <json/json.h>

#include "TypeConverter.h"
#include "WebAppManager.h"


QString PalmSystemBase::getDeviceInfo(QString name)
{
    QString value;
    WebAppManager::instance()->getDeviceInfo(name, value);

    return value;
}

QString PalmSystemBase::country() const
{
    QString q_localcountry;
    QString q_smartServiceCountry;

    WebAppManager::instance()->getDeviceInfo("LocalCountry", q_localcountry);
    WebAppManager::instance()->getDeviceInfo("SmartServiceCountry", q_smartServiceCountry);

    std::string country;
    Json::Value obj(Json::objectValue);
    obj["country"] = q_localcountry.toStdString();
    obj["smartServiceCountry"] = q_smartServiceCountry.toStdString();
    jsonToString(obj, country);
    return QString::fromStdString(country);
}

QString PalmSystemBase::locale() const
{
    QString systemlocale;
    WebAppManager::instance()->getSystemLanguage(systemlocale);
    return systemlocale;
}

QString PalmSystemBase::localeRegion() const
{
    return QString("US");
}

QString PalmSystemBase::phoneRegion() const
{
    return QString("");
}
