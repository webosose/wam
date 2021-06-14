
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

include(common.pri)

PKGCONFIG += gmock_main gmock gtest

VPATH += \
        ./src/tests \
        ./src/tests/mocks \

INCLUDEPATH += $$VPATH

SOURCES += $$files($$PWD/src/tests/*.cpp, true)
message($$SOURCES)

HEADERS += $$files($$PWD/src/tests/*.h, true)
message($$HEADERS)

target.path = $$WEBOS_INSTALL_TESTSDIR

LIBS += -lWebAppMgr -lWebAppMgrCore

TARGET = WebAppMgrUnitTest

INSTALLS += target
