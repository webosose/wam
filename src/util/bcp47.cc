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

#include "bcp47.h"

#include <memory>
#include <regex>
#include <string>

BCP47::BCP47() = default;
BCP47::~BCP47() = default;

std::unique_ptr<BCP47> BCP47::FromString(const std::string& bcp47_string) {
  if (bcp47_string.empty())
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
  std::regex rfc5646_regex(
      R"(^([a-z]{2,3})(?:[\-]{1}([A-z]{4}))?(?:[\-]{1}([A-Z]{2}|[0-9]{3}))?$)");
  std::smatch match;
  if (!std::regex_match(bcp47_string, match, rfc5646_regex))
    return nullptr;

  auto result = std::unique_ptr<BCP47>(new BCP47());

  result->language_ = match[1];
  result->script_ = match[2];
  result->region_ = match[3];

  return result;
}

bool BCP47::HasLanguage() const {
  return !language_.empty();
}
bool BCP47::HasScript() const {
  return !script_.empty();
}
bool BCP47::HasRegion() const {
  return !region_.empty();
}

std::string BCP47::Language() const {
  return language_;
}
std::string BCP47::Script() const {
  return script_;
}
std::string BCP47::Region() const {
  return region_;
}
