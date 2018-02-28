// Copyright (c) 2012-2018 LG Electronics, Inc.
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

#include "PalmSystemBase.h"
#include "WebAppManager.h"

#include <QByteArray>
#include <QFile>

QString PalmSystemBase::getDeviceInfo(QString name)
{
    QString value;
    WebAppManager::instance()->getDeviceInfo(name, value);

    return value;
}

QVariant PalmSystemBase::getResource(QVariant a, QVariant b)
{
    QFile f(a.toString());
    if (!f.open(QIODevice::ReadOnly))
        return QVariant();

    QByteArray data = f.readAll();

    return QVariant(data.constData());
}

QString PalmSystemBase::country() const
{
    QString localcountry;
    QString smartServiceCountry;
    QString country;

    WebAppManager::instance()->getDeviceInfo("LocalCountry", localcountry);
    WebAppManager::instance()->getDeviceInfo("SmartServiceCountry", smartServiceCountry);

    country = QString("{ \"country\": \"%1\", \"smartServiceCountry\": \"%2\" }");
    country = country.arg(localcountry).arg(smartServiceCountry);

    return country;
}

QString PalmSystemBase::locale() const
{
    QString systemlocale;
    WebAppManager::instance()->getSystemLanguage(systemlocale);
    return systemlocale;
}

QString PalmSystemBase::localeRegion() const
{
    return QString("US");
}

QString PalmSystemBase::phoneRegion() const
{
    return QString("");
}

void PalmSystemBase::setContainerAppReady(const QString& appId)
{
    if (appId == WebAppManager::instance()->getContainerAppId())
        WebAppManager::instance()->setContainerAppReady(true);
}
