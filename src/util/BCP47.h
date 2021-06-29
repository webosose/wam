// Copyright (c) 2021 LG Electronics, Inc.
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

#ifndef BCP47_H
#define BCP47_H

#include <string>
#include <memory>

class BCP47 {
public:
    static std::unique_ptr<BCP47> fromString(const std::string& bcp47String);

    bool hasLanguage() const;
    bool hasScript() const;
    bool hasRegion() const;

    std::string language() const;
    std::string script() const;
    std::string region() const;

    ~BCP47();

private:
    BCP47();

    std::string m_language;
    std::string m_script;
    std::string m_region;
};

#endif //  BCP47_H
