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

#ifndef TESTS_MOCKS_WEB_APP_FACTORY_MANAGER_MOCK_H_
#define TESTS_MOCKS_WEB_APP_FACTORY_MANAGER_MOCK_H_

#include <memory>

#include "web_app_factory_manager.h"

class ApplicationDescription;
class WebAppBase;
class WebAppWindowFactory;
class WebViewFactory;

class WebAppFactoryManagerMock : public WebAppFactoryManager {
 public:
  WebAppFactoryManagerMock();
  ~WebAppFactoryManagerMock() override;

  WebAppBase* CreateWebApp(const std::string& win_type,
                           std::shared_ptr<ApplicationDescription> desc,
                           const std::string& app_type) override;
  WebAppBase* CreateWebApp(const std::string& win_type,
                           WebPageBase* page,
                           std::shared_ptr<ApplicationDescription> desc,
                           const std::string& app_type) override;
  WebPageBase* CreateWebPage(const std::string& win_type,
                             const wam::Url& url,
                             std::shared_ptr<ApplicationDescription> desc,
                             const std::string& app_type,
                             const std::string& launchParams) override;

  void SetWebViewFactory(WebViewFactory* view_factory);
  void SetWebAppWindowFactory(WebAppWindowFactory* window_factory);

 private:
  WebViewFactory* view_factory_;
  WebAppWindowFactory* window_factory_;
};

#endif  // TESTS_MOCKS_WEB_APP_FACTORY_MANAGER_MOCK_H_
