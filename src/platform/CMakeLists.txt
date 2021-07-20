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
    PalmSystemWebOS.cpp
    WebAppWayland.cpp
    WebAppWaylandWindow.cpp
    WebAppWindowImpl.cpp
    webengine/BlinkWebProcessManager.cpp
    webengine/BlinkWebView.cpp
    webengine/BlinkWebViewProfileHelper.cpp
    webengine/PalmSystemBlink.cpp
    webengine/WebPageBlink.cpp
    webengine/WebViewImpl.cpp
    ${WAM_ROOT_SOURCE_DIR}/webos/DeviceInfoImpl.cpp
    ${WAM_ROOT_SOURCE_DIR}/webos/PalmServiceBase.cpp
    ${WAM_ROOT_SOURCE_DIR}/webos/PlatformModuleFactoryImpl.cpp
    ${WAM_ROOT_SOURCE_DIR}/webos/PlugInServiceLuna.cpp
    ${WAM_ROOT_SOURCE_DIR}/webos/ServiceSenderLuna.cpp
    ${WAM_ROOT_SOURCE_DIR}/webos/WebAppManagerServiceLuna.cpp
    ${WAM_ROOT_SOURCE_DIR}/webos/WebAppManagerServiceLunaImpl.cpp
)

set(HEADERS
    PalmSystemWebOS.h
    WebAppWayland.h
    WebAppWaylandWindow.h
    WebAppWindow.h
    WebAppWindowFactory.h
    WebAppWindowImpl.h
    webengine/BlinkWebProcessManager.h
    webengine/BlinkWebView.h
    webengine/BlinkWebViewProfileHelper.h
    webengine/PalmSystemBlink.h
    webengine/WebPageBlink.h
    webengine/WebPageBlinkDelegate.h
    webengine/WebPageBlinkObserver.h
    webengine/WebView.h
    webengine/WebViewFactory.h
    webengine/WebViewImpl.h
    ${WAM_ROOT_SOURCE_DIR}/webos/DeviceInfoImpl.h
    ${WAM_ROOT_SOURCE_DIR}/webos/PalmServiceBase.h
    ${WAM_ROOT_SOURCE_DIR}/webos/PlatformModuleFactoryImpl.h
    ${WAM_ROOT_SOURCE_DIR}/webos/PlugInServiceLuna.h
    ${WAM_ROOT_SOURCE_DIR}/webos/ServiceSenderLuna.h
    ${WAM_ROOT_SOURCE_DIR}/webos/WebAppManagerServiceLuna.h
    ${WAM_ROOT_SOURCE_DIR}/webos/WebAppManagerServiceLunaImpl.h
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
