#ifndef WEB_RUNTIME_H
#define WEB_RUNTIME_H

#include <string>
#include <vector>

class WebRuntime {
public:
  virtual int run(int argc, const char** argv) = 0;
};

#endif // WEB_RUNTIME_H
