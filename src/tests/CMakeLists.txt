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
    application_description_test.cc
    bcp47_test.cc
    clear_browsing_data_test.cc
    close_all_apps_test.cc
    device_info_test.cc
    get_web_process_size_test.cc
    json_helper_test.cc
    kill_app_test.cc
    launch_app_test.cc
    list_running_apps_test.cc
    log_control_test.cc
    network_status_test.cc
    palm_system_blink_test.cc
    pause_app_test.cc
    plugin_load_test.cc
    plugin_loader_test.cc
    set_inspector_enable_test.cc
    string_utils_test.cc
    touch_event_test.cc
    url_test.cc
    utils_test.cc
    web_app_manager_config_test.cc
    web_page_blink_test.cc
    web_process_created_test.cc
    mocks/blink_web_process_manager_mock.cc
    mocks/platform_module_factory_impl_mock.cc
    mocks/web_app_factory_manager_mock.cc
    mocks/web_app_manager_config_mock.cc
    mocks/web_app_window_factory_mock.cc
    mocks/web_view_factory_mock.cc
    mocks/web_view_mock_impl.cc
)

set(HEADERS
    mocks/base_mock_initializer.h
    mocks/blink_web_process_manager_mock.h
    mocks/platform_module_factory_impl_mock.h
    mocks/plugin_lib_wrapper_mock.h
    mocks/web_app_base_mock.h
    mocks/web_app_factory_interface_mock.h
    mocks/web_app_factory_manager_mock.h
    mocks/web_app_manager_config_mock.h
    mocks/web_app_window_factory_mock.h
    mocks/web_app_window_mock.h
    mocks/web_view_factory_mock.h
    mocks/web_view_mock.h
    mocks/web_view_mock_impl.h
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
