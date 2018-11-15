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

#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>

#include <json/reader.h>
#include <json/writer.h>

using std::cerr;
using std::endl;

#define LOGE cerr << __PRETTY_FUNCTION__ << ": "

bool readJsonFromString(const std::string &in, Json::Value& out) {
    out = Json::nullValue;
    if (in.empty())
        return true;
    Json::Value jsonObj;
    try {
        std::istringstream(in) >> jsonObj;
        out = std::move(jsonObj);
        return true;
    } catch (const Json::RuntimeError &e) {
        LOGE << " Failed to parse: " << e.what() << endl;
        return false;
    }
}

bool readJsonFromFile(const std::string &path, Json::Value& out) {
    out = Json::nullValue;
    std::ifstream in(path);
    if (!in.is_open()) {
        LOGE << " Failed to open file " << path << endl;
        return false;
    }
    Json::Value jsonObj;
    try {
        in >> jsonObj;
        out = std::move(jsonObj);
        return true;
    } catch (const Json::RuntimeError &e) {
        LOGE << " Failed to parse: " << e.what() << endl;
        return false;
    }
}

// FIXME: Using default writter settings for now (e.g: indentation
// enable, etc).
bool dumpJsonToString(const Json::Value &json, std::string &out) {
    out = {};
    std::stringstream ss;
    try {
        ss << json;
    } catch (const Json::RuntimeError &e) {
        LOGE << " Failed to dump: " << e.what() << endl;
        return false;
    }
    out = ss.str();
    return true;
}

