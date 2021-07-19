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

#include <json/json.h>

#include "TypeConverter.h"
#include "WebAppManager.h"


std::string PalmSystemBase::getDeviceInfo(const std::string& name) const
{
    std::string value;
    WebAppManager::instance()->getDeviceInfo(name, value);

    return value;
}

std::string PalmSystemBase::country() const
{
    std::string q_localcountry;
    std::string q_smartServiceCountry;

    WebAppManager::instance()->getDeviceInfo("LocalCountry", q_localcountry);
    WebAppManager::instance()->getDeviceInfo("SmartServiceCountry", q_smartServiceCountry);

    std::string country;
    Json::Value obj(Json::objectValue);
    obj["country"] = q_localcountry;
    obj["smartServiceCountry"] = q_smartServiceCountry;
    jsonToString(obj, country);
    return country;
}

std::string PalmSystemBase::locale() const
{
    std::string systemlocale;
    WebAppManager::instance()->getSystemLanguage(systemlocale);
    return systemlocale;
}

std::string PalmSystemBase::localeRegion() const
{
    return std::string("US");
}

std::string PalmSystemBase::phoneRegion() const
{
    return std::string();
}
