#ifndef WEBAPPMANAGERSERVICEAGL_H
#define WEBAPPMANAGERSERVICEAGL_H

#include <memory>

#include <QJsonObject>

#include "WebAppManagerService.h"
#include "Timer.h"

class WamSocket;
class WamSocketLockFile;

class WebAppManagerServiceAGL : public WebAppManagerService {
public:
    static WebAppManagerServiceAGL* instance();

    bool initializeAsHostService();
    bool initializeAsHostClient();

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

    void launchStartupAppFromConfig();
    void launchStartupAppFromURL();

    std::string startup_app_;
    int surface_id_;
    OneShotTimer<WebAppManagerServiceAGL> startup_app_timer_;

    std::unique_ptr<WamSocket> socket_;
    std::unique_ptr<WamSocketLockFile> lock_file_;
};

#endif // WEBAPPMANAGERSERVICEAGL_H
