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

#include "PalmSystemBlink.h"

#include <sstream>
#include <QString>

#include "JsonHelper.h"
#include "LogManager.h"
#include "WebAppBase.h"
#include "WebAppWayland.h"
#include "WebPageBlink.h"

PalmSystemBlink::PalmSystemBlink(WebAppBase* app)
    : PalmSystemWebOS(app)
{
}

std::string PalmSystemBlink::handleBrowserControlMessage(const std::string& message, const std::vector<std::string>& params)
{
    if (message == "initialize") {
        std::string json;
        dumpJsonToString(initialize(), json);
        return json;
    } else if (message == "country") {
        return country();
    } else if (message == "locale") {
        return locale();
    } else if (message == "localeRegion") {
        return localeRegion();
    } else if (message == "isMinimal") {
        return isMinimal() ? "true" : "false";
    } else if (message == "screenOrientation") {
        return screenOrientation();
    } else if (message == "currentCountryGroup") {
        return getDeviceInfo("CountryGroup");
    } else if (message == "stageReady") {
        stageReady();
    } else if (message == "containerReady") {
        setContainerAppReady(m_app->appId());
    } else if (message == "activate") {
        LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", m_app->appId().c_str()), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.activate()");
        activate();
    } else if (message == "deactivate") {
        LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", m_app->appId().c_str()), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.deactivate()");
        deactivate();
    } else if (message == "isActivated") {
        return isActivated() ? "true" : "false";
    } else if (message == "isKeyboardVisible") {
        return isKeyboardVisible() ? "true" : "false";
    } else if (message == "getIdentifier" || message == "identifier") {
        return identifier();
    } else if (message == "launchParams") {
        LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", m_app->appId().c_str()), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.launchParams Updated by app; %s", params[0].c_str());
        updateLaunchParams(params[0]);
    } else if (message == "screenOrientation") {
        return screenOrientation();
    } else if (message == "keepAlive") {
        if (params.size() > 0)
            setKeepAlive(params[0] == "true");
    } else if (message == "PmLogInfoWithClock") {
#ifdef HAS_PMLOG
        if (params.size() == 3) {
            auto p0 = QString::fromStdString(params[0]);
            auto p1 = QString::fromStdString(params[1]);
            auto p2 = QString::fromStdString(params[2]);
            pmLogInfoWithClock(p0, p1, p2); // FIXME: PmLog: qvariant-less
        }
#endif
    } else if (message == "PmLogString") {
#ifdef HAS_PMLOG
        if (params.size() > 3) {
            auto level = static_cast<PmLogLevel>(std::stoi(params[0]));
            auto p1 = QString::fromStdString(params[1]);
            auto p2 = QString::fromStdString(params[2]);
            auto p3 = QString::fromStdString(params[3]);
            pmLogString(level, p1, p2, p3); // FIXME: PmLog: qstr2stdstr
        }
#endif
    } else if (message == "setWindowProperty") {
        if (params.size() > 1) {
            LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", m_app->appId().c_str()), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()),
                "PalmSystem.window.setProperty('%s', '%s')", params[0].c_str(), params[1].c_str());
            m_app->setWindowProperty(params[0], params[1]);
        }
    } else if (message == "platformBack") {
        LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", m_app->appId().c_str()), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.platformBack()");
        m_app->platformBack();
    } else if (message == "setCursor") {
        auto v1 = params[0];
        auto v2 = std::stoi(params[1]);
        auto v3 = std::stoi(params[2]);
        m_app->setCursor(v1, v2, v3);
    } else if (message == "setInputRegion") {
        std::stringstream ss;
        for (const auto &param : params)
            ss << param;
        setInputRegion(ss.str());
    } else if (message == "setKeyMask") {
        std::stringstream ss;
        for (const auto &param : params)
            ss << param;
        setGroupClientEnvironment(KeyMask, ss.str());
    } else if (message == "focusOwner") {
        setGroupClientEnvironment(FocusOwner, NULL);
    } else if (message == "focusLayer") {
        setGroupClientEnvironment(FocusLayer, NULL);
    } else if (message == "hide") {
        hide();
    } else if (message == "setLoadErrorPolicy") {
        if (params.size() > 0) {
            LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", m_app->appId().c_str()), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.setLoadErrorPolicy(%s)", params[0].c_str());
            setLoadErrorPolicy(params[0]);
        }
    } else if (message == "onCloseNotify") {
        if (params.size() > 0) {
            LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", m_app->appId().c_str()), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.onCloseNotify(%s)", params[0].c_str());
            onCloseNotify(params[0]);
        }
    } else if (message == "cursorVisibility") {
        return cursorVisibility() ? "true" : "false";
    } else if (message == "serviceCall") {
        if (m_app->page()->isClosing()) {
          LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", m_app->appId().c_str()), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.serviceCall(%s, %s)", params[0].c_str(), params[1].c_str());
          m_app->serviceCall(params[0], params[1], m_app->appId());
        } else {
            LOG_WARNING(MSGID_SERVICE_CALL_FAIL, 2, PMLOGKS("APP_ID", m_app->appId().c_str()),
              PMLOGKS("URL", params[0].c_str()), "Page is NOT in closing");
        }
    }

    return {};
}

void PalmSystemBlink::setCountry()
{
    if (m_initialized)
        static_cast<WebPageBlink*>(m_app->page())->updateExtensionData("country", country());
}

void PalmSystemBlink::setLaunchParams(const std::string& params)
{
    PalmSystemWebOS::setLaunchParams(params);
    static_cast<WebPageBlink*>(m_app->page())->updateExtensionData("launchParams", launchParams());
}

void PalmSystemBlink::setLocale(const std::string& params)
{
    if (m_initialized)
        static_cast<WebPageBlink*>(m_app->page())->updateExtensionData("locale", params);
}

std::string PalmSystemBlink::identifier() const
{
    if (!m_app->page())
        return {};
    return static_cast<WebPageBlink*>(m_app->page())->getIdentifier();
}

void PalmSystemBlink::setLoadErrorPolicy(const std::string& params)
{
    static_cast<WebPageBlink*>(m_app->page())->setLoadErrorPolicy(params);
}

std::string PalmSystemBlink::trustLevel() const
{
    return static_cast<WebPageBlink*>(m_app->page())->trustLevel();
}

void PalmSystemBlink::onCloseNotify(const std::string& params)
{
    if (params == "didSetOnCloseCallback")
        static_cast<WebPageBlink*>(m_app->page())->setHasOnCloseCallback(true);
    else if (params == "didClearOnCloseCallback")
        static_cast<WebPageBlink*>(m_app->page())->setHasOnCloseCallback(false);
    else if (params == "didRunOnCloseCallback")
        static_cast<WebPageBlink*>(m_app->page())->didRunCloseCallback();
}

double PalmSystemBlink::devicePixelRatio()
{
    return static_cast<WebPageBlink*>(m_app->page())->devicePixelRatio();
}

Json::Value PalmSystemBlink::initialize()
{
    Json::Value data = PalmSystemWebOS::initialize();
    data["devicePixelRatio"] = devicePixelRatio();
    data["trustLevel"] = trustLevel();
    return std::move(data);
}

