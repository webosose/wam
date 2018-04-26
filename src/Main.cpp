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

#include <QtGlobal>
#include <QString>

#include <assert.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include "LogManager.h"
#include "PlatformModuleFactoryImpl.h"
#include "WebAppManager.h"

#if defined(HAS_LUNA_SERVICE)
#include "WebAppManagerServiceLuna.h"
#elif defined(HAS_AGL_SERVICE)
#include "WebAppManagerServiceAGL.h"
#endif

#include <webos/app/webos_main.h>

namespace
{
    void qMessageHandler(const QtMsgType type, const QMessageLogContext &context,
                         const QString &msg)
    {
        const char* function = context.function;
        char* userMessage = msg.toUtf8().data();
        switch (type) {
            case QtDebugMsg:
                LOG_DEBUG("%s, %s", function, userMessage);
                break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
            case QtInfoMsg:
                LOG_INFO(MSGID_QINFO, 0, "%s, %s", function, userMessage);
                break;
#endif
            case QtWarningMsg:
                LOG_WARNING(MSGID_QWARNING, 0, "%s, %s", function, userMessage);
                break;
            case QtCriticalMsg:
                LOG_ERROR(MSGID_QCRITICAL, 0, "%s, %s", function, userMessage);
                break;
            case QtFatalMsg:
                LOG_CRITICAL(MSGID_QFATAL, 0, "%s, %s", function, userMessage);
                break;
        }
    }
}

static void changeUserIDGroupID()
{
    char *uid, *gid;
    uid = getenv("WAM_UID");
    gid = getenv("WAM_GID");

    if (uid && gid) {
        struct passwd *pwd = getpwnam(uid);
        struct group *grp = getgrnam(gid);

        assert(pwd);
        assert(grp);

        int ret = -1;
        if (grp) {
            ret = setgid(grp->gr_gid);
            assert(ret == 0);
            ret = initgroups(uid, grp->gr_gid);
            assert(ret == 0);
        }

        if (pwd) {
            ret = setuid(pwd->pw_uid);
            assert(ret == 0);
            setenv("HOME", pwd->pw_dir, 1);
        }
    }
}

static void startWebAppManager(const std::string& app)
{
    // FIXME: Remove this when we don't use qDebug, qWarning any more.
    qInstallMessageHandler(qMessageHandler);

    changeUserIDGroupID();
#if defined(HAS_LUNA_SERVICE) || defined(HAS_AGL_SERVICE)
    WebAppManagerService* webAppManagerService = nullptr;
#if defined(HAS_LUNA_SERVICE)
    webAppManagerService = WebAppManagerServiceLuna::instance();
#elif defined(HAS_AGL_SERVICE)
    webAppManagerService = WebAppManagerServiceAGL::instance();
    if (webAppManagerService)
        static_cast<WebAppManagerServiceAGL*>(webAppManagerService)->setStartupApplication(app);
#endif
    assert(webAppManagerService);
    bool result = webAppManagerService->startService();
    assert(result);
#endif
    WebAppManager::instance()->setPlatformModules(std::unique_ptr<PlatformModuleFactoryImpl>(new PlatformModuleFactoryImpl()));
}

class WebOSMainDelegateWAM : public webos::WebOSMainDelegate {
public:
    WebOSMainDelegateWAM(const std::string& app)
      : app_(app) {}

    void AboutToCreateContentBrowserClient() override {
        startWebAppManager(app_);
    }

private:

    std::string app_;
};

int main (int argc, const char** argv)
{
    std::string app;
    std::vector<const char*> args;
    for (int i=0; i < argc; i++) {
      if (std::string(argv[i]) == "--launch-app") {
          if (i + 1 < argc) {
              app = argv[++i];
          } else {
              fprintf(stderr, "--launch-app option requires one argument.\r\n");
              return 1;
          }
      } else {
          args.push_back(argv[i]);
      }
    }
    WebOSMainDelegateWAM delegate(app);
    webos::WebOSMain webOSMain(&delegate);
    return webOSMain.Run(args.size(), args.data());
}
