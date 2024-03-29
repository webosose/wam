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

project(webappmgr-default-plugin VERSION 1.0.0 DESCRIPTION "Default WAM plugin")

set(SOURCES
    web_app_factory_luna.cc
    web_app_wayland_webos.cc
)

set(HEADERS
    web_app_factory_luna.h
    web_app_wayland_webos.h
)

set(WAM_PLUGIN_INCLUDE_DIRS
    ${CHROMIUM_INCLUDE_DIRS}
    ${WAM_ROOT_SOURCE_DIR}/core
    ${WAM_ROOT_SOURCE_DIR}/platform
    ${WAM_ROOT_SOURCE_DIR}/platform/webengine
    ${WAM_ROOT_SOURCE_DIR}/util
)

set(WAM_PLUGIN_LIBS WebAppMgr)

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${WAM_PLUGIN_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${WAM_PLUGIN_LIBS})
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/webappmanager/plugins)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webappmanager)
