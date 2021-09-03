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

#include <gtest/gtest.h>

#include "bcp47.h"

TEST(BCP47TestSuite, EmptyLanguage) {
  auto bcp47_pieces = BCP47::FromString("");
  ASSERT_FALSE(bcp47_pieces);
}

TEST(BCP47TestSuite, Lang) {
  auto bcp47_pieces = BCP47::FromString("en");
  ASSERT_TRUE(bcp47_pieces);
  EXPECT_TRUE(bcp47_pieces->HasLanguage());
  EXPECT_FALSE(bcp47_pieces->HasScript());
  EXPECT_FALSE(bcp47_pieces->HasRegion());
  EXPECT_STREQ(bcp47_pieces->Language().c_str(), "en");
}

TEST(BCP47TestSuite, OnlyRegion) {
  auto bcp47_pieces = BCP47::FromString("US");
  ASSERT_FALSE(bcp47_pieces);
}

TEST(BCP47TestSuite, ScriptRegionOnly) {
  auto bcp47_pieces = BCP47::FromString("Hant-CN");
  ASSERT_FALSE(bcp47_pieces);
}

TEST(BCP47TestSuite, LangScript) {
  auto bcp47_pieces = BCP47::FromString("zh-Hant");
  ASSERT_TRUE(bcp47_pieces);
}

TEST(BCP47TestSuite, LangRegion) {
  auto bcp47_pieces = BCP47::FromString("en-US");
  ASSERT_TRUE(bcp47_pieces);
  EXPECT_TRUE(bcp47_pieces->HasLanguage());
  EXPECT_TRUE(bcp47_pieces->HasRegion());
  EXPECT_STREQ(bcp47_pieces->Language().c_str(), "en");
  EXPECT_STREQ(bcp47_pieces->Region().c_str(), "US");
  EXPECT_FALSE(bcp47_pieces->HasScript());
}

TEST(BCP47TestSuite, LangRegionNumeric) {
  auto bcp47_pieces = BCP47::FromString("es-005");
  ASSERT_TRUE(bcp47_pieces);
  EXPECT_TRUE(bcp47_pieces->HasLanguage());
  EXPECT_TRUE(bcp47_pieces->HasRegion());
  EXPECT_STREQ(bcp47_pieces->Language().c_str(), "es");
  EXPECT_STREQ(bcp47_pieces->Region().c_str(), "005");
  EXPECT_FALSE(bcp47_pieces->HasScript());
}

TEST(BCP47TestSuite, LangScriptRegion) {
  auto bcp47_pieces = BCP47::FromString("zh-Hans-CN");
  ASSERT_TRUE(bcp47_pieces);
  EXPECT_TRUE(bcp47_pieces->HasLanguage());
  EXPECT_TRUE(bcp47_pieces->HasScript());
  EXPECT_TRUE(bcp47_pieces->HasRegion());
  EXPECT_STREQ(bcp47_pieces->Language().c_str(), "zh");
  EXPECT_STREQ(bcp47_pieces->Script().c_str(), "Hans");
  EXPECT_STREQ(bcp47_pieces->Region().c_str(), "CN");
}

TEST(BCP47TestSuite, LangScriptRegionNumeric) {
  auto bcp47_pieces = BCP47::FromString("es-Latn-005");
  ASSERT_TRUE(bcp47_pieces);
  EXPECT_TRUE(bcp47_pieces->HasLanguage());
  EXPECT_TRUE(bcp47_pieces->HasScript());
  EXPECT_TRUE(bcp47_pieces->HasRegion());
  EXPECT_STREQ(bcp47_pieces->Language().c_str(), "es");
  EXPECT_STREQ(bcp47_pieces->Script().c_str(), "Latn");
  EXPECT_STREQ(bcp47_pieces->Region().c_str(), "005");
}
