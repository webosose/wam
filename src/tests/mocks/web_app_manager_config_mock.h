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

#ifndef TESTS_MOCKS_WEB_APP_MANAGER_CONFIG_MOCK_H_
#define TESTS_MOCKS_WEB_APP_MANAGER_CONFIG_MOCK_H_

#include <map>

#include "web_app_manager_config.h"

class WebAppManagerConfigMock : public WebAppManagerConfig {
 public:
  WebAppManagerConfigMock() = default;
  WebAppManagerConfigMock(
      const std::map<std::string, std::string>* environment_variables);
  ~WebAppManagerConfigMock() override;

 protected:
  std::string WamGetEnv(const char* name) override;

 private:
  const std::map<std::string, std::string>* environment_variables_ = nullptr;
};

#endif  // TESTS_MOCKS_WEB_APP_MANAGER_CONFIG_MOCK_H_
