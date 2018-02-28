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

#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QMap>
#include <QString>

class DeviceInfo {
public:
    virtual ~DeviceInfo() {}

    virtual bool getDisplayWidth(int& value);
    virtual void setDisplayWidth(int value);

    virtual bool getDisplayHeight(int& value);
    virtual void setDisplayHeight(int value);

    virtual bool getSystemLanguage(QString& value);
    virtual void setSystemLanguage(QString value);

    virtual bool getDeviceInfo(QString name, QString& value);
    virtual void setDeviceInfo(QString name, QString value);

private:
    QMap<QString, QString> m_deviceInfo;
};

#endif /* DEVICEINFO_H */
