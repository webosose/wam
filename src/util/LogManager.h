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

#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include "LogMsgId.h"

#if defined(DISABLE_LOGMANAGER) || !defined(HAS_PMLOG)
#include <cstdio>

#define LOG_MSG(level, ...)                \
    do {                                        \
        fprintf(stderr, "## (%s)[%s] ", level, __PRETTY_FUNCTION__); \
        fprintf(stderr, ##__VA_ARGS__);    \
        fputc('\n', stderr);    \
    } while (0)

#define LOG_INFO_APPID(...) \
    do {                    \
    } while (0)
#define LOG_INFO(...) //LOG_MSG("INFO", ##__VA_ARGS__)
#define LOG_DEBUG(...) LOG_MSG("DEBUG", ##__VA_ARGS__)
#define LOG_WARNING(...) //LOG_MSG("WARN", ##__VA_ARGS__)
#define LOG_ERROR(...) //LOG_MSG("ERROR", ##__VA_ARGS__)
#define LOG_CRITICAL(...) //LOG_MSG("CRITICAL", ##__VA_ARGS__)

#define PMLOGKFV(literal_key, literal_fmt, value) \
  literal_key, literal_fmt, value

#define PMLOGKS(literal_key, string_value) \
  literal_key, "\"%s\"", string_value

#define LOG_INFO_WITH_CLOCK(__msgid, ...)
#else

#include "LogManagerPmLog.h"

#endif

#include <string>

class LogManager {
public:
    static void setLogControl(const std::string& keys, const std::string& value);
    static bool getDebugEventsEnabled();
    static bool getDebugBundleMessagesEnabled();
    static bool getDebugMouseMoveEnabled();
};

#endif // LOGMANAGER_H
