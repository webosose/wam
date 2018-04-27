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

using namespace std;

class singleton_socket {
public:
  singleton_socket()
  {
    const char wam_socket_lock_path[] = "/tmp/wamsocket.lock";

    // Create the socket file descriptor
    socket_fd_ = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (socket_fd_ == -1) {
      fprintf(stderr, "Failed to open socket file descriptor\r\n");
      return;
    }

    // Create the lock file
    lock_fd_ = open(wam_socket_lock_path, O_CREAT | O_TRUNC, S_IRWXU);
    if (lock_fd_ == -1) {
      close(socket_fd_);
      fprintf(stderr, "Failed to open lock file descriptor\r\n");
      return;
    }
    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path, wam_socket_path, sizeof(sock_addr.sun_path));
  }
  ~singleton_socket()
  {
    if (lock_fd_ != -1)
      close(lock_fd_);
    if (socket_fd_ != -1)
      close(socket_fd_);
  }
  bool am_i_the_first()
  {
    int ret;

    // Obtain the lock
    ret = flock(lock_fd_, LOCK_EX | LOCK_NB);
    if (ret != 0) {
      close(lock_fd_);
      lock_fd_ = -1;

      ret = connect(socket_fd_, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
      if (ret != 0)
        fprintf(stderr, "Failed to connect to named socket");
      return false;
    }
    // Unlink the named socket, otherwise bind may give address already in use
    unlink(wam_socket_path);

    // Bind it to the named socket
    ret = bind(socket_fd_, (struct sockaddr *) &sock_addr, sizeof(struct sockaddr_un));
    if (ret != 0)
      fprintf(stderr, "Failed to bind to named socket");
    return true;
  }
  void send_msg(int argc, const char **argv)
  {
    string cmd;
    for (int i = 0; i < argc; ++i)
      cmd.append(argv[i]).append(" ");
    // Remove the last appended space if any
    if (argc > 1)
      cmd.pop_back();
    cout << "Sending message=[" << cmd << "]" << endl;
    ssize_t bytes = write(socket_fd_, cmd.c_str(), cmd.length());
    cout << "Wrote " << bytes << "bytes" << endl;
  }
  int wait_for_msg()
  {
    char buf[PATH_MAX] = {};
    ssize_t bytes;

    cout << "Waiting for data..." << endl;
    while (TEMP_FAILURE_RETRY((bytes = recv(socket_fd_, (void *)buf, sizeof(buf), 0)) != -1)) {
      int last = bytes - 1;
      // Remove the new line if there's one
      if (buf[last] == '\n')
        buf[last] = '\0';
      cout << "Got " << bytes << " bytes=[" << buf << "]" << endl;

      std::string data(buf);
      std::istringstream iss(data);
      std::vector<const char*> res;
      for(std::string s; iss >> s; ) {
          res.push_back(strdup(s.c_str()));
      }

      WebAppManagerServiceAGL::instance()->setStartupApplication(std::string(res[0]), atoi(res[1]));
      WebAppManagerServiceAGL::instance()->triggerStartupApp();
      return 0;
    }
    return 0;
  }
private:
  static const char wam_socket_path[];
  int lock_fd_;
  int socket_fd_;
  struct sockaddr_un sock_addr;
};
// static
const char singleton_socket::wam_socket_path[]      = "/tmp/wamsocket";

WebAppManagerServiceAGL::WebAppManagerServiceAGL()
  : socket_(new singleton_socket())
{
}

WebAppManagerServiceAGL::~WebAppManagerServiceAGL()
{
   delete socket_;
}

WebAppManagerServiceAGL* WebAppManagerServiceAGL::instance()
{
    static WebAppManagerServiceAGL *srv = new WebAppManagerServiceAGL();
    return srv;
}

bool WebAppManagerServiceAGL::isHostService()
{
    return socket_->am_i_the_first();
}

void WebAppManagerServiceAGL::launchOnHost(int argc, const char **argv)
{
    socket_->send_msg(argc, argv);
}

void WebAppManagerServiceAGL::setStartupApplication(const std::string& app, int surface_id)
{
    startup_app_ = app;
    surface_id_ = surface_id;
}

void *run_socket(void *socket) {
  singleton_socket *s = (singleton_socket*)socket;
  s->wait_for_msg();
  return 0;
}

bool WebAppManagerServiceAGL::startService()
{
    pthread_t thread_id;
    if( pthread_create( &thread_id , nullptr,  run_socket, socket_) < 0) {
        perror("could not create thread");
        return false;
    }

    triggerStartupApp();

    return true;
}

void WebAppManagerServiceAGL::triggerStartupApp()
{
    if (!startup_app_.empty()) {
      fprintf(stderr, "Startup app: %s\r\n", startup_app_.c_str());
      startup_app_timer_.start(1000, this, 
          &WebAppManagerServiceAGL::launchStartupApp);
    }
}

void WebAppManagerServiceAGL::launchStartupApp()
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
