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

#include "WebAppFactoryManagerMock.h"

#include <gtest/gtest.h>

#include "ApplicationDescription.h"
#include "WebAppWayland.h"
#include "WebAppWindowFactory.h"
#include "WebPageBlink.h"
#include "WebViewFactory.h"

WebAppFactoryManagerMock::WebAppFactoryManagerMock()
    : m_viewFactory(nullptr)
    , m_windowFactory(nullptr)
{
}

WebAppFactoryManagerMock::~WebAppFactoryManagerMock() = default;

WebAppBase* WebAppFactoryManagerMock::createWebApp(QString winType, std::shared_ptr<ApplicationDescription> desc, QString appType)
{
    if (m_windowFactory)
        return new WebAppWayland(winType, std::unique_ptr<WebAppWindowFactory>(m_windowFactory), desc->widthOverride(), desc->heightOverride(), desc->getDisplayAffinity(), desc->locationHint());

    std::cerr << "Missing WindowFactory pointer. Method setWebAppWindowFactory should be called prior to createWebApp" << std::endl;
    return nullptr;
}

WebAppBase* WebAppFactoryManagerMock::createWebApp(QString winType, WebPageBase* page, std::shared_ptr<ApplicationDescription> desc, QString appType)
{
    return createWebApp(winType, desc, appType);
}

WebPageBase* WebAppFactoryManagerMock::createWebPage(QString winType, QUrl url, std::shared_ptr<ApplicationDescription> desc, QString appType, QString launchParams)
{
    if (!m_viewFactory) {
        std::cerr << "Missing ViewFactory pointer. Method setWebViewFactory should be called prior to createWebPage" << std::endl;
        return nullptr;
    }
    auto page = new WebPageBlink(url, desc, launchParams, std::unique_ptr<WebViewFactory>(m_viewFactory));
    page->init();
    return page;
}

void WebAppFactoryManagerMock::setWebViewFactory(WebViewFactory* viewFactory)
{
    m_viewFactory = viewFactory;
}

void WebAppFactoryManagerMock::setWebAppWindowFactory(WebAppWindowFactory* windowFactory)
{
    m_windowFactory = windowFactory;
}
