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

#include "PalmSystemWebOS.h"

#include <memory>

#include <json/json.h>
#include <sys/stat.h>

#include "ApplicationDescription.h"
#include "LogManager.h"
#include "WebAppBase.h"
#include "WebAppWayland.h"
#include "WebPageBase.h"

#include <QFile>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>


namespace {

// TODO: Move it after merge
bool doesPathExist(const std::string& path)
{
    if (path.empty())
        return false;

    struct stat st;
    if (stat(path.c_str(), &st))
        return false;

    return st.st_mode & S_IFDIR || st.st_mode & S_IFREG;
}

// TODO: Remove after finishing QTLess implementaion
bool parseJson(const std::string& source, Json::Value& result)
{
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    return reader->parse(source.c_str(), source.c_str() + source.size(), &result, nullptr);
}
}

PalmSystemWebOS::PalmSystemWebOS(WebAppBase* app)
    : m_app(static_cast<WebAppWayland*>(app))
    , m_launchParams()
{
}

void PalmSystemWebOS::setLaunchParams(const QString& params)
{
    std::string p = params.toStdString();
    Json::Value jsonDoc = Json::nullValue;

    const bool result = parseJson(p, jsonDoc);

    if (!result || jsonDoc.isNull())
        p.erase();

    m_launchParams = p;
}

bool PalmSystemWebOS::isActivated() const
{
    return m_app->isFocused();
}

bool PalmSystemWebOS::isKeyboardVisible() const
{
    return m_app->isKeyboardVisible();
}

bool PalmSystemWebOS::isMinimal() const
{
    return doesPathExist("/var/luna/preferences/ran-firstuse");
}

int PalmSystemWebOS::activityId() const
{
    if (!m_app->page())
        return -1;

    return 0;
}

void PalmSystemWebOS::activate()
{
    ApplicationDescription* appDesc = m_app->getAppDescription();
    if (appDesc && !appDesc->handlesRelaunch())
        return;

    // ask compositor to raise window. Compositor should raise us, then
    // give us focus, so we shouldn't have to specifically request focus.
    LOG_DEBUG("[%s] called webOSSystem.activate() from the app, call raise() to make full screen", qPrintable(m_app->appId()));

    m_app->raise();
}

void PalmSystemWebOS::deactivate()
{
    LOG_DEBUG("[%s] called webOSSystem.deactivate() from the app, call goBackground() to go background", qPrintable(m_app->appId()));
    m_app->goBackground();
}

void PalmSystemWebOS::stagePreparing()
{
    m_app->stagePreparing();
}

void PalmSystemWebOS::stageReady()
{
    m_app->stageReady();
}

void PalmSystemWebOS::hide()
{
    m_app->hide();
}

void PalmSystemWebOS::setInputRegion(const QByteArray& params)
{
    // this function is not related to windowGroup anymore
    QJsonDocument jsonDoc = QJsonDocument::fromJson(params);
    m_app->setInputRegion(jsonDoc);
}

void PalmSystemWebOS::setGroupClientEnvironment(GroupClientCallKey callKey, const QByteArray& params)
{
    ApplicationDescription* appDesc = m_app ? m_app->getAppDescription() : 0;
    if (appDesc) {
        ApplicationDescription::WindowGroupInfo groupInfo = appDesc->getWindowGroupInfo();
        if (!groupInfo.name.isEmpty() && !groupInfo.isOwner) {
            QJsonDocument jsonDoc = QJsonDocument::fromJson(params);
            switch (callKey) {
                case KeyMask:
                    m_app->setKeyMask(jsonDoc);
                break;
                case FocusOwner:
                    m_app->focusOwner();
                break;
                case FocusLayer:
                    m_app->focusLayer();
                break;
                default:
                break;
            }
        }
    }
}

void PalmSystemWebOS::setKeepAlive(bool keep)
{
    //TODO: Need to check that this app is keepalive app.
    m_app->setKeepAlive(keep);
}

void PalmSystemWebOS::pmLogInfoWithClock(const QVariant& msgid, const QVariant& perfType, const QVariant& perfGroup)
{
    PmLogContext context;
    PmLogGetContext(qPrintable(m_app->page()->appId()), &context);
    PmLogInfoWithClock(context, msgid.toByteArray().size() ? msgid.toByteArray().data() : NULL, 2,
                       PMLOGKS("PerfType", perfType.toByteArray().size() ? perfType.toByteArray().data() : "empty"),
                       PMLOGKS("PerfGroup", perfGroup.toByteArray().size() ? perfGroup.toByteArray().data() : "empty"), "");
}

void PalmSystemWebOS::pmLogString(PmLogLevel level, const QVariant& msgid, const QVariant& kvpairs, const QVariant& message)
{
    PmLogContext context;
    PmLogGetContext(qPrintable(m_app->page()->appId()), &context);
    PmLogString(context, level, msgid.toByteArray().size() ? msgid.toByteArray().data() : NULL,
            kvpairs.toByteArray().size() ? kvpairs.toByteArray().data() : NULL, message.toByteArray().data());
}

bool PalmSystemWebOS::cursorVisibility()
{
    return m_app->cursorVisibility();
}

void PalmSystemWebOS::updateLaunchParams(const QString& launchParams)
{
    m_app->page()->setLaunchParams(launchParams);
}

