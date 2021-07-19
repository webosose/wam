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

#include <json/value.h>

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

bool WebAppManagerService::onKillApp(const std::string& appId, const std::string& instanceId, bool force)
{
    return WebAppManager::instance()->onKillApp(appId, instanceId, force);
}

bool WebAppManagerService::onPauseApp(const std::string& instanceId)
{
    return WebAppManager::instance()->onPauseApp(instanceId);
}

Json::Value WebAppManagerService::onLogControl(const std::string& keys, const std::string& value)
{
    LogManager::setLogControl(keys, value);

    Json::Value reply;

    reply["event"] = LogManager::getDebugEventsEnabled();
    reply["bundleMessage"] = LogManager::getDebugBundleMessagesEnabled();
    reply["mouseMove"] = LogManager::getDebugMouseMoveEnabled();
    reply["returnValue"] = true;

    return reply;
}

bool WebAppManagerService::onCloseAllApps(uint32_t pid)
{
    LOG_INFO(MSGID_LUNA_API, 2, PMLOGKS("API", "closeAllApps"), PMLOGKFV("PID", "%d", pid), "");
    return WebAppManager::instance()->closeAllApps(pid);
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

Json::Value WebAppManagerService::getWebProcessProfiling()
{
    return WebAppManager::instance()->getWebProcessProfiling();
}

void WebAppManagerService::onClearBrowsingData(const int removeBrowsingDataMask)
{
    WebAppManager::instance()->clearBrowsingData(removeBrowsingDataMask);
}

void WebAppManagerService::onAppInstalled(const std::string& app_id) {
  WebAppManager::instance()->appInstalled(app_id);
}

void WebAppManagerService::onAppRemoved(const std::string& app_id) {
  WebAppManager::instance()->appRemoved(app_id);
}

void WebAppManagerService::setDeviceInfo(const std::string& name, const std::string& value)
{
    WebAppManager::instance()->setDeviceInfo(name, value);
}

void WebAppManagerService::setUiSize(int width, int height)
{
    WebAppManager::instance()->setUiSize(width, height);
}

void WebAppManagerService::setSystemLanguage(const std::string& language)
{
    WebAppManager::instance()->setSystemLanguage(language);
}

std::string WebAppManagerService::getSystemLanguage()
{
    std::string language;
    WebAppManager::instance()->getSystemLanguage(language);
    return language;
}

void WebAppManagerService::setForceCloseApp(const std::string& appId, const std::string& instanceId)
{
    WebAppManager::instance()->setForceCloseApp(appId, instanceId);
}

void WebAppManagerService::deleteStorageData(const std::string &identifier)
{
    WebAppManager::instance()->deleteStorageData(identifier);
}

void WebAppManagerService::killCustomPluginProcess(const std::string &appBasePath)
{
    WebAppManager::instance()->killCustomPluginProcess(appBasePath);
}

void WebAppManagerService::requestKillWebProcess(uint32_t pid)
{
    WebAppManager::instance()->requestKillWebProcess(pid);
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

void WebAppManagerService::setAccessibilityEnabled(bool enable)
{
    WebAppManager::instance()->setAccessibilityEnabled(enable);
}

uint32_t WebAppManagerService::getWebProcessId(const std::string& appId, const std::string& instanceId)
{
    return WebAppManager::instance()->getWebProcessId(appId, instanceId);
}

void WebAppManagerService::updateNetworkStatus(const Json::Value& object)
{
    WebAppManager::instance()->updateNetworkStatus(object);
}

void WebAppManagerService::notifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level)
{
    WebAppManager::instance()->notifyMemoryPressure(level);
}

bool WebAppManagerService::isEnyoApp(const std::string& appId)
{
    return WebAppManager::instance()->isEnyoApp(appId);
}

int WebAppManagerService::maskForBrowsingDataType(const char* type)
{
    return WebAppManager::instance()->maskForBrowsingDataType(type);
}
