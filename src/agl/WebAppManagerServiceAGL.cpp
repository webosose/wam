#include "WebAppManagerServiceAGL.h"

#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cassert>
#include <cstdlib>
#include <climits>
#include <exception>
#include <iostream>
#include <sstream>
#include <pthread.h>

#include <json/value.h>
#include <libxml/parser.h>

#include "LogManager.h"
#include "JsonHelper.h"

class WamSocketLockFile {
public:
  ~WamSocketLockFile() {
    if (lock_fd_ != -1)
      releaseLock(lock_fd_);
    if (lock_fd_ != -1)
      close(lock_fd_);
  }

  bool createAndLock() {
    lock_fd_ = openLockFile();
    if (!acquireLock(lock_fd_)) {
      LOG_DEBUG("Failed to lock file %d", lock_fd_);
      return false;
    }
    return true;
  }

  bool ownsLock() {
    return lock_fd_ != -1;
  }

  bool tryAcquireLock() {
    int fd = openLockFile();
    if (fd != -1) {
      if (acquireLock(fd)) {
        releaseLock(fd);
        return true;
      }
    }
    return false;
  }

private:

  int openLockFile() {
    int fd = open(lock_file_.c_str(), O_CREAT | O_TRUNC, S_IRWXU);
    if (fd == -1) {
      LOG_DEBUG("Failed to open lock file descriptor");
      return fd;
    }

    int flags = fcntl(fd, F_GETFD);
    if (flags == -1)
      LOG_DEBUG("Could not get flags for lock file %d", fd);

     flags |= FD_CLOEXEC;

     if (fcntl(fd, F_SETFD, flags) == -1)
       LOG_DEBUG("Could not set flags for lock file %d", fd);

     return fd;
  }

  bool acquireLock(int fd) {
    if (flock(fd, LOCK_EX | LOCK_NB) != 0)
      return false;
    return true;
  }

  void releaseLock(int fd) {
    flock(fd, LOCK_UN);
  }

  std::string lock_file_ = std::string("/tmp/wamsocket.lock");
  int lock_fd_ = -1;
};

class WamSocket {
public:
  ~WamSocket() {
    if (socket_fd_ != -1)
      close(socket_fd_);
  }

  bool createSocket(bool server) {
        // Create the socket file descriptor
    socket_fd_ = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd_ == -1) {
      LOG_DEBUG("Failed to open socket file descriptor");
      return false;
    }

    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path, wam_socket_path_.c_str(), sizeof(sock_addr.sun_path));

    if (server) {
      LOG_DEBUG("service binding");
      unlink(wam_socket_path_.c_str());
      if (bind(socket_fd_, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un)) != 0) {
        LOG_DEBUG("Failed to bind to named socket");
        return false;
      }
    } else {
      LOG_DEBUG("client connecting");
      if (connect(socket_fd_, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un)) != 0) {
        LOG_DEBUG("Failed to connect to named socket");
        return false;
      }
    }
    return true;
  }

  void sendMsg(int argc, const char **argv) {
    std::string cmd;
    for (int i = 0; i < argc; ++i)
      cmd.append(argv[i]).append(" ");
    // Remove the last appended space if any
    if (argc > 1)
      cmd.pop_back();
    LOG_DEBUG("Sending message=[%s]", cmd.c_str());
    ssize_t bytes = write(socket_fd_, cmd.c_str(), cmd.length());
    LOG_DEBUG("Wrote %ld bytes.", bytes);
  }

  int waitForMsg() {
    char buf[PATH_MAX] = {};
    ssize_t bytes;

    LOG_DEBUG("Waiting for data...");
    while (TEMP_FAILURE_RETRY((bytes = recv(socket_fd_, (void *)buf, sizeof(buf), 0)) != -1)) {
      int last = bytes - 1;
      // Remove the new line if there's one
      if (buf[last] == '\n')
        buf[last] = '\0';
      LOG_DEBUG("Got %ld bytes=[%s].",bytes, buf);

      std::string data(buf);
      std::istringstream iss(data);
      std::vector<const char*> res;
      for(std::string s; iss >> s; ) {
          res.push_back(strdup(s.c_str()));
      }

      WebAppManagerServiceAGL::instance()->setStartupApplication(
        std::string(res[0]), std::string(res[1]), atoi(res[2]),
        std::string(res[3]));

      WebAppManagerServiceAGL::instance()->triggerStartupApp();
      return 1;
    }
    return 0;
  }

private:

  const std::string wam_socket_path_ = std::string("/tmp/wamsocket");
  int socket_fd_;
  struct sockaddr_un sock_addr;
};

WebAppManagerServiceAGL::WebAppManagerServiceAGL()
  : socket_(std::make_unique<WamSocket>()),
  lock_file_(std::make_unique<WamSocketLockFile>())
{
}

WebAppManagerServiceAGL* WebAppManagerServiceAGL::instance() {
  static WebAppManagerServiceAGL *srv = new WebAppManagerServiceAGL();
  return srv;
}

bool WebAppManagerServiceAGL::initializeAsHostService() {
  if (lock_file_->createAndLock())
    return socket_->createSocket(true);
  return false;
}

bool WebAppManagerServiceAGL::initializeAsHostClient() {
  return socket_->createSocket(false);
}

bool WebAppManagerServiceAGL::isHostServiceRunning()
{
    return !lock_file_->tryAcquireLock();
}

void WebAppManagerServiceAGL::launchOnHost(int argc, const char **argv)
{
    LOG_DEBUG("Dispatching launchOnHost");
    socket_->sendMsg(argc, argv);
}

void WebAppManagerServiceAGL::setStartupApplication(
    const std::string& startup_app_id,
    const std::string& startup_app_uri, int startup_app_surface_id,
    const std::string& startup_proxy_port)
{
    startup_app_id_ = startup_app_id;
    startup_app_uri_ = startup_app_uri;
    startup_app_surface_id_ = startup_app_surface_id;
    startup_proxy_port_ = startup_proxy_port;
}

void *run_socket(void *socket) {
  WamSocket *s = (WamSocket*)socket;
  while(s->waitForMsg());
  return 0;
}

bool WebAppManagerServiceAGL::startService()
{
    if (lock_file_->ownsLock()) {
      pthread_t thread_id;
      if( pthread_create( &thread_id , nullptr,  run_socket, socket_.get()) < 0) {
          perror("could not create thread");
          LOG_DEBUG("Couldnt create thread...");
          return false;
      }
    }

    triggerStartupApp();

    return true;
}

void WebAppManagerServiceAGL::triggerStartupApp()
{
    LOG_DEBUG("Triggering app start: %s", startup_app_uri_.c_str());
    if (!startup_app_uri_.empty()) {
      if (startup_app_uri_.find("http://") == 0) {
        startup_app_timer_.start(1000, this,
              &WebAppManagerServiceAGL::launchStartupAppFromURL);
      } else {
        startup_app_timer_.start(1000, this,
              &WebAppManagerServiceAGL::launchStartupAppFromConfig);
      }
    }
}

void WebAppManagerServiceAGL::launchStartupAppFromConfig()
{
    std::string configfile;
    configfile.append(startup_app_uri_);
    configfile.append("/config.xml");

    xmlDoc *doc = xmlReadFile(configfile.c_str(), nullptr, 0);
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
    fprintf(stdout, "...\n");
    LOG_DEBUG("id: %s", id);
    LOG_DEBUG("version: %s", version);
    LOG_DEBUG("name: %s", name);
    LOG_DEBUG("content: %s", content);
    LOG_DEBUG("description: %s", description);
    LOG_DEBUG("author: %s", author);
    LOG_DEBUG("icon: %s", icon);


    Json::Value obj(Json::objectValue);
    obj["id"] = (const char*)id;
    obj["version"] = (const char*)version;
    obj["vendor"] = (const char*)author;
    obj["type"] = "web";
    obj["main"] = (const char*)content;
    obj["title"] = (const char*)name;
    obj["uiRevision"] = "2";
    obj["icon"] = (const char*)icon;
    obj["folderPath"] = startup_app_uri_.c_str();
    obj["surfaceId"] = startup_app_surface_id_;

    xmlFree(id);
    xmlFree(version);
    xmlFree(name);
    xmlFree(content);
    xmlFree(description);
    xmlFree(author);
    xmlFree(icon);
    xmlFreeDoc(doc);

    std::string appDesc;
    dumpJsonToString(obj, appDesc);
    std::string params;
    std::string app_id = obj["id"].asString();
    int errCode = 0;
    std::string errMsg;
    WebAppManagerService::onLaunch(appDesc, params, app_id, errCode, errMsg);
}

void WebAppManagerServiceAGL::launchStartupAppFromURL()
{
    LOG_DEBUG("WebAppManagerServiceAGL::launchStartupAppFromURL");
    LOG_DEBUG("    url: %s", startup_app_uri_.c_str());
    Json::Value obj(Json::objectValue);
    obj["id"] = startup_app_id_;
    obj["version"] = "1.0";
    obj["vendor"] = "some vendor";
    obj["type"] = "web";
    obj["main"] = startup_app_uri_;
    obj["title"] = "webapp";
    obj["uiRevision"] = "2";
    //obj["icon"] = (const char*)icon;
    //obj["folderPath"] = startup_app_.c_str();
    obj["surfaceId"] = startup_app_surface_id_;

    std::string appDesc;
    dumpJsonToString(obj, appDesc);
    std::string app_id = startup_app_id_;
    int errCode = 0;
    std::string params, errMsg;

    LOG_DEBUG("Launching with appDesc=[%s]", appDesc.c_str());

    if (!startup_proxy_port_.empty()) {
        WebAppManagerService::buildWebViewProfile(app_id, "localhost", startup_proxy_port_);
        LOG_DEBUG("buildWebViewProfile: Done.");
    }

    WebAppManagerService::onLaunch(appDesc, params, app_id, errCode, errMsg);
    LOG_DEBUG("onLaunch: Done.");
}

Json::Value WebAppManagerServiceAGL::launchApp(const Json::Value &request)
{
    return Json::Value(Json::objectValue);
}

Json::Value WebAppManagerServiceAGL::killApp(const Json::Value &request)
{
    return Json::Value(Json::objectValue);
}

Json::Value WebAppManagerServiceAGL::logControl(const Json::Value &request)
{
    return Json::Value(Json::objectValue);
}

Json::Value WebAppManagerServiceAGL::setInspectorEnable(const Json::Value &request)
{
    return Json::Value(Json::objectValue);
}

Json::Value WebAppManagerServiceAGL::closeAllApps(const Json::Value &request)
{
    return Json::Value(Json::objectValue);
}

Json::Value WebAppManagerServiceAGL::discardCodeCache(const Json::Value &request)
{
    return Json::Value(Json::objectValue);
}

Json::Value WebAppManagerServiceAGL::listRunningApps(const Json::Value &request, bool subscribed)
{
    return Json::Value(Json::objectValue);
}

Json::Value WebAppManagerServiceAGL::closeByProcessId(const Json::Value &request)
{
    return Json::Value(Json::objectValue);
}

Json::Value WebAppManagerServiceAGL::getWebProcessSize(const Json::Value &request)
{
    return Json::Value(Json::objectValue);
}

Json::Value WebAppManagerServiceAGL::clearBrowsingData(const Json::Value &request)
{
    return Json::Value(Json::objectValue);
}

Json::Value WebAppManagerServiceAGL::webProcessCreated(const Json::Value &request, bool subscribed)
{
    return Json::Value(Json::objectValue);
}
