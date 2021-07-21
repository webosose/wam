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
#include "Utils.h"
#include "WebAppBase.h"
#include "WebAppWayland.h"
#include "WebPageBase.h"

PalmSystemWebOS::PalmSystemWebOS(WebAppBase* app)
    : m_app(static_cast<WebAppWayland*>(app))
    , m_launchParams()
{
}

void PalmSystemWebOS::setLaunchParams(const std::string& params)
{
    Json::Value jsonDoc = Json::nullValue;

    const bool result = util::stringToJson(params, jsonDoc);

    if (!result || jsonDoc.isNull())
        m_launchParams.erase();
    else
        m_launchParams = params;
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
    return util::doesPathExist("/var/luna/preferences/ran-firstuse");
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
    LOG_DEBUG("[%s] called webOSSystem.activate() from the app, call raise() to make full screen", m_app->appId().c_str());

    m_app->raise();
}

void PalmSystemWebOS::deactivate()
{
    LOG_DEBUG("[%s] called webOSSystem.deactivate() from the app, call goBackground() to go background", m_app->appId().c_str());
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

void PalmSystemWebOS::setInputRegion(const std::string& params)
{
    // this function is not related to windowGroup anymore
    Json::Value jsonDoc;
    const bool result = util::stringToJson(params, jsonDoc);
    if (result)
        m_app->setInputRegion(jsonDoc);
    else
        LOG_ERROR(MSGID_TYPE_ERROR, 0, "[%s] setInputRegion failed, params='%s'", m_app->appId().c_str(), params.c_str());
}

void PalmSystemWebOS::setGroupClientEnvironment(GroupClientCallKey callKey, const std::string& params)
{
    ApplicationDescription* appDesc = m_app ? m_app->getAppDescription() : 0;
    if (appDesc) {
        ApplicationDescription::WindowGroupInfo groupInfo = appDesc->getWindowGroupInfo();
        if (!groupInfo.name.empty() && !groupInfo.isOwner) {
            switch (callKey) {
                case KeyMask:
                {
                    Json::Value jsonDoc;
                    const bool result = util::stringToJson(params, jsonDoc);
                    if (result)
                        m_app->setKeyMask(jsonDoc);
                    else
                        LOG_ERROR(MSGID_TYPE_ERROR, 0, "[%s] failed to get key mask from params='%s'", m_app->appId().c_str(), params.c_str());
                }
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

void PalmSystemWebOS::pmLogInfoWithClock(const std::string& msgid, const std::string& perfType, const std::string& perfGroup)
{
    LOG_INFO_WITH_CLOCK(msgid.size() ? msgid.c_str() : NULL, 2,
                        PMLOGKS("PerfType", perfType.size() ? perfType.c_str() : "empty"),
                        PMLOGKS("PerfGroup", perfGroup.size() ? perfGroup.c_str() : "empty"), "");
}

void PalmSystemWebOS::pmLogString(int32_t level, const std::string& msgid, const std::string& kvpairs, const std::string& message)
{
    LOG_STRING(level, msgid.size() ? msgid.c_str() : NULL,
            kvpairs.size() ? kvpairs.c_str() : NULL, message.c_str());
}

bool PalmSystemWebOS::cursorVisibility()
{
    return m_app->cursorVisibility();
}

void PalmSystemWebOS::updateLaunchParams(const std::string& launchParams)
{
    m_app->page()->setLaunchParams(launchParams);
}

