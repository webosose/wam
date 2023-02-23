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

#include "web_app_manager_config_mock.h"

namespace {
const std::map<std::string, std::string> kEnvironmentVariables = {
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
    {"WAM_NAME", "Testing"}};

}  // namespace

class WebAppManagerConfigTest : public ::testing::Test {
 public:
  WebAppManagerConfigTest()
      : config_with_set_variables_(&kEnvironmentVariables) {}

  ~WebAppManagerConfigTest() override = default;

  WebAppManagerConfigMock config_with_no_variables_;
  WebAppManagerConfigMock config_with_set_variables_;
};

TEST_F(WebAppManagerConfigTest, checkDynamicPluggableLoadEnabledIfNotDefined) {
  EXPECT_FALSE(config_with_no_variables_.IsDynamicPluggableLoadEnabled());
}

TEST_F(WebAppManagerConfigTest, checkDynamicPluggableLoadEnabledIfDefined) {
  EXPECT_TRUE(config_with_set_variables_.IsDynamicPluggableLoadEnabled());
}

TEST_F(WebAppManagerConfigTest,
       checkPostWebProcessCreatedDisabledIfNotDefined) {
  EXPECT_FALSE(config_with_no_variables_.IsPostWebProcessCreatedDisabled());
}

TEST_F(WebAppManagerConfigTest, checkPostWebProcessCreatedDisabledIfDefined) {
  EXPECT_TRUE(config_with_set_variables_.IsPostWebProcessCreatedDisabled());
}

TEST_F(WebAppManagerConfigTest, checkLaunchTimeEnabledIfNotDefined) {
  EXPECT_FALSE(config_with_no_variables_.IsCheckLaunchTimeEnabled());
}

TEST_F(WebAppManagerConfigTest, checkLaunchTimeEnabledIfDefined) {
  EXPECT_TRUE(config_with_set_variables_.IsCheckLaunchTimeEnabled());
}

TEST_F(WebAppManagerConfigTest, checkUseSystemAppOptimizationIfNotDefined) {
  EXPECT_FALSE(config_with_no_variables_.IsUseSystemAppOptimization());
}

TEST_F(WebAppManagerConfigTest, checkUseSystemAppOptimizationIfDefined) {
  EXPECT_TRUE(config_with_set_variables_.IsUseSystemAppOptimization());
}

TEST_F(WebAppManagerConfigTest, checkLaunchOptimizationEnabledIfNotDefined) {
  EXPECT_FALSE(config_with_no_variables_.IsLaunchOptimizationEnabled());
}

TEST_F(WebAppManagerConfigTest, checkLaunchOptimizationEnabledIfDefined) {
  EXPECT_TRUE(config_with_set_variables_.IsLaunchOptimizationEnabled());
}

TEST_F(WebAppManagerConfigTest, checkSuspendDelayTimeIfNotDefined) {
  EXPECT_EQ(1, config_with_no_variables_.GetSuspendDelayTime());
}

TEST_F(WebAppManagerConfigTest, checkSuspendDelayTimeIfDefined) {
  EXPECT_EQ(15, config_with_set_variables_.GetSuspendDelayTime());
}

TEST_F(WebAppManagerConfigTest, checkMaxCustomSuspendDelayTimeIfNotDefined) {
  EXPECT_EQ(0, config_with_no_variables_.GetMaxCustomSuspendDelayTime());
}

TEST_F(WebAppManagerConfigTest, checkMaxCustomSuspendDelayTimeIfDefined) {
  EXPECT_EQ(125, config_with_set_variables_.GetMaxCustomSuspendDelayTime());
}

TEST_F(WebAppManagerConfigTest, checkWebAppFactoryPluginTypesIfNotDefined) {
  EXPECT_STREQ("",
               config_with_no_variables_.GetWebAppFactoryPluginTypes().c_str());
}

TEST_F(WebAppManagerConfigTest, checkWebAppFactoryPluginTypesIfDefined) {
  EXPECT_STREQ(
      "Some.types.definition.string",
      config_with_set_variables_.GetWebAppFactoryPluginTypes().c_str());
}

TEST_F(WebAppManagerConfigTest, checkWebAppFactoryPluginPathIfNotDefined) {
  EXPECT_STREQ("/usr/lib/webappmanager/plugins",
               config_with_no_variables_.GetWebAppFactoryPluginPath().c_str());
}

TEST_F(WebAppManagerConfigTest, checkWebAppFactoryPluginPathIfDefined) {
  EXPECT_STREQ("/usr/lib/webappmanager/alternate_plugins",
               config_with_set_variables_.GetWebAppFactoryPluginPath().c_str());
}

TEST_F(WebAppManagerConfigTest, checkWebProcessConfigPathIfNotDefined) {
  EXPECT_STREQ("/etc/wam/com.webos.wam.json",
               config_with_no_variables_.GetWebProcessConfigPath().c_str());
}

TEST_F(WebAppManagerConfigTest, checkWebProcessConfigPathIfDefined) {
  EXPECT_STREQ("/etc/wam/com.webos.wam.extended.json",
               config_with_set_variables_.GetWebProcessConfigPath().c_str());
}

TEST_F(WebAppManagerConfigTest, checkErrorPageUrlIfNotDefined) {
  EXPECT_STREQ("", config_with_no_variables_.GetErrorPageUrl().c_str());
}

TEST_F(WebAppManagerConfigTest, checkErrorPageUrlIfDefined) {
  EXPECT_STREQ("https://www.lg.com/uk/support",
               config_with_set_variables_.GetErrorPageUrl().c_str());
}

TEST_F(WebAppManagerConfigTest, checkUserScriptPathIfNotDefined) {
  EXPECT_STREQ("webOSUserScripts/userScript.js",
               config_with_no_variables_.GetUserScriptPath().c_str());
}

TEST_F(WebAppManagerConfigTest, checkUserScriptPathIfDefined) {
  EXPECT_STREQ("webOSUserScripts/userScriptModified.js",
               config_with_set_variables_.GetUserScriptPath().c_str());
}

TEST_F(WebAppManagerConfigTest, checkNameIfNotDefined) {
  EXPECT_STREQ("", config_with_no_variables_.GetName().c_str());
}

TEST_F(WebAppManagerConfigTest, checkNameIfDefined) {
  EXPECT_STREQ("Testing", config_with_set_variables_.GetName().c_str());
}
