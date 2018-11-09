// Copyright (c) 2008-2018 LG Electronics, Inc.
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

#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <json/value.h>
#include <string>

bool readJsonFromString(const std::string &in, Json::Value& out);

bool readJsonFromFile(const std::string &path, Json::Value& out);

bool dumpJsonToString(const Json::Value &json, std::string &out);

#endif // JSONHELPER_H
