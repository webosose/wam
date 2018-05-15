#include "RunWAMAGL.h"

#include <regex>
#include <unistd.h>

#include <libhomescreen.hpp>
#include <libwindowmanager.h>

#include <webos/app/webos_main.h>

#include "LogManager.h"
#include "PlatformModuleFactoryImpl.h"
#include "WebAppManager.h"
#include "WebAppManagerServiceAGL.h"


volatile sig_atomic_t e_flag = 0;

class WebOSMainDelegateWAM : public webos::WebOSMainDelegate {
public:
    void AboutToCreateContentBrowserClient() override {
        WebAppManagerServiceAGL::instance()->startService();
        WebAppManager::instance()->setPlatformModules(new PlatformModuleFactoryImpl());
    }
};

int SingleBrowserProcessLauncher::launch(std::string& name) {
  m_rid = (int)getpid();
  WebAppManagerServiceAGL::instance()->setStartupApplication(name, m_rid);
  return m_rid;
}

int SingleBrowserProcessLauncher::loop(int argc, const char** argv, volatile sig_atomic_t& e_flag) {
  WebOSMainDelegateWAM delegate;
  webos::WebOSMain webOSMain(&delegate);
  return webOSMain.Run(argc, argv);
}

void SingleBrowserProcessLauncher::register_surfpid (pid_t surf_pid)
{
  if (surf_pid == m_rid) {
    if (!std::count(m_pid_v.begin(), m_pid_v.end(), surf_pid)) {
      fprintf(stderr, "surface creator(pid=%d) registered\r\n", surf_pid);
      m_pid_v.push_back(surf_pid);
      fprintf(stderr, "m_pid_v.count(%d) = %d\r\n", surf_pid,
                (int)std::count(m_pid_v.begin(), m_pid_v.end(), surf_pid));
    }
  }
}

pid_t SingleBrowserProcessLauncher::find_surfpid_by_rid (pid_t rid)
{
  fprintf(stderr, "find surfpid by rid(%d)\r\n", rid);
  if (std::count(m_pid_v.begin(), m_pid_v.end(), rid)) {
    fprintf(stderr, "found return(%d)\r\n", rid);
    return rid;
  }
  return -1;
}

RunWAMAGL::RunWAMAGL(const std::string& url)
  : m_id(url),
    m_role("WebApp") {

  // based on https://tools.ietf.org/html/rfc3986#page-50
  unsigned counter = 0;
  std::regex url_regex (
    R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
    std::regex::extended
  );

  std::smatch url_match_result;
  if (std::regex_match(url, url_match_result, url_regex)) {
    for (const auto& res : url_match_result) {
      fprintf(stderr, "    %d: %s\r\n", counter++, res.str().c_str());
    }

    if (url_match_result.size() > 4) {
      std::string authority = url_match_result[4].str();
      std::size_t n = authority.find(':');
      if (n != std::string::npos) {
        std::string sport = authority.substr(n+1);
        m_role.append("-");
        m_role.append(sport);
        m_port = std::stoi(sport);
      }
    }

    if (url_match_result.size() > 7) {
      std::string query = url_match_result[7].str();
      std::size_t n = query.find('=');
      if (n != std::string::npos) {
        m_token = query.substr(n+1);
      }
    }

    m_launcher = new SingleBrowserProcessLauncher();

    // Setup HomeScreen/WindowManager API
    if (!init_wm())
      fprintf(stderr, "cannot setup wm API\r\n");

    if (!init_hs())
      fprintf(stderr, "cannot setup hs API\r\n");

    // Setup ilmController API
    m_ic = new ILMControl(notify_ivi_control_cb_static, this);
  } else {
    fprintf(stderr, "Malformed url.\r\n");
  }
}

int RunWAMAGL::run(int argc, const char** argv) {
  // Initialize SIGTERM handler
  // TODO: init_signal();

  /* Launch WAM application */
  m_launcher->m_rid = m_launcher->launch(m_id);

  if (m_launcher->m_rid < 0) {
    fprintf(stderr, "cannot launch WAM app (%s)\r\n", m_id.c_str());
  }

  // take care 1st time launch
  fprintf(stderr, "waiting for notification: surface created\r\n");
  m_pending_create = true;

  return m_launcher->loop(argc, argv, e_flag);
}

bool RunWAMAGL::init_wm() {
  m_wm = new LibWindowmanager();
  if (m_wm->init(m_port, m_token.c_str())) {
    fprintf(stderr, "cannot initialize windowmanager\r\n");
    return false;
  }

  std::function< void(json_object*) > h_active = [](json_object* object) {
    fprintf(stderr, "Got Event_Active\r\n");
  };

  std::function< void(json_object*) > h_inactive = [](json_object* object) {
    fprintf(stderr, "Got Event_Inactive\r\n");
  };

  std::function< void(json_object*) > h_visible = [](json_object* object) {
    fprintf(stderr, "Got Event_Visible\r\n");
  };

  std::function< void(json_object*) > h_invisible = [](json_object* object) {
    fprintf(stderr, "Got Event_Invisible\r\n");
  };

  std::function< void(json_object*) > h_syncdraw =
      [this](json_object* object) {
    fprintf(stderr, "Got Event_SyncDraw\r\n");
    json_object* obj = json_object_new_object();
    json_object_object_add(obj, this->m_wm->kKeyDrawingName,
                           json_object_new_string(this->m_role.c_str()));
    this->m_wm->endDraw(obj);
  };

  std::function< void(json_object*) > h_flushdraw= [](json_object* object) {
    fprintf(stderr, "Got Event_FlushDraw\r\n");
  };

  m_wm->set_event_handler(LibWindowmanager::Event_Active, h_active);
  m_wm->set_event_handler(LibWindowmanager::Event_Inactive, h_inactive);
  m_wm->set_event_handler(LibWindowmanager::Event_Visible, h_visible);
  m_wm->set_event_handler(LibWindowmanager::Event_Invisible, h_invisible);
  m_wm->set_event_handler(LibWindowmanager::Event_SyncDraw, h_syncdraw);
  m_wm->set_event_handler(LibWindowmanager::Event_FlushDraw, h_flushdraw);

  return true;
}

bool RunWAMAGL::init_hs() {
  m_hs = new LibHomeScreen();
  if (m_hs->init(m_port, m_token.c_str())) {
    fprintf(stderr, "cannot initialize homescreen\r\n");
    return false;
  }

  std::function< void(json_object*) > handler = [this] (json_object* object) {
    json_object *val;

    if (json_object_object_get_ex(object, "application_name", &val)) {
      const char *name = json_object_get_string(val);

      fprintf(stderr, "Event_TapShortcut <%s>\r\n", name);

      if (strcmp(name, this->m_role.c_str()) == 0) {
        // check app exist and re-launch if needed
        fprintf(stderr, "Activesurface %s \r\n", this->m_role.c_str());

        json_object *obj = json_object_new_object();
        json_object_object_add(obj, this->m_wm->kKeyDrawingName,
                               json_object_new_string(this->m_role.c_str()));
        json_object_object_add(obj, this->m_wm->kKeyDrawingArea,
                               json_object_new_string("normal.full"));

        this->m_wm->activateSurface(obj);
      }
    }
  };
  m_hs->set_event_handler(LibHomeScreen::Event_TapShortcut, handler);

  std::function< void(json_object*) > h_default= [](json_object* object) {
    const char *j_str = json_object_to_json_string(object);
    fprintf(stderr, "Got event [%s]\r\n", j_str);
  };
  m_hs->set_event_handler(LibHomeScreen::Event_OnScreenMessage, h_default);

  return true;
}

void RunWAMAGL::setup_surface (int id)
{
  std::string sid = std::to_string(id);

  // This surface is mine, register pair app_name and ivi id.
  json_object *obj = json_object_new_object();
  json_object_object_add(obj, m_wm->kKeyDrawingName,
                         json_object_new_string(m_role.c_str()));
  json_object_object_add(obj, m_wm->kKeyIviId,
                         json_object_new_string(sid.c_str()));

  fprintf(stderr, "requestSurfaceXDG(%s,%s)\r\n", m_role.c_str(), sid.c_str());
  m_wm->requestSurfaceXDG(obj);

  if (m_pending_create) {
    // Recovering 1st time tap_shortcut is dropped because
    // the application has not been run yet (1st time launch)
    m_pending_create = false;

    json_object *obj = json_object_new_object();
    json_object_object_add(obj, m_wm->kKeyDrawingName,
                           json_object_new_string(m_role.c_str()));
    json_object_object_add(obj, m_wm->kKeyDrawingArea,
                           json_object_new_string("normal.full"));
    m_wm->activateSurface(obj);
  }
}

void RunWAMAGL::notify_ivi_control_cb (ilmObjectType object, t_ilm_uint id,
                                    t_ilm_bool created)
{
  if (object == ILM_SURFACE) {
    struct ilmSurfaceProperties surf_props;

    ilm_getPropertiesOfSurface(id, &surf_props);
    pid_t surf_pid = surf_props.creatorPid;

    if (!created) {
      fprintf(stderr, "ivi surface (id=%d, pid=%d) destroyed.\r\n", id, surf_pid);
      //m_launcher->unregister_surfpid(surf_pid);
      //m_surfaces.erase(surf_pid);
      return;
    }

    fprintf(stderr, "ivi surface (id=%d, pid=%d) is created.\r\n", id, surf_pid);

    m_launcher->register_surfpid(surf_pid);
    if (m_launcher->m_rid &&
        surf_pid == m_launcher->find_surfpid_by_rid(m_launcher->m_rid)) {
      setup_surface(id);
    }
    m_surfaces[surf_pid] = id;
  } else if (object == ILM_LAYER) {
    if (created)
      fprintf(stderr, "ivi layer: %d created.\r\n", id);
    else
      fprintf(stderr, "ivi layer: %d destroyed.\r\n", id);
  }
}

void RunWAMAGL::notify_ivi_control_cb_static (ilmObjectType object, t_ilm_uint id,
                                           t_ilm_bool created, void *user_data)
{
  RunWAMAGL *wam = static_cast<RunWAMAGL*>(user_data);
  wam->notify_ivi_control_cb(object, id, created);
}
