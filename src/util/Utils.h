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

std::vector<std::string> getErrorPagePaths(const std::string& errorPageLocation, const std::string& language = "");
std::string getHostname(const std::string& url);

bool doesPathExist(const std::string& path);
std::string readFile(const std::string& path);

std::string uriToLocal(const std::string& uri);
std::string localToUri(const std::string& uri);

bool strToInt(const std::string str, int32_t& value);
int32_t strToIntWithDefault(const std::string& str, int32_t defaultValue);

void replaceAll(std::string& source, const std::string& what, const std::string& with);
std::string getEnvVar(const char* env);

#endif // UTILS_H
