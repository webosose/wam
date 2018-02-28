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

#ifndef PALMSYSTEMBLINK_H_
#define PALMSYSTEMBLINK_H_

#include "PalmSystemWebOS.h"

class PalmSystemBlink : public PalmSystemWebOS {
public:
    PalmSystemBlink(WebAppBase* app);

    QString handleBrowserControlMessage(const QString& message, const QStringList& params);

    // PalmSystemWebOS
    void setCountry() override;
    void setLaunchParams(const QString& params) override;

    virtual void setLocale(const QString& params);
    virtual double devicePixelRatio();

protected:
    // PalmSystemWebOS
    QJsonDocument initialize() override;
    QString identifier() const override;
    void setLoadErrorPolicy(const QString& params) override;

    virtual QString trustLevel() const;
    virtual void onCloseNotify(const QString& params);
};

#endif // PALMSYSTEMBLINK_H_
