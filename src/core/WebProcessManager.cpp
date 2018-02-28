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
#include <QJsonDocument>
#include <QJsonArray>

#include "ApplicationDescription.h"
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
            vmrss = strdup(&line[8]);
            break;
        }
    }

    fclose(fd);
    return vmrss.simplified();
}

void WebProcessManager::readWebProcessPolicy()
{
    QString webProcessConfigurationPath = WebAppManager::instance()->config()->getWebProcessConfigPath();

    QFile file(webProcessConfigurationPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString jsonStr = file.readAll();
    file.close();

    QJsonDocument webProcessEnvironment = QJsonDocument::fromJson(jsonStr.toUtf8());
    if (webProcessEnvironment.isNull()) {
        LOG_ERROR(MSGID_WEBPROCESSENV_READ_FAIL, 1, PMLOGKS("CONTENT", jsonStr.toStdString().c_str()), "");
        return;
    }

    bool createProcessForEachApp = webProcessEnvironment.object().value("createProcessForEachApp").toBool();
    if (createProcessForEachApp)
        m_maximumNumberOfProcesses = UINT_MAX;
    else {
        QJsonArray webProcessArray = webProcessEnvironment.object().value("webProcessList").toArray();
        Q_FOREACH (const QJsonValue &value, webProcessArray) {
            QJsonObject obj = value.toObject();
            if (!obj.value("id").isUndefined()) {
                QString id = obj.value("id").toString();

                m_webProcessGroupAppIDList.append(id);
                setWebProcessCacheProperty(obj, id);
            }
            else if (!obj.value("trustLevel").isUndefined()) {
                QString trustLevel = obj.value("trustLevel").toString();

                m_webProcessGroupTrustLevelList.append(trustLevel);
                setWebProcessCacheProperty(obj, trustLevel);
            }
        }
        m_maximumNumberOfProcesses = (m_webProcessGroupTrustLevelList.size() + m_webProcessGroupAppIDList.size());
    }

    LOG_INFO(MSGID_SET_WEBPROCESS_ENVIRONMENT, 3, PMLOGKFV("MAXIMUM_WEBPROCESS_NUMBER", "%u", m_maximumNumberOfProcesses),
            PMLOGKFV("GROUP_TRUSTLEVELS_COUNT", "%d", m_webProcessGroupTrustLevelList.size()),
            PMLOGKFV("GROUP_APP_IDS_COUNT", "%d", m_webProcessGroupAppIDList.size()), "");
}

void WebProcessManager::setWebProcessCacheProperty(QJsonObject object, QString key)
{
    WebProcessInfo info = WebProcessInfo(0, 0);
    QString memoryCacheStr, codeCacheStr;
    if (!object.value("memoryCache").isUndefined()) {
        memoryCacheStr = object.value("memoryCache").toString();
        if (memoryCacheStr.contains("MB"))
            memoryCacheStr.remove(QString("MB"));

        if (memoryCacheStr.toUInt())
            info.memoryCacheSize = memoryCacheStr.toUInt();
    }
    if (!object.value("codeCache").isUndefined()) {
        codeCacheStr = object.value("codeCache").toString();
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
