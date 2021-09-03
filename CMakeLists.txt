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

cmake_minimum_required(VERSION 3.13.0)

project(WebAppMgr VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

include(webOS/webOS)
include(FindPkgConfig)

# Usage of LTTNG lead to runtime error in the zygote process
# due to creation of the additional threads.
# So, it is supressed.
# Cromium code at zygote_main_linux.cc contains following comment:
# "Check that the pre-sandbox initialization didn't spawn threads.
# It's not just our code which may do so - some system-installed libraries
# are known to be culprits, e.g. lttng."
unset(WEBOS_LTTNG_ENABLED CACHE)

webos_modules_init(1 6 3)
webos_component(1 0 2)

webos_build_system_bus_files(${CMAKE_SOURCE_DIR}/files/sysbus)

add_subdirectory(src)
