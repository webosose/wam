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

#ifndef BLINKWEBPROCESSMANAGER_H
#define BLINKWEBPROCESSMANAGER_H

#include "WebProcessManager.h"

class QString;
class WebAppBase;

namespace Json {
class Value;
}

class BlinkWebProcessManager : public WebProcessManager {
public:
    // WebProcessManager
    Json::Value getWebProcessProfiling() override;
    uint32_t getWebProcessPID(const WebAppBase* app) const override;
    void deleteStorageData(const QString& identifier) override;
    uint32_t getInitialWebViewProxyID() const override;
    void clearBrowsingData(const int removeBrowsingDataMask) override;
    int maskForBrowsingDataType(const char* type) override;
    void setProxyRules(const std::string& proxy_rules) override;
};

#endif /* BLINKEBPROCESSMANAGER_H */
