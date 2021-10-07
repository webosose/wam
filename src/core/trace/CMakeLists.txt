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

project(WebAppMgrTrace VERSION 1.0.0 DESCRIPTION "LTTng tracer of the Web Application Manager")

set (TARGET_SO_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

set(SOURCES
    web_app_manager_tracer_lib.cc
    pmtrace_webappmanager3_provider.c
)

set(HEADERS
    pmtrace_webappmanager3_provider.h
    web_app_manager_tracer_lib.h
)

set(TRACE_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLIB_INCLUDE_DIRS}
)

set(TRACE_LIBS
    ${GLIB_LDFLAGS}
    ${PMLOGLIB_LDFLAGS}
    ${LTTNG_LDFLAGS}
    dl
)

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${TRACE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${TRACE_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${TARGET_SO_VERSION})

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webappmanager)

webos_build_pkgconfig(${CMAKE_SOURCE_DIR}/files/pkgconfig/WebAppMgrCore)

set(WAM_TRACE_LIB_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/lib${PROJECT_NAME}.so.${TARGET_SO_VERSION} PARENT_SCOPE)
