#include "WebAppManagerServiceAGL.h"

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
    fprintf(stderr, "WebAppManagerServiceAGL::startService\r\n");
    if (!startup_app_.empty()) {
      fprintf(stderr, "    Startup app: %s\r\n", startup_app_.c_str());
      startup_app_timer_.start(1000, this, 
          &WebAppManagerServiceAGL::launchStartupApp);
    }
    return true;
}

void WebAppManagerServiceAGL::launchStartupApp()
{
  fprintf(stderr, "WebAppManagerServiceAGL::launchStartupApp\r\n");

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
