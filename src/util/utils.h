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

#ifndef UTIL_UTILS_H_
#define UTIL_UTILS_H_

#include <string>
#include <vector>

#include <json/json.h>

namespace util {

std::vector<std::string> GetErrorPagePaths(
    const std::string& error_page_location,
    const std::string& language = {});
std::string GetHostname(const std::string& url);

bool DoesPathExist(const std::string& path);
std::string ReadFile(const std::string& path);

std::string UriToLocal(const std::string& uri);
std::string LocalToUri(const std::string& uri);

std::string GetEnvVar(const char* env);

// STRING
bool StrToInt(const std::string str, int32_t& value);
int StrToIntWithDefault(const std::string& str, int default_value);
std::vector<std::string> SplitString(const std::string& str, char delimiter);
std::string TrimString(const std::string& str);
void ReplaceSubstr(std::string& in,
                   const std::string& to_search,
                   const std::string& replace_str = {});

// JSON
bool StringToJson(const std::string& str, Json::Value& value);
Json::Value StringToJson(const std::string& str);
std::string JsonToString(const Json::Value& value);

}  // namespace util

#endif  // UTIL_UTILS_H_
