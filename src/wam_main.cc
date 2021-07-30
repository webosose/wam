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

#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <webos/app/webos_main.h>

#include "base_check.h"
#include "log_manager.h"
#include "platform_module_factory_impl.h"
#include "utils.h"
#include "web_app_manager.h"
#include "web_app_manager_service_luna.h"

static void ChangeUserIDGroupID() {
  std::string uid, gid;
  uid = util::GetEnvVar("WAM_UID");
  gid = util::GetEnvVar("WAM_GID");

  if (uid.size() && gid.size()) {
    struct passwd* pwd = getpwnam(uid.c_str());
    struct group* grp = getgrnam(gid.c_str());

    UTIL_ASSERT(pwd);
    UTIL_ASSERT(grp);

    int ret = -1;
    if (grp) {
      ret = setgid(grp->gr_gid);
      UTIL_ASSERT(ret == 0);
      ret = initgroups(uid.c_str(), grp->gr_gid);
      UTIL_ASSERT(ret == 0);
    }

    if (pwd) {
      ret = setuid(pwd->pw_uid);
      UTIL_ASSERT(ret == 0);
      setenv("HOME", pwd->pw_dir, 1);
    }
  }
}

static void StartWebAppManager() {
  ChangeUserIDGroupID();

  WebAppManagerServiceLuna* luna_service = WebAppManagerServiceLuna::Instance();
  UTIL_ASSERT(luna_service);
  bool result = luna_service->StartService();
  UTIL_ASSERT(result);
  WebAppManager::Instance()->SetPlatformModules(
      std::unique_ptr<PlatformModuleFactoryImpl>(
          new PlatformModuleFactoryImpl()));
}

class WebOSMainDelegateWAM : public webos::WebOSMainDelegate {
 public:
  void AboutToCreateContentBrowserClient() override { StartWebAppManager(); }
};

int main(int argc, const char** argv) {
  WebOSMainDelegateWAM delegate;
  webos::WebOSMain webOSMain(&delegate);
  return webOSMain.Run(argc, argv);
}
