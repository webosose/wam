#ifndef WEBAPPMANAGERSERVICEAGL_H
#define WEBAPPMANAGERSERVICEAGL_H

#include <memory>

#include "WebAppManagerService.h"
#include "Timer.h"

constexpr char kStartApp[] = "start-app";
constexpr char kActivateEvent[] = "activate-event";
constexpr char kDeactivateEvent[] = "deactivate-event";

class WamSocket;
class WamSocketLockFile;

class WebAppManagerServiceAGL : public WebAppManagerService {
public:
    static WebAppManagerServiceAGL* instance();

    bool initializeAsHostService();
    bool initializeAsHostClient();

    bool isHostServiceRunning();

    void setStartupApplication(const std::string& startup_app_id,
        const std::string& startup_app_uri, int startup_app_surface_id);
    void setAppIdForEventTarget(const std::string& app_id);

    void launchOnHost(int argc, const char **argv);
    void sendEvent(int argc, const char **argv);

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
    void triggetEventForApp(const std::string& action);

private:

    WebAppManagerServiceAGL();

    void launchStartupAppFromConfig();
    void launchStartupAppFromURL();

    void onActivateEvent();
    void onDeactivateEvent();

    std::string app_id_event_target_;

    std::string startup_app_id_;
    std::string startup_app_uri_;
    int startup_app_surface_id_;
    OneShotTimer<WebAppManagerServiceAGL> startup_app_timer_;

    std::unique_ptr<WamSocket> socket_;
    std::unique_ptr<WamSocketLockFile> lock_file_;
};

#endif // WEBAPPMANAGERSERVICEAGL_H
