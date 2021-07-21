// Copyright (c) 2015-2021 LG Electronics, Inc.
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

#include "WebAppManagerServiceLunaImpl.h"

#include "json/json.h"

#include "LogManager.h"
#include "Utils.h"

#define LS2_CALL(FUNC, SERVICE, PARAMS) call<WebAppManagerServiceLunaImpl, &WebAppManagerServiceLunaImpl::FUNC>(SERVICE, PARAMS, this)

WebAppManagerServiceLuna* WebAppManagerServiceLuna::instance()
{
    static WebAppManagerServiceLuna* service = new WebAppManagerServiceLunaImpl();
    return service;
}

void WebAppManagerServiceLunaImpl::systemServiceConnectCallback(const Json::Value& reply)
{
    WebAppManagerServiceLuna::systemServiceConnectCallback(reply);

    if (reply.isObject() && reply.isMember("connected")) {
        Json::Value optionParams;
        optionParams["subscribe"] = true;
        optionParams["category"] = "option";
        Json::Value optionList;
        optionList.append("country");
        optionList.append("smartServiceCountryCode3");
        optionList.append("audioGuidance");
        optionList.append("screenRotation");
        optionParams["keys"] = optionList;
        LS2_CALL(getSystemOptionCallback, "luna://com.webos.settingsservice/getSystemSettings", optionParams);
    }
}

Json::Value WebAppManagerServiceLunaImpl::setInspectorEnable(const Json::Value& request)
{
    return util::stringToJson(R"({"returnValue": true})");
}

void WebAppManagerServiceLunaImpl::getSystemOptionCallback(const Json::Value& replyJson)
{
    Json::Value settings = replyJson["settings"];
    //The settings is empty when service is crashed
    //The right value will be notified again when service is restarted
    if (!replyJson.isObject() || !replyJson["settings"].isObject() || replyJson["settings"].empty()) {
        LOG_WARNING(MSGID_RECEIVED_INVALID_SETTINGS, 1,
            PMLOGKFV("MSG", "%s", util::jsonToString(replyJson).c_str()),
            "");
        return;
    }
    LOG_INFO(MSGID_SETTING_SERVICE, 0, "Notified from settingsservice/getSystemSettings");

    std::string country = replyJson["settings"]["country"].isString() ?
        settings["country"].asString() : "";
    std::string smartServiceCountry = replyJson["settings"]["country"].isString() ?
        settings["smartServiceCountryCode3"].asString() : "";
    std::string audioGuidance = replyJson["settings"]["country"].isString() ?
        settings["audioGuidance"].asString() : "";
    std::string screenRotation = replyJson["settings"]["country"].isString() ?
        settings["screenRotation"].asString() : "";

    LOG_INFO(MSGID_SETTING_SERVICE, 1,
        PMLOGKS("BroadcastCountry", country.empty() ? "Empty" : country.c_str()), "");
    LOG_INFO(MSGID_SETTING_SERVICE, 1,
        PMLOGKS("SmartServiceCountry", smartServiceCountry.empty() ? "Empty" : smartServiceCountry.c_str()), "");
    LOG_INFO(MSGID_SETTING_SERVICE, 1,
        PMLOGKS("AudioGuidance", audioGuidance.empty() ? "Empty" : audioGuidance.c_str()), "");
    LOG_INFO(MSGID_SETTING_SERVICE, 1,
        PMLOGKS("ScreenRotation", screenRotation.empty() ? "Empty" : screenRotation.c_str()), "");
    if (!country.empty())
        WebAppManagerService::setDeviceInfo("LocalCountry", country.c_str());
    if (!smartServiceCountry.empty())
        WebAppManagerService::setDeviceInfo("SmartServiceCountry", smartServiceCountry.c_str());
    if (!audioGuidance.empty())
        WebAppManagerService::setAccessibilityEnabled(audioGuidance == "on");
    if (!screenRotation.empty())
        WebAppManagerService::setDeviceInfo("ScreenRotation", screenRotation.c_str());
}
