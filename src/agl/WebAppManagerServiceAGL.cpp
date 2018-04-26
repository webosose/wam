#include "WebAppManagerServiceAGL.h"

#include <QFile>
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
    QString filename = QString::fromStdString(startup_app_);
    filename.append("/appinfo.json");
    QFile file;
    file.setFileName(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString appinfo = file.readAll();
    file.close();

    QJsonDocument json = QJsonDocument::fromJson(appinfo.toUtf8());
    QJsonObject obj = json.object();
    obj["folderPath"] = QJsonValue(startup_app_.c_str());

    QJsonDocument doc(obj);
    std::string appDesc = doc.toJson(QJsonDocument::Compact).toStdString();
    std::string params;
    std::string id = obj["id"].toString().toStdString();
    int errCode = 0;
    std::string errMsg;
    WebAppManagerService::onLaunch(appDesc, params, id, errCode, errMsg);
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
