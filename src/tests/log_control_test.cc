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

#include <gtest/gtest.h>
#include <json/json.h>

#include "utils.h"
#include "web_app_manager_service_luna.h"

TEST(LogControl, SetEventOn) {
  const char json_parameters[] = R"({
        "keys":"event",
        "value":"on"
    })";

  Json::Value reuest;
  ASSERT_TRUE(util::StringToJson(json_parameters, reuest));

  const auto reply = WebAppManagerServiceLuna::Instance()->logControl(reuest);

  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("event"));
  ASSERT_TRUE(reply.isMember("bundleMessage"));
  ASSERT_TRUE(reply.isMember("mouseMove"));
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["event"].asBool());
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(LogControl, SetEventOff) {
  const char json_parameters[] = R"({
        "keys":"event",
        "value":"off"
    })";

  Json::Value reuest;
  ASSERT_TRUE(util::StringToJson(json_parameters, reuest));

  const auto reply = WebAppManagerServiceLuna::Instance()->logControl(reuest);

  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("event"));
  ASSERT_TRUE(reply.isMember("bundleMessage"));
  ASSERT_TRUE(reply.isMember("mouseMove"));
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_FALSE(reply["event"].asBool());
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(LogControl, SetBundleMessageOn) {
  const char json_parameters[] = R"({
        "keys":"bundleMessage",
        "value":"on"
    })";

  Json::Value reuest;
  ASSERT_TRUE(util::StringToJson(json_parameters, reuest));

  const auto reply = WebAppManagerServiceLuna::Instance()->logControl(reuest);

  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("event"));
  ASSERT_TRUE(reply.isMember("bundleMessage"));
  ASSERT_TRUE(reply.isMember("mouseMove"));
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["bundleMessage"].asBool());
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(LogControl, SetBundleMessageOff) {
  const char json_parameters[] = R"({
        "keys":"bundleMessage",
        "value":"off"
    })";

  Json::Value reuest;
  ASSERT_TRUE(util::StringToJson(json_parameters, reuest));

  const auto reply = WebAppManagerServiceLuna::Instance()->logControl(reuest);

  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("event"));
  ASSERT_TRUE(reply.isMember("bundleMessage"));
  ASSERT_TRUE(reply.isMember("mouseMove"));
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_FALSE(reply["bundleMessage"].asBool());
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(LogControl, SetMouseMoveOn) {
  const char json_parameters[] = R"({
        "keys":"mouseMove",
        "value":"on"
    })";

  Json::Value reuest;
  ASSERT_TRUE(util::StringToJson(json_parameters, reuest));

  const auto reply = WebAppManagerServiceLuna::Instance()->logControl(reuest);

  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("event"));
  ASSERT_TRUE(reply.isMember("bundleMessage"));
  ASSERT_TRUE(reply.isMember("mouseMove"));
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_TRUE(reply["mouseMove"].asBool());
  EXPECT_TRUE(reply["returnValue"].asBool());
}

TEST(LogControl, SetMouseMoveOff) {
  const char json_parameters[] = R"({
        "keys":"mouseMove",
        "value":"off"
    })";

  Json::Value reuest;
  ASSERT_TRUE(util::StringToJson(json_parameters, reuest));

  const auto reply = WebAppManagerServiceLuna::Instance()->logControl(reuest);

  ASSERT_TRUE(reply.isObject());
  ASSERT_TRUE(reply.isMember("event"));
  ASSERT_TRUE(reply.isMember("bundleMessage"));
  ASSERT_TRUE(reply.isMember("mouseMove"));
  ASSERT_TRUE(reply.isMember("returnValue"));
  EXPECT_FALSE(reply["mouseMove"].asBool());
  EXPECT_TRUE(reply["returnValue"].asBool());
}
