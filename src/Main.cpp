// Copyright (c) 2015-2018 LG Electronics, Inc.
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

#include <assert.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include "LogManager.h"
#include "PlatformModuleFactoryImpl.h"
#include "Utils.h"
#include "WebAppManager.h"
#include "WebAppManagerServiceLuna.h"
#include <webos/app/webos_main.h>

static void changeUserIDGroupID()
{
    std::string uid, gid;
    uid = util::getEnvVar("WAM_UID");
    gid = util::getEnvVar("WAM_GID");

    if (uid.size() && gid.size()) {
        struct passwd *pwd = getpwnam(uid.c_str());
        struct group *grp = getgrnam(gid.c_str());

        assert(pwd);
        assert(grp);

        int ret = -1;
        if (grp) {
            ret = setgid(grp->gr_gid);
            assert(ret == 0);
            ret = initgroups(uid.c_str(), grp->gr_gid);
            assert(ret == 0);
        }

        if (pwd) {
            ret = setuid(pwd->pw_uid);
            assert(ret == 0);
            setenv("HOME", pwd->pw_dir, 1);
        }
    }
}

static void startWebAppManager()
{
    changeUserIDGroupID();

    WebAppManagerServiceLuna* webAppManagerServiceLuna = WebAppManagerServiceLuna::instance();
    assert(webAppManagerServiceLuna);
    bool result = webAppManagerServiceLuna->startService();
    assert(result);
    WebAppManager::instance()->setPlatformModules(std::unique_ptr<PlatformModuleFactoryImpl>(new PlatformModuleFactoryImpl()));
}

class WebOSMainDelegateWAM : public webos::WebOSMainDelegate {
public:
    void AboutToCreateContentBrowserClient() override {
        startWebAppManager();
    }
};

int main (int argc, const char** argv)
{
    WebOSMainDelegateWAM delegate;
    webos::WebOSMain webOSMain(&delegate);
    return webOSMain.Run(argc, argv);
}
