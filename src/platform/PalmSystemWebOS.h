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

#include "PalmSystemBase.h"
#include <PmLogLib.h>

class WebAppBase;
class WebAppWayland;

class PalmSystemWebOS : public PalmSystemBase {
public:
    PalmSystemWebOS(WebAppBase* app);

    void setInitialized(bool initialize) { m_initialized = initialize; }

    virtual void setCountry() {}
    virtual void setFolderPath(const QString& params) {}
    virtual void setLaunchParams(const QString& params);

protected:
    enum GroupClientCallKey {
        KeyMask = 1,
        FocusOwner,
        FocusLayer
    };

    virtual QJsonDocument initialize();

    virtual QString identifier() const = 0;
    virtual QString launchParams() const { return m_launchParams; }
    virtual QString version() const { return QString(); }

    virtual QString screenOrientation() const { return QString("up"); }
    virtual QString windowOrientation() const { return QString("free"); }

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
    virtual void setLoadErrorPolicy(const QString& params) {}
    virtual void setInputRegion(const QByteArray& params);
    virtual void setGroupClientEnvironment(GroupClientCallKey callKey, const QByteArray& params);

    virtual void pmLogInfoWithClock(const QVariant& msgid, const QVariant& perfType, const QVariant& perfGroup);
    virtual void pmLogString(PmLogLevel level, const QVariant& msgid, const QVariant& kvpairs, const QVariant& message);
    virtual bool cursorVisibility();
    virtual void updateLaunchParams(const QString& launchParams);

protected:
    WebAppWayland* m_app;
    bool m_initialized;
    QString m_launchParams;
};

#endif // PALMSYSTEMWEBOS_H_
