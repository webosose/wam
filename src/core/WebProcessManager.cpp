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

#include <signal.h>
#include <QFile>

#include "ApplicationDescription.h"
#include "JsonHelper.h"
#include "LogManager.h"
#include "WebAppBase.h"
#include "WebAppManagerConfig.h"
#include "WebAppManagerUtils.h"
#include "WebAppManager.h"
#include "WebPageBase.h"

#include <glib.h>

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

WebAppBase* WebProcessManager::findAppById(const QString& appId)
{
    return WebAppManager::instance()->findAppById(appId);
}

WebAppBase* WebProcessManager::getContainerApp()
{
    return WebAppManager::instance()->getContainerApp();
}

bool WebProcessManager::webProcessInfoMapReady()
{
    uint32_t count = 0;
    for (const auto& it : m_webProcessInfoMap) {
        if (it.proxyID != 0)
            count++;
    }

    return count == m_maximumNumberOfProcesses;
}

uint32_t WebProcessManager::getWebProcessProxyID(const ApplicationDescription *desc) const
{
    if (!desc)
        return 0;

    QString key = getProcessKey(desc);

    QMap<QString, WebProcessInfo>::const_iterator it = m_webProcessInfoMap.find(key);
    if (it == m_webProcessInfoMap.end() || !it.value().proxyID) {
        return getInitialWebViewProxyID();
    }

    return it.value().proxyID;
}

uint32_t WebProcessManager::getWebProcessProxyID(uint32_t pid) const
{
    for (QMap<QString, WebProcessInfo>::const_iterator it = m_webProcessInfoMap.begin(); it != m_webProcessInfoMap.end(); it++) {
        if (it.value().webProcessPid == pid)
            return it.value().proxyID;
    }
    return 0;
}

QString WebProcessManager::getWebProcessMemSize(uint32_t pid) const
{
    QString filePath = QString("/proc/") + QString::number(pid) + QString("/status");
    FILE *fd = fopen(filePath.toStdString().c_str(), "r");
    QString vmrss;
    char line[128];

    if (!fd)
        return vmrss;

    while (fgets(line, 128, fd) != NULL) {
        if(!strncmp(line, "VmRSS:", 6)) {
            vmrss = QString(&line[8]);
            break;
        }
    }

    fclose(fd);
    return vmrss.simplified();
}

void WebProcessManager::readWebProcessPolicy()
{
    Json::Value webProcessEnvironment;
    std::string configPath = WebAppManager::instance()->config()->getWebProcessConfigPath().toStdString();
    bool configOk = readJsonFromFile(configPath, webProcessEnvironment);

    if (!configOk || webProcessEnvironment.isNull()) {
        LOG_ERROR(MSGID_WEBPROCESSENV_READ_FAIL, 1, PMLOGKS("PATH", configPath.c_str()), "JSON parsging failed");
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
                    QString qid = QString::fromStdString(id.asString());
                    m_webProcessGroupAppIDList.append(qid);
                    setWebProcessCacheProperty(value, qid);
                }
                auto trustLevel = value["trustLevel"];
                if (trustLevel.isString()) {
                    QString qtl = QString::fromStdString(trustLevel.asString());
                    m_webProcessGroupTrustLevelList.append(qtl);
                    setWebProcessCacheProperty(value, qtl);
                }

            }
        }
        m_maximumNumberOfProcesses = (m_webProcessGroupTrustLevelList.size() + m_webProcessGroupAppIDList.size());
    }

    LOG_INFO(MSGID_SET_WEBPROCESS_ENVIRONMENT, 3, PMLOGKFV("MAXIMUM_WEBPROCESS_NUMBER", "%u", m_maximumNumberOfProcesses),
            PMLOGKFV("GROUP_TRUSTLEVELS_COUNT", "%d", m_webProcessGroupTrustLevelList.size()),
            PMLOGKFV("GROUP_APP_IDS_COUNT", "%d", m_webProcessGroupAppIDList.size()), "");
}

void WebProcessManager::setWebProcessCacheProperty(const Json::Value &object, QString key)
{
    WebProcessInfo info = WebProcessInfo(0, 0);
    QString memoryCacheStr, codeCacheStr;
    auto memoryCache = object["memoryCache"];
    if (memoryCache.isString()) {
        memoryCacheStr = QString::fromStdString(memoryCache.asString());
        if (memoryCacheStr.contains("MB"))
            memoryCacheStr.remove(QString("MB"));

        if (memoryCacheStr.toUInt())
            info.memoryCacheSize = memoryCacheStr.toUInt();
    }
    auto codeCahe = object["codeCahe"];
    if (codeCahe.isString()) {
        codeCacheStr = QString::fromStdString(codeCahe.asString());
        if (codeCacheStr.contains("MB"))
            codeCacheStr.remove(QString("MB"));

        if (codeCacheStr.toUInt())
            info.codeCacheSize = codeCacheStr.toUInt();
    }

    m_webProcessInfoMap.insert(key, info);
}

QString WebProcessManager::getProcessKey(const ApplicationDescription* desc) const
{
    if (!desc)
        return QString();

    QString key;
    QStringList idList, trustLevelList;
    if (m_maximumNumberOfProcesses == 1)
        key = QStringLiteral("system");
    else if (m_maximumNumberOfProcesses == UINT_MAX) {
        if (desc->trustLevel() == "default" || desc->trustLevel() == "trusted")
            key = QStringLiteral("system");
        else
            key = desc->id().c_str();
    }
    else {
        for (int i = 0; i < m_webProcessGroupAppIDList.size(); i++) {
            QString appId = m_webProcessGroupAppIDList.at(i);
            if (appId.contains("*")) {
                appId.remove(QChar('*'));
                idList.append(appId.split(","));
                Q_FOREACH(QString id, idList) {
                    if (QString::fromUtf8(desc->id().c_str()).startsWith(id))
                        key = m_webProcessGroupAppIDList.at(i);
                }
            } else {
                idList.append(appId.split(","));
                Q_FOREACH(QString id, idList) {
                    if (!id.compare(desc->id().c_str()))
                        return m_webProcessGroupAppIDList.at(i);
                }
            }
        }
        if (!key.isEmpty())
            return key;

        for (int i = 0; i < m_webProcessGroupTrustLevelList.size(); i++) {
            QString trustLevel = m_webProcessGroupTrustLevelList.at(i);
            trustLevelList.append(trustLevel.split(","));
            Q_FOREACH(QString trust, trustLevelList) {
                if (!trust.compare(desc->trustLevel().c_str())) {
                    return m_webProcessGroupTrustLevelList.at(i);
                }
            }
        }
        key = QStringLiteral("system");
    }
    return key;
}

void WebProcessManager::killWebProcess(uint32_t pid)
{
    for(QMap<QString, WebProcessInfo>::iterator it = m_webProcessInfoMap.begin(); it != m_webProcessInfoMap.end(); it++) {
        if (it.value().webProcessPid == pid) {
            it.value().requestKill = false;
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
    for (QMap<QString, WebProcessInfo>::iterator it = m_webProcessInfoMap.begin(); it != m_webProcessInfoMap.end(); it++) {
        if (it.value().webProcessPid == pid) {
            LOG_INFO(MSGID_KILL_WEBPROCESS_DELAYED, 1, PMLOGKFV("PID", "%u", pid), "");
            it.value().requestKill = true;
            return;
        }
    }
}
