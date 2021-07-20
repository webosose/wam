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
    ApplicationDescription.cpp
    DeviceInfo.cpp
    PalmSystemBase.cpp
    PlugInService.cpp
    PluginLibWrapper.cpp
    PluginLoader.cpp
    WebAppBase.cpp
    WebAppFactoryManagerImpl.cpp
    WebAppManager.cpp
    WebAppManagerConfig.cpp
    WebAppManagerService.cpp
    WebPageBase.cpp
    WebPageObserver.cpp
    WebProcessManager.cpp
    ${WAM_ROOT_SOURCE_DIR}/util/BCP47.cpp
    ${WAM_ROOT_SOURCE_DIR}/util/LogManager.cpp
    ${WAM_ROOT_SOURCE_DIR}/util/NetworkStatus.cpp
    ${WAM_ROOT_SOURCE_DIR}/util/NetworkStatusManager.cpp
    ${WAM_ROOT_SOURCE_DIR}/util/Timer.cpp
    ${WAM_ROOT_SOURCE_DIR}/util/Url.cpp
    ${WAM_ROOT_SOURCE_DIR}/util/Utils.cpp
    ${WAM_ROOT_SOURCE_DIR}/util/WebAppManagerUtils.cpp
)

set(HEADERS
    ApplicationDescription.h
    DeviceInfo.h
    PalmSystemBase.h
    PlatformModuleFactory.h
    PlugInService.h
    PluginLibWrapper.h
    PluginLoader.h
    ServiceSender.h
    WebAppBase.h
    WebAppFactoryInterface.h
    WebAppFactoryManager.h
    WebAppFactoryManagerImpl.h
    WebAppManager.h
    WebAppManagerConfig.h
    WebAppManagerService.h
    WebPageBase.h
    WebPageObserver.h
    WebProcessManager.h
    WindowTypes.h
    ${WAM_ROOT_SOURCE_DIR}/util/BCP47.h
    ${WAM_ROOT_SOURCE_DIR}/util/BaseCheck.h
    ${WAM_ROOT_SOURCE_DIR}/util/LogManager.h
    ${WAM_ROOT_SOURCE_DIR}/util/LogMsgId.h
    ${WAM_ROOT_SOURCE_DIR}/util/NetworkStatus.h
    ${WAM_ROOT_SOURCE_DIR}/util/NetworkStatusManager.h
    ${WAM_ROOT_SOURCE_DIR}/util/Timer.h
    ${WAM_ROOT_SOURCE_DIR}/util/Url.h
    ${WAM_ROOT_SOURCE_DIR}/util/Utils.h
    ${WAM_ROOT_SOURCE_DIR}/util/WebAppManagerUtils.h
)

if(DISABLE_PMLOG)
    list(APPEND SOURCES ${WAM_ROOT_SOURCE_DIR}/util/LogManagerConsole.cpp)
    list(APPEND HEADERS ${WAM_ROOT_SOURCE_DIR}/util/LogManagerConsole.h
                        ${WAM_ROOT_SOURCE_DIR}/util/LogManagerConsoleHelpers.h)
else()
    list(APPEND SOURCES ${WAM_ROOT_SOURCE_DIR}/util/LogManagerPmLog.cpp)
    list(APPEND HEADERS ${WAM_ROOT_SOURCE_DIR}/util/LogManagerPmLog.h)
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
