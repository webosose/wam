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

#include <libxml/parser.h>

#include <QJsonDocument>

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
      fprintf(stderr, "Failed to lock file %d\r\n", lock_fd_);
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
      fprintf(stderr, "Failed to open lock file descriptor\r\n");
      return fd;
    }

    int flags = fcntl(fd, F_GETFD);
    if (flags == -1)
      fprintf(stderr, "Could not get flags for lock file %d\r\n", fd);

     flags |= FD_CLOEXEC;

     if (fcntl(fd, F_SETFD, flags) == -1)
       fprintf(stderr, "Could not set flags for lock file %d\r\n", fd);

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
      fprintf(stderr, "Failed to open socket file descriptor\r\n");
      return false;
    }

    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path, wam_socket_path_.c_str(), sizeof(sock_addr.sun_path));

    if (server) {
      fprintf(stderr, "service binding\r\n");
      unlink(wam_socket_path_.c_str());
      if (bind(socket_fd_, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un)) != 0) {
        fprintf(stderr, "Failed to bind to named socket");
        return false;
      }
    } else {
      fprintf(stderr, "client connecting\r\n");
      if (connect(socket_fd_, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un)) != 0) {
        fprintf(stderr, "Failed to connect to named socket");
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
    std::cout << "Sending message=[" << cmd << "]" << std::endl;
    ssize_t bytes = write(socket_fd_, cmd.c_str(), cmd.length());
    std::cout << "Wrote " << bytes << "bytes" << std::endl;
  }

  int waitForMsg() {
    char buf[PATH_MAX] = {};
    ssize_t bytes;

    std::cout << "Waiting for data..." << std::endl;
    while (TEMP_FAILURE_RETRY((bytes = recv(socket_fd_, (void *)buf, sizeof(buf), 0)) != -1)) {
      int last = bytes - 1;
      // Remove the new line if there's one
      if (buf[last] == '\n')
        buf[last] = '\0';
      std::cout << "Got " << bytes << " bytes=[" << buf << "]" << std::endl;

      std::string data(buf);
      std::istringstream iss(data);
      std::vector<const char*> res;
      for(std::string s; iss >> s; ) {
          res.push_back(strdup(s.c_str()));
      }

      WebAppManagerServiceAGL::instance()->setStartupApplication(std::string(res[0]), atoi(res[1]));
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
    socket_->sendMsg(argc, argv);
}

void WebAppManagerServiceAGL::setStartupApplication(const std::string& app, int surface_id)
{
    startup_app_ = app;
    surface_id_ = surface_id;
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
          fprintf(stderr, "Coudlnt create thread...\r\n");
          return false;
      }
    }

    triggerStartupApp();

    return true;
}

void WebAppManagerServiceAGL::triggerStartupApp()
{
    if (!startup_app_.empty()) {
      if (startup_app_.find("http://") == 0) {
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
    configfile.append(startup_app_);
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
    fprintf(stderr, "id: %s\r\n", id);
    fprintf(stderr, "version: %s\r\n", version);
    fprintf(stderr, "name: %s\r\n", name);
    fprintf(stderr, "content: %s\r\n", content);
    fprintf(stderr, "description: %s\r\n", description);
    fprintf(stderr, "author: %s\r\n", author);
    fprintf(stderr, "icon: %s\r\n", icon);


    QJsonObject obj;
    obj["id"] = QJsonValue((const char*)id);
    obj["version"] = QJsonValue((const char*)version);
    obj["vendor"] = QJsonValue((const char*)author);
    obj["type"] = QJsonValue("web");
    obj["main"] = QJsonValue((const char*)content);
    obj["title"] = QJsonValue((const char*)name);
    obj["uiRevision"] = QJsonValue("2");
    obj["icon"] = QJsonValue((const char*)icon);
    obj["folderPath"] = QJsonValue(startup_app_.c_str());
    obj["surfaceId"] = QJsonValue(surface_id_);

    xmlFree(id);
    xmlFree(version);
    xmlFree(name);
    xmlFree(content);
    xmlFree(description);
    xmlFree(author);
    xmlFree(icon);
    xmlFreeDoc(doc);

    QJsonDocument appDoc(obj);
    std::string appDesc = appDoc.toJson(QJsonDocument::Compact).toStdString();
    std::string params;
    std::string app_id = obj["id"].toString().toStdString();
    int errCode = 0;
    std::string errMsg;
    WebAppManagerService::onLaunch(appDesc, params, app_id, errCode, errMsg);
}

void WebAppManagerServiceAGL::launchStartupAppFromURL()
{
    QJsonObject obj;
    obj["id"] = QJsonValue("some.id");
    obj["version"] = QJsonValue("1.0");
    obj["vendor"] = QJsonValue("some vendor");
    obj["type"] = QJsonValue("web");
    obj["main"] = QJsonValue((const char*)startup_app_.c_str());
    obj["title"] = QJsonValue("webapp");
    obj["uiRevision"] = QJsonValue("2");
    //obj["icon"] = QJsonValue((const char*)icon);
    //obj["folderPath"] = QJsonValue(startup_app_.c_str());
    obj["surfaceId"] = QJsonValue(surface_id_);

    QJsonDocument appDoc(obj);
    std::string appDesc = appDoc.toJson(QJsonDocument::Compact).toStdString();
    std::string params;
    std::string app_id = obj["id"].toString().toStdString();
    int errCode = 0;
    std::string errMsg;
    WebAppManagerService::onLaunch(appDesc, params, app_id, errCode, errMsg);
}

QJsonObject WebAppManagerServiceAGL::launchApp(QJsonObject request)
{
    return QJsonObject();
}

QJsonObject WebAppManagerServiceAGL::killApp(QJsonObject request)
{
    return QJsonObject();
}

QJsonObject WebAppManagerServiceAGL::logControl(QJsonObject request)
{
    return QJsonObject();
}

QJsonObject WebAppManagerServiceAGL::setInspectorEnable(QJsonObject request)
{
    return QJsonObject();
}

QJsonObject WebAppManagerServiceAGL::closeAllApps(QJsonObject request)
{
    return QJsonObject();
}


QJsonObject WebAppManagerServiceAGL::discardCodeCache(QJsonObject request)
{
    return QJsonObject();
}

QJsonObject WebAppManagerServiceAGL::listRunningApps(QJsonObject request, bool subscribed)
{
    return QJsonObject();
}

QJsonObject WebAppManagerServiceAGL::closeByProcessId(QJsonObject request)
{
    return QJsonObject();
}

QJsonObject WebAppManagerServiceAGL::getWebProcessSize(QJsonObject request)
{
    return QJsonObject();
}

QJsonObject WebAppManagerServiceAGL::clearBrowsingData(QJsonObject request)
{
    return QJsonObject();
}

QJsonObject WebAppManagerServiceAGL::webProcessCreated(QJsonObject request, bool subscribed)
{
    return QJsonObject();
}
