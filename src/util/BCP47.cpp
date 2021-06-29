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

#include "BCP47.h"

#include <memory>
#include <regex>
#include <string>

BCP47::BCP47() = default;
BCP47::~BCP47() = default;

std::unique_ptr<BCP47> BCP47::fromString(const std::string& bcp47String)
{
    if (bcp47String.empty())
        return nullptr;

    // We should parse language tag according to rfc5646
    // In general the tag format is: language - script - region
    // language - 2*3ALPHA - shortest ISO 639 code
    // script - 4ALPHA - ISO 15924 code
    // region - 2ALPHA / 3DIGIT  - ISO 3166-1 code

    // Actual values of all pocible pieces located at
    // https://www.iana.org/assignments/language-subtag-registry/language-subtag-registry
    // But we will just use regex for parsing

    // first group - mandatory 2 or 3 letters - represents language
    // second group - optional 4 letters - represents script
    // third group - optional 2 letters OR 3 digits - represents region
    std::regex rfc5646Regex(R"(^([a-z]{2,3})(?:[\-]{1}([A-z]{4}))?(?:[\-]{1}([A-Z]{2}|[0-9]{3}))?$)");
    std::smatch match;
    if (!std::regex_match(bcp47String, match, rfc5646Regex))
        return nullptr;

    auto result = std::unique_ptr<BCP47>(new BCP47());

    result->m_language = match[1];
    result->m_script = match[2];
    result->m_region = match[3];

    return result;
}

bool BCP47::hasLanguage() const { return !m_language.empty(); }
bool BCP47::hasScript() const { return !m_script.empty(); }
bool BCP47::hasRegion() const { return !m_region.empty(); }

std::string BCP47::language() const { return m_language; }
std::string BCP47::script() const { return m_script; }
std::string BCP47::region() const { return m_region; }
