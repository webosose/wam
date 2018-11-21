/* @@@LICENSE
*
*      Copyright (c) 2015 LG Electronics, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */

#include "WebAppManagerServiceLunaImpl.h"
#include "LogManager.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

#define LS2_CALL(FUNC, SERVICE, PARAMS) call<WebAppManagerServiceLunaImpl, &WebAppManagerServiceLunaImpl::FUNC>(SERVICE, PARAMS, this)

WebAppManagerServiceLuna* WebAppManagerServiceLuna::instance()
{
    static WebAppManagerServiceLuna* service = new WebAppManagerServiceLunaImpl();
    return service;
}

void WebAppManagerServiceLunaImpl::systemServiceConnectCallback(QJsonObject reply)
{
    WebAppManagerServiceLuna::systemServiceConnectCallback(reply);

    if (reply.contains("connected")) {
        QJsonObject optionParams;
        optionParams["subscribe"] = true;
        optionParams["category"] = QStringLiteral("option");
        QStringList optionList;
        optionList << "country" << "smartServiceCountryCode3" << "audioGuidance" << "screenRotation";
        optionParams["keys"] = QJsonArray::fromStringList(optionList);
        LS2_CALL(getSystemOptionCallback, "luna://com.webos.settingsservice/getSystemSettings", optionParams);
    }
}

QJsonObject WebAppManagerServiceLunaImpl::setInspectorEnable(QJsonObject request)
{
    QJsonObject reply{{"returnValue", true}};
    return reply;
}

void WebAppManagerServiceLunaImpl::getSystemOptionCallback(QJsonObject reply)
{
    QJsonObject settings = reply.value("settings").toObject();
    //The settings is empty when service is crashed
    //The right value will be notified again when service is restarted
    if(settings.isEmpty()){
        QJsonDocument doc(reply);
        LOG_WARNING(MSGID_RECEIVED_INVALID_SETTINGS, 1,
            PMLOGKFV("MSG", "%s", qPrintable(QString(doc.toJson()))),
            "");
        return;
    }
    LOG_INFO(MSGID_SETTING_SERVICE, 0, "Notified from settingsservice/getSystemSettings");
    std::string country(settings.value("country").toString().toStdString());
    std::string smartServiceCountry(settings.value("smartServiceCountryCode3").toString().toStdString());
    std::string audioGuidance(settings.value("audioGuidance").toString().toStdString());
    std::string screenRotation(settings.value("screenRotation").toString().toStdString());

    LOG_INFO(MSGID_SETTING_SERVICE, 1,
        PMLOGKS("BroadcastCountry", country.empty() ? "Empty" : country.c_str()), "");
    LOG_INFO(MSGID_SETTING_SERVICE, 1,
            PMLOGKS("SmartServiceCountry", smartServiceCountry.empty() ? "Empty" : smartServiceCountry.c_str()), "");
    LOG_INFO(MSGID_SETTING_SERVICE, 1,
            PMLOGKS("AudioGuidance", audioGuidance.empty() ? "Empty" : audioGuidance.c_str()), "");
    LOG_INFO(MSGID_SETTING_SERVICE, 1,
            PMLOGKS("ScreenRotation", screenRotation.empty() ? "Empty" : screenRotation.c_str()), "");

    if (!country.empty())
        WebAppManagerService::setDeviceInfo("LocalCountry", country);
    if (!smartServiceCountry.empty())
        WebAppManagerService::setDeviceInfo("SmartServiceCountry", smartServiceCountry);
    if (!audioGuidance.empty())
        WebAppManagerService::setAccessibilityEnabled(audioGuidance == "on");
    if (!screenRotation.emtpy())
        WebAppManagerService::setDeviceInfo("ScreenRotation", screenRotation);
}
