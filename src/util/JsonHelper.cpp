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

#include "JsonHelper.h"

#include <memory>
#include <fstream>
#include <sstream>

#include <json/reader.h>
#include <json/writer.h>

#include "LogManager.h"

// TODO: Log error/warn messages when something wrong happens(?)
bool readJsonFromString(const std::string &in, Json::Value& out) {
    Json::Value jsonObj;
    try {
        std::istringstream(in) >> jsonObj;
    } catch (const Json::RuntimeError &) {
        return false;
    }
    out = jsonObj;
    return true;
}

bool readJsonFromFile(const std::string &path, Json::Value& out) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }
    Json::Value jsonObj;
    try {
        in >> jsonObj;
    } catch (const Json::RuntimeError &) {
        return false;
    }
    out = jsonObj;
    return true;
}

// FIXME: Using default writter settings for now (e.g: indentation
// enable, etc).
bool dumpJsonToString(const Json::Value &json, std::string &out) {
    std::stringstream ss;
    try {
        ss << json;
    } catch (const Json::RuntimeError &) {
        return false;
    }
    out = ss.str();
    return true;
}

