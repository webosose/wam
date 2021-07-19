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

#ifndef PALMSYSTEMWEBOS_H_
#define PALMSYSTEMWEBOS_H_

#include <string>

#include "PalmSystemBase.h"

class WebAppBase;
class WebAppWayland;

class PalmSystemWebOS : public PalmSystemBase {
public:
    PalmSystemWebOS(WebAppBase* app);

    virtual void setCountry() {}
    virtual void setFolderPath(const std::string& params) {}
    virtual void setLaunchParams(const std::string& params);

protected:
    enum GroupClientCallKey {
        KeyMask = 1,
        FocusOwner,
        FocusLayer
    };

    virtual std::string identifier() const = 0;
    virtual std::string launchParams() const { return  m_launchParams; }
    virtual std::string version() const { return std::string(); }

    virtual std::string screenOrientation() const { return std::string("up"); }
    virtual std::string windowOrientation() const { return std::string("free"); }

    virtual bool isActivated() const;
    virtual bool isKeyboardVisible() const;
    virtual bool isMinimal() const;
    virtual int activityId() const;
    virtual void activate();
    virtual void deactivate();
    virtual void stagePreparing();
    virtual void stageReady();

    virtual void show() {}
    virtual void hide();

    virtual void setKeepAlive(bool keep);
    virtual void setLoadErrorPolicy(const std::string& params) {}
    virtual void setInputRegion(const std::string& params);
    virtual void setGroupClientEnvironment(GroupClientCallKey callKey, const std::string& params);

    virtual void pmLogInfoWithClock(const std::string& msgid, const std::string& perfType, const std::string& perfGroup);
    virtual void pmLogString(int32_t level, const std::string& msgid, const std::string& kvpairs, const std::string& message);
    virtual bool cursorVisibility();
    virtual void updateLaunchParams(const std::string& launchParams);

protected:
    WebAppWayland* m_app;
    std::string m_launchParams;
};

#endif // PALMSYSTEMWEBOS_H_
