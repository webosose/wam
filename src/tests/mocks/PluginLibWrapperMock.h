//  (c) 2021 LG Electronics, Inc.
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

#ifndef PLUGINLIBWRAPPERMOCK_H
#define PLUGINLIBWRAPPERMOCK_H

#include <gmock/gmock.h>

#include "PluginLibWrapper.h"

class PluginLibWrapperMock : public PluginLibWrapper {
public:
  PluginLibWrapperMock() = default;
  ~PluginLibWrapperMock() override = default;

  MOCK_METHOD(void*, Load, (const std::string&), (override));
  MOCK_METHOD(void, Unload, (void*), (override));
  MOCK_METHOD(CreateInstanceFunc, GetCreateInstanceFunction, (void*), (override));
  MOCK_METHOD(DeleteInstanceFunc, GetDeleteInstanceFunction, (void*), (override));
  MOCK_METHOD(std::string, GetAppType, (void*), (override));
  MOCK_METHOD(std::string, GetLastError, (), (override));
};

#endif // PLUGINLIBWRAPPERMOCK_H
