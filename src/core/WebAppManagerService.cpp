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

#include "WebAppManagerService.h"

#include <vector>

#include "LogManager.h"
#include "WebAppBase.h"

WebAppManagerService::WebAppManagerService()
{
}

std::string WebAppManagerService::onLaunch(const std::string& appDescString, const std::string& params,
        const std::string& launchingAppId, int& errCode, std::string& errMsg)
{
    return WebAppManager::instance()->launch(appDescString, params, launchingAppId, errCode, errMsg);
}

bool WebAppManagerService::onKillApp(const std::string& appId)
{
    LOG_INFO(MSGID_LUNA_API, 2, PMLOGKS("APP_ID", qPrintable(QString::fromStdString(appId))), PMLOGKS("API", "killApp"), "");
    return WebAppManager::instance()->onKillApp(appId);
}

QJsonObject WebAppManagerService::onLogControl(const std::string& keys, const std::string& value)
{
    LogManager::setLogControl(keys, value);

    QJsonObject reply;

    reply["event"] = LogManager::getDebugEventsEnabled();
    reply["bundleMessage"] = LogManager::getDebugBundleMessagesEnabled();
    reply["mouseMove"] = LogManager::getDebugMouseMoveEnabled();

    return reply;
}

bool WebAppManagerService::onCloseAllApps(uint32_t pid)
{
    LOG_INFO(MSGID_LUNA_API, 2, PMLOGKS("API", "closeAllApps"), PMLOGKFV("PID", "%d", pid), "");
    return WebAppManager::instance()->closeAllApps(pid);
}

bool WebAppManagerService::closeContainerApp()
{
    return WebAppManager::instance()->closeContainerApp();
}

bool WebAppManagerService::isDiscardCodeCacheRequired()
{
    return WebAppManager::instance()->isDiscardCodeCacheRequired();
}

void WebAppManagerService::onDiscardCodeCache(uint32_t pid)
{
    LOG_INFO(MSGID_LUNA_API, 2, PMLOGKS("API", "discardCodeCache"), PMLOGKFV("PID", "%d", pid), "");
    WebAppManager::instance()->discardCodeCache(pid);
}

bool WebAppManagerService::onPurgeSurfacePool(uint32_t pid)
{
    return WebAppManager::instance()->purgeSurfacePool(pid);
}

QJsonObject WebAppManagerService::getWebProcessProfiling()
{
    return WebAppManager::instance()->getWebProcessProfiling();
}

void WebAppManagerService::onClearBrowsingData(const int removeBrowsingDataMask)
{
    WebAppManager::instance()->clearBrowsingData(removeBrowsingDataMask);
}

WebAppBase* WebAppManagerService::getContainerApp()
{
    return WebAppManager::instance()->getContainerApp();
}

#ifndef PRELOADMANAGER_ENABLED
void WebAppManagerService::reloadContainerApp()
{
    WebAppManager::instance()->reloadContainerApp();
}

void WebAppManagerService::startContainerTimer()
{
    WebAppManager::instance()->startContainerTimer();
}

void WebAppManagerService::restartContainerApp()
{
    WebAppManager::instance()->restartContainerApp();
}
#endif

void WebAppManagerService::setDeviceInfo(const QString &name, const QString &value)
{
    WebAppManager::instance()->setDeviceInfo(name, value);
}

void WebAppManagerService::setUiSize(int width, int height)
{
    WebAppManager::instance()->setUiSize(width, height);
}

void WebAppManagerService::setSystemLanguage(const QString &language)
{
    WebAppManager::instance()->setSystemLanguage(language);
}

QString WebAppManagerService::getSystemLanguage()
{
    QString language;
    WebAppManager::instance()->getSystemLanguage(language);
    return language;
}

void WebAppManagerService::setForceCloseApp(const QString &appId)
{
    WebAppManager::instance()->setForceCloseApp(appId);
}

void WebAppManagerService::deleteStorageData(const QString &identifier)
{
    WebAppManager::instance()->deleteStorageData(identifier);
}

void WebAppManagerService::killCustomPluginProcess(const QString &appBasePath)
{
    WebAppManager::instance()->killCustomPluginProcess(appBasePath);
}

void WebAppManagerService::requestKillWebProcess(uint32_t pid)
{
    WebAppManager::instance()->requestKillWebProcess(pid);
}

bool WebAppManagerService::shouldLaunchContainerAppOnDemand()
{
    return WebAppManager::instance()->shouldLaunchContainerAppOnDemand();
}

std::list<const WebAppBase*> WebAppManagerService::runningApps()
{
    return WebAppManager::instance()->runningApps();
}

std::list<const WebAppBase*> WebAppManagerService::runningApps(uint32_t pid)
{
    return WebAppManager::instance()->runningApps(pid);
}

std::vector<ApplicationInfo> WebAppManagerService::list(bool includeSystemApps)
{
    return WebAppManager::instance()->list(includeSystemApps);
}

QJsonObject WebAppManagerService::closeByInstanceId(QString instanceId)
{
    LOG_INFO(MSGID_LUNA_API, 2, PMLOGKS("INSTANCE_ID", qPrintable(instanceId)), PMLOGKS("API", "closeByInstanceId"), "");
    WebAppBase* app = WebAppManager::instance()->findAppByInstanceId(instanceId);
    QString appId;
    if (app) {
        appId = app->appId();
        WebAppManager::instance()->forceCloseAppInternal(app);
    }

    QJsonObject reply;
    if(!appId.isNull()) {
        reply["appId"] = appId;
        reply["processId"] = instanceId;
        reply["returnValue"] = true;
    }
    else {
        LOG_INFO(MSGID_LUNA_API, 2, PMLOGKS("INSTANCE_ID", qPrintable(instanceId)), PMLOGKS("API", "closeByInstanceId"), "No matched App; return false");
        QString errMsg("Unknown Process");
        reply["returnValue"] = false;
        reply["errorText"] = errMsg;
    }
    return reply;
}

void WebAppManagerService::setAccessibilityEnabled(bool enable)
{
    WebAppManager::instance()->setAccessibilityEnabled(enable);
}

uint32_t WebAppManagerService::getWebProcessId(const QString& appId)
{
    return WebAppManager::instance()->getWebProcessId(appId);
}

void WebAppManagerService::updateNetworkStatus(const QJsonObject& object)
{
    WebAppManager::instance()->updateNetworkStatus(object);
}

void WebAppManagerService::notifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level)
{
    WebAppManager::instance()->notifyMemoryPressure(level);
}

bool WebAppManagerService::isEnyoApp(const QString& appId)
{
    return WebAppManager::instance()->isEnyoApp(appId);
}

int WebAppManagerService::maskForBrowsingDataType(const char* type)
{
    return WebAppManager::instance()->maskForBrowsingDataType(type);
}
