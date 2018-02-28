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

#include "WebAppManagerUtils.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <grp.h>

int WebAppManagerUtils::updateAndGetCpuIdle(bool updateOnly)
{
    static long oldCpuTime[4];
    long curCpuTime[4];
    long* cpuTime = curCpuTime;

    if (updateOnly)
        cpuTime = oldCpuTime;

    int fd;
    if ((fd = open("/proc/stat", O_RDONLY)) != -1) {
        char buffer[4096+1];
        int len = read(fd, buffer, sizeof(buffer)-1);
        if (len > 0) {
            buffer[len] = '\0';
            char* p = skipToken(buffer); /* "cpu" */
            cpuTime[0] = strtoul(p, &p, 0);
            cpuTime[1] = strtoul(p, &p, 0);
            cpuTime[2] = strtoul(p, &p, 0);
            cpuTime[3] = strtoul(p, &p, 0);
        }
        close(fd);
    }

    if (updateOnly)
        return 1000; // max value of percentages();

    long cpuDiff[4];
    int cpuStates[4];
    percentages(4, cpuStates, curCpuTime, oldCpuTime, cpuDiff);
    memcpy(oldCpuTime, curCpuTime, sizeof(oldCpuTime));

    return cpuStates[3];
}

char* WebAppManagerUtils::skipToken(const char* p)
{
    while (isspace(*p))
        p++;
    while (*p && !isspace(*p))
        p++;
    return (char*)p;
}

long WebAppManagerUtils::percentages(int cnt, int* out, long* now, long* old, long* diffs)
{
    register long totalChange = 0;
    register long* dp = diffs;

    for (int i = 0; i < cnt; i++) {
        register long change = *now - *old;
        if (change < 0)
            change = (int)((unsigned long)*now-(unsigned long)*old);
        totalChange += (*dp++ = change);
        *old++ = *now++;
    }

    if (0 == totalChange)
        totalChange = 1;

    long halfTotal = totalChange / 2l;
    for (int i = 0; i < cnt; i++)
        *out++ = (int)((*diffs++ * 1000 + halfTotal) / totalChange);

    return totalChange;
}

void WebAppManagerUtils::tokenize(std::string &str, std::vector<std::string> &tokens,
        const std::string &delimiters) {
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos) {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}

bool WebAppManagerUtils::inVector(std::vector<std::string> &tokens, const char *arg) {
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

bool WebAppManagerUtils::inGroup(std::string line, const char* user_name) {
    // only tokenize the lines that have users in the groups.
    // empty groups have the last character as ":".
    size_t pos = line.find_last_of(":");

    if (pos == (line.size() - 1)) {
        return false;
    } else {
        std::vector<std::string> groupmembers;
        const std::string delimiters(",");
        std::string token = line.substr(pos + 1);
        tokenize(token, groupmembers, delimiters);
        return inVector(groupmembers, user_name);
    }
}

bool WebAppManagerUtils::setGroups() {
    gid_t glist[128];
    size_t ngroups = 0;

    std::string line;
    std::string newgrouppath = "/etc/group";

    std::ifstream ifs(newgrouppath.c_str());

    if (ifs.is_open()) {
        while (! ifs.eof()) {
            getline(ifs, line);

            if (line[0] != 0 && line[0] != '#' && line[0] != '\r') {
                if (inGroup(line, "webappmanager3")) {
                    std::vector<std::string> tok;
                    tokenize(line, tok, ":");
                    glist[ngroups++] = atoi(tok[2].c_str());
                }
            }
        }
    } else {
      return false;
    }
    ifs.close();

    if ((setgroups(ngroups, glist)) == -1) {
        return false;
    }
    return true;
}

