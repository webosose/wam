// Copyright (c) 2014-2019 LG Electronics, Inc.
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

#ifndef UTIL_WEB_APP_MANAGER_UTILS_H_
#define UTIL_WEB_APP_MANAGER_UTILS_H_

#include <string>
#include <vector>

class WebAppManagerUtils {
 public:
  static int UpdateAndGetCpuIdle(bool update_only = false);
  static bool SetGroups();
  static std::string TruncateURL(const std::string& url);

 private:
  static long Percentages(int cnt, int* out, long* now, long* old, long* diffs);
  static char* SkipToken(const char* p);
  static void Tokenize(std::string& str,
                       std::vector<std::string>& tokens,
                       const std::string& delimiters);
  static bool InGroup(std::string line, const char* user_name);
  static bool InVector(std::vector<std::string>& tokens, const char* arg);
};

#endif  // UTIL_WEB_APP_MANAGER_UTILS_H_
