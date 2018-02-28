# Copyright (c) 2015-2018 LG Electronics, Inc.
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

CONFIG = qt

BUILD_TYPE = release            # or debug
ENV_BUILD_TYPE = $$(BUILD_TYPE)
!isEmpty(ENV_BUILD_TYPE) {
    CONFIG -= release debug
    CONFIG += $$ENV_BUILD_TYPE
}

isEmpty(PREFIX) {
    warning("PREFIX undefined. Setting PREFIX from environment")
    PREFIX=$$(PREFIX)
    isEmpty(PREFIX) {
        warning("PREFIX not found. Library will be installed to INSTALL_ROOT/lib and headers to INSTALL_ROOT/include")
    }
}

CONFIG += link_pkgconfig
QT = core

################################################################################
# Decide MACHINE_NAME and PLATFORM

MACHINE_NAME = $$(MACHINE)

isEmpty(PLATFORM) {
    error("PLATFORM wasn't set")
}

DEFINES += $$PLATFORM

contains(PLATFORM, PLATFORM_WEBOS) {
    include(device.pri)
} else {
    isEmpty(MACHINE_NAME) {
        error(MACHINE_NAME should not be empty. $$MACHINE_NAME)
    }
}

################################################################################
# snapshot-boot

# TODO: Using SNAPSHOT_BOOT is discouraged. Please use CONFIG_BUILD in
#       your recipe.  For example,
#
#       EXTRA_QMAKEVARS_PRE +=  "CONFIG_BUILD+=snapshot_boot"
contains(CONFIG_BUILD, snapshot_boot) {
    CONFIG += snapshot_boot
    message($$MACHINE_NAME $$PLATFORM use snapshot boot)
}

snapshot_boot {
    LIBS += -lsnapshot-boot
}


################################################################################
# Path and CFLAGS

VPATH += \
    ./src \
    ./src/core \
    ./src/platform \
    ./src/platform/webengine \
    ./src/plugin \
    ./src/util \
    ./src/webos

PKGCONFIG += glib-2.0

INCLUDEPATH += $$VPATH

isEmpty(CHROMIUM_SRC_DIR) {
    error("CHROMIUM_SRC_DIR was not set")
}
INCLUDEPATH += $${CHROMIUM_SRC_DIR}

DEFINES += PRELOADMANAGER_ENABLED

QMAKE_CXXFLAGS += -fno-rtti -fno-exceptions -Wall -fpermissive -funwind-tables
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS += -rdynamic

QMAKE_CFLAGS += -Wno-psabi
QMAKE_CXXFLAGS += -Wno-psabi
QMAKE_LFLAGS += -Wl,--no-as-needed

QMAKE_CXXFLAGS += -Werror \

################################################################################
# .o and .moc

DESTDIR = ./$${BUILD_TYPE}-$${MACHINE_NAME}
OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
LIBS += -L$$DESTDIR

LIBS += -lcbe
