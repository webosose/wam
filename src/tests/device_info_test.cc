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

#include <gtest/gtest.h>

#include "device_info.h"

class DeviceInfoTest : public ::testing::Test {
 public:
  DeviceInfoTest() = default;
  ~DeviceInfoTest() override = default;

  DeviceInfo device_info_;
};

TEST_F(DeviceInfoTest, checkAsciStringDeviceInfo) {
  const char* expected_value = "ASCII String";
  std::string actual_value;

  device_info_.SetDeviceInfo("SimpleString", expected_value);
  ASSERT_TRUE(device_info_.GetDeviceInfo("SimpleString", actual_value));
  EXPECT_STREQ(expected_value, actual_value.c_str());
}

TEST_F(DeviceInfoTest, checkNotAsciStringDeviceInfo) {
  const char* expected_value = "String with not ASCII symbols (föö)";
  std::string actual_value;

  device_info_.SetDeviceInfo("ComplexString", expected_value);
  ASSERT_TRUE(device_info_.GetDeviceInfo("ComplexString", actual_value));
  EXPECT_STREQ(expected_value, actual_value.c_str());
}

TEST_F(DeviceInfoTest, checkSystemLanguage) {
  const char* expected_value = "ENG";
  std::string actual_value;

  device_info_.SetSystemLanguage(expected_value);
  ASSERT_TRUE(device_info_.GetSystemLanguage(actual_value));
  EXPECT_STREQ(expected_value, actual_value.c_str());
}

TEST_F(DeviceInfoTest, checkDisplayWidth) {
  int expected_value = 1900;
  int actual_value = 0;

  device_info_.SetDisplayWidth(expected_value);
  ASSERT_TRUE(device_info_.GetDisplayWidth(actual_value));
  EXPECT_EQ(expected_value, actual_value);
}

TEST_F(DeviceInfoTest, checkDisplayHeight) {
  int expected_value = 1080;
  int actual_value = 0;

  device_info_.SetDisplayHeight(expected_value);
  ASSERT_TRUE(device_info_.GetDisplayHeight(actual_value));
  EXPECT_EQ(expected_value, actual_value);
}
