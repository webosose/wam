# Copyright (c) 2019 LG Electronics, Inc.
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

set(IS_WEBOS FALSE)
set(IS_AGL FALSE)

string(TOLOWER ${PLATFORM_NAME} PLATFORM)

if(${PLATFORM} STREQUAL "webos")
    set(IS_WEBOS TRUE)
elseif(${PLATFORM} MATCHES ".*agl")
    set(IS_AGL TRUE)
else()
    message(FATAL_ERROR "Unsupported platform: ${PLATFORM}")
endif()

if(EXISTS ${CHROMIUM_SRC_DIR})
    set(CHROMIUM_INCLUDE_DIRS ${CHROMIUM_SRC_DIR})
    set(CHROMIUM_LDFLAGS -lcbe)
else()
    message(FATAL_ERROR "Invalid Chromium source dir: ${CHROMIUM_SRC_DIR}")
endif()

# Disable compiler-specific extensions
set(CMAKE_CXX_EXTENSIONS OFF)

set(WAM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WAM_DEST_DIR ${CMAKE_INSTALL_PREFIX})
set(WAM_PKGCONFIG_DIR "${CMAKE_SOURCE_DIR}/files/pkgconfig")
set(WAM_COMMON_DEFINES PRELOADMANAGER_ENABLED _REENTRANT)

message(STATUS "** Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "** Build platform: ${PLATFORM}")
message(STATUS "** Chromium src dir: ${CHROMIUM_SRC_DIR}")
message(STATUS "** Destination dir: ${WAM_DEST_DIR}")

find_package(Boost COMPONENTS filesystem REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(LIBXML REQUIRED libxml-2.0)
pkg_search_module(JSONCPP REQUIRED jsoncpp)

if(IS_WEBOS)
    set(CMAKE_AUTOMOC ON)
    if(USE_LUNA_SERVICE)
        pkg_search_module(LIBLUNAPREFS REQUIRED luna-prefs)
        list(APPEND WAM_COMMON_DEFINES HAS_LUNA_SERVICE)
    endif()
    if(USE_PMLOG)
        pkg_search_module(LIBPMLOG REQUIRED PmLogLib)
        list(APPEND WAM_COMMON_DEFINES HAS_LUNA_SERVICE)
    endif()
elseif(IS_AGL)
    pkg_search_module(LIBWM REQUIRED libwindowmanager)
    pkg_search_module(LIBHS REQUIRED libhomescreen)
    pkg_search_module(ILMCTRL REQUIRED ilmControl)
    list(APPEND WAM_COMMON_DEFINES HAS_AGL_SERVICE)
endif()

set(WAM_COMMON_CXX_FLAGS
    -Wall
    -Werror
    -Wno-psabi
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -fpermissive
    -funwind-tables
    -fno-rtti
)

###########################################################
# WebAppMgrCore Shared Library
###########################################################
set(WAM_CORE_LIB_NAME WebAppMgrCore)
set(WAM_CORE_SOURCES
    core/ApplicationDescription.cpp
    core/ContainerAppManager.cpp
    core/DeviceInfo.cpp
    core/PalmSystemBase.cpp
    core/PlugInService.cpp
    core/WebAppBase.cpp
    core/WebAppFactoryManager.cpp
    core/WebAppManager.cpp
    core/WebAppManagerConfig.cpp
    core/WebAppManagerService.cpp
    core/WebPageBase.cpp
    core/WebPageObserver.cpp
    core/WebProcessManager.cpp
    core/WindowTypes.cpp
    util/JsonHelper.cpp
    util/LogManager.cpp
    util/NetworkStatus.cpp
    util/NetworkStatusManager.cpp
    util/StringUtils.cpp
    util/Timer.cpp
    util/Url.cpp
    util/WebAppManagerUtils.cpp
)

set(WAM_CORE_HEADERS
    core/ApplicationDescription.h
    core/ContainerAppManager.h
    core/DeviceInfo.h
    core/PalmSystemBase.h
    core/PlatformModuleFactory.h
    core/PlugInService.h
    core/ServiceSender.h
    core/WebAppBase.h
    core/WebAppFactoryInterface.h
    core/WebAppFactoryManager.h
    core/WebAppManagerConfig.h
    core/WebAppManager.h
    core/WebAppManagerService.h
    core/WebPageBase.h
    core/WebPageObserver.h
    core/WebProcessManager.h
    core/WebRuntime.h
    core/WindowTypes.h
    util/JsonHelper.h
    util/LogManager.h
    util/LogMsgId.h
    util/NetworkStatus.h
    util/NetworkStatusManager.h
    util/ObserverList.h
    util/StringUtils.h
    util/Timer.h
    util/Url.h
    util/WebAppManagerUtils.h
)

set(WAM_CORE_INCLUDE_DIRS
    ${WAM_SOURCE_DIR}/core
    ${WAM_SOURCE_DIR}/util
    ${CHROMIUM_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
)

set(WAM_CORE_LIBRARIES
    Boost::filesystem
    ${GLIB_LDFLAGS}
    ${CHROMIUM_LDFLAGS}
)

if(IS_WEBOS)
    list(APPEND WAM_CORE_SOURCES plugin/WebAppFactoryManagerQtPlugin.cpp)
    list(APPEND WAM_CORE_LIBRARIES Qt5::Core)
    if(USE_LUNA_SERVICE)
        list(APPEND WAM_CORE_LIBRARIES ${LIBLUNAPREFS_LDFLAGS})
    endif()
    if(USE_PMLOG)
        list(APPEND WAM_CORE_SOURCES util/LogManagerPmLog.cpp)
        list(APPEND WAM_CORE_LIBRARIES ${LIBPMLOG_LDFLAGS})
    endif()
    if(USE_SNAPSHOT_BOOT)
        list(APPEND WAM_CORE_LIBRARIES -lsnapshot-boot)
    endif()
elseif(IS_AGL)
    list(APPEND WAM_CORE_SOURCES agl/WebAppFactoryManagerAGL.cpp)
endif()

add_library(wamcorelib SHARED ${WAM_CORE_HEADERS} ${WAM_CORE_SOURCES})
target_compile_options(wamcorelib PRIVATE ${WAM_COMMON_CXX_FLAGS})
target_compile_definitions(wamcorelib PRIVATE ${WAM_COMMON_DEFINES})
target_include_directories(wamcorelib PUBLIC ${WAM_CORE_INCLUDE_DIRS})
target_link_libraries(wamcorelib PRIVATE ${WAM_CORE_LIBRARIES})
set_target_properties(wamcorelib PROPERTIES
    OUTPUT_NAME ${WAM_CORE_LIB_NAME}
    VERSION "1.0.0"
    SOVERSION "1.0"
)
install(TARGETS wamcorelib LIBRARY DESTINATION lib)
install(FILES ${WAM_CORE_HEADERS} DESTINATION include/webappmanager)

# Generate and Install pkgconfig file for WebAppMgrCore library
set(PKGCONFIG_WAM_CORE_LIBS "-l${WAM_CORE_LIB_NAME}")
CONFIGURE_FILE(${WAM_PKGCONFIG_DIR}/WebAppMgrCore.pc.in "WebAppMgrCore.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/WebAppMgrCore.pc DESTINATION lib/pkgconfig)

###########################################################
# WebAppMgr Shared Library
###########################################################
set(WAM_LIB_NAME WebAppMgr)
set(WAM_LIB_SOURCES
    platform/PalmSystemWebOS.cpp
    platform/WebAppWayland.cpp
    platform/WebAppWaylandWindow.cpp
    platform/webengine/BlinkWebProcessManager.cpp
    platform/webengine/BlinkWebView.cpp
    platform/webengine/BlinkWebViewProfileHelper.cpp
    platform/webengine/PalmSystemBlink.cpp
    platform/webengine/WebPageBlink.cpp
    webos/PlatformModuleFactoryImpl.cpp
    webos/DeviceInfoImpl.cpp
)

set(WAM_LIB_HEADERS
    platform/PalmSystemWebOS.h
    platform/WebAppWayland.h
    platform/WebAppWaylandWindow.h
    platform/webengine/BlinkWebProcessManager.h
    platform/webengine/BlinkWebView.h
    platform/webengine/BlinkWebViewProfileHelper.h
    platform/webengine/PalmSystemBlink.h
    platform/webengine/WebPageBlinkDelegate.h
    platform/webengine/WebPageBlink.h
    platform/webengine/WebPageBlinkObserver.h
    webos/PlatformModuleFactoryImpl.h
)

set(WAM_LIB_DEFINES ${WAM_COMMON_DEFINES})

set(WAM_LIB_INCLUDE_DIRS
    ${WAM_SOURCE_DIR}/platform
    ${WAM_SOURCE_DIR}/platform/webengine
    ${WAM_SOURCE_DIR}/webos # TODO: webos-specific?
    ${LIBXML_INCLUDE_DIRS}
)

set(WAM_LIB_LIBRARIES
    ${GLIB_LDFLAGS}
    ${CHROMIUM_LDFLAGS}
    ${LIBWM_LDFLAGS}
    ${LIBHS_LDFLAGS}
    ${ILMCTRL_LDFLAGS}
    ${JSONCPP_LDFLAGS}
    ${LIBXML_LDFLAGS}
    Boost::filesystem
    wamcorelib
)

# TODO: Remove this after replacing pub/prv luna bus API with unified one
list (APPEND WAM_LIB_DEFINES SECURITY_COMPATIBILITY)

if(IS_WEBOS)
    list(APPEND WAM_LIB_SOURCES
        webos/PalmServiceBase.cpp
        webos/PlugInServiceLuna.cpp
        webos/ServiceSenderLuna.cpp
        webos/WebAppManagerServiceLuna.cpp
        webos/WebAppManagerServiceLunaImpl.cpp
        webos/WebRuntimeWebOS.cpp
    )
    list(APPEND WAM_LIB_HEADERS
        webos/DeviceInfoImpl.h
        webos/PalmServiceBase.h
        webos/PlugInServiceLuna.h
        webos/ServiceSenderLuna.h
        webos/WebAppManagerServiceLuna.h
        webos/WebAppManagerServiceLunaImpl.h
        webos/WebRuntimeWebOS.h
    )
elseif(IS_AGL)
		list(APPEND WAM_LIB_SOURCES
        agl/WebAppFactoryAGL.cpp
        agl/WebAppManagerServiceAGL.cpp
        agl/WebAppWaylandAGL.cpp
        agl/WebRuntimeAGL.cpp
    )
    list(APPEND WAM_LIB_HEADERS
        agl/WebAppFactoryAGL.h
        agl/WebAppManagerServiceAGL.h
        agl/WebAppWaylandAGL.h
        agl/WebRuntimeAGL.h
    )
endif()

add_library(wamlib SHARED ${WAM_LIB_HEADERS} ${WAM_LIB_SOURCES})
target_compile_options(wamlib PRIVATE ${WAM_COMMON_CXX_FLAGS})
target_compile_definitions(wamlib PRIVATE ${WAM_LIB_DEFINES})
target_include_directories(wamlib PUBLIC ${WAM_LIB_INCLUDE_DIRS})
target_link_libraries(wamlib PRIVATE ${WAM_LIB_LIBRARIES})
set_target_properties(wamlib PROPERTIES
    OUTPUT_NAME ${WAM_LIB_NAME}
    VERSION "1.0.0"
    SOVERSION "1.0"
)
install(TARGETS wamlib DESTINATION lib)
install(FILES ${WAM_LIB_HEADERS} DESTINATION include/webappmanager)

# Generate and Install pkgconfig file for WebAppMgr library
set(PKGCONFIG_WAM_LIBS "-l${WAM_LIB_NAME}")
CONFIGURE_FILE(${WAM_PKGCONFIG_DIR}/WebAppMgr.pc.in "WebAppMgr.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/WebAppMgr.pc DESTINATION lib/pkgconfig)

###########################################################
# WebAppMgr Executable
###########################################################
set(WAM_EXE_INCLUDE_DIRS
  ${WAM_SOURCE_DIR}/core
  ${WAM_SOURCE_DIR}/util
)
set(WAM_EXE_LIBRARIES
    ${CHROMIUM_LDFLAGS}
    Boost::filesystem
    wamlib
)
set(WAM_EXE_LD_FLAGS -Wl,--no-as-needed)

if(IS_WEBOS)
    if(USE_LUNA_SERVICE)
        list(APPEND WAM_EXE_INCLUDE_DIRS ${WAM_SOURCE_DIR}/webos)
    endif()
elseif(IS_AGL)
    list(APPEND WAM_EXE_INCLUDE_DIRS ${WAM_SOURCE_DIR}/agl)
endif()

add_executable(wamexec Main.cpp)
target_compile_options(wamexec PRIVATE ${WAM_COMMON_CXX_FLAGS})
target_compile_definitions(wamexec PRIVATE ${WAM_COMMON_DEFINES})
target_include_directories(wamexec PRIVATE ${WAM_EXE_INCLUDE_DIRS})
target_link_libraries(wamexec PRIVATE ${WAM_EXE_LIBRARIES})
set_target_properties(wamexec PROPERTIES OUTPUT_NAME "WebAppMgr")
set_property(TARGET wamexec APPEND_STRING PROPERTY LINK_FLAGS " ${WAM_EXE_LD_FLAGS}")
install(TARGETS wamexec DESTINATION bin)

###########################################################
# WebAppMgr Default Plugin (webos-only)
###########################################################
if(IS_WEBOS)
    set(WAM_PLUGIN_SOURCES
        plugin/WebAppFactoryLuna.cpp
        plugin/WebAppWaylandWebOS.cpp
    )
    set(WAM_PLUGIN_HEADERS
        plugin/WebAppFactoryLuna.h
        plugin/WebAppWaylandWebOS.h
    )
    set(WAM_PLUGIN_INCLUDE_DIRS
        ${WAM_SOURCE_DIR}/platform
        ${WAM_SOURCE_DIR}/platform/webengine
        ${WAM_SOURCE_DIR}/webos
        ${LIBXML_INCLUDE_DIRS}
    )
    set(WAM_PLUGIN_LIBRARIES wamlib Qt5::Core)

    add_library(wamplugin SHARED ${WAM_PLUGIN_HEADERS} ${WAM_PLUGIN_SOURCES})
    target_compile_options(wamplugin PRIVATE ${WAM_COMMON_CXX_FLAGS})
    target_compile_definitions(wamplugin PRIVATE ${WAM_COMMON_DEFINES})
    target_include_directories(wamplugin PRIVATE ${WAM_PLUGIN_INCLUDE_DIRS})
    target_link_libraries(wamplugin PRIVATE ${WAM_PLUGIN_LIBRARIES})
    set_target_properties(wamplugin PROPERTIES OUTPUT_NAME "webappmgr-default-plugin")
    install(TARGETS wamplugin DESTINATION lib/webappmanager/plugins)
    install(FILES ${WAM_PLUGIN_HEADERS} DESTINATION include/webappmanager)
endif()
