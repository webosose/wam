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

#include "WebAppWindowFactoryMock.h"

#include <iostream>

#include "WebAppWindow.h"

WebAppWindowFactoryMock::WebAppWindowFactoryMock()
    : m_webAppWindow(nullptr)
{
}
WebAppWindowFactoryMock::~WebAppWindowFactoryMock() = default;

WebAppWindow* WebAppWindowFactoryMock::createWindow()
{
    if (m_webAppWindow)
        return m_webAppWindow;

    std::cerr << "Missing WebAppWindow pointer. Method setWebAppWindow should be called prior to createWindow" << std::endl;
    return nullptr;
}

void WebAppWindowFactoryMock::setWebAppWindow(WebAppWindow* webAppWindow)
{
    m_webAppWindow = webAppWindow;
}
