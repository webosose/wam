#ifndef WEBAPPMANAGERSERVICEAGL_H
#define WEBAPPMANAGERSERVICEAGL_H

#include <memory>

#include "WebAppManagerService.h"
#include "Timer.h"

class WamSocket;
class WamSocketLockFile;

class WebAppManagerServiceAGL : public WebAppManagerService {
public:
    static WebAppManagerServiceAGL* instance();

    bool initializeAsHostService();
    bool initializeAsHostClient();

    bool isHostServiceRunning();

    void setStartupApplication(const std::string& startup_app_id,
        const std::string& startup_app_uri, int startup_app_surface_id,
        const std::string& startup_proxy_port =  std::string());

    void launchOnHost(int argc, const char **argv);

    // WebAppManagerService
    bool startService() override;
    Json::Value launchApp(const Json::Value &request) override;
    Json::Value killApp(const Json::Value &request) override;
    Json::Value logControl(const Json::Value &request) override;
    Json::Value setInspectorEnable(const Json::Value &request) override;
    Json::Value closeAllApps(const Json::Value &request) override;
    Json::Value discardCodeCache(const Json::Value &request) override;
    Json::Value listRunningApps(const Json::Value &request, bool subscribed) override;
    Json::Value closeByProcessId(const Json::Value &request) override;
    Json::Value getWebProcessSize(const Json::Value &request) override;
    Json::Value clearBrowsingData(const Json::Value &request) override;
    Json::Value webProcessCreated(const Json::Value &request, bool subscribed) override;

    void triggerStartupApp();

private:

    WebAppManagerServiceAGL();

    void launchStartupAppFromConfig();
    void launchStartupAppFromURL();

    std::string startup_app_id_;
    std::string startup_app_uri_;
    std::string startup_proxy_port_;
    int startup_app_surface_id_;
    OneShotTimer<WebAppManagerServiceAGL> startup_app_timer_;

    std::unique_ptr<WamSocket> socket_;
    std::unique_ptr<WamSocketLockFile> lock_file_;
};

#endif // WEBAPPMANAGERSERVICEAGL_H
