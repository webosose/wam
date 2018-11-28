// Copyright (c) 2018 LG Electronics, Inc.
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

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <vector>
#include <string>

typedef std::vector<std::string> StringList;

StringList splitString(const std::string &str, char delimiter,
                       bool keepEmpty = false, bool ignCase = false);

void replaceSubstrings(std::string& in, const std::string& toSearch,
                       const std::string& replaceStr = "");

bool stringToUInt(const std::string& in, unsigned int& out);

std::string trimString(const std::string& str);

#endif // STRING_UTILS_H
