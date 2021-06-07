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

#include "DeviceInfo.h"

class DeviceInfoTest : public ::testing::Test {
public:
    DeviceInfoTest() = default;
    ~DeviceInfoTest() override = default;

    DeviceInfo m_deviceInfo;
};

TEST_F(DeviceInfoTest, checkAsciStringDeviceInfo)
{
    const char *expectedValue = "ASCII String";
    QString actualValue;

    m_deviceInfo.setDeviceInfo("SimpleString", expectedValue);
    ASSERT_TRUE(m_deviceInfo.getDeviceInfo("SimpleString", actualValue));
    EXPECT_STREQ(expectedValue, actualValue.toStdString().c_str());
}

TEST_F(DeviceInfoTest, checkNotAsciStringDeviceInfo)
{
    const char *expectedValue = "String with not ASCII symbols (föö)";
    QString actualValue;

    m_deviceInfo.setDeviceInfo("ComplexString", expectedValue);
    ASSERT_TRUE(m_deviceInfo.getDeviceInfo("ComplexString", actualValue));
    EXPECT_STREQ(expectedValue, actualValue.toStdString().c_str());
}

TEST_F(DeviceInfoTest, checkSystemLanguage)
{
    const char *expectedValue = "ENG";
    QString actualValue;

    m_deviceInfo.setSystemLanguage(expectedValue);
    ASSERT_TRUE(m_deviceInfo.getSystemLanguage(actualValue));
    EXPECT_STREQ(expectedValue, actualValue.toStdString().c_str());
}

TEST_F(DeviceInfoTest, checkDisplayWidth)
{
    int expectedValue = 1900;
    int actualValue = 0;

    m_deviceInfo.setDisplayWidth(expectedValue);
    ASSERT_TRUE(m_deviceInfo.getDisplayWidth(actualValue));
    EXPECT_EQ(expectedValue, actualValue);
}

TEST_F(DeviceInfoTest, checkDisplayHeight)
{
    int expectedValue = 1080;
    int actualValue = 0;

    m_deviceInfo.setDisplayHeight(expectedValue);
    ASSERT_TRUE(m_deviceInfo.getDisplayHeight(actualValue));
    EXPECT_EQ(expectedValue, actualValue);
}
