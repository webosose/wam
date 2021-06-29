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

#include <string>
#include <vector>

#include <json/json.h>

#include "PalmSystemWebOS.h"

class PalmSystemBlink : public PalmSystemWebOS {
public:
    PalmSystemBlink(WebAppBase* app);

    std::string handleBrowserControlMessage(const std::string& command, const std::vector<std::string>& arguments);

    // PalmSystemWebOS
    void setCountry() override;
    void setLaunchParams(const QString& params) override;

    virtual void setLocale(const QString& params);
    virtual double devicePixelRatio();
    void resetInitialized() { m_initialized = false; }
    bool isInitialized() { return m_initialized; }

protected:
    // PalmSystemWebOS
    Json::Value initialize();
    QString identifier() const override;
    void setLoadErrorPolicy(const QString& params) override;

    virtual std::string trustLevel() const;
    virtual void onCloseNotify(const QString& params);

private:
    bool m_initialized;
};

#endif // PALMSYSTEMBLINK_H_
