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

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

#include <json/json.h>

namespace util {

std::vector<std::string> getErrorPagePaths(const std::string& errorPageLocation, const std::string& language = "");
std::string getHostname(const std::string& url);

bool doesPathExist(const std::string& path);
std::string readFile(const std::string& path);

std::string uriToLocal(const std::string& uri);
std::string localToUri(const std::string& uri);

std::string getEnvVar(const char* env);

// STRING
bool strToInt(const std::string str, int32_t& value);
int strToIntWithDefault(const std::string& str, int defaultValue);
std::vector<std::string> splitString(const std::string &str, char delimiter);
std::string trimString(const std::string& str);
void replaceSubstr(std::string& in, const std::string& toSearch,
                   const std::string& replaceStr = "");

// JSON
bool stringToJson(const std::string& str, Json::Value& value);
Json::Value stringToJson(const std::string& str);
std::string jsonToString(const Json::Value& value);

} // namespace

#endif // UTILS_H
