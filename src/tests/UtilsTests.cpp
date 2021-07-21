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

// NOTICE: the next ApplicationDescription fields are read from JSON
// but never used (at least in WebOS-OSE version of WAM):
// * ApplicationDescription::m_requestedWindowOrientation
//   (is read from "requestedWindowOrientation" tag)
// * ApplicationDescription::m_vendorExtension
//   (is read from "vendorExtension" tag)
//   is accessible via std::string& vendorExtension(), which is never called

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits>

#include "Utils.h"

// Check for search order:
// searchPath/resources/<language>/<script>/<region>/html/fileName
// searchPath/resources/<language>/<region>/html/fileName
// searchPath/resources/<language>/html/fileName
// searchPath/resources/html/fileName
// searchPath/fileName

namespace {

using ::testing::StrEq;

constexpr char path[] = "/usr/share/localization/wam/loaderror.html";
}

TEST(UtilsTestSuite, emptyErrorPagePath)
{
    const auto& paths = util::getErrorPagePaths("", "");
    ASSERT_TRUE(paths.empty());
}

TEST(UtilsTestSuite, nonEmptyErrorPagePath)
{
    const auto& paths = util::getErrorPagePaths(path, "");
    ASSERT_EQ(paths.size(), 2);
    EXPECT_THAT(paths[0], StrEq("/usr/share/localization/wam/resources/html/loaderror.html"));
    EXPECT_THAT(paths[1], ::testing::HasSubstr(path));
}

TEST(UtilsTestSuite, nonEmptyErrorPagePathWithLang)
{
    constexpr char langTag[] = "en";
    const auto& paths = util::getErrorPagePaths(path, langTag);
    ASSERT_EQ(paths.size(), 3);
    EXPECT_THAT(paths[0], StrEq("/usr/share/localization/wam/resources/en/html/loaderror.html"));
    EXPECT_THAT(paths[1], StrEq("/usr/share/localization/wam/resources/html/loaderror.html"));
    EXPECT_THAT(paths[2], StrEq(path));
}

TEST(UtilsTestSuite, nonEmptyErrorPagePathWithLangRegion)
{
    constexpr char langTag[] = "en-US";
    const auto& paths = util::getErrorPagePaths(path, langTag);
    ASSERT_EQ(paths.size(), 4);
    EXPECT_THAT(paths[0], StrEq("/usr/share/localization/wam/resources/en/US/html/loaderror.html"));
    EXPECT_THAT(paths[1], StrEq("/usr/share/localization/wam/resources/en/html/loaderror.html"));
    EXPECT_THAT(paths[2], StrEq("/usr/share/localization/wam/resources/html/loaderror.html"));
    EXPECT_THAT(paths[3], StrEq(path));
}

TEST(UtilsTestSuite, nonEmptyErrorPagePathWithLangRegionScript)
{
    constexpr char langTag[] = "zh-Hant-HK";
    const auto& paths = util::getErrorPagePaths(path, langTag);
    ASSERT_EQ(paths.size(), 5);
    EXPECT_THAT(paths[0], StrEq("/usr/share/localization/wam/resources/zh/Hant/HK/html/loaderror.html"));
    EXPECT_THAT(paths[1], StrEq("/usr/share/localization/wam/resources/zh/HK/html/loaderror.html"));
    EXPECT_THAT(paths[2], StrEq("/usr/share/localization/wam/resources/zh/html/loaderror.html"));
    EXPECT_THAT(paths[3], StrEq("/usr/share/localization/wam/resources/html/loaderror.html"));
    EXPECT_THAT(paths[4], StrEq(path));
}

TEST(UtilsTestSuite, replaceAll)
{
    std::string source("\\");
    util::replaceSubstr(source, "\\", "\\\\");
    EXPECT_STREQ("\\\\", source.c_str());
    source = "'";
    util::replaceSubstr(source, "'", "\\'");
    EXPECT_STREQ("\\'", source.c_str());
    source = "\n";
    util::replaceSubstr(source, "\n", "\\n");
    EXPECT_STREQ("\\n", source.c_str());
    source = "\r";
    util::replaceSubstr(source, "\r", "\\r");
    EXPECT_STREQ("\\r", source.c_str());
}

TEST(UtilsTestSuite, strToInt_IncorrectString)
{
    std::string str = "not a number";
    int value;
    bool result = util::strToInt(str, value);
    EXPECT_FALSE(result);
}

TEST(UtilsTestSuite, strToInt_CorrectPositive)
{
    std::string str = "10";
    const int32_t expected = 10;
    int value;
    bool result = util::strToInt(str, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, expected);
}

TEST(UtilsTestSuite, strToInt_CorrectNegative)
{
    std::string str = "-10";
    const int32_t expected = -10;
    int value;
    bool result = util::strToInt(str, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, expected);
}

TEST(UtilsTestSuite, strToInt_Overflow)
{
    std::string str = "+2147483648";
    int value;
    bool result = util::strToInt(str, value);
    EXPECT_FALSE(result);
}

TEST(UtilsTestSuite, strToInt_MAX_VALUE)
{
    std::string str = "+2147483647";
    const int expected = std::numeric_limits<int>::max();
    int value;
    bool result = util::strToInt(str, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, expected);
}

TEST(UtilsTestSuite, strToInt_Underflow)
{
    std::string str = "-2147483649";
    int value;
    bool result = util::strToInt(str, value);
    EXPECT_FALSE(result);
}

TEST(UtilsTestSuite, strToInt_MIN_VALUE)
{
    std::string str = std::to_string(std::numeric_limits<int>::min());
    const int32_t expected = std::numeric_limits<int>::min();
    int value;
    bool result = util::strToInt(str, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, expected);
}

TEST(UtilsTestSuite, strToInt_Mixed)
{
    std::string str = "21 some words";
    const int32_t expected = 21;
    int value;
    bool result = util::strToInt(str, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, expected);
}

TEST(UtilsTestSuite, strToInt_Mixed_Underscore)
{
    std::string str = "21_some_words";
    const int32_t expected = 21;
    int value;
    bool result = util::strToInt(str, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, expected);
}

TEST(UtilsTestSuite, strToInt_Mixed_Underscore_Suffix)
{
    std::string str = "some_words_21";
    int value;
    bool result = util::strToInt(str, value);
    EXPECT_FALSE(result);
}
