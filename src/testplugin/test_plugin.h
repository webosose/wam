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

#ifndef TESTPLUGIN_TEST_PLUGIN_H_
#define TESTPLUGIN_TEST_PLUGIN_H_

#include "web_app_factory_interface.h"

class TestPlugin : public WebAppFactoryInterface {
 public:
  WebAppBase* CreateWebApp(
      const std::string& win_type,
      std::shared_ptr<ApplicationDescription> desc = nullptr) override;
  WebAppBase* CreateWebApp(
      const std::string& win_type,
      WebPageBase* page,
      std::shared_ptr<ApplicationDescription> desc = nullptr) override;
  WebPageBase* CreateWebPage(const wam::Url& url,
                             std::shared_ptr<ApplicationDescription> desc,
                             const std::string& launch_params = {}) override;
};

#endif  // TESTPLUGIN_TEST_PLUGIN_H_
