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

#include <string>

#include <gtest/gtest.h>
#include <json/json.h>

#include "JsonHelper.h"

namespace {

const char* kTestJsonString =
    "{\n    \"id\": \"bareapp\",\n    \"returnValue\": true\n}";

}  // namespace

TEST(JsonValueFromString, CheckStrictMode) {
  std::string json_string = "5858h{igjmbn";
  Json::Value value;
  EXPECT_FALSE(util::JsonValueFromString(json_string, value));
}

TEST(JsonValueFromString, ParseError) {
  std::string json_string = "abvgd";
  Json::Value value;
  EXPECT_FALSE(util::JsonValueFromString(json_string, value));
  EXPECT_TRUE(value.isNull());
}

TEST(JsonValueFromString, ParseEmpty) {
  std::string json_string = "";
  Json::Value value;
  EXPECT_FALSE(util::JsonValueFromString(json_string, value));
  EXPECT_TRUE(value.isNull());
}

TEST(JsonValueFromString, ParseOk) {
  Json::Value value;
  ASSERT_TRUE(util::JsonValueFromString(kTestJsonString, value));
  ASSERT_TRUE(value.isObject());
  EXPECT_TRUE(value["returnValue"].asBool());
  EXPECT_STREQ(value["id"].asCString(), "bareapp");
}

TEST(StringFromJsonValue, StringFromJsonValue) {
  Json::Value object;
  object["id"] = "bareapp";
  object["returnValue"] = true;
  EXPECT_STREQ(util::StringFromJsonValue(object).c_str(), kTestJsonString);
}
