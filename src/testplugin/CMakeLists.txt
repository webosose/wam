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

project(webappmgr-test-plugin VERSION 1.0.0 DESCRIPTION "WAM test plugin used to test the plugin loader.")

set(SOURCES TestPlugin.cpp)
set(HEADERS TestPlugin.h)

set(TEST_PLUGIN_INCLUDE_DIRS
    ${CHROMIUM_SRC_DIR}
    ${WAM_ROOT_SOURCE_DIR}/core
    ${WAM_ROOT_SOURCE_DIR}/tests/mocks
)

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${TEST_PLUGIN_INCLUDE_DIRS})
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/tests/webappmanager/plugins)
