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

#include "LogManager.h"
#include "PalmSystemBlink.h"
#include "WebAppBase.h"
#include "WebAppWayland.h"
#include "WebPageBlink.h"

#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QDataStream>

PalmSystemBlink::PalmSystemBlink(WebAppBase* app)
    : PalmSystemWebOS(app)
{
}

QString PalmSystemBlink::handleBrowserControlMessage(const QString& message, const QStringList& params)
{
    if (message == "initialize") {
        return initialize().toJson();
    } else if (message == "country") {
        return country();
    } else if (message == "locale") {
        return locale();
    } else if (message == "localeRegion") {
        return localeRegion();
    } else if (message == "isMinimal") {
        if(isMinimal())
            return QString("true");
        else
            return QString("false");
    } else if (message == "identifier") {
        return identifier();
    } else if (message == "screenOrientation") {
        return screenOrientation();
    } else if (message == "currentCountryGroup") {
        return getDeviceInfo("CountryGroup");
    } else if (message == "stageReady") {
        stageReady();
    } else if (message == "containerReady") {
        setContainerAppReady(m_app->appId());
    } else if (message == "activate") {
        LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", qPrintable(m_app->appId())), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.activate()");
        activate();
    } else if (message == "deactivate") {
        LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", qPrintable(m_app->appId())), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.deactivate()");
        deactivate();
    } else if (message == "isActivated") {
        if(isActivated())
            return QString("true");
        else
            return QString("false");
    } else if (message == "isKeyboardVisible") {
        if(isKeyboardVisible())
            return QString("true");
        else
            return QString("false");
    } else if (message == "getIdentifier" || message == "identifier") {
        return QString(identifier().toUtf8());
    } else if (message == "launchParams") {
        LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", qPrintable(m_app->appId())), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.launchParams Updated by app; %s", qPrintable(params[0]));
        updateLaunchParams(params[0]);
    } else if (message == "screenOrientation") {
        QByteArray res;
        QDataStream out(res);
        out << QVariant(screenOrientation());
        return QString(res);
    } else if (message == "keepAlive") {
        if (params.size() > 0)
            setKeepAlive(params[0] == "true");
    } else if (message == "PmLogInfoWithClock") {
        if (params.size() == 3)
            pmLogInfoWithClock(params[0], params[1], params[2]);
    } else if (message == "PmLogString") {
        if (params.size() > 3)
            pmLogString(static_cast<PmLogLevel>(params[0].toInt()), params[1], params[2], params[3]);
    } else if (message == "setWindowProperty") {
        if (params.size() > 1) {
            LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", qPrintable(m_app->appId())), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()),
                "PalmSystem.window.setProperty('%s', '%s')", qPrintable(params[0]), qPrintable(params[1]));
            m_app->setWindowProperty(params[0], params[1]);
        }
    } else if (message == "platformBack") {
        LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", qPrintable(m_app->appId())), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.platformBack()");
        m_app->platformBack();
    } else if (message == "setCursor") {
        QVariant v1, v2, v3;
        v1 = params.at(0);
        v2 = params.at(1);
        v3 = params.at(2);
        m_app->setCursor(v1.toString(), v2.toInt(), v3.toInt());
    } else if (message == "setInputRegion") {
        QByteArray data;
        for (int i = 0; i < params.count(); i++) {
            data.append(params.at(i));
        }
        setInputRegion(data);
    } else if (message == "setKeyMask") {
        QByteArray data;
        for (int i = 0; i < params.count(); i++) {
            data.append(params.at(i));
        }
        setGroupClientEnvironment(KeyMask, data);
    } else if (message == "focusOwner") {
        setGroupClientEnvironment(FocusOwner, NULL);
    } else if (message == "focusLayer") {
        setGroupClientEnvironment(FocusLayer, NULL);
    } else if (message == "hide") {
        hide();
    } else if (message == "setLoadErrorPolicy") {
        if (params.size() > 0) {
            LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", qPrintable(m_app->appId())), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.setLoadErrorPolicy(%s)", qPrintable(params[0]));
            setLoadErrorPolicy(params[0]);
        }
    } else if (message == "onCloseNotify") {
        if (params.size() > 0) {
            LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", qPrintable(m_app->appId())), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.onCloseNotify(%s)", qPrintable(params[0]));
            onCloseNotify(params[0]);
        }
    } else if (message == "cursorVisibility") {
        return cursorVisibility() ? "true" : "false";
    } else if (message == "serviceCall") {
        if (m_app->page()->isClosing()) {
          LOG_INFO(MSGID_PALMSYSTEM, 2, PMLOGKS("APP_ID", qPrintable(m_app->appId())), PMLOGKFV("PID", "%d", m_app->page()->getWebProcessPID()), "PalmSystem.serviceCall(%s, %s)", qPrintable(params[0]), qPrintable(params[1]));
          m_app->serviceCall(params[0], params[1], m_app->appId());
        } else {
            LOG_WARNING(MSGID_SERVICE_CALL_FAIL, 2, PMLOGKS("APP_ID", qPrintable(m_app->appId())),
              PMLOGKS("URL", qPrintable(params[0])), "Page is NOT in closing");
        }
    }

    return QString();
}

void PalmSystemBlink::setCountry()
{
    if (m_initialized)
        static_cast<WebPageBlink*>(m_app->page())->updateExtensionData(QStringLiteral("country"), country());
}

void PalmSystemBlink::setLaunchParams(const QString& params)
{
    PalmSystemWebOS::setLaunchParams(params);
    static_cast<WebPageBlink*>(m_app->page())->updateExtensionData(QStringLiteral("launchParams"), launchParams());
}

void PalmSystemBlink::setLocale(const QString& params)
{
    if (m_initialized)
        static_cast<WebPageBlink*>(m_app->page())->updateExtensionData(QStringLiteral("locale"), params);
}

QString PalmSystemBlink::identifier() const
{
    if (!m_app->page())
        return QStringLiteral("");

    return static_cast<WebPageBlink*>(m_app->page())->getIdentifier();
}

void PalmSystemBlink::setLoadErrorPolicy(const QString& params)
{
    static_cast<WebPageBlink*>(m_app->page())->setLoadErrorPolicy(params);
}

QString PalmSystemBlink::trustLevel() const
{
    return static_cast<WebPageBlink*>(m_app->page())->trustLevel();
}

void PalmSystemBlink::onCloseNotify(const QString& params)
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

QJsonDocument PalmSystemBlink::initialize()
{
    QJsonObject data = PalmSystemWebOS::initialize().object();
    data["devicePixelRatio"] = devicePixelRatio();
    data["trustLevel"] = trustLevel();
    QJsonDocument doc(data);
    return doc;
}


