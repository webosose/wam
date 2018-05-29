#ifndef WEBRUNTIME_WEBOS_H
#define WEBRUNTIME_WEBOS_H

#include "WebRuntime.h"

class WebRuntimeWebOS : public WebRuntime {
public:
  int run(int argc, const char** argv) override;
};

#endif // WEBRUNTIME_WEBOS_H
