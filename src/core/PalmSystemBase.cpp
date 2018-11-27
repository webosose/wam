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

#include <sstream>
#include <boost/filesystem.hpp>

#include "JsonHelper.h"
#include "LogManager.h"
#include "WebAppManager.h"
#include "WebAppManagerUtils.h"

std::string PalmSystemBase::getDeviceInfo(const std::string& name)
{
    std::string value;
    WebAppManager::instance()->getDeviceInfo(name, value);

    return value;
}

std::string PalmSystemBase::country() const
{
    std::string localcountry;
    std::string smartServiceCountry;

    WebAppManager::instance()->getDeviceInfo("LocalCountry", localcountry);
    WebAppManager::instance()->getDeviceInfo("SmartServiceCountry", smartServiceCountry);

    std::string json;
    Json::Value obj(Json::objectValue);
    obj["country"] = localcountry;
    obj["smartServiceCountry"] = smartServiceCountry;
    dumpJsonToString(obj, json);
    return json;
}

std::string PalmSystemBase::locale() const
{
    std::string systemlocale;
    WebAppManager::instance()->getSystemLanguage(systemlocale);
    return systemlocale;
}

std::string PalmSystemBase::localeRegion() const
{
    return "US";
}

std::string PalmSystemBase::phoneRegion() const
{
    return {};
}

void PalmSystemBase::setContainerAppReady(const std::string& appId)
{
    if (appId == WebAppManager::instance()->getContainerAppId())
        WebAppManager::instance()->setContainerAppReady(true);
}
