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

#include "WebProcessManager.h"

#include <climits>
#include <cstdio>
#include <fstream>
#include <signal.h>
#include <string>

#include <glib.h>
#include <json/json.h>

#include "ApplicationDescription.h"
#include "LogManager.h"
#include "TypeConverter.h"
#include "WebAppBase.h"
#include "WebAppManager.h"
#include "WebAppManagerConfig.h"
#include "WebAppManagerUtils.h"
#include "WebPageBase.h"

WebProcessManager::WebProcessManager()
    : m_maximumNumberOfProcesses(1)
{
    readWebProcessPolicy();
}

std::list<const WebAppBase*> WebProcessManager::runningApps()
{
    return WebAppManager::instance()->runningApps();
}

std::list<const WebAppBase*> WebProcessManager::runningApps(uint32_t pid)
{
    return WebAppManager::instance()->runningApps(pid);
}

WebAppBase* WebProcessManager::findAppById(const std::string& appId)
{
    return WebAppManager::instance()->findAppById(appId);
}

WebAppBase* WebProcessManager::findAppByInstanceId(const std::string& instanceId)
{
    return WebAppManager::instance()->findAppByInstanceId(instanceId);
}

bool WebProcessManager::webProcessInfoMapReady()
{
    uint32_t count = 0;
    for (const auto& it : m_webProcessInfoMap) {
        if (it.second.proxyID != 0)
            count++;
    }

    return count == m_maximumNumberOfProcesses;
}

uint32_t WebProcessManager::getWebProcessProxyID(const ApplicationDescription *desc) const
{
    if (!desc)
        return 0;

    std::string key = getProcessKey(desc);

    auto it = m_webProcessInfoMap.find(key);
    if (it == m_webProcessInfoMap.end() || !it->second.proxyID) {
        return getInitialWebViewProxyID();
    }

    return it->second.proxyID;
}

uint32_t WebProcessManager::getWebProcessProxyID(uint32_t pid) const
{
    auto res = find_if(m_webProcessInfoMap.begin(), m_webProcessInfoMap.end(), [pid](const auto& item) {
        return (item.second.webProcessPid == pid);
    });

    if (res != m_webProcessInfoMap.end()) {
        return res->second.proxyID;
    }
    return 0;
}

std::string WebProcessManager::getWebProcessMemSize(uint32_t pid) const
{
    std::string path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream in(path);

    if (!in.is_open())
        return {};

    std::string line;
    while (std::getline(in, line)) {
        if(!line.find("VmRSS:", 0, 6)) {
            return trimString(std::string(line, 6)).c_str();
        }
    }
    return {};
}

void WebProcessManager::readWebProcessPolicy()
{
    Json::Value webProcessEnvironment;
    std::string configPath = WebAppManager::instance()->config()->getWebProcessConfigPath();
    bool config = fileToJson(configPath, webProcessEnvironment);

    if (!config || webProcessEnvironment.isNull()) {
        LOG_ERROR(MSGID_WEBPROCESSENV_READ_FAIL, 1, PMLOGKS("PATH", configPath.c_str()), "JSON parsing failed");
        return;
    }

    auto createProcessForEachApp = webProcessEnvironment["createProcessForEachApp"];
    if (createProcessForEachApp.isBool() && createProcessForEachApp.asBool())
        m_maximumNumberOfProcesses = UINT_MAX;
    else {
        auto webProcessArray = webProcessEnvironment["webProcessList"];
        if (webProcessArray.isArray()) {
            for (const auto &value : webProcessArray) {
                if (!value.isObject())
                    continue;
                auto id = value["id"];
                if (id.isString()) {
                    m_webProcessGroupAppIDList.push_back(id.asString());
                    setWebProcessCacheProperty(value, id.asString());
                }
                auto trustLevel = value["trustLevel"];
                if (trustLevel.isString()) {
                    m_webProcessGroupTrustLevelList.push_back(trustLevel.asString());
                    setWebProcessCacheProperty(value, trustLevel.asString());
                }
            }
        }
        m_maximumNumberOfProcesses = (m_webProcessGroupTrustLevelList.size() + m_webProcessGroupAppIDList.size());
    }

    LOG_INFO(MSGID_SET_WEBPROCESS_ENVIRONMENT, 3, PMLOGKFV("MAXIMUM_WEBPROCESS_NUMBER", "%u", m_maximumNumberOfProcesses),
            PMLOGKFV("GROUP_TRUSTLEVELS_COUNT", "%d", m_webProcessGroupTrustLevelList.size()),
            PMLOGKFV("GROUP_APP_IDS_COUNT", "%d", m_webProcessGroupAppIDList.size()), "");
}

void WebProcessManager::setWebProcessCacheProperty(const Json::Value &object, const std::string& key)
{
    WebProcessInfo info = WebProcessInfo(0, 0);
    auto memoryCache = object["memoryCache"];
    if (memoryCache.isString()) {
        int memCacheSize = 0;
        stringToInt(memoryCache.asString(), memCacheSize);
        info.memoryCacheSize = memCacheSize;
    }

    auto codeCache = object["codeCache"];
    if (codeCache.isString()) {
        int codeCacheInt = 0;
        stringToInt(codeCache.asString(), codeCacheInt);
        info.codeCacheSize = codeCacheInt;
    }

    m_webProcessInfoMap.emplace(key, info);
}

std::string WebProcessManager::getProcessKey(const ApplicationDescription* desc) const
{
    if (!desc)
        return std::string();

    std::string key;
    std::vector<std::string> idList, trustLevelList;
    if (m_maximumNumberOfProcesses == 1)
        key = "system";
    else if (m_maximumNumberOfProcesses == UINT_MAX) {
        if (desc->trustLevel() == "default" || desc->trustLevel() == "trusted")
            key = "system";
        else
            key = desc->id();
    }
    else {
        for (size_t i = 0; i < m_webProcessGroupAppIDList.size(); i++) {
            std::string appId = m_webProcessGroupAppIDList.at(i);
            if (appId.find('*') != std::string::npos) {
                replaceSubstrings(appId, "*");
                auto l = splitString(appId, ',');
                idList.insert(idList.end(), l.begin(), l.end());
                for (const auto& id : idList)
                    if (!desc->id().compare(0, id.size(), id))
                        key = m_webProcessGroupAppIDList.at(i);
            } else {
                auto l = splitString(appId, ',');
                idList.insert(idList.end(), l.begin(), l.end());
                for (const auto& id : idList)
                    if (id == desc->id())
                        return m_webProcessGroupAppIDList.at(i);
            }
        }
        if (!key.empty())
            return key;

        for (size_t i = 0; i < m_webProcessGroupTrustLevelList.size(); i++) {
            std::string trustLevel = m_webProcessGroupTrustLevelList.at(i);
            auto l = splitString(trustLevel, ',');
            trustLevelList.insert(trustLevelList.end(), l.begin(), l.end());
            for (const auto& trust : trustLevelList) {
                if (trust == desc->trustLevel()) {
                    return m_webProcessGroupTrustLevelList.at(i);
                }
            }
        }
        key = "system";
    }
    return key;
}

void WebProcessManager::killWebProcess(uint32_t pid)
{
    for(auto &it : m_webProcessInfoMap) {
        if (it.second.webProcessPid == pid) {
            it.second.requestKill = false;
            break;
        }
    }

    LOG_INFO(MSGID_KILL_WEBPROCESS, 1, PMLOGKFV("PID", "%u", pid), "");
    int ret = kill(pid, SIGKILL);
    if (ret == -1)
        LOG_ERROR(MSGID_KILL_WEBPROCESS_FAILED, 1, PMLOGKS("ERROR", strerror(errno)), "SystemCall failed");
}

void WebProcessManager::requestKillWebProcess(uint32_t pid)
{
    for(auto &it : m_webProcessInfoMap) {
        if (it.second.webProcessPid == pid) {
            LOG_INFO(MSGID_KILL_WEBPROCESS_DELAYED, 1, PMLOGKFV("PID", "%u", pid), "");
            it.second.requestKill = true;
            return;
        }
    }
}
