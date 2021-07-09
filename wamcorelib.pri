# Copyright (c) 2013-2018 LG Electronics, Inc.
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

TEMPLATE = lib

include(common.pri)

SOURCES += \
        ApplicationDescription.cpp \
        BCP47.cpp \
        DeviceInfo.cpp \
        JsonHelper.cpp \
        LogManager.cpp \
        LogManagerPmLog.cpp \
        NetworkStatus.cpp \
        NetworkStatusManager.cpp \
        PalmSystemBase.cpp \
        PlugInService.cpp \
        PluginLibWrapper.cpp \
        PluginLoader.cpp \
        StringUtils.cpp \
        Timer.cpp \
        TypeConverter.cpp \
        Url.cpp \
        Utils.cpp \
        WebAppBase.cpp \
        WebAppFactoryManagerImpl.cpp \
        WebAppManager.cpp \
        WebAppManagerConfig.cpp \
        WebAppManagerService.cpp \
        WebAppManagerUtils.cpp \
        WebPageBase.cpp \
        WebPageObserver.cpp \
        WebProcessManager.cpp

HEADERS += \
        ApplicationDescription.h \
        BCP47.h \
        DeviceInfo.h \
        JsonHelper.h \
        LogManager.h \
        LogManagerPmLog.h \
        LogMsgId.h \
        NetworkStatus.h \
        NetworkStatusManager.h \
        ObserverList.h \
        PalmSystemBase.h \
        PlatformModuleFactory.h \
        PlugInService.h \
        PluginLibWrapper.h \
        PluginLoader.h \
        ServiceSender.h \
        StringUtils.h \
        Timer.h \
        TypeConverter.h \
        Url.h \
        Utils.h \
        WebAppBase.h \
        WebAppFactoryInterface.h \
        WebAppFactoryManager.h \
        WebAppFactoryManagerImpl.h \
        WebAppManager.h \
        WebAppManagerConfig.h \
        WebAppManagerService.h \
        WebAppManagerUtils.h \
        WebPageBase.h \
        WebPageObserver.h \
        WebProcessManager.h \
        WebViewBase.h \
        WindowTypes.h

lttng {
    DEFINES += HAS_LTTNG
    SOURCES += pmtrace_webappmanager3_provider.c
    # LTTng uses weak symbols, so if these libraries are not present
    # at runtime, it does not prevent dynamic linking.
    # LTTng libraries are only installed in the system image for
    # development, so in the production runtime, these libraries are
    # not loaded and have no memory impact.
    PKGCONFIG += lttng-ust
}

TARGET = WebAppMgrCore

LIBS += -ldl

headers.files = $$HEADERS
headers.path = $${PREFIX}/include/webappmanager
target.path = $${PREFIX}/lib

# All of the three are needed to create to .pc file
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_NAME = libWebAppMgrCore
QMAKE_PKGCONFIG_DESCRIPTION = Web Application Manager
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

INSTALLS += target headers
