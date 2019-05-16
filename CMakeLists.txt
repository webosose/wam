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

cmake_minimum_required(VERSION 3.0.0)
project(WebAppMgr VERSION 1.0.0 LANGUAGES CXX)

set(PLATFORM_NAME "WebOS" CACHE STRING "Target platform name")
set(CHROMIUM_SRC_DIR "" CACHE PATH "Chromium source directory path")

option(USE_PMLOG "Use PmLog library (WebOS)")
option(USE_LUNA_SERVICE "Use Luna service (WebOS)")
option(USE_SNAPSHOT_BOOT "Use snapshot boot (WebOS)")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)

