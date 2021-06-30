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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "PluginLibWrapper.h"
#include "PluginLibWrapperMock.h"
#include "PluginLoader.h"
#include "WebAppFactoryInterfaceMock.h"

TEST(PluginLoaderUnitTest, Load) {
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  std::string file = "libwebappmgr-default-plugin.so";
  void* fake_handle = &file;
  EXPECT_CALL(*lib_wrapper, Load(file))
      .Times(1)
      .WillRepeatedly(::testing::Return(fake_handle));
  PluginLoader loader(std::move(lib_wrapper));
  EXPECT_TRUE(loader.Load(file));
}

TEST(PluginLoaderUnitTest, LoadTwice) {
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  std::string file = "libwebappmgr-default-plugin.so";
  void* fake_handle = &file;
  EXPECT_CALL(*lib_wrapper, Load(file))
      .Times(1)
      .WillRepeatedly(::testing::Return(fake_handle));
  PluginLoader loader(std::move(lib_wrapper));
  EXPECT_TRUE(loader.Load(file));
  EXPECT_TRUE(loader.Load(file));
}

TEST(PluginLoaderUnitTest, LoadFail) {
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  std::string file = "libwebappmgr-default-plugin.so";
  void* fake_handle = nullptr;
  EXPECT_CALL(*lib_wrapper, Load(file))
      .Times(1)
      .WillRepeatedly(::testing::Return(fake_handle));
  PluginLoader loader(std::move(lib_wrapper));
  EXPECT_FALSE(loader.Load(file));
}

TEST(PluginLoaderUnitTest, GetAppType) {
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  std::string file = "libwebappmgr-default-plugin.so";
  std::string app_type = "default";
  void* fake_handle = &file;
  EXPECT_CALL(*lib_wrapper, Load(file))
      .Times(1)
      .WillRepeatedly(::testing::Return(fake_handle));
  EXPECT_CALL(*lib_wrapper, GetAppType(fake_handle))
      .Times(1)
      .WillRepeatedly(::testing::Return(app_type));
  PluginLoader loader(std::move(lib_wrapper));
  ASSERT_TRUE(loader.Load(file));
  EXPECT_EQ(loader.GetAppType(file), app_type);
}

TEST(PluginLoaderUnitTest, GetInstance) {
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  std::string file = "libwebappmgr-default-plugin.so";
  void* fake_handle = &file;
  EXPECT_CALL(*lib_wrapper, Load(file))
      .Times(1)
      .WillRepeatedly(::testing::Return(fake_handle));
  WebAppFactoryInterfaceMock factory_interface_mock;
  EXPECT_CALL(*lib_wrapper, GetCreateInstanceFunction(fake_handle))
      .Times(1)
      .WillRepeatedly([&](void* handle) {
        return [&]() { return &factory_interface_mock; };
      });
  EXPECT_CALL(*lib_wrapper, GetDeleteInstanceFunction(testing::_)).Times(1);
  PluginLoader loader(std::move(lib_wrapper));
  ASSERT_TRUE(loader.Load(file));
  EXPECT_EQ(loader.GetWebAppFactoryInstance(file), &factory_interface_mock);
}

TEST(PluginLoaderUnitTest, GetInstanceTwice) {
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  std::string file = "libwebappmgr-default-plugin.so";
  void* fake_handle = &file;
  EXPECT_CALL(*lib_wrapper, Load(file))
      .Times(1)
      .WillRepeatedly(::testing::Return(fake_handle));
  WebAppFactoryInterfaceMock factory_interface_mock;
  EXPECT_CALL(*lib_wrapper, GetCreateInstanceFunction(fake_handle))
      .Times(1)
      .WillRepeatedly([&](void* handle) {
        return [&]() { return &factory_interface_mock; };
      });
  EXPECT_CALL(*lib_wrapper, GetDeleteInstanceFunction(testing::_)).Times(1);
  PluginLoader loader(std::move(lib_wrapper));
  ASSERT_TRUE(loader.Load(file));
  EXPECT_EQ(loader.GetWebAppFactoryInstance(file), &factory_interface_mock);
  EXPECT_EQ(loader.GetWebAppFactoryInstance(file), &factory_interface_mock);
}

TEST(PluginLoaderUnitTest, GetInstanceFail) {
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  EXPECT_CALL(*lib_wrapper, GetCreateInstanceFunction(testing::_)).Times(0);
  PluginLoader loader(std::move(lib_wrapper));
  EXPECT_EQ(loader.GetWebAppFactoryInstance("filename"), nullptr);
}

TEST(PluginLoaderUnitTest, Unload) {
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  std::string file = "libwebappmgr-default-plugin.so";
  void* fake_handle = &file;
  EXPECT_CALL(*lib_wrapper, Load(file))
      .Times(1)
      .WillRepeatedly(::testing::Return(fake_handle));
  EXPECT_CALL(*lib_wrapper, Unload(fake_handle)).Times(1);
  EXPECT_CALL(*lib_wrapper, GetDeleteInstanceFunction(testing::_)).Times(0);
  PluginLoader loader(std::move(lib_wrapper));
  ASSERT_TRUE(loader.Load(file));
  loader.Unload(file);
}

TEST(PluginLoaderUnitTest, UnloadWithActiveInstance) {
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  std::string file = "libwebappmgr-default-plugin.so";
  void* fake_handle = &file;
  EXPECT_CALL(*lib_wrapper, Load(file))
      .Times(1)
      .WillRepeatedly(::testing::Return(fake_handle));
  EXPECT_CALL(*lib_wrapper, Unload(fake_handle)).Times(1);
  WebAppFactoryInterfaceMock factory_interface_mock;
  EXPECT_CALL(*lib_wrapper, GetCreateInstanceFunction(fake_handle))
      .Times(1)
      .WillRepeatedly([&](void* handle) {
        return [&]() { return &factory_interface_mock; };
      });
  bool is_instance_deleted = false;
  EXPECT_CALL(*lib_wrapper, GetDeleteInstanceFunction(fake_handle))
      .Times(1)
      .WillRepeatedly([&](void* handle) {
        return [&](WebAppFactoryInterface*) { is_instance_deleted = true; };
      });
  PluginLoader loader(std::move(lib_wrapper));
  ASSERT_TRUE(loader.Load(file));
  ASSERT_EQ(loader.GetWebAppFactoryInstance(file), &factory_interface_mock);
  loader.Unload(file);
  EXPECT_TRUE(is_instance_deleted);
}

TEST(PluginLoaderUnitTest, UnloadNotExistPlugin) {
  auto lib_wrapper = std::make_unique<PluginLibWrapperMock>();
  std::string file = "libwebappmgr-default-plugin.so";
  void* fake_handle = &file;
  EXPECT_CALL(*lib_wrapper, Unload(fake_handle)).Times(0);
  EXPECT_CALL(*lib_wrapper, GetDeleteInstanceFunction(testing::_)).Times(0);
  PluginLoader loader(std::move(lib_wrapper));
  loader.Unload(file);
}
