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

project(WebAppMgrCore VERSION 1.0.0 DESCRIPTION "Core of the Web Application Manager")

set(SOURCES
    application_description.cc
    device_info.cc
    palm_system_base.cc
    plugin_service.cc
    plugin_lib_wrapper.cc
    plugin_loader.cc
    web_app_base.cc
    web_app_factory_manager_impl.cc
    web_app_manager.cc
    web_app_manager_config.cc
    web_app_manager_service.cc
    web_page_base.cc
    web_page_observer.cc
    web_process_manager.cc
    ${WAM_ROOT_SOURCE_DIR}/util/bcp47.cc
    ${WAM_ROOT_SOURCE_DIR}/util/log_manager.cc
    ${WAM_ROOT_SOURCE_DIR}/util/network_status.cc
    ${WAM_ROOT_SOURCE_DIR}/util/network_status_manager.cc
    ${WAM_ROOT_SOURCE_DIR}/util/timer.cc
    ${WAM_ROOT_SOURCE_DIR}/util/url.cc
    ${WAM_ROOT_SOURCE_DIR}/util/utils.cc
    ${WAM_ROOT_SOURCE_DIR}/util/web_app_manager_utils.cc
)

set(HEADERS
    application_description.h
    device_info.h
    palm_system_base.h
    platform_module_factory.h
    plugin_service.h
    plugin_lib_wrapper.h
    plugin_loader.h
    service_sender.h
    web_app_base.h
    web_app_factory_interface.h
    web_app_factory_manager.h
    web_app_factory_manager_impl.h
    web_app_manager.h
    web_app_manager_config.h
    web_app_manager_service.h
    web_page_base.h
    web_page_observer.h
    web_process_manager.h
    window_types.h
    ${WAM_ROOT_SOURCE_DIR}/util/bcp47.h
    ${WAM_ROOT_SOURCE_DIR}/util/base_check.h
    ${WAM_ROOT_SOURCE_DIR}/util/log_manager.h
    ${WAM_ROOT_SOURCE_DIR}/util/log_msg_id.h
    ${WAM_ROOT_SOURCE_DIR}/util/network_status.h
    ${WAM_ROOT_SOURCE_DIR}/util/network_status_manager.h
    ${WAM_ROOT_SOURCE_DIR}/util/timer.h
    ${WAM_ROOT_SOURCE_DIR}/util/url.h
    ${WAM_ROOT_SOURCE_DIR}/util/utils.h
    ${WAM_ROOT_SOURCE_DIR}/util/web_app_manager_utils.h
)

if(DISABLE_PMLOG)
    list(APPEND SOURCES ${WAM_ROOT_SOURCE_DIR}/util/log_manager_console.cc)
    list(APPEND HEADERS ${WAM_ROOT_SOURCE_DIR}/util/log_manager_console.h
                        ${WAM_ROOT_SOURCE_DIR}/util/log_manager_console_helpers.h)
else()
    list(APPEND SOURCES ${WAM_ROOT_SOURCE_DIR}/util/log_manager_pmlog.cc)
    list(APPEND HEADERS ${WAM_ROOT_SOURCE_DIR}/util/log_manager_pmlog.h)
endif()

set(CORE_INCLUDE_DIRS
    ${CHROMIUM_SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${WAM_ROOT_SOURCE_DIR}
    ${WAM_ROOT_SOURCE_DIR}/util
)

set(CORE_LIBS
    ${CHROMIUM_LDFLAGS}
    ${GLIB_LDFLAGS}
    ${PMLOGLIB_LDFLAGS}
    Boost::filesystem
    dl
)

if(WEBOS_LTTNG_ENABLED)
    list(APPEND SOURCES pmtrace_webappmanager3_provider.c)
    list(APPEND HEADERS pmtrace_webappmanager3_provider.h)
    list(APPEND CORE_LIBS ${LTTNG_LDFLAGS})
endif()

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CORE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${CORE_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "1.0.0" SOVERSION "1.0")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION include/webappmanager)

set(PKGCONFIG_WAM_CORE_LIBS "-l${PROJECT_NAME}")
webos_build_pkgconfig(${CMAKE_SOURCE_DIR}/files/pkgconfig/WebAppMgrCore)
