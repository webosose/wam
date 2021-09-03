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

project(WebAppMgr VERSION 1.0.0 DESCRIPTION "Web Application Manager library")

set(SOURCES
    palm_system_webos.cc
    web_app_wayland.cc
    web_app_wayland_window.cc
    web_app_window_impl.cc
    webengine/blink_web_process_manager.cc
    webengine/blink_web_view.cc
    webengine/blink_web_view_profile_helper.cc
    webengine/palm_system_blink.cc
    webengine/web_page_blink.cc
    webengine/web_view_impl.cc
    ${WAM_ROOT_SOURCE_DIR}/webos/device_info_impl.cc
    ${WAM_ROOT_SOURCE_DIR}/webos/palm_service_base.cc
    ${WAM_ROOT_SOURCE_DIR}/webos/platform_module_factory_impl.cc
    ${WAM_ROOT_SOURCE_DIR}/webos/plugin_service_luna.cc
    ${WAM_ROOT_SOURCE_DIR}/webos/service_sender_luna.cc
    ${WAM_ROOT_SOURCE_DIR}/webos/web_app_manager_service_luna.cc
    ${WAM_ROOT_SOURCE_DIR}/webos/web_app_manager_service_luna_impl.cc
)

set(HEADERS
    palm_system_webos.h
    web_app_wayland.h
    web_app_wayland_window.h
    web_app_window.h
    web_app_window_factory.h
    web_app_window_impl.h
    webengine/blink_web_process_manager.h
    webengine/blink_web_view.h
    webengine/blink_web_view_profile_helper.h
    webengine/palm_system_blink.h
    webengine/web_page_blink.h
    webengine/web_page_blink_delegate.h
    webengine/web_page_blink_observer.h
    webengine/web_view.h
    webengine/web_view_factory.h
    webengine/web_view_impl.h
    ${WAM_ROOT_SOURCE_DIR}/webos/device_info_impl.h
    ${WAM_ROOT_SOURCE_DIR}/webos/palm_service_base.h
    ${WAM_ROOT_SOURCE_DIR}/webos/platform_module_factory_impl.h
    ${WAM_ROOT_SOURCE_DIR}/webos/plugin_service_luna.h
    ${WAM_ROOT_SOURCE_DIR}/webos/service_sender_luna.h
    ${WAM_ROOT_SOURCE_DIR}/webos/web_app_manager_service_luna.h
    ${WAM_ROOT_SOURCE_DIR}/webos/web_app_manager_service_luna_impl.h
)

set(WAM_LIB_INCLUDE_DIRS
    ${CHROMIUM_SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/webengine
    ${JSONCPP_INCLUDE_DIRS}
    ${LIBLUNAPREFS_INCLUDE_DIRS}
    ${WAM_ROOT_SOURCE_DIR}/core
    ${WAM_ROOT_SOURCE_DIR}/util
    ${WAM_ROOT_SOURCE_DIR}/webos
)

set(WAM_LIB_LIBS
    ${JSONCPP_LDFLAGS}
    ${LIBLUNAPREFS_LDFLAGS}
    WebAppMgrCore
    lunaservice
)

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${WAM_LIB_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${WAM_LIB_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION "1.0.0" SOVERSION "1.0")

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/webappmanager)

set(PKGCONFIG_WAM_LIBS "-l${PROJECT_NAME}")
webos_build_pkgconfig(${CMAKE_SOURCE_DIR}/files/pkgconfig/WebAppMgr)
