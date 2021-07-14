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

#include <cstdio>
#include <unordered_map>

#include <stdarg.h>

#include "LogManagerConsole.h"

namespace {
// Source: /usr/include/PmLogLib.h
enum PmLogLevelSubstitution {
    kPmLogLevel_None = -1, /* no output */
    kPmLogLevel_Emergency = 0, /* system is unusable */
    kPmLogLevel_Alert = 1, /* action must be taken immediately */
    kPmLogLevel_Critical = 2, /* critical conditions */
    kPmLogLevel_Error = 3, /* error conditions */
    kPmLogLevel_Warning = 4, /* warning conditions */
    kPmLogLevel_Notice = 5, /* normal but significant condition */
    kPmLogLevel_Info = 6, /* informational */
    kPmLogLevel_Debug = 7 /* debug-level messages */
};

const std::string& getLogLevelName(int32_t level)
{
    static std::string error;
    static const std::unordered_map<PmLogLevelSubstitution, std::string> enumStrings = {
        { kPmLogLevel_None, "None" },
        { kPmLogLevel_Emergency, "EMERGENCY" },
        { kPmLogLevel_Alert, "ALERT" },
        { kPmLogLevel_Critical, "CRITICAL" },
        { kPmLogLevel_Error, "ERROR" },
        { kPmLogLevel_Warning, "WARNING" },
        { kPmLogLevel_Notice, "NOTICE" },
        { kPmLogLevel_Info, "INFO" },
        { kPmLogLevel_Debug, "DEBUG" },
    };
    auto it = enumStrings.find(static_cast<PmLogLevelSubstitution>(level));
    return it == enumStrings.end() ? error : it->second;
}

const char * valueOrEmpty(const char * value) {
    return value ? value : "";
}

} // namspace

void LogMsgImpl(const char* level, const char* msgid, const char* fmt, ...)
{
    fprintf(stderr, "[%s] %s ", valueOrEmpty(level), valueOrEmpty(msgid));

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
}

void LogString(int32_t level, const char* msgid, const char* kvpairs, const char* message)
{
    fprintf(stderr, "[%s] %s %s\n", getLogLevelName(level).c_str(), valueOrEmpty(kvpairs), valueOrEmpty(message));
}
