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

# TODO: Remove this after replacing pub/prv luna bus API with unified one
DEFINES += SECURITY_COMPATIBILITY

SOURCES += \
    BlinkWebProcessManager.cpp \
    BlinkWebView.cpp \
    BlinkWebViewProfileHelper.cpp \
    DeviceInfoImpl.cpp \
    PalmServiceBase.cpp \
    PalmSystemBlink.cpp \
    PalmSystemWebOS.cpp \
    PlatformModuleFactoryImpl.cpp \
    PlugInServiceLuna.cpp \
    ServiceSenderLuna.cpp \
    WebAppManagerServiceLuna.cpp \
    WebAppManagerServiceLunaImpl.cpp \
    WebAppWayland.cpp \
    WebAppWaylandWindow.cpp \
    WebPageBlink.cpp \


HEADERS += \
    BlinkWebProcessManager.h \
    BlinkWebView.h \
    BlinkWebViewProfileHelper.h \
    DeviceInfoImpl.h \
    PalmServiceBase.h \
    PalmSystemBlink.h \
    PalmSystemWebOS.h \
    PlatformModuleFactoryImpl.h \
    PlugInServiceLuna.h \
    ServiceSenderLuna.h \
    WebAppManagerServiceLuna.h \
    WebAppManagerServiceLunaImpl.h \
    WebAppWayland.h \
    WebAppWaylandWindow.h \
    WebPageBlinkDelegate.h \
    WebPageBlink.h \

TARGET = WebAppMgr

LIBS += -llunaservice

headers.files = $$HEADERS
headers.path = $${PREFIX}/include/webappmanager
target.path = $${PREFIX}/lib

# All of the three are needed to create to .pc file
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_NAME = libWebAppMgr
QMAKE_PKGCONFIG_DESCRIPTION = Web Application Manager
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$headers.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

INSTALLS += target headers
