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

#include <sstream>

#include <QByteArray>
#include <QFile>

#include "WebAppManager.h"


std::string PalmSystemBase::getDeviceInfo(const std::string& name)
{
    std::string value;
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
    std::string localcountry;
    std::string smartServiceCountry;

    WebAppManager::instance()->getDeviceInfo("LocalCountry", localcountry);
    WebAppManager::instance()->getDeviceInfo("SmartServiceCountry", smartServiceCountry);

    std::stringstream jss;
    jss << "{ \"country\": \"" << localcountry
        << "\", \"smartServiceCountry\": \"" << smartServiceCountry << "\" }";

    return QString::fromStdString(jss.str()); // FIXME: PalmSystem: qstr2stdstr
}

std::string PalmSystemBase::locale() const
{
    std::string systemlocale;
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
    // FIXME: PalmSystem: qstr2stdstr
    if (appId.toStdString() == WebAppManager::instance()->getContainerAppId())
        WebAppManager::instance()->setContainerAppReady(true);
}
