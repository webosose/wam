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
#include <webos/public/runtime.h>

#include <cassert>

#include "log_manager.h"
#include "platform/platform_factory.h"
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

    assert(pwd);
    assert(grp);

    [[maybe_unused]] int ret = -1;
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

static void StartWebAppManager() {
  ChangeUserIDGroupID();

  WebAppManagerServiceLuna* luna_service = WebAppManagerServiceLuna::Instance();
  assert(luna_service);
  [[maybe_unused]] bool result = luna_service->StartService();
  assert(result);
  WebAppManager::Instance()->SetPlatformModules(
      std::make_unique<PlatformModuleFactoryImpl>());
}

class WebOSMainDelegateWAM : public webos::WebOSMainDelegate {
 public:
  void AboutToCreateContentBrowserClient() override { StartWebAppManager(); }
};

int main(int argc, const char** argv) {
  WebOSMainDelegateWAM delegate;
  webos::Runtime::GetInstance()->SetPlatformFactory(
      std::make_unique<PlatformFactory>());
  webos::WebOSMain webos_main(&delegate);
  return webos_main.Run(argc, argv);
}
