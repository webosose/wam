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

#include <memory>
#include <string>
#include <unordered_set>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ApplicationDescription.h"
#include "PluginLibWrapper.h"
#include "PluginLibWrapperMock.h"
#include "WebAppBaseMock.h"
#include "WebAppFactoryInterfaceMock.h"
#include "WebAppFactoryManagerImpl.h"

TEST(PluginLoadTest, Load) {
  std::string app_type = "default";
  void* fake_handle = &app_type;
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  EXPECT_CALL(
      *lib_wrapper,
      Load("/usr/lib/webappmanager/plugins/libwebappmgr-default-plugin.so"))
      .Times(1)
      .WillRepeatedly(::testing::Return(fake_handle));
  EXPECT_CALL(*lib_wrapper, GetAppType(fake_handle))
      .Times(1)
      .WillRepeatedly(::testing::Return(app_type));
  WebAppFactoryInterfaceMock factory_interface_mock;
  EXPECT_CALL(*lib_wrapper, GetCreateInstanceFunction(fake_handle))
      .Times(1)
      .WillRepeatedly([&](void* handle) {
        return [&]() { return &factory_interface_mock; };
      });
  EXPECT_CALL(*lib_wrapper, GetDeleteInstanceFunction(fake_handle)).Times(1);

  bool load_plugin_on_demand = false;
  WebAppFactoryManagerImpl::RemovableManagerPtr factory_manager =
      WebAppFactoryManagerImpl::testInstance("/usr/lib/webappmanager/plugins",
                                             "", load_plugin_on_demand,
                                             std::move(lib_wrapper));
  (void)factory_manager;
}

TEST(PluginLoadTest, LoadOnDemand) {
  std::string app_type = "default";
  void* fake_handle = &app_type;
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  EXPECT_CALL(*lib_wrapper, Load("testing::_")).Times(0);
  EXPECT_CALL(*lib_wrapper, GetAppType(fake_handle)).Times(0);
  WebAppFactoryInterfaceMock factory_interface_mock;
  EXPECT_CALL(*lib_wrapper, GetCreateInstanceFunction(fake_handle)).Times(0);

  bool load_plugin_on_demand = true;
  WebAppFactoryManagerImpl::RemovableManagerPtr factory_manager =
      WebAppFactoryManagerImpl::testInstance("/usr/lib/webappmanager/plugins",
                                             "", load_plugin_on_demand,
                                             std::move(lib_wrapper));
  (void)factory_manager;
}

TEST(PluginLoadTest, DefaultWebApp) {
  std::string app_type = "default";
  void* fake_handle = &app_type;
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  EXPECT_CALL(
      *lib_wrapper,
      Load("/usr/lib/webappmanager/plugins/libwebappmgr-default-plugin.so"))
      .Times(1)
      .WillRepeatedly(::testing::Return(fake_handle));
  EXPECT_CALL(*lib_wrapper, GetAppType(fake_handle))
      .Times(1)
      .WillRepeatedly(::testing::Return(app_type));
  WebAppFactoryInterfaceMock factory_interface_mock;
  WebAppBaseMock app_base_mock;
  EXPECT_CALL(factory_interface_mock,
              createWebApp(std::string("_WEBOS_WINDOW_TYPE_CARD"), testing::_))
      .Times(1)
      .WillRepeatedly(
          [&](const std::string&, std::shared_ptr<ApplicationDescription>) {
            return &app_base_mock;
          });
  EXPECT_CALL(*lib_wrapper, GetCreateInstanceFunction(fake_handle))
      .Times(1)
      .WillRepeatedly([&](void* handle) {
        return [&]() { return &factory_interface_mock; };
      });
  EXPECT_CALL(*lib_wrapper, GetDeleteInstanceFunction(fake_handle)).Times(1);

  bool load_plugin_on_demand = true;
  WebAppFactoryManagerImpl::RemovableManagerPtr factory_manager =
      WebAppFactoryManagerImpl::testInstance("/usr/lib/webappmanager/plugins",
                                             "", load_plugin_on_demand,
                                             std::move(lib_wrapper));
  ASSERT_NE(factory_manager, nullptr);
  WebAppBase* app_base = factory_manager->createWebApp(
      "_WEBOS_WINDOW_TYPE_CARD", nullptr, "default");
  EXPECT_EQ(app_base, &app_base_mock);
}

TEST(PluginLoadTest, CustomWebApp) {
  std::string app_type = "custom";
  void* fake_handle = &app_type;
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  EXPECT_CALL(
      *lib_wrapper,
      Load("/usr/lib/webappmanager/plugins/libwebappmgr-default-plugin.so"))
      .Times(1)
      .WillRepeatedly(::testing::Return(fake_handle));
  EXPECT_CALL(*lib_wrapper, GetAppType(fake_handle))
      .Times(1)
      .WillRepeatedly(::testing::Return(app_type));
  WebAppFactoryInterfaceMock factory_interface_mock;
  WebAppBaseMock app_base_mock;
  EXPECT_CALL(factory_interface_mock,
              createWebApp(std::string("_WEBOS_WINDOW_TYPE_CARD"), testing::_))
      .Times(1)
      .WillRepeatedly(
          [&](const std::string&, std::shared_ptr<ApplicationDescription>) {
            return &app_base_mock;
          });
  EXPECT_CALL(*lib_wrapper, GetCreateInstanceFunction(fake_handle))
      .Times(1)
      .WillRepeatedly([&](void* handle) {
        return [&]() { return &factory_interface_mock; };
      });
  EXPECT_CALL(*lib_wrapper, GetDeleteInstanceFunction(fake_handle)).Times(1);

  bool load_plugin_on_demand = true;
  WebAppFactoryManagerImpl::RemovableManagerPtr factory_manager =
      WebAppFactoryManagerImpl::testInstance(
          "/usr/lib/webappmanager/plugins", "extended:custom:minimal",
          load_plugin_on_demand, std::move(lib_wrapper));
  ASSERT_NE(factory_manager, nullptr);
  WebAppBase* app_base = factory_manager->createWebApp(
      "_WEBOS_WINDOW_TYPE_CARD", nullptr, "custom");
  EXPECT_EQ(app_base, &app_base_mock);
}

TEST(PluginLoadTest, NotAllowedCustomWebApp) {
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  EXPECT_CALL(*lib_wrapper, Load(testing::_)).Times(0);
  EXPECT_CALL(*lib_wrapper, GetAppType(testing::_)).Times(0);
  EXPECT_CALL(*lib_wrapper, GetCreateInstanceFunction(testing::_)).Times(0);
  EXPECT_CALL(*lib_wrapper, GetDeleteInstanceFunction(testing::_)).Times(0);

  bool load_plugin_on_demand = true;
  WebAppFactoryManagerImpl::RemovableManagerPtr factory_manager =
      WebAppFactoryManagerImpl::testInstance(
          "/usr/lib/webappmanager/plugins", "extended:minimal",
          load_plugin_on_demand, std::move(lib_wrapper));
  ASSERT_NE(factory_manager, nullptr);
  WebAppBase* app_base = factory_manager->createWebApp(
      "_WEBOS_WINDOW_TYPE_CARD", nullptr, "custom");
  EXPECT_EQ(app_base, nullptr);
}

TEST(PluginLoadTest, LoadTestPlugin) {
  auto lib_wrapper = std::make_unique<PluginLibWrapper>();
  bool load_plugin_on_demand = true;
  WebAppFactoryManagerImpl::RemovableManagerPtr factory_manager =
      WebAppFactoryManagerImpl::testInstance(
          "/usr/libexec/tests/webappmanager/plugins", "testplugin",
          load_plugin_on_demand, std::move(lib_wrapper));
  ASSERT_NE(factory_manager, nullptr);
  WebAppBase* app_base =
      factory_manager->createWebApp("", nullptr, "testplugin");
  ASSERT_NE(app_base, nullptr);
  EXPECT_EQ(app_base->appId(), "pluginTestID");
}
