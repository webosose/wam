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

#ifndef TESTS_MOCKS_BLINK_WEB_PROCESS_MANAGER_MOCK_H_
#define TESTS_MOCKS_BLINK_WEB_PROCESS_MANAGER_MOCK_H_

#include <gmock/gmock.h>

#include "blink_web_process_manager.h"

class WebAppBase;

class BlinkWebProcessManagerMock : public BlinkWebProcessManager {
 public:
  BlinkWebProcessManagerMock() = default;
  ~BlinkWebProcessManagerMock() override = default;

  MOCK_METHOD(uint32_t, GetWebProcessPIDMock, (), (const));
  MOCK_METHOD(std::string, GetWebProcessMemSize, (uint32_t), (const, override));
  MOCK_METHOD(void, ClearBrowsingData, (const int), (override));

  uint32_t GetWebProcessPID(const WebAppBase* app) const override;
};

#endif  // TESTS_MOCKS_BLINK_WEB_PROCESS_MANAGER_MOCK_H_
