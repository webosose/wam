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

#ifndef WEBPROCESSMANAGER_H
#define WEBPROCESSMANAGER_H

#include <list>

#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QString>

class ApplicationDescription;
class WebPageBase;
class WebAppBase;

class WebProcessManager {
public:
    WebProcessManager();
    virtual ~WebProcessManager() {}

    uint32_t getWebProcessProxyID(const ApplicationDescription* desc) const;
    uint32_t getWebProcessProxyID(uint32_t pid) const;
    QString getWebProcessMemSize(uint32_t pid) const; //change name from webProcessSize(uint32_t pid)
    void killWebProcess(uint32_t pid);
    void requestKillWebProcess(uint32_t pid);
    bool webProcessInfoMapReady();
    void setWebProcessCacheProperty(QJsonObject object, QString key); //change name from setWebProcessProperty()
    void readWebProcessPolicy(); //chane name from setWebProcessEnvironment()
    QString getProcessKey(const ApplicationDescription* desc) const; //change name from getKey()

    virtual QJsonObject getWebProcessProfiling() = 0;
    virtual uint32_t getWebProcessPID(const WebAppBase* app) const = 0;
    virtual void deleteStorageData(const QString& identifier) = 0;
    virtual uint32_t getInitialWebViewProxyID() const = 0;
    virtual void clearBrowsingData(const int removeBrowsingDataMask) = 0;
    virtual int maskForBrowsingDataType(const char* type) = 0;

protected:
    std::list<const WebAppBase*> runningApps();
    std::list<const WebAppBase*> runningApps(uint32_t pid);
    WebAppBase* findAppById(const QString& appId);
    WebAppBase* getContainerApp();

protected:
    class WebProcessInfo {
    public:
        // FIXME: Fix default cache values when WebKit defaults change.
        static const uint32_t DEFAULT_MEMORY_CACHE = 32;
        static const uint32_t DEFAULT_CODE_CACHE = 8;

        WebProcessInfo(uint32_t id, uint32_t pid,
            uint32_t memoryCache = DEFAULT_MEMORY_CACHE,
            uint32_t codeCache = DEFAULT_CODE_CACHE)
            : proxyID(id)
            , webProcessPid(pid)
            , numberOfApps(1)
            , memoryCacheSize(memoryCache)
            , codeCacheSize(codeCache)
            , requestKill(false)
        {
        }

        uint32_t proxyID;
        uint32_t webProcessPid;
        uint32_t numberOfApps;
        uint32_t memoryCacheSize;
        uint32_t codeCacheSize;
        bool requestKill;
    };
    QMap<QString, WebProcessInfo> m_webProcessInfoMap;

    uint32_t m_maximumNumberOfProcesses;
    QList<QString> m_webProcessGroupAppIDList;
    QList<QString> m_webProcessGroupTrustLevelList;
};

#endif /* WEBPROCESSMANAGER_H */
