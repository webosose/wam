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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <json/json.h>

#include "base_mock_initializer.h"
#include "utils.h"
#include "web_app_manager_service_luna.h"
#include "webos/webview_profile.h"

TEST(ClearBrowsingData, invalidValue) {
  const char json_parameter[] = R"({
        "types":[1]
    })";

  BaseMockInitializer<> mockInitializer;

  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));
  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  ASSERT_TRUE(reply.isMember("errorText"));
  ASSERT_TRUE(reply.isMember("errorCode"));
  EXPECT_FALSE(reply["returnValue"].asBool());
  EXPECT_EQ(kErrCodeClearDataBrawsingInvalidValue, reply["errorCode"].asInt());
}

TEST(ClearBrowsingData, emptyArray) {
  const char json_parameter[] = R"({
        "types":[]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  ASSERT_TRUE(reply.isMember("errorText"));
  ASSERT_TRUE(reply.isMember("errorCode"));
  EXPECT_FALSE(reply["returnValue"].asBool());
  EXPECT_EQ(kErrCodeClearDataBrawsingEmptyArray, reply["errorCode"].asInt());
}

TEST(ClearBrowsingData, unknownData) {
  const char json_parameter[] = R"({
        "types":["unknown"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  ASSERT_TRUE(reply.isMember("errorText"));
  ASSERT_TRUE(reply.isMember("errorCode"));
  EXPECT_FALSE(reply["returnValue"].asBool());
  EXPECT_EQ(kErrCodeClearDataBrawsingUnknownData, reply["errorCode"].asInt());
}

TEST(ClearBrowsingData, all) {
  const char json_parameter[] = R"({
        "types":["all"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(ClearBrowsingData, appcache) {
  const char json_parameter[] = R"({
        "types":["appcache"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(ClearBrowsingData, cache) {
  const char json_parameter[] = R"({
        "types":["cache"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(ClearBrowsingData, codecache) {
  const char json_parameter[] = R"({
        "types":["codecache"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(ClearBrowsingData, cookies) {
  const char json_parameter[] = R"({
        "types":["codecache"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(ClearBrowsingData, fileSystems) {
  const char json_parameter[] = R"({
        "types":["fileSystems"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(ClearBrowsingData, localStorage) {
  const char json_parameter[] = R"({
        "types":["localStorage"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(ClearBrowsingData, channelIDs) {
  const char json_parameter[] = R"({
        "types":["channelIDs"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(ClearBrowsingData, serviceWorkers) {
  const char json_parameter[] = R"({
        "types":["serviceWorkers"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(ClearBrowsingData, cacheStorage) {
  const char json_parameter[] = R"({
        "types":["cacheStorage"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(ClearBrowsingData, webSQL) {
  const char json_parameter[] = R"({
        "types":["webSQL"]
    })";

  BaseMockInitializer<> mockInitializer;
  Json::Value request;
  ASSERT_TRUE(util::StringToJson(json_parameter, request));

  const auto reply =
      WebAppManagerServiceLuna::Instance()->clearBrowsingData(request);
  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["returnValue"].asBool());
}
