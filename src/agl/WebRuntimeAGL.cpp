#include "WebRuntimeAGL.h"

#include <cassert>
#include <regex>
#include <unistd.h>

#include <glib.h>
#include <libxml/parser.h>

#include <libhomescreen.hpp>
#include <libwindowmanager.h>

#include <webos/app/webos_main.h>

#include "LogManager.h"
#include "PlatformModuleFactoryImpl.h"
#include "StringUtils.h"
#include "WebAppManager.h"
#include "WebAppManagerServiceAGL.h"


#define WEBAPP_CONFIG "config.xml"

volatile sig_atomic_t e_flag = 0;


static std::string getAppId(const std::vector<std::string>& args) {
  const char *afm_id = getenv("AFM_ID");
  if (afm_id == nullptr || !afm_id[0]) {
    return args[0];
  } else {
    return std::string(afm_id);
  }
}

static std::string getAppUrl(const std::vector<std::string>& args) {
  for (size_t i=0; i < args.size(); i++) {
    std::size_t found = args[i].find(std::string("http://"));
    if (found != std::string::npos)
        return args[i];
  }
  return std::string();
}

static bool isBrowserProcess(const std::vector<std::string>& args) {
  // if type is not given then we are browser process
  for (size_t i=0; i < args.size(); i++) {
    std::string param("--type=");
    std::size_t found = args[i].find(param);
    if (found != std::string::npos)
        return false;
  }
  return true;
}

static bool isSharedBrowserProcess(const std::vector<std::string>& args) {
  // if 'http://' param is not present then assume shared browser process
  for (size_t i=0; i < args.size(); i++) {
    std::size_t found = args[i].find(std::string("http://"));
    if (found != std::string::npos)
        return false;
  }
  return true;
}

static bool isWaitForHostService(const std::vector<std::string>& args) {
  const char *value = getenv("WAIT_FOR_HOST_SERVICE");
  if (value == nullptr || !value[0]) {
    return false;
  } else {
    return (strcmp(value, "1") == 0);
  }
}

class AGLMainDelegateWAM : public webos::WebOSMainDelegate {
public:
    void AboutToCreateContentBrowserClient() override {
      WebAppManagerServiceAGL::instance()->startService();
      WebAppManager::instance()->setPlatformModules(std::unique_ptr<PlatformModuleFactoryImpl>(new PlatformModuleFactoryImpl()));
    }
};

class AGLRendererDelegateWAM : public webos::WebOSMainDelegate {
public:
    void AboutToCreateContentBrowserClient() override {
      // do nothing
    }
};

void Launcher::register_surfpid(pid_t app_pid, pid_t surf_pid)
{
  if (app_pid != m_rid)
    return;
  bool result = m_pid_map.insert({app_pid, surf_pid}).second;
  if (!result) {
    LOG_DEBUG("register_surfpid, (app_pid=%d) already registered surface_id with (surface_id=%d)",
            (int)app_pid, (int)surf_pid);
  }
}

void Launcher::unregister_surfpid(pid_t app_pid, pid_t surf_pid)
{
  size_t erased_count = m_pid_map.erase(app_pid);
  if (erased_count == 0) {
    LOG_DEBUG("unregister_surfpid, (app_pid=%d) doesn't have a registered surface",
            (int)app_pid);
  }
}

pid_t Launcher::find_surfpid_by_rid(pid_t app_pid)
{
  auto surface_id = m_pid_map.find(app_pid);
  if (surface_id != m_pid_map.end()) {
    LOG_DEBUG("found return(%d, %d)", (int)app_pid, (int)surface_id->second);
    return surface_id->second;
  }
  return -1;
}

int SingleBrowserProcessWebAppLauncher::launch(const std::string& id, const std::string& uri) {
  m_rid = (int)getpid();
  WebAppManagerServiceAGL::instance()->setStartupApplication(id, uri, m_rid);
  return m_rid;
}

int SingleBrowserProcessWebAppLauncher::loop(int argc, const char** argv, volatile sig_atomic_t& e_flag) {
  AGLMainDelegateWAM delegate;
  webos::WebOSMain webOSMain(&delegate);
  return webOSMain.Run(argc, argv);
}

int SharedBrowserProcessWebAppLauncher::launch(const std::string& id, const std::string& uri) {
  if (!WebAppManagerServiceAGL::instance()->initializeAsHostClient()) {
    LOG_DEBUG("Failed to initialize as host client");
    return -1;
  }

  m_rid = (int)getpid();
  std::string m_rid_s = std::to_string(m_rid);
  std::vector<const char*> data;
  data.push_back(kStartApp);
  data.push_back(id.c_str());
  data.push_back(uri.c_str());
  data.push_back(m_rid_s.c_str());

  WebAppManagerServiceAGL::instance()->launchOnHost(data.size(), data.data());
  return m_rid;
}

int SharedBrowserProcessWebAppLauncher::loop(int argc, const char** argv, volatile sig_atomic_t& e_flag) {
  // TODO: wait for a pid
  while (1)
    sleep(1);
  return 0;
}

int WebAppLauncherRuntime::run(int argc, const char** argv) {
  std::vector<std::string> args(argv + 1, argv + argc);
  bool isWaitHostService = isWaitForHostService(args);
  m_id = getAppId(args);
  m_url = getAppUrl(args);

  if(isWaitHostService) {
    while(!WebAppManagerServiceAGL::instance()->isHostServiceRunning()) {
      LOG_DEBUG("WebAppLauncherRuntime::run - waiting for host service");
      sleep(1);
    }
  }

  if(isWaitHostService || WebAppManagerServiceAGL::instance()->isHostServiceRunning()) {
    LOG_DEBUG("WebAppLauncherRuntime::run - creating SharedBrowserProcessWebAppLauncher");
    m_launcher = new SharedBrowserProcessWebAppLauncher();
  } else {
    LOG_DEBUG("WebAppLauncherRuntime::run - creating SingleBrowserProcessWebAppLauncher");
    m_launcher = new SingleBrowserProcessWebAppLauncher();
  }


  // Initialize SIGTERM handler
  // TODO: init_signal();

  if (!init())
    return -1;

  /* Launch WAM application */
  m_launcher->m_rid = m_launcher->launch(m_id, m_url);

  if (m_launcher->m_rid < 0) {
    LOG_DEBUG("cannot launch WAM app (%s)", m_id.c_str());
  }

  // take care 1st time launch
  LOG_DEBUG("waiting for notification: surface created");
  m_pending_create = true;

  return m_launcher->loop(argc, argv, e_flag);
}

bool WebAppLauncherRuntime::init() {
  // based on https://tools.ietf.org/html/rfc3986#page-50
  std::regex url_regex (
    R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
    std::regex::extended
  );

  std::smatch url_match_result;
  if (std::regex_match(m_url, url_match_result, url_regex)) {
    unsigned counter = 0;
    for (const auto& res : url_match_result) {
      LOG_DEBUG("    %d: %s", counter++, res.str().c_str());
    }

    if (url_match_result[4].length()) {
      std::string authority = url_match_result[4].str();
      std::size_t n = authority.find(':');
      if (n != std::string::npos) {
        std::string sport = authority.substr(n+1);
        m_host = authority.substr(0, n);
        m_port = stringTo<int>(sport);
      } else {
        m_host = authority;
      }
    }

    bool url_misses_token = true;
    if (url_match_result[7].length()) {
      std::string query = url_match_result[7].str();
      std::size_t n = query.find('=');
      if (n != std::string::npos) {
        m_token = query.substr(n+1);
        url_misses_token = false;
      }
    }
    if (url_misses_token) {
      char *tokenv = getenv("CYNAGOAUTH_TOKEN");
      if (tokenv) {
        m_token = tokenv;
        m_url.push_back(url_match_result[7].length() ? '&' : '?');
        m_url.append("token=");
        m_url.append(m_token);
      }
    }

    auto path = std::string(getenv("AFM_APP_INSTALL_DIR"));
    path = path + "/" + WEBAPP_CONFIG;

    // Parse config file of runxdg
    if (parse_config(path.c_str())) {
      LOG_DEBUG("Error in config");
      return false;
    }

    // Special cases for windowmanager roles
    if (m_id.rfind("webapps-html5-homescreen", 0) == 0)
      m_role = "homescreen";
    else if (m_id.rfind("webapps-homescreen", 0) == 0)
      m_role = "homescreen";
    else {
      m_role = m_id.substr(0,12);
    }

    LOG_DEBUG("id=[%s], name=[%s], role=[%s], url=[%s], host=[%s], port=%d, token=[%s]",
            m_id.c_str(), m_name.c_str(), m_role.c_str(), m_url.c_str(),
            m_host.c_str(), m_port, m_token.c_str());

    // Setup HomeScreen/WindowManager API
    if (!init_wm()) {
      LOG_DEBUG("cannot setup wm API");
      return false;
    }

    if (!init_hs()) {
      LOG_DEBUG("cannot setup hs API");
      return false;
    }

    // Setup ilmController API
    m_ic = new ILMControl(notify_ivi_control_cb_static, this);

    return true;
  } else {
    LOG_DEBUG("Malformed url.");
    return false;
  }
}

bool WebAppLauncherRuntime::init_wm() {
  m_wm = new LibWindowmanager();
  if (m_wm->init(m_host.c_str(), m_port, m_token.c_str())) {
    LOG_DEBUG("cannot initialize windowmanager");
    return false;
  }

  std::function< void(json_object*) > h_active = [](json_object* object) {
    LOG_DEBUG("Got Event_Active");
  };

  std::function< void(json_object*) > h_inactive = [](json_object* object) {
    LOG_DEBUG("Got Event_Inactive");
  };

  std::function< void(json_object*) > h_visible = [this](json_object* object) {
    LOG_DEBUG("Got Event_Visible");

    std::vector<const char*> data;
    data.push_back(kActivateEvent);
    data.push_back(this->m_id.c_str());

    WebAppManagerServiceAGL::instance()->sendEvent(data.size(), data.data());
  };

  std::function< void(json_object*) > h_invisible = [this](json_object* object) {
    LOG_DEBUG("Got Event_Invisible");

    std::vector<const char*> data;
    data.push_back(kDeactivateEvent);
    data.push_back(this->m_id.c_str());

    WebAppManagerServiceAGL::instance()->sendEvent(data.size(), data.data());
  };

  std::function< void(json_object*) > h_syncdraw =
      [this](json_object* object) {
    LOG_DEBUG("Got Event_SyncDraw");
    this->m_wm->endDraw(this->m_role.c_str());
  };

  std::function< void(json_object*) > h_flushdraw= [](json_object* object) {
    LOG_DEBUG("Got Event_FlushDraw");
  };

  m_wm->set_event_handler(LibWindowmanager::Event_Active, h_active);
  m_wm->set_event_handler(LibWindowmanager::Event_Inactive, h_inactive);
  m_wm->set_event_handler(LibWindowmanager::Event_Visible, h_visible);
  m_wm->set_event_handler(LibWindowmanager::Event_Invisible, h_invisible);
  m_wm->set_event_handler(LibWindowmanager::Event_SyncDraw, h_syncdraw);
  m_wm->set_event_handler(LibWindowmanager::Event_FlushDraw, h_flushdraw);

  return true;
}

bool WebAppLauncherRuntime::init_hs() {
  m_hs = new LibHomeScreen();
  if (m_hs->init(m_host.c_str(), m_port, m_token.c_str())) {
    LOG_DEBUG("cannot initialize homescreen");
    return false;
  }

  std::function< void(json_object*) > handler = [this] (json_object* object) {
    LOG_DEBUG("Activesurface %s ", this->m_role.c_str());
    this->m_wm->activateWindow(this->m_role.c_str(), "normal.full");
  };
  m_hs->set_event_handler(LibHomeScreen::Event_ShowWindow, handler);

  std::function< void(json_object*) > h_default= [](json_object* object) {
    const char *j_str = json_object_to_json_string(object);
    LOG_DEBUG("Got event [%s]", j_str);
  };
  m_hs->set_event_handler(LibHomeScreen::Event_OnScreenMessage, h_default);

  return true;
}

int WebAppLauncherRuntime::parse_config (const char *path_to_config)
{
  xmlDoc *doc = xmlReadFile(path_to_config, nullptr, 0);
  xmlNode *root = xmlDocGetRootElement(doc);

  xmlChar *id = nullptr;
  xmlChar *version = nullptr;
  xmlChar *name = nullptr;
  xmlChar *content = nullptr;
  xmlChar *description = nullptr;
  xmlChar *author = nullptr;
  xmlChar *icon = nullptr;

  id = xmlGetProp(root, (const xmlChar*)"id");
  version = xmlGetProp(root, (const xmlChar*)"version");
  for (xmlNode *node = root->children; node; node = node->next) {
    if (!xmlStrcmp(node->name, (const xmlChar*)"name"))
      name = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    if (!xmlStrcmp(node->name, (const xmlChar*)"icon"))
      icon = xmlGetProp(node, (const xmlChar*)"src");
    if (!xmlStrcmp(node->name, (const xmlChar*)"content"))
      content = xmlGetProp(node, (const xmlChar*)"src");
    if (!xmlStrcmp(node->name, (const xmlChar*)"description"))
      description = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
    if (!xmlStrcmp(node->name, (const xmlChar*)"author"))
      author = xmlNodeListGetString(doc, node->xmlChildrenNode, 1);
  }
  fprintf(stdout, "...parse_config...\n");
  LOG_DEBUG("id: %s", id);
  LOG_DEBUG("version: %s", version);
  LOG_DEBUG("name: %s", name);
  LOG_DEBUG("content: %s", content);
  LOG_DEBUG("description: %s", description);
  LOG_DEBUG("author: %s", author);
  LOG_DEBUG("icon: %s", icon);

  m_name = std::string((const char*)name);

  xmlFree(id);
  xmlFree(version);
  xmlFree(name);
  xmlFree(content);
  xmlFree(description);
  xmlFree(author);
  xmlFree(icon);
  xmlFreeDoc(doc);

  return 0;
}

void WebAppLauncherRuntime::setup_surface (int id)
{
  std::string sid = std::to_string(id);

  // This surface is mine, register pair app_name and ivi id.
  LOG_DEBUG("requestSurfaceXDG(%s,%d)", m_role.c_str(), id);
  m_wm->requestSurfaceXDG(this->m_role.c_str(), id);

  if (m_pending_create) {
    // Recovering 1st time tap_shortcut is dropped because
    // the application has not been run yet (1st time launch)
    m_pending_create = false;
    m_wm->activateWindow(this->m_role.c_str(), "normal.full");
  }
}

void WebAppLauncherRuntime::notify_ivi_control_cb (ilmObjectType object, t_ilm_uint id,
                                    t_ilm_bool created)
{
  if (object == ILM_SURFACE) {
    // This call is broadcasted and all the launchers receive this call with the same id,
    // but we cannot rely on surf_pid when calling find_surfpid_by_rid, because all the
    // created surfaces are created by WebAppMgr, which has only one pid. This results
    // in a wrong launcher requesting surfaces, which may lead to a sitatution, when app just
    // creashes. Thus, let's temporarely use an assumption that each launcher has only one
    // surface.
    if (!m_launcher->m_pid_map.empty()) {
      LOG_DEBUG("This launcher has already had a surface");
      return;
    }

    struct ilmSurfaceProperties surf_props;

    ilm_getPropertiesOfSurface(id, &surf_props);
    pid_t surf_pid = surf_props.creatorPid;

    if (!created) {
      LOG_DEBUG("ivi surface (id=%d, surf_pid=%d) [m_rid:%d] destroyed.", id, surf_pid, m_launcher->m_rid);
      m_launcher->unregister_surfpid(id, surf_pid);
      m_surfaces.erase(surf_pid);
      return;
    }

    LOG_DEBUG("ivi surface (id=%d, surf_pid=%d) [m_rid:%d] is created.", id, surf_pid, m_launcher->m_rid);

    m_launcher->register_surfpid(id, surf_pid);
    if (m_launcher->m_rid &&
        surf_pid == m_launcher->find_surfpid_by_rid(m_launcher->m_rid)) {
      setup_surface(id);
    }
    m_surfaces[surf_pid] = id;
  } else if (object == ILM_LAYER) {
    if (created)
      LOG_DEBUG("ivi layer: %d created.", id);
    else
      LOG_DEBUG("ivi layer: %d destroyed.", id);
  }
}

void WebAppLauncherRuntime::notify_ivi_control_cb_static (ilmObjectType object, t_ilm_uint id,
                                           t_ilm_bool created, void *user_data)
{
  WebAppLauncherRuntime *wam = static_cast<WebAppLauncherRuntime*>(user_data);
  wam->notify_ivi_control_cb(object, id, created);
}


int SharedBrowserProcessRuntime::run(int argc, const char** argv) {
  if (WebAppManagerServiceAGL::instance()->initializeAsHostService()) {
    AGLMainDelegateWAM delegate;
    webos::WebOSMain webOSMain(&delegate);
    return webOSMain.Run(argc, argv);
  } else {
    LOG_DEBUG("Trying to start shared browser process but process is already running");
    return -1;
  }
}

int RenderProcessRuntime::run(int argc, const char** argv) {
  AGLMainDelegateWAM delegate;
  webos::WebOSMain webOSMain(&delegate);
  return webOSMain.Run(argc, argv);
}

int WebRuntimeAGL::run(int argc, const char** argv) {
  LOG_DEBUG("WebRuntimeAGL::run");
  std::vector<std::string> args(argv + 1, argv + argc);
  if (isBrowserProcess(args)) {
    if (isSharedBrowserProcess(args)) {
      LOG_DEBUG("WebRuntimeAGL - creating SharedBrowserProcessRuntime");
      m_runtime = new SharedBrowserProcessRuntime();
    }  else {
      LOG_DEBUG("WebRuntimeAGL - creating WebAppLauncherRuntime");
      m_runtime = new WebAppLauncherRuntime();
    }
  } else {
    LOG_DEBUG("WebRuntimeAGL - creating RenderProcessRuntime");
    m_runtime = new RenderProcessRuntime();
  }

  return m_runtime->run(argc, argv);
}

