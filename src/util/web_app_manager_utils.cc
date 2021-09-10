// Copyright (c) 2014-2021 LG Electronics, Inc.
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

#include "web_app_manager_utils.h"

#include <fcntl.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fstream>

int WebAppManagerUtils::UpdateAndGetCpuIdle(bool update_only) {
  static long old_cpu_time[4];
  long cur_cpu_time[4] = {0};
  long* cpu_time = cur_cpu_time;

  if (update_only)
    cpu_time = old_cpu_time;

  int fd;
  if ((fd = open("/proc/stat", O_RDONLY)) != -1) {
    char buffer[4096 + 1];
    int len = read(fd, buffer, sizeof(buffer) - 1);
    if (len > 0) {
      buffer[len] = '\0';
      char* p = SkipToken(buffer); /* "cpu" */
      cpu_time[0] = strtoul(p, &p, 0);
      cpu_time[1] = strtoul(p, &p, 0);
      cpu_time[2] = strtoul(p, &p, 0);
      cpu_time[3] = strtoul(p, &p, 0);
    }
    close(fd);
  }

  if (update_only)
    return 1000;  // max value of percentages();

  long cpu_diff[4];
  int cpu_states[4];
  Percentages(4, cpu_states, cur_cpu_time, old_cpu_time, cpu_diff);
  memcpy(old_cpu_time, cur_cpu_time, sizeof(old_cpu_time));

  return cpu_states[3];
}

char* WebAppManagerUtils::SkipToken(const char* p) {
  while (isspace(*p))
    p++;
  while (*p && !isspace(*p))
    p++;
  return (char*)p;
}

long WebAppManagerUtils::Percentages(int cnt,
                                     int* out,
                                     long* now,
                                     long* old,
                                     long* diffs) {
  long total_change = 0;
  long* dp = diffs;

  for (int i = 0; i < cnt; i++) {
    long change = *now - *old;
    if (change < 0)
      change = (int)((unsigned long)*now - (unsigned long)*old);
    total_change += (*dp++ = change);
    *old++ = *now++;
  }

  if (0 == total_change)
    total_change = 1;

  long half_total = total_change / 2l;
  for (int i = 0; i < cnt; i++)
    *out++ = static_cast<int>((*diffs++ * 1000 + half_total) / total_change);

  return total_change;
}

void WebAppManagerUtils::Tokenize(std::string& str,
                                  std::vector<std::string>& tokens,
                                  const std::string& delimiters) {
  std::string::size_type last_pos = str.find_first_not_of(delimiters, 0);
  std::string::size_type pos = str.find_first_of(delimiters, last_pos);

  while (std::string::npos != pos || std::string::npos != last_pos) {
    tokens.push_back(str.substr(last_pos, pos - last_pos));
    last_pos = str.find_first_not_of(delimiters, pos);
    pos = str.find_first_of(delimiters, last_pos);
  }
}

bool WebAppManagerUtils::InVector(std::vector<std::string>& tokens,
                                  const char* arg) {
  unsigned int i;
  int len;
  len = strlen(arg);

  for (i = 0; i < tokens.size(); i++) {
    int tlen = strlen(tokens[i].c_str());

    if (strncmp(arg, tokens[i].c_str(), (len > tlen) ? len : tlen) == 0) {
      return true;
    }
  }

  return false;
}

bool WebAppManagerUtils::InGroup(std::string line, const char* user_name) {
  // only tokenize the lines that have users in the groups.
  // empty groups have the last character as ":".
  size_t pos = line.find_last_of(":");

  if (pos == (line.size() - 1)) {
    return false;
  } else {
    std::vector<std::string> group_members;
    const std::string delimiters(",");
    std::string token = line.substr(pos + 1);
    Tokenize(token, group_members, delimiters);
    return InVector(group_members, user_name);
  }
}

bool WebAppManagerUtils::SetGroups() {
  gid_t gid_list[128];
  size_t num_groups = 0;

  std::string line;
  std::string new_group_path = "/etc/group";

  std::ifstream ifs(new_group_path.c_str());

  if (ifs.is_open()) {
    while (!ifs.eof()) {
      getline(ifs, line);

      if (line[0] != 0 && line[0] != '#' && line[0] != '\r') {
        if (InGroup(line, "webappmanager3")) {
          std::vector<std::string> tok;
          Tokenize(line, tok, ":");
          gid_list[num_groups++] = atoi(tok[2].c_str());
        }
      }
    }
  } else {
    return false;
  }
  ifs.close();

  if ((setgroups(num_groups, gid_list)) == -1) {
    return false;
  }
  return true;
}

// For PmLog, to meet maximum length(1024) 824 is selected approximately
#define URL_SIZE_LIMIT 824
std::string WebAppManagerUtils::TruncateURL(const std::string& url) {
  if (url.size() < URL_SIZE_LIMIT)
    return url;
  std::string res = url;
  return res.erase(URL_SIZE_LIMIT, url.size() - URL_SIZE_LIMIT);
}
