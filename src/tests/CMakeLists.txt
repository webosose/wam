# Copyright (c) 2021 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

project(WebAppMgrUnitTest VERSION 1.0.0 DESCRIPTION "WAM unit tests bin")

pkg_search_module(GMOCKMAIN REQUIRED gmock_main)
pkg_search_module(GMOCK REQUIRED gmock)
pkg_search_module(GTEST REQUIRED gtest)

set(SOURCES
    ApplicationDescriptionUnitTest.cpp
    BCP47Tests.cpp
    ClearBrowsingDataComponentTest.cpp
    CloseAllAppsComponentTest.cpp
    DeviceInfoUnitTest.cpp
    GetWebProcessSizeTest.cpp
    JsonHelperUnitTest.cpp
    KillAppTest.cpp
    LaunchAppComponentTest.cpp
    ListRunningAppsTest.cpp
    LogControlComponentTest.cpp
    NetworkStatusUnitTest.cpp
    PalmSystemBlinkTests.cpp
    PauseAppTest.cpp
    PluginLoadTest.cpp
    PluginLoaderUnitTest.cpp
    SetInspectorEnableTest.cpp
    StringUtilsUnitTest.cpp
    TouchEventTest.cpp
    UrlUnitTest.cpp
    UtilsTests.cpp
    WebAppManagerConfigUnitTest.cpp
    WebPageBlinkUnitTest.cpp
    WebProcessCreatedTest.cpp
    mocks/BlinkWebProcessManagerMock.cpp
    mocks/PlatformModuleFactoryImplMock.cpp
    mocks/WebAppFactoryManagerMock.cpp
    mocks/WebAppManagerConfigMock.cpp
    mocks/WebAppWindowFactoryMock.cpp
    mocks/WebViewFactoryMock.cpp
    mocks/WebViewMockImpl.cpp
)

set(HEADERS
    mocks/BaseMockInitializer.h
    mocks/BlinkWebProcessManagerMock.h
    mocks/PlatformModuleFactoryImplMock.h
    mocks/PluginLibWrapperMock.h
    mocks/WebAppBaseMock.h
    mocks/WebAppFactoryInterfaceMock.h
    mocks/WebAppFactoryManagerMock.h
    mocks/WebAppManagerConfigMock.h
    mocks/WebAppWindowFactoryMock.h
    mocks/WebAppWindowMock.h
    mocks/WebViewFactoryMock.h
    mocks/WebViewMock.h
    mocks/WebViewMockImpl.h
)

set(TEST_BIN_INCLUDE_DIRS
    ${CHROMIUM_SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${GLIB_INCLUDE_DIRS}
    ${WAM_ROOT_SOURCE_DIR}/core
    ${WAM_ROOT_SOURCE_DIR}/platform
    ${WAM_ROOT_SOURCE_DIR}/platform/webengine
    ${WAM_ROOT_SOURCE_DIR}/util
    ${WAM_ROOT_SOURCE_DIR}/webos
)

set(TEST_BIN_LIBS
   WebAppMgrCore
   WebAppMgr
   ${GMOCKMAIN_LIBRARIES}
   ${GMOCK_LIBRARIES}
   ${GTEST_LIBRARIES}
)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${TEST_BIN_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${TEST_BIN_LIBS})
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/opt/webos/tests)
