//  (c) 2021 LG Electronics, Inc.
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

// TODO: At the end of QT-less implementation defined in that module classes
// should be complete designed or deleted becase of alternative functionality
// is implemented by other team members. As that's a temporary solution, only one
// helper (util) package will be ised finally.
// Defined methods references (in depending codeunits) also should be corrected.
// That codeunit is a temporary solution will be deteted anyway.

#ifndef QTLESS_TEMPORARY_HELPERS_H
#define QTLESS_TEMPORARY_HELPERS_H

#include <fstream>
#include <iostream>
#include <sstream>

#include <json/json.h>

namespace qtless {
class FileHelper {
public:
    static bool tryGetFileEntry(const std::string& path, std::string& entry)
    {
        std::ifstream file;
        file.open(path, std::ios::ate);
        if (!file.is_open())
            return false;

        entry.clear();
        std::ostringstream ss;
        ss << file.rdbuf();
        entry = ss.str();

        return entry.length() > 0;
    }
}; // class FileHelper

class JsonHelper {
public:
    static bool tryParseJsonString(const std::string& strJson, Json::Value& objJson)
    {
        Json::CharReaderBuilder builder;
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        return reader->parse(
            strJson.c_str(),
            strJson.c_str() + strJson.size(),
            &objJson,
            nullptr);
    }

    static std::string jsonCppToString(const Json::Value& cppJson)
    {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "    ";
        builder["enableYAMLCompatibility"] = true;
        return Json::writeString(builder, cppJson);
    }

    static Json::Value jsonCppFromString(const std::string& strJson)
    {
        Json::Value result;
        tryParseJsonString(strJson, result);

        return result;
    }

}; // Class JsonHelper

class StringHelper {
public:
    static int strToInt(const std::string& str)
    {
        int result = 0;
        if (!str.empty()) {
            std::stringstream ss(str);
            ss >> result;
        }

        return result;
    }

    static std::string intToStr(const int& val)
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }
}; // Class StringHelper

} // Namespace qtless

#endif // QTLESS_TEMPORARY_HELPERS_H