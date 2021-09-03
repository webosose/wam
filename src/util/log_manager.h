// Copyright (c) 2014-2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef UTIL_LOG_MANAGER_H_
#define UTIL_LOG_MANAGER_H_

#ifdef DISABLE_LOGMANAGER

#define LOG_INFO_APPID(...) \
  do {                      \
  } while (0)
#define LOG_INFO(...) \
  do {                \
  } while (0)
#define LOG_DEBUG(...) \
  do {                 \
  } while (0)
#define LOG_WARNING(...) \
  do {                   \
  } while (0)
#define LOG_ERROR(...) \
  do {                 \
  } while (0)
#define LOG_CRITICAL(...) \
  do {                    \
  } while (0)
#define LOG_INFO_WITH_CLOCK(...) \
  do {                           \
  } while (0)
#define LOG_STRING(...) \
  do {                  \
  } while (0)

#else

#include "log_msg_id.h"

#ifdef DISABLE_PMLOG
#include "log_manager_console.h"
#else
#include "log_manager_pmlog.h"
#endif

#endif

#include <string>

class LogManager {
 public:
  static void SetLogControl(const std::string& keys, const std::string& value);
  static bool GetDebugEventsEnabled();
  static bool GetDebugBundleMessagesEnabled();
  static bool GetDebugMouseMoveEnabled();
};

#endif  // UTIL_LOG_MANAGER_H_
