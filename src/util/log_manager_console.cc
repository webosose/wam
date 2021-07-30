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

#include <stdarg.h>

#include <cstdio>
#include <unordered_map>

#include "log_manager_console.h"

namespace {
// Source: /usr/include/PmLogLib.h
enum PmLogLevelSubstitution {
  kPmLogLevel_None = -1,     /* no output */
  kPmLogLevel_Emergency = 0, /* system is unusable */
  kPmLogLevel_Alert = 1,     /* action must be taken immediately */
  kPmLogLevel_Critical = 2,  /* critical conditions */
  kPmLogLevel_Error = 3,     /* error conditions */
  kPmLogLevel_Warning = 4,   /* warning conditions */
  kPmLogLevel_Notice = 5,    /* normal but significant condition */
  kPmLogLevel_Info = 6,      /* informational */
  kPmLogLevel_Debug = 7      /* debug-level messages */
};

const std::string& GetLogLevelName(int32_t level) {
  static std::string error;
  static const std::unordered_map<PmLogLevelSubstitution, std::string>
      enum_strings = {
          {kPmLogLevel_None, "None"},     {kPmLogLevel_Emergency, "EMERGENCY"},
          {kPmLogLevel_Alert, "ALERT"},   {kPmLogLevel_Critical, "CRITICAL"},
          {kPmLogLevel_Error, "ERROR"},   {kPmLogLevel_Warning, "WARNING"},
          {kPmLogLevel_Notice, "NOTICE"}, {kPmLogLevel_Info, "INFO"},
          {kPmLogLevel_Debug, "DEBUG"},
      };
  auto it = enum_strings.find(static_cast<PmLogLevelSubstitution>(level));
  return it == enum_strings.end() ? error : it->second;
}

const char* ValueOrEmpty(const char* value) {
  return value ? value : "";
}

}  // namespace

void LogMsgImpl(const char* level, const char* msgid, const char* fmt, ...) {
  fprintf(stderr, "[%s] %s ", ValueOrEmpty(level), ValueOrEmpty(msgid));

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\n");
}

void LogString(int32_t level,
               const char* msgid,
               const char* kvpairs,
               const char* message) {
  fprintf(stderr, "[%s] %s %s %s\n", GetLogLevelName(level).c_str(),
          ValueOrEmpty(msgid), ValueOrEmpty(kvpairs), ValueOrEmpty(message));
}
