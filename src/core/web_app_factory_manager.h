// Copyright (c) 2008-2021 LG Electronics, Inc.
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

#ifndef CORE_WEB_APP_FACTORY_MANAGER_H_
#define CORE_WEB_APP_FACTORY_MANAGER_H_

#include <memory>
#include <string>

namespace wam {
class Url;
}

class ApplicationDescription;
class WebAppBase;
class WebPageBase;

class WebAppFactoryManager {
 public:
  WebAppFactoryManager() = default;
  virtual ~WebAppFactoryManager() = default;

  virtual WebAppBase* CreateWebApp(
      const std::string& win_type,
      std::shared_ptr<ApplicationDescription> desc = nullptr,
      const std::string& app_type = {}) = 0;
  virtual WebAppBase* CreateWebApp(
      const std::string& win_type,
      WebPageBase* page,
      std::shared_ptr<ApplicationDescription> desc = nullptr,
      const std::string& app_type = {}) = 0;
  virtual WebPageBase* CreateWebPage(
      const std::string& win_type,
      const wam::Url& url,
      std::shared_ptr<ApplicationDescription> desc,
      const std::string& app_type = {},
      const std::string& launch_params = {}) = 0;
};

#endif  // CORE_WEB_APP_FACTORY_MANAGER_H_
