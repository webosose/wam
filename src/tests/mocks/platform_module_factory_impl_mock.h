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

#ifndef TESTS_MOCKS_PLATFORM_MODULE_FACTORY_IMPL_MOCK_H_
#define TESTS_MOCKS_PLATFORM_MODULE_FACTORY_IMPL_MOCK_H_

#include "platform_module_factory_impl.h"

class WebProcessManager;

class PlatformModuleFactoryImplMock : public PlatformModuleFactoryImpl {
 public:
  PlatformModuleFactoryImplMock() = default;
  static void SetDefaultConfig(
      const std::map<std::string, std::string>& config);

 protected:
  std::unique_ptr<WebProcessManager> CreateWebProcessManager() override;
  std::unique_ptr<WebAppManagerConfig> CreateWebAppManagerConfig() override;

 private:
  static std::map<std::string, std::string> default_config_;
};

#endif  // TESTS_MOCKS_PLATFORM_MODULE_FACTORY_IMPL_MOCK_H_
