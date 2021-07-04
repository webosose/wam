// Copyright (c) 2008-2021 LG Electronics, Inc.
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

#ifndef TYPECONVERTER_H
#define TYPECONVERTER_H

//TODO: Temporary solution: Make complete class design at the end of QT-less implementation

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <json/json.h>
std::vector<std::string> splitString(const std::string &str, char delimiter);
bool stringToJson(const std::string& str, Json::Value& value);
void jsonToString(const Json::Value& value, std::string& str);
bool stringToInt(const std::string& str, int& out);
std::string trimString(const std::string& str);
bool fileToJson(const std::string &path, Json::Value& out);
void replaceSubstrings(std::string& in, const std::string& toSearch,
                       const std::string& replaceStr = "");
#endif //TYPECONVERTER_H
