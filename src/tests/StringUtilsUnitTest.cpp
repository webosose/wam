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

#include "Utils.h"

#include <gtest/gtest.h>

TEST(StringUtils, TrimTest) {
  std::string test_string = " \f\n\r\t\v  test  \f\n\r\t\v ";
  EXPECT_EQ(util::trimString(test_string), "test");
}

TEST(StringUtils, TrimRightTest) {
  std::string test_string = "test  \f\n\r\t\v ";
  EXPECT_EQ(util::trimString(test_string), "test");
}

TEST(StringUtils, TrimLeftTest) {
  std::string test_string = " \f\n\r\t\v  test";
  EXPECT_EQ(util::trimString(test_string), "test");
}

TEST(StringUtils, NoTrimTest) {
  std::string test_string = "test";
  EXPECT_EQ(util::trimString(test_string), "test");
}
