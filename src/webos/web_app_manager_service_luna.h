// Copyright (c) 2014-2021 LG Electronics, Inc.
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

#ifndef WEBOS_WEB_APP_MANAGER_SERVICE_LUNA_H_
#define WEBOS_WEB_APP_MANAGER_SERVICE_LUNA_H_

#include <string>

#include "palm_service_base.h"
#include "web_app_manager_service.h"

namespace Json {
class Value;
};

class WebAppManagerServiceLuna : public PalmServiceBase,
                                 public WebAppManagerService {
 public:
  WebAppManagerServiceLuna();
  ~WebAppManagerServiceLuna() override;

  static WebAppManagerServiceLuna* Instance();

  bool StartService();

  // WebAppManagerService
  // NOTE: Names of the functions are used for LUNA mapping so, we keep them in
  // lowerCamelCase for compatibility with LUNA definitions
  Json::Value launchApp(const Json::Value& request) override;
  Json::Value killApp(const Json::Value& request) override;
  Json::Value logControl(const Json::Value& request) override;
  Json::Value setInspectorEnable(const Json::Value& request) override;
  Json::Value closeAllApps(const Json::Value& request) override;
  Json::Value discardCodeCache(const Json::Value& request) override;
  Json::Value listRunningApps(const Json::Value& request,
                              bool subscribed) override;
  Json::Value getWebProcessSize(const Json::Value& request) override;
  Json::Value pauseApp(const Json::Value& request) override;
  Json::Value clearBrowsingData(const Json::Value& request) override;
  Json::Value webProcessCreated(const Json::Value& request,
                                bool subscribed) override;

  // PlamServiceBase
  void DidConnect() override;

  // WebAppManagerServiceLuna
  virtual void SystemServiceConnectCallback(const Json::Value& reply);

  void GetSystemLocalePreferencesCallback(const Json::Value& reply);

  void MemoryManagerConnectCallback(const Json::Value& reply);
  void GetCloseAppIdCallback(const Json::Value& reply);
  void ThresholdChangedCallback(const Json::Value& reply);

  void ApplicationManagerConnectCallback(const Json::Value& reply);
  void GetAppStatusCallback(const Json::Value& reply);
  void GetForegroundAppInfoCallback(const Json::Value& reply);

  void BootdConnectCallback(const Json::Value& reply);
  void GetBootStatusCallback(const Json::Value& reply);

  void NetworkConnectionStatusCallback(const Json::Value& reply);
  void GetNetworkConnectionStatusCallback(const Json::Value& reply);

  void CloseApp(const std::string& id);
  void CloseAppCallback(const Json::Value& reply);

 protected:
  // methods implementation of PalmServiceBase
  LSMethod* Methods() const override { return methods_; };
  const char* ServiceName() const override { return "com.palm.webappmanager"; };

  static LSMethod methods_[];

  bool cleared_cache_;
  bool boot_done_;
  std::string debug_level_;

 private:
  bool IsValidInstanceId(const std::string& instance_id);
};

#endif  // WEBOS_WEB_APP_MANAGER_SERVICE_LUNA_H_
