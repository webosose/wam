// Copyright (c) 2021 LG Electronics, Inc.
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

#ifndef WEBAPPMANAGERCONFIGMOCK_H
#define WEBAPPMANAGERCONFIGMOCK_H

#include "WebAppManagerConfig.h"

class WebAppManagerConfigMock : public WebAppManagerConfig {
public:
    WebAppManagerConfigMock() = default;
    WebAppManagerConfigMock(const std::map<std::string, std::string> *environmentVariables);
    ~WebAppManagerConfigMock() override;

protected:
    std::string wamGetEnv(const char *name) override;

private:
    const std::map<std::string, std::string> *m_environmentVariables = nullptr;
};

#endif /* WEBAPPMANAGERCONFIGMOCK_H */
