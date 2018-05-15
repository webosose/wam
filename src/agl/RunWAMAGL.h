#ifndef RUNWAMAGL_H
#define RUNWAMAGL_H

#include <map>
#include <signal.h>
#include <string>
#include <vector>

#include <ilm/ilm_control.h>

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
  virtual void register_surfpid(pid_t surf_pid) = 0;
  virtual pid_t find_surfpid_by_rid(pid_t app_pid) = 0;
  virtual int launch(std::string& name) = 0;
  virtual int loop(int argc, const char** argv, volatile sig_atomic_t& e_flag) = 0;

  int m_rid = 0;
};

class SingleBrowserProcessLauncher : public Launcher {
public:
  void register_surfpid(pid_t surf_pid);
  pid_t find_surfpid_by_rid(pid_t rid);
  int launch(std::string& name);
  int loop(int argc, const char** argv, volatile sig_atomic_t& e_flag);

private:
  std::vector<pid_t> m_pid_v;
};

class RunWAMAGL {
public:
  RunWAMAGL(const std::string& url);

  int run(int argc, const char** argv);

  void notify_ivi_control_cb(ilmObjectType object, t_ilm_uint id,
                               t_ilm_bool created);
  static void notify_ivi_control_cb_static (ilmObjectType object,
                                              t_ilm_uint id,
                                              t_ilm_bool created,
                                              void *user_data);

private:

  bool init_wm();
  bool init_hs();
  void setup_surface (int id);

  std::string m_id;
  std::string m_role;

  int m_port;
  std::string m_token;

  Launcher *m_launcher;

  LibWindowmanager *m_wm = nullptr;
  LibHomeScreen *m_hs = nullptr;
  ILMControl *m_ic = nullptr;

  std::map<int, int> m_surfaces;  // pair of <afm:rid, ivi:id>

  bool m_pending_create = false;
};

#endif // RUNWAMAGL_H
