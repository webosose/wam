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

#ifndef LOGMANAGERPMLOG_H
#define LOGMANAGERPMLOG_H

#include <PmLogLib.h>

#define LOG_CONTEXT "WAM"
#define LOG_APP_ID "APP_ID"

// convenience macro for logging just app id to a specific msgid
#define LOG_INFO_APPID(__msgid, __appid) \
    PmLogInfo(GetWAMPmLogContext(), __msgid, 1, PMLOGKS(LOG_APP_ID, __appid), "")
#define LOG_INFO_APPID_WITH_CLOCK(__msgid, __appid)                                        \
    PmLogInfoWithClock(GetWAMPmLogContext(), __msgid, 3, PMLOGKS("PerfType", "AppLaunch"), \
        PMLOGKS("PerfGroup", __appid),                                                     \
        PMLOGKS(LOG_APP_ID, __appid), "")

// Use these to log using PmLogLib v3 API
#define LOG_INFO(__msgid, ...) PmLogInfo(GetWAMPmLogContext(), __msgid, ##__VA_ARGS__)
#define LOG_INFO_WITH_CLOCK(__msgid, ...) PmLogInfoWithClock(GetWAMPmLogContext(), __msgid, ##__VA_ARGS__)
#define LOG_DEBUG(...) PmLogDebug(GetWAMPmLogContext(), ##__VA_ARGS__)
#define LOG_WARNING(__msgid, ...) PmLogWarning(GetWAMPmLogContext(), __msgid, ##__VA_ARGS__)
#define LOG_ERROR(__msgid, ...) PmLogError(GetWAMPmLogContext(), __msgid, ##__VA_ARGS__)
#define LOG_CRITICAL(__msgid, ...) PmLogCritical(GetWAMPmLogContext(), __msgid, ##__VA_ARGS__)

PmLogContext GetWAMPmLogContext();

#endif // LOGMANAGERPMLOG_H
