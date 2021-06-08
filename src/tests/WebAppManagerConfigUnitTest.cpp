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

#include "WebAppManagerConfigMock.h"

namespace
{
    const std::map<std::string, std::string> environmentVariables = {
        {"WAM_SUSPEND_DELAY_IN_MS", "15"},
        {"MAX_CUSTOM_SUSPEND_DELAY_IN_MS", "125"},
        {"LOAD_DYNAMIC_PLUGGABLE", "1"},
        {"POST_WEBPROCESS_CREATED_DISABLED", "1"},
        {"LAUNCH_TIME_CHECK", "1"},
        {"USE_SYSTEM_APP_OPTIMIZATION", "1"},
        {"ENABLE_LAUNCH_OPTIMIZATION", "1"},
        {"WEBAPPFACTORY", "Some.types.definition.string"},
        {"WEBAPPFACTORY_PLUGIN_PATH", "/usr/lib/webappmanager/alternate_plugins"},
        {"WEBPROCESS_CONFIGURATION_PATH", "/etc/wam/com.webos.wam.extended.json"},
        {"WAM_ERROR_PAGE", "https://www.lg.com/uk/support"},
        {"USER_SCRIPT_PATH", "webOSUserScripts/userScriptModified.js"},
        {"WAM_NAME", "Testing"}
    };

} // namespace

class WebAppManagerConfigTest : public ::testing::Test
{
public:
    WebAppManagerConfigTest()
        : m_configWithSetVariables(&environmentVariables)
    {
    }

    ~WebAppManagerConfigTest() override = default;

    WebAppManagerConfigMock m_configWithNoVariables;
    WebAppManagerConfigMock m_configWithSetVariables;
};

TEST_F(WebAppManagerConfigTest, checkDynamicPluggableLoadEnabledIfNotDefined)
{
    EXPECT_FALSE(m_configWithNoVariables.isDynamicPluggableLoadEnabled());
}

TEST_F(WebAppManagerConfigTest, checkDynamicPluggableLoadEnabledIfDefined)
{
    EXPECT_TRUE(m_configWithSetVariables.isDynamicPluggableLoadEnabled());
}

TEST_F(WebAppManagerConfigTest, checkPostWebProcessCreatedDisabledIfNotDefined)
{
    EXPECT_FALSE(m_configWithNoVariables.isPostWebProcessCreatedDisabled());
}

TEST_F(WebAppManagerConfigTest, checkPostWebProcessCreatedDisabledIfDefined)
{
    EXPECT_TRUE(m_configWithSetVariables.isPostWebProcessCreatedDisabled());
}

TEST_F(WebAppManagerConfigTest, checkLaunchTimeEnabledIfNotDefined)
{
    EXPECT_FALSE(m_configWithNoVariables.isCheckLaunchTimeEnabled());
}

TEST_F(WebAppManagerConfigTest, checkLaunchTimeEnabledIfDefined)
{
    EXPECT_TRUE(m_configWithSetVariables.isCheckLaunchTimeEnabled());
}

TEST_F(WebAppManagerConfigTest, checkUseSystemAppOptimizationIfNotDefined)
{
    EXPECT_FALSE(m_configWithNoVariables.isUseSystemAppOptimization());
}

TEST_F(WebAppManagerConfigTest, checkUseSystemAppOptimizationIfDefined)
{
    EXPECT_TRUE(m_configWithSetVariables.isUseSystemAppOptimization());
}

TEST_F(WebAppManagerConfigTest, checkLaunchOptimizationEnabledIfNotDefined)
{
    EXPECT_FALSE(m_configWithNoVariables.isLaunchOptimizationEnabled());
}

TEST_F(WebAppManagerConfigTest, checkLaunchOptimizationEnabledIfDefined)
{
    EXPECT_TRUE(m_configWithSetVariables.isLaunchOptimizationEnabled());
}

TEST_F(WebAppManagerConfigTest, checkSuspendDelayTimeIfNotDefined)
{
    EXPECT_EQ(1, m_configWithNoVariables.getSuspendDelayTime());
}

TEST_F(WebAppManagerConfigTest, checkSuspendDelayTimeIfDefined)
{
    EXPECT_EQ(15, m_configWithSetVariables.getSuspendDelayTime());
}

TEST_F(WebAppManagerConfigTest, checkMaxCustomSuspendDelayTimeIfNotDefined)
{
    EXPECT_EQ(0, m_configWithNoVariables.getMaxCustomSuspendDelayTime());
}

TEST_F(WebAppManagerConfigTest, checkMaxCustomSuspendDelayTimeIfDefined)
{
    EXPECT_EQ(125, m_configWithSetVariables.getMaxCustomSuspendDelayTime());
}

TEST_F(WebAppManagerConfigTest, checkWebAppFactoryPluginTypesIfNotDefined)
{
    EXPECT_STREQ("", m_configWithNoVariables.getWebAppFactoryPluginTypes().toStdString().c_str());
}

TEST_F(WebAppManagerConfigTest, checkWebAppFactoryPluginTypesIfDefined)
{
    EXPECT_STREQ("Some.types.definition.string", m_configWithSetVariables.getWebAppFactoryPluginTypes().toStdString().c_str());
}

TEST_F(WebAppManagerConfigTest, checkWebAppFactoryPluginPathIfNotDefined)
{
    EXPECT_STREQ("/usr/lib/webappmanager/plugins", m_configWithNoVariables.getWebAppFactoryPluginPath().toStdString().c_str());
}

TEST_F(WebAppManagerConfigTest, checkWebAppFactoryPluginPathIfDefined)
{
    EXPECT_STREQ("/usr/lib/webappmanager/alternate_plugins", m_configWithSetVariables.getWebAppFactoryPluginPath().toStdString().c_str());
}

TEST_F(WebAppManagerConfigTest, checkWebProcessConfigPathIfNotDefined)
{
    EXPECT_STREQ("/etc/wam/com.webos.wam.json", m_configWithNoVariables.getWebProcessConfigPath().toStdString().c_str());
}

TEST_F(WebAppManagerConfigTest, checkWebProcessConfigPathIfDefined)
{
    EXPECT_STREQ("/etc/wam/com.webos.wam.extended.json", m_configWithSetVariables.getWebProcessConfigPath().toStdString().c_str());
}

TEST_F(WebAppManagerConfigTest, checkErrorPageUrlIfNotDefined)
{
    EXPECT_STREQ("", m_configWithNoVariables.getErrorPageUrl().toStdString().c_str());
}

TEST_F(WebAppManagerConfigTest, checkErrorPageUrlIfDefined)
{
    EXPECT_STREQ("https://www.lg.com/uk/support", m_configWithSetVariables.getErrorPageUrl().toStdString().c_str());
}

TEST_F(WebAppManagerConfigTest, checkUserScriptPathIfNotDefined)
{
    EXPECT_STREQ("webOSUserScripts/userScript.js", m_configWithNoVariables.getUserScriptPath().toStdString().c_str());
}

TEST_F(WebAppManagerConfigTest, checkUserScriptPathIfDefined)
{
    EXPECT_STREQ("webOSUserScripts/userScriptModified.js", m_configWithSetVariables.getUserScriptPath().toStdString().c_str());
}

TEST_F(WebAppManagerConfigTest, checkNameIfNotDefined)
{
    EXPECT_STREQ("", m_configWithNoVariables.getName().c_str());
}

TEST_F(WebAppManagerConfigTest, checkNameIfDefined)
{
    EXPECT_STREQ("Testing", m_configWithSetVariables.getName().c_str());
}
