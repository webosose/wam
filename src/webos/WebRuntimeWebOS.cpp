#include "WebRuntimeWebOS.h"

#include "LogManager.h"
#include "PlatformModuleFactoryImpl.h"
#include "WebAppManager.h"

#include "WebAppManagerServiceLuna.h"

#include <webos/app/webos_main.h>

class WebOSMainDelegateWAM : public webos::WebOSMainDelegate {
public:
    void AboutToCreateContentBrowserClient() override {
        WebAppManagerServiceLuna* webAppManagerServiceLuna = WebAppManagerServiceLuna::instance();
        assert(webAppManagerServiceLuna);
        bool result = webAppManagerServiceLuna->startService();
        assert(result);
        WebAppManager::instance()->setPlatformModules(std::unique_ptr<PlatformModuleFactoryTv>(new PlatformModuleFactoryTv()));
    }
};

int WebRuntimeWebOS::run(int argc, const char** argv)
{
  WebOSMainDelegateWAM delegate;
  webos::WebOSMain webOSMain(&delegate);
  return webOSMain.Run(argc, argv);
}
