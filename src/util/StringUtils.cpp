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

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

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

std::string trimString(const std::string& str)
{
    std::string trimmed(str);
    boost::trim_right(trimmed);
    boost::trim_left(trimmed);
    return trimmed;
}

template<typename Target>
Target stringTo(const std::string& str, bool *ok)
{
    try {
        Target t = boost::lexical_cast<Target>(str);
        if (ok != nullptr) *ok = true;
        return t;
    } catch (const std::exception &e) {
        std::cerr << "Failed to convert: '" << str
            << "' (err=" << e.what() << ")" << std::endl;
        if (ok != nullptr) *ok = false;
        return Target();
    }
}

template int stringTo<int>(const std::string& str, bool *success = nullptr);

template double stringTo<double>(const std::string& str, bool *success = nullptr);

template unsigned int stringTo<unsigned int>(const std::string& str, bool *success = nullptr);

