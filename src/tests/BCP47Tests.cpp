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

#include "BCP47.h"

TEST(BCP47TestSuite, EmptyLanguage) {
  auto bcp47pieces = BCP47::fromString("");
  ASSERT_FALSE(bcp47pieces);
}

TEST(BCP47TestSuite, Lang) {
  auto bcp47pieces = BCP47::fromString("en");
  ASSERT_TRUE(bcp47pieces);
  EXPECT_TRUE(bcp47pieces->hasLanguage());
  EXPECT_FALSE(bcp47pieces->hasScript());
  EXPECT_FALSE(bcp47pieces->hasRegion());
  EXPECT_STREQ(bcp47pieces->language().c_str(), "en");
}

TEST(BCP47TestSuite, OnlyRegion) {
  auto bcp47pieces = BCP47::fromString("US");
  ASSERT_FALSE(bcp47pieces);
}

TEST(BCP47TestSuite, ScriptRegionOnly) {
  auto bcp47pieces = BCP47::fromString("Hant-CN");
  ASSERT_FALSE(bcp47pieces);
}


TEST(BCP47TestSuite, LangScript) {
  auto bcp47pieces = BCP47::fromString("zh-Hant");
  ASSERT_TRUE(bcp47pieces);
}

TEST(BCP47TestSuite, LangRegion) {
  auto bcp47pieces = BCP47::fromString("en-US");
  ASSERT_TRUE(bcp47pieces);
  EXPECT_TRUE(bcp47pieces->hasLanguage());
  EXPECT_TRUE(bcp47pieces->hasRegion());
  EXPECT_STREQ(bcp47pieces->language().c_str(), "en");
  EXPECT_STREQ(bcp47pieces->region().c_str(), "US");
  EXPECT_FALSE(bcp47pieces->hasScript());
}

TEST(BCP47TestSuite, LangRegionNumeric) {
  auto bcp47pieces = BCP47::fromString("es-005");
  ASSERT_TRUE(bcp47pieces);
  EXPECT_TRUE(bcp47pieces->hasLanguage());
  EXPECT_TRUE(bcp47pieces->hasRegion());
  EXPECT_STREQ(bcp47pieces->language().c_str(), "es");
  EXPECT_STREQ(bcp47pieces->region().c_str(), "005");
  EXPECT_FALSE(bcp47pieces->hasScript());
}

TEST(BCP47TestSuite, LangScriptRegion) {
  auto bcp47pieces = BCP47::fromString("zh-Hans-CN");
  ASSERT_TRUE(bcp47pieces);
  EXPECT_TRUE(bcp47pieces->hasLanguage());
  EXPECT_TRUE(bcp47pieces->hasScript());
  EXPECT_TRUE(bcp47pieces->hasRegion());
  EXPECT_STREQ(bcp47pieces->language().c_str(), "zh");
  EXPECT_STREQ(bcp47pieces->script().c_str(), "Hans");
  EXPECT_STREQ(bcp47pieces->region().c_str(), "CN");
}

TEST(BCP47TestSuite, LangScriptRegionNumeric) {
  auto bcp47pieces = BCP47::fromString("es-Latn-005");
  ASSERT_TRUE(bcp47pieces);
  EXPECT_TRUE(bcp47pieces->hasLanguage());
  EXPECT_TRUE(bcp47pieces->hasScript());
  EXPECT_TRUE(bcp47pieces->hasRegion());
  EXPECT_STREQ(bcp47pieces->language().c_str(), "es");
  EXPECT_STREQ(bcp47pieces->script().c_str(), "Latn");
  EXPECT_STREQ(bcp47pieces->region().c_str(), "005");
}
