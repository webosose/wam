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

#ifndef WEBAPPFACTORYMANAGERMOCK_H
#define WEBAPPFACTORYMANAGERMOCK_H

#include <memory>

#include <QString>

#include "WebAppFactoryManager.h"

class ApplicationDescription;
class WebAppBase;
class WebAppWindowFactory;
class WebViewFactory;

class WebAppFactoryManagerMock : public WebAppFactoryManager {
public:
    WebAppFactoryManagerMock();
    ~WebAppFactoryManagerMock() override;

    WebAppBase* createWebApp(QString winType, std::shared_ptr<ApplicationDescription> desc, QString appType) override;
    WebAppBase* createWebApp(QString winType, WebPageBase* page, std::shared_ptr<ApplicationDescription> desc, QString appType) override;
    WebPageBase* createWebPage(QString winType, QUrl url, std::shared_ptr<ApplicationDescription> desc, QString appType, QString launchParams) override;

    void setWebViewFactory(WebViewFactory* viewFactory);
    void setWebAppWindowFactory(WebAppWindowFactory* windowFactory);

private:
    WebViewFactory* m_viewFactory;
    WebAppWindowFactory* m_windowFactory;
};

#endif // WEBAPPFACTORYMANAGERMOCK_H
