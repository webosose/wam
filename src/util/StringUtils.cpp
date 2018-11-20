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

#include "StringUtils.h"

#include <sstream>
#include <stdexcept>

// TODO: Initial simple implementation using standard
// API. Maybe will be replaced by a boost version in the future.
//
StringList splitString(const std::string &str, char delimiter,
                       bool keepEmptyParts, bool ignoreCase)
{
    std::istringstream tokenStream(str);
    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

void replaceSubstrings(std::string& in, const std::string& toSearch,
                       const std::string& replaceStr)
{
    size_t pos = in.find(toSearch);
    while (pos != std::string::npos) {
        in.replace(pos, toSearch.size(), replaceStr);
        pos = in.find(toSearch, pos + toSearch.size());
    }
}

bool stringToUInt(const std::string& in, unsigned int& out)
{
    try {
        unsigned long lresult = std::stoul(in, nullptr, 10);
        unsigned int result = lresult;
        if (result != lresult)
            return false;
        out = result;
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    }
}
