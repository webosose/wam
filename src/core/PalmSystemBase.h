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

#include <string>

#include <QVariant>

class PalmSystemBase {
public:
    PalmSystemBase() {}
    virtual ~PalmSystemBase() {}

protected:
    virtual std::string getDeviceInfo(const std::string& name);
    virtual QVariant getResource(QVariant a, QVariant b); // FIXME: PalmSystem: qvariant-less
    virtual std::string country() const;
    virtual std::string locale() const;
    virtual std::string localeRegion() const;
    virtual std::string phoneRegion() const;
    virtual void setContainerAppReady(const std::string& appId);
};
#endif
