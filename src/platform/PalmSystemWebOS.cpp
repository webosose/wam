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

#include "ApplicationDescription.h"
#include "JsonHelper.h"
#include "LogManager.h"
#include "WebAppBase.h"
#include "WebAppWayland.h"
#include "WebPageBase.h"

#include <QFile>

PalmSystemWebOS::PalmSystemWebOS(WebAppBase* app)
    : m_app(static_cast<WebAppWayland*>(app))
    , m_initialized(false)
    , m_launchParams()
{
}

void PalmSystemWebOS::setLaunchParams(const std::string& params)
{
    std::string p = params;
    Json::Value jsonDoc;
    readJsonFromString(params, jsonDoc);

    if (jsonDoc.isNull())
        p = "";

    m_launchParams = p;
}

Json::Value PalmSystemWebOS::initialize()
{
    m_initialized = true;

    // Setup initial data set
    Json::Value data;

    data["launchParams"] = launchParams();
    data["country"] = country().toStdString();
    data["currentCountryGroup"] = getDeviceInfo("CountryGroup");
    data["locale"] = locale();
    data["localeRegion"] = localeRegion().toStdString();
    data["isMinimal"] = isMinimal();
    data["identifier"] = identifier();
    data["screenOrientation"] = screenOrientation().toStdString();
    data["activityId"] = (double)activityId();
    data["phoneRegion"] = phoneRegion().toStdString();
    data["folderPath"] = m_app->getAppDescription()->folderPath();

    return std::move(data);
}

bool PalmSystemWebOS::isActivated() const
{
    return m_app->isFocused();
}

bool PalmSystemWebOS::isKeyboardVisible() const
{
    return m_app->page()->isKeyboardVisible();
}

bool PalmSystemWebOS::isMinimal() const
{
    return QFile::exists("/var/luna/preferences/ran-firstuse");
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
    LOG_DEBUG("[%s] called PalmSystem.activate() from the app, call raise() to make full screen", m_app->appId().c_str());

    m_app->raise();
}

void PalmSystemWebOS::deactivate()
{
    LOG_DEBUG("[%s] called PalmSystem.deactivate() from the app, call goBackground() to go background", m_app->appId().c_str());
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
    Json::Value obj;
    readJsonFromString(params.toStdString(), obj);
    m_app->setInputRegion(obj);
}

void PalmSystemWebOS::setGroupClientEnvironment(GroupClientCallKey callKey, const QByteArray& params)
{
    ApplicationDescription* appDesc = m_app ? m_app->getAppDescription() : 0;
    if (appDesc) {
        ApplicationDescription::WindowGroupInfo groupInfo = appDesc->getWindowGroupInfo();
        if (!groupInfo.name.empty() && !groupInfo.isOwner) {
            Json::Value jsonDoc;
            readJsonFromString(params.toStdString(), jsonDoc);
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

#ifdef HAS_PMLOG
void PalmSystemWebOS::pmLogInfoWithClock(const QVariant& msgid, const QVariant& perfType, const QVariant& perfGroup)
{
    PmLogContext context;
    PmLogGetContext(m_app->page()->appId().c_str(), &context);
    PmLogInfoWithClock(context, msgid.toByteArray().size() ? msgid.toByteArray().data() : NULL, 2,
                       PMLOGKS("PerfType", perfType.toByteArray().size() ? perfType.toByteArray().data() : "empty"),
                       PMLOGKS("PerfGroup", perfGroup.toByteArray().size() ? perfGroup.toByteArray().data() : "empty"), "");
}

void PalmSystemWebOS::pmLogString(PmLogLevel level, const QVariant& msgid, const QVariant& kvpairs, const QVariant& message)
{
    PmLogContext context;
    PmLogGetContext(m_app->page()->appId().c_str(), &context);
    PmLogString(context, level, msgid.toByteArray().size() ? msgid.toByteArray().data() : NULL,
            kvpairs.toByteArray().size() ? kvpairs.toByteArray().data() : NULL, message.toByteArray().data());
}
#endif

bool PalmSystemWebOS::cursorVisibility()
{
    return m_app->cursorVisibility();
}

void PalmSystemWebOS::updateLaunchParams(const std::string& launchParams)
{
    m_app->page()->setLaunchParams(launchParams);
}

