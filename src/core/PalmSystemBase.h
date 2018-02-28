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

#ifndef PALMSYSTEMBASE_H_
#define PALMSYSTEMBASE_H_

#include <QString>
#include <QVariant>

class PalmSystemBase {
public:
    PalmSystemBase() {}
    virtual ~PalmSystemBase() {}

protected:
    virtual QString getDeviceInfo(QString name);
    virtual QVariant getResource(QVariant a, QVariant b);
    virtual QString country() const;
    virtual QString locale() const;
    virtual QString localeRegion() const;
    virtual QString phoneRegion() const;
    virtual void setContainerAppReady(const QString& appId);
};
#endif
