#ifndef WEBRUNTIME_AGL_H
#define WEBRUNTIME_AGL_H

#include <map>
#include <signal.h>
#include <string>
#include <vector>

#include <ilm/ilm_control.h>

#include "WebRuntime.h"

class LibHomeScreen;
class LibWindowmanager;

class ILMControl
{
  public:
    ILMControl(notificationFunc callback, void *user_data) {
        ilm_init();
        ilm_registerNotification(callback, user_data);
    }

    ~ILMControl(void) {
        ilm_unregisterNotification();
        ilm_destroy();
        fprintf(stderr, "ilm_destory().\r\n");
    }
};

class Launcher {
public:
  virtual void register_surfpid(pid_t surf_pid);
  virtual void unregister_surfpid(pid_t surf_pid);
  virtual pid_t find_surfpid_by_rid(pid_t app_pid);
  virtual int launch(const std::string& id, const std::string& uri) = 0;
  virtual int loop(int argc, const char** argv, volatile sig_atomic_t& e_flag) = 0;

  int m_rid = 0;
  std::vector<pid_t> m_pid_v;
};

class SharedBrowserProcessWebAppLauncher : public Launcher {
public:
  int launch(const std::string& id, const std::string& uri) override;
  int loop(int argc, const char** argv, volatile sig_atomic_t& e_flag) override;
};

class SingleBrowserProcessWebAppLauncher : public Launcher {
public:
  int launch(const std::string& id, const std::string& uri) override;
  int loop(int argc, const char** argv, volatile sig_atomic_t& e_flag) override;
};

class WebAppLauncherRuntime  : public WebRuntime {
public:
  int run(int argc, const char** argv) override;

  void notify_ivi_control_cb(ilmObjectType object, t_ilm_uint id,
                               t_ilm_bool created);
  static void notify_ivi_control_cb_static (ilmObjectType object,
                                              t_ilm_uint id,
                                              t_ilm_bool created,
                                              void *user_data);

private:

  bool init();
  bool init_wm();
  bool init_hs();
  int parse_config(const char *file);
  void setup_surface (int id);

  std::string m_id;
  std::string m_role;
  std::string m_url;
  std::string m_name;

  int m_port;
  std::string m_token;

  Launcher *m_launcher;

  LibWindowmanager *m_wm = nullptr;
  LibHomeScreen *m_hs = nullptr;
  ILMControl *m_ic = nullptr;

  std::map<int, int> m_surfaces;  // pair of <afm:rid, ivi:id>

  bool m_pending_create = false;
};

class SharedBrowserProcessRuntime  : public WebRuntime {
public:
  int run(int argc, const char** argv) override;
};

class RenderProcessRuntime  : public WebRuntime {
public:
  int run(int argc, const char** argv) override;
};

class WebRuntimeAGL : public WebRuntime {
public:
  int run(int argc, const char** argv) override;

private:

  WebRuntime *m_runtime;
};

#endif // WEBRUNTIME_AGL_H
