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
#include "TypeConverter.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <json/json.h>

std::vector<std::string> splitString(const std::string &str, char delimiter)
{
    std::vector<std::string> resList;
    std::stringstream ss(str);
    std::string s;

    while (std::getline(ss, s, delimiter)) {
        resList.push_back(s);
    }

    return resList;
}

bool stringToJson(const std::string& str, Json::Value& value) {
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

    return reader->parse(str.c_str(), str.c_str() + str.size(), &value, nullptr);
}

void jsonToString(const Json::Value& json, std::string& str) {
    Json::StreamWriterBuilder builder;
    str = Json::writeString(builder, json);
}

bool stringToInt(const std::string& str, int& out)
{
    out = 0;
    if (str.empty())
        return true;

    std::stringstream ss(str);
    if (ss >> out)
        return true;
    return false;
}

std::string trimString(const std::string& str)
{
    std::string trimmed(str);
    boost::trim_right(trimmed);
    boost::trim_left(trimmed);

    return trimmed;
}

bool fileToJson(const std::string &path, Json::Value& out) {
    std::ifstream file(path);
    if (!file.is_open()) {
        out = Json::nullValue;
        return false;
    }
    std::ostringstream tmp;
    tmp << file.rdbuf();
    std::string s = tmp.str();

    return stringToJson(s, out);
}

void replaceSubstrings(std::string& in, const std::string& toSearch,
                       const std::string& replaceStr /* ="" */)
{
    size_t pos = in.find(toSearch);
    while (pos != std::string::npos) {
        in.replace(pos, toSearch.size(), replaceStr);
        pos = in.find(toSearch, pos + replaceStr.size());
    }
}

#endif //TYPECONVERTER_H