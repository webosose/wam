// Copyright (c) 2022 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "web_app_luna.h"

#include <luna-service2/lunaservice.h>

#include "util/log_manager.h"

namespace {

class Error : public LSError {
 public:
  Error() { LSErrorInit(this); }
  virtual ~Error() { LSErrorFree(this); }
  void PrintLog() {
    LOG_DEBUG("%d: %s (%s @ %s:%d)", error_code, message, func, file, line);
  }
};

}  // namespace

static LSHandle* GetHandle() {
  static LSHandle* s_handle = nullptr;
  if (s_handle) {
    return s_handle;
  }
  Error error;
  if (!LSRegisterApplicationService("com.webos.notification.client", nullptr,
                                    &s_handle, &error)) {
    error.PrintLog();
    return s_handle;  // nullptr
  }

  if (!LSGmainContextAttach(s_handle, g_main_context_default(), &error)) {
    error.PrintLog();
    if (!LSUnregister(s_handle, &error)) {
      error.PrintLog();
    }
    s_handle = nullptr;
    return s_handle;
  }
  return s_handle;
}

static bool HandleResponse(LSHandle* sh, LSMessage* reply, void* ctx) {
  return true;
}

WebAppLuna::WebAppLuna(const std::string& app_id) : app_id_(app_id) {
  GetHandle();
}

WebAppLuna::~WebAppLuna() = default;

bool WebAppLuna::Call(const char* uri, const char* param) {
  Error error;
  LSMessageToken token;
  if (!LSCallFromApplicationOneReply(GetHandle(), uri, param, app_id_.c_str(),
                                     HandleResponse, nullptr, &token, &error)) {
    error.PrintLog();
    return false;
  }
  return true;
}
