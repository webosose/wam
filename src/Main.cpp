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

static void startWebAppManager()
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
#endif
    assert(webAppManagerService);
    bool result = webAppManagerService->startService();
    assert(result);
#endif
    WebAppManager::instance()->setPlatformModules(std::unique_ptr<PlatformModuleFactoryImpl>(new PlatformModuleFactoryImpl()));
}

class WebOSMainDelegateWAM : public webos::WebOSMainDelegate {
public:
    void AboutToCreateContentBrowserClient() override {
        startWebAppManager();
    }
};

static int runWamMain(int argc, const char** argv)
{
    WebOSMainDelegateWAM delegate;
    webos::WebOSMain webOSMain(&delegate);
    return webOSMain.Run(argc, argv);
}

static bool isUIProcess(int argc, const char** argv) {
  if (argc > 0) {
    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i=0; i < args.size(); i++) {
      std::string param("--type=");
      std::size_t found = args[i].find(param);
      if (found != std::string::npos)
          return false;
    }
  }
  return true;
}

static bool isUIProcessService(int argc, const char** argv) {
  if (argc > 0) {
    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i=0; i < args.size(); i++) {
      // if launch-app is given then dont start socket service
      std::string param("--launch-app=");
      std::size_t found = args[i].find(param);
      if (found != std::string::npos)
          return false;
    }
  }
  return true;
}

static std::string getStartUpApp(int argc, const char** argv) {
  if (argc > 0) {
    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i=0; i < args.size(); i++) {
      std::string param("--launch-app=");
      std::size_t found = args[i].find(param);
      if (found != std::string::npos)
          return args[i].substr(found+param.length());
    }
  }
  return std::string();
}

static std::string getSurfaceId(int argc, const char** argv) {
  if (argc > 0) {
    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i=0; i < args.size(); i++) {
      std::string param("--surface-id=");
      std::size_t found = args[i].find(param);
      if (found != std::string::npos)
          return args[i].substr(found+param.length());
    }
  }
  return std::string();
}

int main (int argc, const char** argv)
{
#if defined(HAS_AGL_SERVICE)
  if (isUIProcess(argc, argv)) {
    fprintf(stderr, "UIProcess!!!!!\r\n");
    if (isUIProcessService(argc, argv)) {
      fprintf(stderr, "UIProcessService!!!!!\r\n");
      if (WebAppManagerServiceAGL::instance()->initializeAsHostService()) {
        runWamMain(argc, argv);
      } else {
        fprintf(stderr, "Trying to start as UIProcess service but there's service already running\r\n");
        return 1;
      }
    } else {
      std::string app(getStartUpApp(argc, argv));
      if (app.empty()) {
        fprintf(stderr, "--launch-app parameter is required\r\n");
        return -1;
      }

      if (WebAppManagerServiceAGL::instance()->isHostService()) {
        WebAppManagerServiceAGL::instance()->setStartupApplication(app, (int)getpid());
        return runWamMain(argc, argv);
      } else {
        if (!WebAppManagerServiceAGL::instance()->initializeAsHostClient()) {
          fprintf(stderr,"Failed to initialize as host client\r\n");
          return -1;
        }
        std::string surface_id(getSurfaceId(argc, argv));
        if (surface_id.empty())
          surface_id = std::to_string((int)getpid());

        std::vector<const char*> args;
        args.push_back(app.c_str());
        args.push_back(surface_id.c_str());
        WebAppManagerServiceAGL::instance()->launchOnHost(args.size(), args.data());
        while (1)
          sleep(1);
      }
    }
  } else {
    fprintf(stderr, "Not UIProcess\r\n");
    return runWamMain(argc, argv);
  }
#else
    return runWamMain(argc, argv);
#endif
}

