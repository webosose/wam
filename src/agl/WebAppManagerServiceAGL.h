#ifndef WEBAPPMANAGERSERVICEAGL_H
#define WEBAPPMANAGERSERVICEAGL_H

#include <QJsonObject>

#include "WebAppManagerService.h"
#include "Timer.h"

class singleton_socket;

class WebAppManagerServiceAGL : public WebAppManagerService {
public:
    static WebAppManagerServiceAGL* instance();

    bool isHostService();
    void setStartupApplication(const std::string& app, int surface_id);

    void launchOnHost(int argc, const char **argv);

    // WebAppManagerService
    bool startService() override;
    QJsonObject launchApp(QJsonObject request) override;
    QJsonObject killApp(QJsonObject request) override;
    QJsonObject logControl(QJsonObject request) override;
    QJsonObject setInspectorEnable(QJsonObject request) override;
    QJsonObject closeAllApps(QJsonObject request) override;
    QJsonObject discardCodeCache(QJsonObject request) override;
    QJsonObject listRunningApps(QJsonObject request, bool subscribed) override;
    QJsonObject closeByProcessId(QJsonObject request) override;
    QJsonObject getWebProcessSize(QJsonObject request) override;
    QJsonObject clearBrowsingData(QJsonObject request) override;
    QJsonObject webProcessCreated(QJsonObject request, bool subscribed) override;

    void triggerStartupApp();

private:

    WebAppManagerServiceAGL();
    ~WebAppManagerServiceAGL();

    void launchStartupApp();

    std::string startup_app_;
    int surface_id_;
    OneShotTimer<WebAppManagerServiceAGL> startup_app_timer_;
 
    singleton_socket *socket_;
};

#endif // WEBAPPMANAGERSERVICEAGL_H
