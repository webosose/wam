// Copyright (c) 2014-2018 LG Electronics, Inc.
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

#include "ContainerAppManager.h"

#include <QtCore/QFile>
#include <QtCore/QJsonDocument>

#include "ApplicationDescription.h"
#include "LogManager.h"
#include "WebAppBase.h"
#include "WebAppFactoryManager.h"
#include "WebAppManager.h"
#include "WebAppManagerUtils.h"
#include "WebPageBase.h"
#include "WindowTypes.h"

static QString s_containerAppId = "com.webos.app.container";
static int kContainerAppLaunchDuration = 300;
static int kContainerAppLaunchCpuThresh = 500; // 100 = 10%
static int kContainerAppLaunchTryMax = 20;

static inline char * skipToken(const char *p)
{
    while (isspace(*p))
        p++;
    while (*p && !isspace(*p))
        p++;
    return (char *)p;
}

ContainerAppManager::ContainerAppManager()
    : m_containerApp(0)
    , m_containerAppRelaunchCounter(0)
    , m_containerAppIsLaunched(false)
    , m_containerAppIsReady(false)
    , m_launchContainerAppOnDemand(false)
    , m_useContainerAppOptimization(false)
{
#ifndef PRELOADMANAGER_ENABLED
    loadContainerInfo();
#endif
}

ContainerAppManager::~ContainerAppManager()
{
    closeContainerApp();
}

void ContainerAppManager::loadContainerInfo()
{
    QFile file;
    file.setFileName("/var/luna/preferences/container.json");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString str;
        str = file.readAll();
        file.close();

        QJsonDocument containerDoc = QJsonDocument::fromJson(str.toUtf8());
        if(!containerDoc.isNull()) {
            QJsonObject containerSettings = containerDoc.object();
            if(!containerSettings["appId"].isUndefined())
                s_containerAppId = containerSettings["appId"].toString();
            if(!containerSettings["relaunchDelay"].isUndefined())
                kContainerAppLaunchDuration = containerSettings["relaunchDelay"].toDouble();
            if(!containerSettings["relaunchCpuThresh"].isUndefined())
                kContainerAppLaunchCpuThresh = containerSettings["relaunchCpuThresh"].toDouble();
        }
    }

    LOG_DEBUG("Container settings: app_id=%s, delay=%d, thresh=%d", qPrintable(s_containerAppId), kContainerAppLaunchDuration, kContainerAppLaunchCpuThresh);
}

void ContainerAppManager::startContainerTimer()
{
    m_containerAppLaunchTimer.stop();
    WebAppManagerUtils::updateAndGetCpuIdle(true);
    m_containerAppLaunchTimer.start(kContainerAppLaunchDuration, this,
                                    &ContainerAppManager::containerAppLaunch);
}

void ContainerAppManager::stopContainerTimer()
{
    m_containerAppLaunchTimer.stop();
}

QString& ContainerAppManager::getContainerAppId()
{
    return s_containerAppId;
}

void ContainerAppManager::containerAppLaunch()
{
    if (++m_containerAppRelaunchCounter >= kContainerAppLaunchTryMax || WebAppManagerUtils::updateAndGetCpuIdle() > kContainerAppLaunchCpuThresh) {
        m_containerAppRelaunchCounter = 0;
        int errorCode;
        if (!m_containerApp) {
            std::string instanceId = WebAppManager::instance()->generateInstanceId();
            launchContainerAppInternal(instanceId, errorCode);
        }
        else if (!m_containerAppIsReady)
            reloadContainerApp();
        m_containerAppLaunchTimer.stop();
    }
}

WebAppBase* ContainerAppManager::launchContainerAppInternal(const std::string& instanceId, int& errorCode)
{
    if (m_containerApp)
        return m_containerApp;

#ifndef PRELOADMANAGER_ENABLED
    if (!m_containerDesc.size()) {
        WebAppManager::instance()->sendLaunchContainerApp();
        return 0;
    }
#endif

    ApplicationDescription* desc = ApplicationDescription::fromJsonString(m_containerDesc.c_str());
    if (!desc) {
        LOG_ERROR(MSGID_LAUNCH_URL_BAD_APP_DESC, 1, PMLOGKS("APP_DESC", m_containerDesc.c_str()), "");
        return 0;
    }
    WebAppBase* app = WebAppFactoryManager::instance()->createWebApp(WT_CARD, desc, desc->subType().c_str());

    if (!app) {
        delete desc;
        return 0;
    }

    std::string url = desc->entryPoint();
    WebPageBase* page = WebAppFactoryManager::instance()->createWebPage(WT_CARD, QUrl(url.c_str()), desc, desc->subType().c_str());

    // Turning off inline caching on container app, too.
    if (m_useContainerAppOptimization)
        page->setUseSystemAppOptimization(true);

    app->setAppDescription(desc);
    app->setHiddenWindow(true);

    app->setInstanceId(QString::fromStdString(instanceId));
    app->attach(page);
    page->load();
    WebAppManager::instance()->webPageAdded(page);

    m_containerApp = app;

#ifdef PRELOADMANAGER_ENABLED
    WebAppManager::instance()->insertAppIntoList(m_containerApp);
#endif

    LOG_INFO(MSGID_CONTAINER_APP_RELAUNCHED, 2, PMLOGKS("APP_ID", qPrintable(QString::fromStdString(desc->id()))), PMLOGKFV("PID", "%d", page->getWebProcessPID()), "");

    return m_containerApp;
}

WebAppBase* ContainerAppManager::launchContainerApp(const std::string& appDesc, const std::string& instanceId, int& errorCode)
{
    m_containerDesc = appDesc;
    return launchContainerAppInternal(instanceId, errorCode);
}

void ContainerAppManager::closeContainerApp()
{
    if (!m_containerAppIsReady && !m_containerApp) {
        // Stop containerAppTimer
        m_containerAppLaunchTimer.stop();
        LOG_INFO(MSGID_CONTAINER_APP_STATUS_CHANGED, 1, PMLOGKS("Status","Timer Stopped"), "");
        return;
    }

#ifdef PRELOADMANAGER_ENABLED
    WebAppManager::instance()->deleteAppIntoList(m_containerApp);
#endif

    if (m_containerApp)
        delete m_containerApp;

    m_containerApp = 0;
    m_containerAppIsLaunched = false;
    m_containerAppIsReady = false;
    m_containerAppRelaunchCounter = 0;
    LOG_INFO(MSGID_CONTAINER_APP_STATUS_CHANGED, 1, PMLOGKS("Status","Container Closed"), "");

}

void ContainerAppManager::reloadContainerApp()
{
    if (m_containerApp) {
        m_containerAppIsLaunched = false;
        m_containerAppIsReady = false;
        m_containerApp->page()->reloadDefaultPage();
        // FIXME: Container app should be ready when reloading is done
    }
}

void ContainerAppManager::restartContainerApp()
{
    if (m_containerApp) {
        closeContainerApp();
        m_containerApp = 0;
    }

    startContainerTimer();
}

bool ContainerAppManager::isContainerAppReady()
{
    return m_containerAppIsReady && m_containerApp && m_containerApp->page() && !m_containerApp->page()->isClosing();
}

void ContainerAppManager::resetContainerAppManager()
{
    // Do not delete m_containerApp since this API is signaled after launching container-based app
    m_containerAppIsLaunched = false;
    m_containerAppIsReady = false;
    m_containerApp = 0;
    m_containerAppLaunchTimer.stop();
}
