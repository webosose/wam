// Copyright (c) 2021 LG Electronics, Inc.
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

#ifndef UTIL_LOG_MANAGER_CONSOLE_H_
#define UTIL_LOG_MANAGER_CONSOLE_H_

#include <cstdint>

#include "log_manager_console_helpers.h"

#ifndef __GNUC__
#define __attribute__(x)
#endif

void LogMsgImpl(const char* level, const char* msgid, const char* fmt, ...)
    __attribute__((format(printf, 3, 4)));

void LogString(int32_t level,
               const char* msgid,
               const char* kvpairs,
               const char* message);

#define LogMsg(level, msgid, kv_count, ...) \
  LogMsgKV##kv_count(level, msgid, __VA_ARGS__)

#define LogInfo(msgid, kv_count, ...) \
  LogMsg("INFO", msgid, kv_count, __VA_ARGS__)

#define LogDebug(...) LogMsg("DEBUG", nullptr, 0, __VA_ARGS__)

#define LogWarning(msgid, kv_count, ...) \
  LogMsg("WARNING", msgid, kv_count, __VA_ARGS__)

#define LogError(msgid, kv_count, ...) \
  LogMsg("ERROR", msgid, kv_count, __VA_ARGS__)

#define LogCritical(msgid, kv_count, ...) \
  LogMsg("CRITICAL", msgid, kv_count, __VA_ARGS__)

#define LOG_INFO(__msgid, ...) LogInfo(__msgid, __VA_ARGS__)
#define LOG_INFO_WITH_CLOCK(__msgid, ...) LogInfo(__msgid, __VA_ARGS__)
#define LOG_DEBUG(...) LogDebug(__VA_ARGS__)
#define LOG_WARNING(__msgid, ...) LogWarning(__msgid, __VA_ARGS__)
#define LOG_ERROR(__msgid, ...) LogError(__msgid, __VA_ARGS__)
#define LOG_CRITICAL(__msgid, ...) LogCritical(__msgid, __VA_ARGS__)

#define LOG_STRING(__level, __msgid, ...) \
  ::LogString(__level, __msgid, ##__VA_ARGS__)

#define PMLOGKFV(literal_key, literal_fmt, value) \
  literal_key, literal_fmt, value

#define PMLOGKS(literal_key, string_value) literal_key, "\"%s\"", string_value

#endif  // UTIL_LOG_MANAGER_CONSOLE_H_
