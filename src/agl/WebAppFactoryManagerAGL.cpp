// Copyright (c) 2008-2018 LG Electronics, Inc.
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

#include "WebAppFactoryManager.h"

#include "LogManager.h"
#include "WebAppBase.h"
#include "WebAppFactoryAGL.h"
#include "WebAppManagerConfig.h"
#include "WebAppManager.h"
#include "WebPageBase.h"

class WebAppFactoryManagerAGL : public WebAppFactoryManager {
protected:
    virtual WebAppFactoryInterface* loadInterfaceInstance(QString appType);

private:
    friend class WebAppFactoryManager;
    WebAppFactoryManagerAGL();

    WebAppFactoryInterface *m_defaultInterface;
};

WebAppFactoryManager* WebAppFactoryManager::instance()
{
    if(!m_instance) {
        m_instance = new WebAppFactoryManagerAGL();
    }
    return m_instance;
}

WebAppFactoryManagerAGL::WebAppFactoryManagerAGL()
    : m_defaultInterface(new WebAppFactoryAGL())
{
    m_interfaces.insert(kDefaultAppType, m_defaultInterface);
}

WebAppFactoryInterface* WebAppFactoryManagerAGL::loadInterfaceInstance(QString appType)
{
    if (!appType.isEmpty())
        return nullptr;

    return m_defaultInterface;
}

