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

project(WebAppMgrExec VERSION 1.0.0 DESCRIPTION "WAM")

set(WAM_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT DEFINED CHROMIUM_SRC_DIR)
    message(FATAL_ERROR "CHROMIUM_SRC_DIR was not set")
endif()
set(CHROMIUM_LDFLAGS -lcbe)

find_package(PkgConfig REQUIRED)
find_package(Boost COMPONENTS filesystem REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(JSONCPP REQUIRED jsoncpp)
pkg_search_module(LIBLUNAPREFS REQUIRED luna-prefs)

if(DISABLE_PMLOG)
    message(STATUS "PMLog disabled")
    add_compile_definitions(DISABLE_PMLOG)
else()
    pkg_search_module(PMLOGLIB REQUIRED PmLogLib)
endif()

if(WEBOS_LTTNG_ENABLED)
    pkg_search_module(LTTNG REQUIRED lttng-ust)
    add_compile_definitions(HAS_LTTNG)
endif()

webos_add_compiler_flags(ALL CXX -fno-rtti
                                 -fno-exceptions
                                 -Wall
                                 -fpermissive
                                 -funwind-tables
                                 -std=c++14
                                 -Wno-psabi
                                 -Werror
)

add_link_options(-Wl,--no-as-needed -Wno-psabi -rdynamic)

add_subdirectory(core)
add_subdirectory(platform)
add_subdirectory(plugin)
add_subdirectory(tests)
add_subdirectory(testplugin)

set(WAM_INCLUDE_DIRS
    ${GLIB_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/util
    ${CMAKE_CURRENT_SOURCE_DIR}/webos
)

set(WAM_LIBS
    ${CHROMIUM_LDFLAGS}
    ${LIBLUNAPREFS_LDFLAGS}
    ${PMLOGLIB_LDFLAGS}
    WebAppMgr
    WebAppMgrCore
)

add_executable(${PROJECT_NAME} wam_main.cc)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "WebAppMgr")
target_include_directories(${PROJECT_NAME} PRIVATE ${WAM_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${WAM_LIBS})
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
