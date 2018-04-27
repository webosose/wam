#include "WebAppManagerServiceAGL.h"

#include <libxml/parser.h>

#include <QJsonDocument>

WebAppManagerServiceAGL::WebAppManagerServiceAGL()
{
}

WebAppManagerServiceAGL::~WebAppManagerServiceAGL()
{
}

WebAppManagerService* WebAppManagerServiceAGL::instance()
{
    static WebAppManagerServiceAGL *srv = new WebAppManagerServiceAGL();
    return srv;
}

void WebAppManagerServiceAGL::setStartupApplication(const std::string& app)
{
  startup_app_ = app;
}

bool WebAppManagerServiceAGL::startService()
{
    if (!startup_app_.empty()) {
      fprintf(stderr, "Startup app: %s\r\n", startup_app_.c_str());
      startup_app_timer_.start(1000, this, 
          &WebAppManagerServiceAGL::launchStartupApp);
    }
    return true;
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
