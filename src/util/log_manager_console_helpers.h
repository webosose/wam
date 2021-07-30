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

#ifndef UTIL_LOG_MANAGER_CONSOLE_HELPERS_H_
#define UTIL_LOG_MANAGER_CONSOLE_HELPERS_H_

// Based on macros from usr/include/PmLogMsg.h

#define LogMsgKV0(level, msgid, free_text_fmt, ...) \
  LogMsgImpl(level, msgid, free_text_fmt, ##__VA_ARGS__)

#define LogMsgKV1(level, msgid, key1, format1, value1, free_text_fmt, ...) \
  LogMsgImpl(level, msgid, "{" key1 ":" format1 "}" free_text_fmt, value1, \
             ##__VA_ARGS__)

#define LogMsgKV2(level, msgid, key1, format1, value1, key2, format2, value2, \
                  free_text_fmt, ...)                                         \
  LogMsgImpl(level, msgid,                                                    \
             "{" key1 ":" format1 "}, {" key2 ":" format2 "}" free_text_fmt,  \
             value1, value2, ##__VA_ARGS__)

#define LogMsgKV3(level, msgid, key1, format1, value1, key2, format2, value2, \
                  key3, format3, value3, free_text_fmt, ...)                  \
  LogMsgImpl(level, msgid,                                                    \
             "{" key1 ":" format1 "}, {" key2 ":" format2 "}, {" key3         \
             ":" format3 "} " free_text_fmt,                                  \
             value1, value2, value3, ##__VA_ARGS__);

#define LogMsgKV4(level, msgid, key1, format1, value1, key2, format2, value2,  \
                  key3, format3, value3, key4, format4, value4, free_text_fmt, \
                  ...)                                                         \
  LogMsgImpl(level, msgid,                                                     \
             "{" key1 ":" format1 "}, {" key2 ":" format2 "}, {" key3          \
             ":" format3 "}, {" key4 ":" format4 "} " free_text_fmt,           \
             value1, value2, value3, value4, ##__VA_ARGS__);

#define LogMsgKV5(level, msgid, key1, format1, value1, key2, format2, value2,  \
                  key3, format3, value3, key4, format4, value4, key5, format5, \
                  value5, free_text_fmt, ...)                                  \
  LogMsgImpl(level, msgid,                                                     \
             "{" key1 ":" format1 "}, {" key2 ":" format2 "}, {" key3          \
             ":" format3 "}, {" key4 ":" format4 "}, {" key5 ":" format5       \
             "} " free_text_fmt,                                               \
             value1, value2, value3, value4, value5, ##__VA_ARGS__);

#define LogMsgKV6(level, msgid, key1, format1, value1, key2, format2, value2,  \
                  key3, format3, value3, key4, format4, value4, key5, format5, \
                  value5, key6, format6, value6, free_text_fmt, ...)           \
  LogMsgImpl(level, msgid,                                                     \
             "{" key1 ":" format1 "}, {" key2 ":" format2 "}, {" key3          \
             ":" format3 "}, {" key4 ":" format4 "}, {" key5 ":" format5       \
             "}, {" key6 ":" format6 "} " free_text_fmt,                       \
             value1, value2, value3, value4, value5, value6, ##__VA_ARGS__);

#define LogMsgKV7(level, msgid, key1, format1, value1, key2, format2, value2,  \
                  key3, format3, value3, key4, format4, value4, key5, format5, \
                  value5, key6, format6, value6, key7, format7, value7,        \
                  free_text_fmt, ...)                                          \
  LogMsgImpl(                                                                  \
      level, msgid,                                                            \
      "{" key1 ":" format1 "}, {" key2 ":" format2 "}, {" key3 ":" format3     \
      "}, {" key4 ":" format4 "}, {" key5 ":" format5 "}, {" key6 ":" format6  \
      "}, {" key7 ":" format7 "} " free_text_fmt,                              \
      value1, value2, value3, value4, value5, value6, value7, ##__VA_ARGS__);

#define LogMsgKV8(level, msgid, key1, format1, value1, key2, format2, value2,  \
                  key3, format3, value3, key4, format4, value4, key5, format5, \
                  value5, key6, format6, value6, key7, format7, value7, key8,  \
                  format8, value8, free_text_fmt, ...)                         \
  LogMsgImpl(level, msgid,                                                     \
             "{" key1 ":" format1 "}, {" key2 ":" format2 "}, {" key3          \
             ":" format3 "}, {" key4 ":" format4 "}, {" key5 ":" format5       \
             "}, {" key6 ":" format6 "}, {" key7 ":" format7 "} , {" key8      \
             ":" format8 "} " free_text_fmt,                                   \
             value1, value2, value3, value4, value5, value6, value7, value8,   \
             ##__VA_ARGS__);

#define LogMsgKV9(level, msgid, key1, format1, value1, key2, format2, value2,  \
                  key3, format3, value3, key4, format4, value4, key5, format5, \
                  value5, key6, format6, value6, key7, format7, value7, key8,  \
                  format8, value8, key9, format9, value9, free_text_fmt, ...)  \
  LogMsgImpl(level, msgid,                                                     \
             "{" key1 ":" format1 "}, {" key2 ":" format2 "}, {" key3          \
             ":" format3 "}, {" key4 ":" format4 "}, {" key5 ":" format5       \
             "}, {" key6 ":" format6 "}, {" key7 ":" format7 "} , {" key8      \
             ":" format8 "} , {" key9 ":" format9 "} " free_text_fmt,          \
             value1, value2, value3, value4, value5, value6, value7, value8,   \
             value9, ##__VA_ARGS__);

#define LogMsgKV10(level, msgid, key1, format1, value1, key2, format2, value2, \
                   key3, format3, value3, key4, format4, value4, key5,         \
                   format5, value5, key6, format6, value6, key7, format7,      \
                   value7, key8, format8, value8, key9, format9, value9,       \
                   key10, format10, value10, free_text_fmt, ...)               \
  LogMsgImpl(level, msgid,                                                     \
             "{" key1 ":" format1 "}, {" key2 ":" format2 "}, {" key3          \
             ":" format3 "}, {" key4 ":" format4 "}, {" key5 ":" format5       \
             "}, {" key6 ":" format6 "}, {" key7 ":" format7 "} , {" key8      \
             ":" format8 "} , {" key9 ":" format9 "} , {" key10 ":" format10   \
             "} " free_text_fmt,                                               \
             value1, value2, value3, value4, value5, value6, value7, value8,   \
             value9, value10, ##__VA_ARGS__);

#endif  // UTIL_LOG_MANAGER_CONSOLE_HELPERS_H_
