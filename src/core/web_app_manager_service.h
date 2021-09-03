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

#ifndef CORE_WEB_APP_MANAGER_SERVICE_H_
#define CORE_WEB_APP_MANAGER_SERVICE_H_

#include <list>
#include <string>
#include <vector>

#include "web_app_manager.h"
#include "webos/webview_base.h"

namespace Json {
class Value;
};

enum ErrorCode {
  kErrCodeLaunchappMissParam = 1000,
  kErrCodeLaunchappUnsupportedType = 1001,
  kErrCodeLaunchappInvalidTrustlevel = 1002,
  kErrCodeNoRunningApp = 2000,
  kErrCodeClearDataBrawsingEmptyArray = 3000,
  kErrCodeClearDataBrawsingInvalidValue = 3001,
  kErrCodeClearDataBrawsingUnknownData = 3002,
  kErrCodeKillAppInvalidParam = 4100,
  kErrCodePauseAppInvalidParam = 4200,
  kErrCodeLogControlInvalidParam = 4300,
  kErrCodeDiscardCodeCacheInvalidParam = 4400,
  kErrCodeClearBrowsingDataInvalidParam = 4500,
  kErrCodeWebProcessCreatedInvalidParam = 4600
};

const std::string kErrInvalidParam =
    "Incoming JSON is invalid or not completed";

const std::string kErrMissParam = "Miss launch parameter(s)";
const std::string kErrUnsupportedType = "Unsupported app type (Check subType)";
const std::string kErrInvalidTrustLevel =
    "Invalid trust level (Check trustLevel)";

const std::string kErrNoRunningApp = "App is not running";

const std::string kErrEmptyArray = "Empty array is not allowed.";
const std::string kErrInvalidValue = "Invalid value";
const std::string kErrUnknownData = "Unknown data";
const std::string kErrOnlyAllowedForString = "Only allowed for string type";

class WebAppBase;

class WebAppManagerService {
 public:
  WebAppManagerService();
  virtual ~WebAppManagerService() = default;

  virtual bool StartService() = 0;
  // methods published to the bus
  virtual Json::Value launchApp(const Json::Value& request) = 0;
  virtual Json::Value killApp(const Json::Value& request) = 0;
  virtual Json::Value pauseApp(const Json::Value& request) = 0;
  virtual Json::Value logControl(const Json::Value& request) = 0;
  virtual Json::Value setInspectorEnable(const Json::Value& request) = 0;
  virtual Json::Value closeAllApps(const Json::Value& request) = 0;
  virtual Json::Value discardCodeCache(const Json::Value& request) = 0;
  virtual Json::Value listRunningApps(const Json::Value& request,
                                      bool subscribed) = 0;
  virtual Json::Value getWebProcessSize(const Json::Value& request) = 0;
  virtual Json::Value clearBrowsingData(const Json::Value& request) = 0;
  virtual Json::Value webProcessCreated(const Json::Value& request,
                                        bool subscribed) = 0;

 protected:
  std::string OnLaunch(const std::string& app_desc_string,
                       const std::string& params,
                       const std::string& launching_app_id,
                       int& err_code,
                       std::string& errMsg);

  bool OnKillApp(const std::string& appId,
                 const std::string& instance_id,
                 bool force = false);
  bool OnPauseApp(const std::string& instance_id);
  Json::Value OnLogControl(const std::string& keys, const std::string& value);
  bool OnCloseAllApps(uint32_t pid = 0);
  bool IsDiscardCodeCacheRequired();
  void OnDiscardCodeCache(uint32_t pid);
  bool OnPurgeSurfacePool(uint32_t pid);
  Json::Value GetWebProcessProfiling();
  int MaskForBrowsingDataType(const char* type);
  void OnClearBrowsingData(const int remove_browsing_data_mask);
  void OnAppInstalled(const std::string& app_id);
  void OnAppRemoved(const std::string& app_id);

  void SetDeviceInfo(const std::string& name, const std::string& value);
  void SetUiSize(int width, int height);
  void SetSystemLanguage(const std::string& language);
  std::string GetSystemLanguage();
  void SetForceCloseApp(const std::string& appId,
                        const std::string& instance_id);
  void DeleteStorageData(const std::string& identifier);
  void KillCustomPluginProcess(const std::string& app_base_path);
  void RequestKillWebProcess(uint32_t pid);
  void UpdateNetworkStatus(const Json::Value& object);
  void NotifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level);
  void SetAccessibilityEnabled(bool enable);
  uint32_t GetWebProcessId(const std::string& app_id,
                           const std::string& instance_id);

  std::list<const WebAppBase*> RunningApps();
  std::list<const WebAppBase*> RunningApps(uint32_t pid);
  std::vector<ApplicationInfo> List(bool include_system_apps = false);

  bool IsEnyoApp(const std::string& appp_id);
};

#endif  // CORE_WEB_APP_MANAGER_SERVICE_H_
