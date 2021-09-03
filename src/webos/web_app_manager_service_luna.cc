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

#include "web_app_manager_service_luna.h"

#include <string>
#include <vector>

#include <json/json.h>
#include "webos/public/runtime.h"
#include "webos/webview_base.h"

#include "log_manager.h"
#include "utils.h"

// just to save some typing, the template filled out with the name of this class
#define QCB(FUNC) \
  bus_callback_json<WebAppManagerServiceLuna, &WebAppManagerServiceLuna::FUNC>
#define QCB_subscription(FUNC)                             \
  bus_subscription_callback_json<WebAppManagerServiceLuna, \
                                 &WebAppManagerServiceLuna::FUNC>
#define LS2_METHOD_ENTRY(FUNC) \
  { #FUNC, QCB(FUNC) }
#define LS2_SUBSCRIPTION_ENTRY(FUNC) \
  { #FUNC, QCB_subscription(FUNC) }

#define GET_LS2_SERVER_STATUS(FUNC, PARAMS)                        \
  Call<WebAppManagerServiceLuna, &WebAppManagerServiceLuna::FUNC>( \
      "luna://com.palm.lunabus/signal/registerServerStatus", PARAMS, this)
#define LS2_CALL(FUNC, SERVICE, PARAMS)                            \
  Call<WebAppManagerServiceLuna, &WebAppManagerServiceLuna::FUNC>( \
      SERVICE, PARAMS, this)

LSMethod WebAppManagerServiceLuna::methods_[] = {
    LS2_METHOD_ENTRY(launchApp),
    LS2_METHOD_ENTRY(killApp),
    LS2_METHOD_ENTRY(pauseApp),
    LS2_METHOD_ENTRY(closeAllApps),
    LS2_METHOD_ENTRY(setInspectorEnable),
    LS2_METHOD_ENTRY(logControl),
    LS2_METHOD_ENTRY(discardCodeCache),
    LS2_METHOD_ENTRY(getWebProcessSize),
    LS2_METHOD_ENTRY(clearBrowsingData),
    LS2_SUBSCRIPTION_ENTRY(listRunningApps),
    LS2_SUBSCRIPTION_ENTRY(webProcessCreated),
    {0, 0}};

WebAppManagerServiceLuna::WebAppManagerServiceLuna()
    : cleared_cache_(false), boot_done_(false), debug_level_("release") {}

WebAppManagerServiceLuna::~WebAppManagerServiceLuna() {}

bool WebAppManagerServiceLuna::StartService() {
  return PalmServiceBase::StartService();
}

Json::Value WebAppManagerServiceLuna::launchApp(const Json::Value& request) {
  int err_code;
  std::string err_msg;
  Json::Value reply;

  if (!request.isObject() ||
      (!request.isMember("appDesc") || !request["appDesc"].isObject() ||
       !request["appDesc"]["id"].isString()) ||
      (request.isMember("parameters") && !request["parameters"].isObject()) ||
      (request.isMember("launchingAppId") &&
       !request["launchingAppId"].isString()) ||
      (request.isMember("launchingProcId") &&
       !request["launchingProcId"].isString()) ||
      (!request.isMember("instanceId") || !request["instanceId"].isString())) {
    reply["returnValue"] = false;
    reply["errorCode"] = kErrCodeLaunchappMissParam;
    reply["errorText"] = kErrMissParam;
    return reply;
  }

  Json::Value json_params = request["parameters"];
  if (request.isMember("launchHidden") && request["launchHidden"] == true) {
    json_params["launchedHidden"] = true;
  }

  // if "preload" parameter is not "full" or "partial" or "minimal", there is no
  // preload parameter.
  if (request.isMember("preload") && request["preload"].isString()) {
    json_params["preload"] = request["preload"];
  }

  if (request.isMember("keepAlive") && request["keepAlive"] == true) {
    json_params["keepAlive"] = true;
  }

  std::string instance_id = request["instanceId"].asString();
  if (!IsValidInstanceId(instance_id)) {
    reply["returnValue"] = false;
    reply["errorCode"] = kErrCodeLaunchappMissParam;
    reply["errorText"] = kErrMissParam;
    return reply;
  }
  json_params["instanceId"] = instance_id;

  std::string str_params = util::JsonToString(json_params);

  std::string appId = request["appDesc"]["id"].asString();
  LOG_INFO_WITH_CLOCK(
      MSGID_APPLAUNCH_START, 4, PMLOGKS("PerfType", "AppLaunch"),
      PMLOGKS("PerfGroup", appId.c_str()), PMLOGKS("APP_ID", appId.c_str()),
      PMLOGKS("INSTANCE_ID", instance_id.c_str()), "params : %s",
      str_params.c_str());

  std::string str_app_desc = util::JsonToString(request["appDesc"]);
  instance_id = WebAppManagerService::OnLaunch(
      str_app_desc, str_params, request["launchingAppId"].asString(), err_code,
      err_msg);

  if (instance_id.empty()) {
    reply["returnValue"] = false;
    reply["errorCode"] = err_code;
    reply["errorText"] = err_msg;
  } else {
    reply["returnValue"] = true;
    reply["appId"] = request["appDesc"]["id"].asString();
    reply["instanceId"] = instance_id;
  }
  return reply;
}

bool WebAppManagerServiceLuna::IsValidInstanceId(
    const std::string& instanceId) {
  return instanceId.find_first_not_of("\f\n\r\v") != std::string::npos;
}

Json::Value WebAppManagerServiceLuna::killApp(const Json::Value& request) {
  Json::Value reply;

  if (!request.isObject() ||
      (request.isMember("instanceId") && !request["instanceId"].isString()) ||
      (request.isMember("appId") && !request["appId"].isString()) ||
      (request.isMember("reason") && !request["reason"].isString())) {
    reply["returnValue"] = false;
    reply["errorCode"] = kErrCodeKillAppInvalidParam;
    reply["errorText"] = kErrInvalidParam;
    return reply;
  }

  bool instances;
  std::string instance_id = request["instanceId"].asString();
  std::string app_id = request["appId"].asString();
  std::string reason;

  if (request.isMember("reason"))
    reason = request["reason"].asString();

  LOG_INFO(MSGID_LUNA_API, 3, PMLOGKS("APP_ID", app_id.c_str()),
           PMLOGKS("INSTANCE_ID", instance_id.c_str()),
           PMLOGKS("API", "killApp"), "reason : %s", reason.c_str());

  bool memory_reclaim = reason.compare("memoryReclaim") == 0;
  instances =
      WebAppManagerService::OnKillApp(app_id, instance_id, memory_reclaim);

  if (instances) {
    reply["appId"] = app_id;
    reply["instanceId"] = instance_id;
    reply["returnValue"] = true;
  } else {
    reply["returnValue"] = false;
    reply["errorCode"] = kErrCodeNoRunningApp;
    reply["errorText"] = kErrNoRunningApp;
  }
  return reply;
}

Json::Value WebAppManagerServiceLuna::pauseApp(const Json::Value& request) {
  Json::Value reply;

  if (!request.isObject() ||
      (!request.isMember("instanceId") || !request["instanceId"].isString())) {
    reply["returnValue"] = false;
    reply["errorCode"] = kErrCodePauseAppInvalidParam;
    reply["errorText"] = kErrInvalidParam;
    return reply;
  }

  std::string id = request["instanceId"].asString();

  LOG_INFO(MSGID_LUNA_API, 2, PMLOGKS("INSTANCE_ID", id.c_str()),
           PMLOGKS("API", "pauseApp"), "");

  if (WebAppManagerService::OnPauseApp(id)) {
    reply["returnValue"] = true;
    reply["appId"] = request["appId"].asString();
    reply["instanceId"] = request["instanceId"].asString();
  } else {
    reply["returnValue"] = false;
    reply["errorCode"] = kErrCodeNoRunningApp;
    reply["errorText"] = kErrNoRunningApp;
  }
  return reply;
}

Json::Value WebAppManagerServiceLuna::setInspectorEnable(
    const Json::Value& request) {
  LOG_DEBUG("WebAppManagerService::SetInspectorEnable");
  Json::Value reply;
  std::string error_message("Not supported on this platform");

  LOG_DEBUG("errorMessage : %s", error_message.c_str());
  reply["errorMessage"] = error_message;
  reply["returnValue"] = false;
  return reply;
}

Json::Value WebAppManagerServiceLuna::closeAllApps(const Json::Value& request) {
  bool val = WebAppManagerService::OnCloseAllApps();

  Json::Value reply;
  reply["returnValue"] = val;
  return reply;
}

Json::Value WebAppManagerServiceLuna::logControl(const Json::Value& request) {
  if (!request.isObject() ||
      (!request.isMember("keys") || !request["keys"].isString()) ||
      (!request.isMember("value") || !request["value"].isString())) {
    Json::Value reply;
    reply["returnValue"] = false;
    reply["errorCode"] = kErrCodeLogControlInvalidParam;
    reply["errorText"] = kErrInvalidParam;
    return reply;
  }

  return WebAppManagerService::OnLogControl(request["keys"].asString(),
                                            request["value"].asString());
}

Json::Value WebAppManagerServiceLuna::discardCodeCache(
    const Json::Value& request) {
  Json::Value reply;

  if (!request.isObject()) {
    Json::Value reply;
    reply["returnValue"] = false;
    reply["errorCode"] = kErrCodeDiscardCodeCacheInvalidParam;
    reply["errorText"] = kErrInvalidParam;
    return reply;
  }

  bool forced_clear_cache = false;
  uint32_t pid = 0;
  std::list<const WebAppBase*> running;

  if (!WebAppManagerService::IsDiscardCodeCacheRequired()) {
    reply["returnValue"] = true;
    return reply;
  }

  if (request.isMember("force"))
    forced_clear_cache = request["force"] == true;

  if (request.isMember("pid") && request["pid"].isUInt())
    pid = request["pid"].asUInt();

  if (!pid)
    running = WebAppManagerService::RunningApps();
  else
    running = WebAppManagerService::RunningApps(pid);

  if (running.size() != 0 && !forced_clear_cache) {
    reply["returnValue"] = false;
    return reply;
  }

  if (!WebAppManagerService::OnCloseAllApps(pid)) {
    reply["returnValue"] = false;
    return reply;
  }

  cleared_cache_ = true;
  WebAppManagerService::OnDiscardCodeCache(pid);
  if (forced_clear_cache)
    WebAppManagerService::OnPurgeSurfacePool(pid);
  reply["returnValue"] = true;
  return reply;
}

Json::Value WebAppManagerServiceLuna::getWebProcessSize(
    const Json::Value& request) {
  return WebAppManagerService::GetWebProcessProfiling();
}

Json::Value WebAppManagerServiceLuna::listRunningApps(
    const Json::Value& request,
    bool subscribed) {
  bool include_sys_apps = request["includeSysApps"] == true;

  std::vector<ApplicationInfo> apps =
      WebAppManagerService::List(include_sys_apps);

  Json::Value reply;
  Json::Value running_apps;
  for (auto it = apps.begin(); it != apps.end(); ++it) {
    Json::Value app;
    app["id"] = it->app_id_;
    app["instanceId"] = it->instance_id_;
    app["webprocessid"] = std::to_string(it->pid_);
    running_apps.append(app);
  }
  reply["running"] = running_apps;
  reply["returnValue"] = true;
  return reply;
}

Json::Value WebAppManagerServiceLuna::clearBrowsingData(
    const Json::Value& request) {
  Json::Value reply;

  if (!request.isObject()) {
    Json::Value reply;
    reply["returnValue"] = false;
    reply["errorCode"] = kErrCodeClearBrowsingDataInvalidParam;
    reply["errorText"] = kErrInvalidParam;
    return reply;
  }

  Json::Value value = request["types"];
  bool return_value = true;
  int remove_browsing_data_mask = 0;

  switch (value.type()) {
    case Json::ValueType::nullValue:
      remove_browsing_data_mask =
          WebAppManagerService::MaskForBrowsingDataType("all");
      break;
    case Json::ValueType::arrayValue: {
      if (value.size() < 1) {
        reply["errorCode"] = kErrCodeClearDataBrawsingEmptyArray;
        reply["errorText"] = kErrEmptyArray;
        return_value = false;
        break;
      }

      for (Json::Value::ArrayIndex i = 0; i < value.size(); ++i) {
        if (!value[i].isString()) {
          std::stringstream error_text;
          error_text << kErrInvalidValue << " (" << kErrOnlyAllowedForString
                     << ")";
          reply["errorCode"] = kErrCodeClearDataBrawsingInvalidValue;
          reply["errorText"] = error_text.str();
          return_value = false;
          break;
        }

        int mask = WebAppManagerService::MaskForBrowsingDataType(
            value[i].asString().c_str());
        if (mask == 0) {
          std::stringstream error_text;
          error_text << kErrUnknownData << " (" << value[i].asString() << ")";
          reply["errorCode"] = kErrCodeClearDataBrawsingUnknownData;
          reply["errorText"] = error_text.str();
          return_value = false;
          break;
        }

        remove_browsing_data_mask |= mask;
      }
      break;
    }
    default:
      reply["errorCode"] = kErrCodeClearDataBrawsingInvalidValue;
      reply["errorText"] = kErrInvalidValue;
      return_value = false;
  }

  LOG_DEBUG("removeBrowsingDataMask: %d", remove_browsing_data_mask);

  if (return_value)
    WebAppManagerService::OnClearBrowsingData(remove_browsing_data_mask);

  reply["returnValue"] = return_value;
  return reply;
}

void WebAppManagerServiceLuna::DidConnect() {
  Json::Value params;
  params["subscribe"] = true;

  params["serviceName"] = std::string("com.webos.settingsservice");
  if (!GET_LS2_SERVER_STATUS(SystemServiceConnectCallback, params)) {
    LOG_WARNING(MSGID_SERVICE_CONNECT_FAIL, 0,
                "Failed to connect to settingsservice");
  }

  params["serviceName"] = std::string("com.webos.memorymanager");
  if (!GET_LS2_SERVER_STATUS(MemoryManagerConnectCallback, params)) {
    LOG_WARNING(MSGID_MEMORY_CONNECT_FAIL, 0,
                "Failed to connect to memory manager");
  }

  params["serviceName"] = std::string("com.webos.applicationManager");
  if (!GET_LS2_SERVER_STATUS(ApplicationManagerConnectCallback, params)) {
    LOG_WARNING(MSGID_APPMANAGER_CONNECT_FAIL, 0,
                "Failed to connect to application manager");
  }

  params["serviceName"] = std::string("com.webos.bootManager");
  if (!GET_LS2_SERVER_STATUS(BootdConnectCallback, params)) {
    LOG_WARNING(MSGID_BOOTD_CONNECT_FAIL, 0, "Failed to connect to bootd");
  }

  params["serviceName"] = std::string("com.webos.service.connectionmanager");
  if (!GET_LS2_SERVER_STATUS(NetworkConnectionStatusCallback, params)) {
    LOG_WARNING(MSGID_NETWORK_CONNECT_FAIL, 0,
                "Failed to connect to connectionmanager");
  }
}

void WebAppManagerServiceLuna::SystemServiceConnectCallback(
    const Json::Value& reply) {
  if (!reply.isObject()) {
    LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", kErrInvalidParam.c_str());
    return;
  }

  if (reply["connected"] == true) {
    Json::Value locale_params;
    locale_params["subscribe"] = true;
    Json::Value locale_list;
    locale_list.append(std::string("localeInfo"));
    locale_params["keys"] = locale_list;
    LS2_CALL(GetSystemLocalePreferencesCallback,
             "luna://com.webos.settingsservice/getSystemSettings",
             locale_params);
  }
}

void WebAppManagerServiceLuna::GetSystemLocalePreferencesCallback(
    const Json::Value& reply) {
  if (!reply.isObject() || !reply["settings"].isObject()) {
    LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", kErrInvalidParam.c_str());
    return;
  }

  Json::Value locale_info = reply["settings"]["localeInfo"];

  // LocaleInfo(language, etc) is empty when service is crashed
  // The right value will be notified again when service is restarted
  if (!locale_info.isObject() || locale_info.empty() ||
      !locale_info["locales"].isObject() ||
      !locale_info["locales"]["UI"].isString()) {
    std::string doc = util::JsonToString(reply);
    LOG_WARNING(MSGID_RECEIVED_INVALID_SETTINGS, 1,
                PMLOGKFV("MSG", "%s", doc.c_str()), "");
    return;
  }

  std::string language(locale_info["locales"]["UI"].asString());

  LOG_INFO(MSGID_SETTING_SERVICE, 1,
           PMLOGKS("LANGUAGE", language.empty() ? "None" : language.c_str()),
           "");

  if (language.empty())
    return;

  if (language.compare(WebAppManagerService::GetSystemLanguage()) == 0)
    return;

  WebAppManagerService::SetSystemLanguage(language.c_str());
}

void WebAppManagerServiceLuna::MemoryManagerConnectCallback(
    const Json::Value& reply) {
  if (!reply.isObject()) {
    LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", kErrInvalidParam.c_str());
    return;
  }

  if (reply["connected"] == true) {
    Json::Value closeAppObj;
    closeAppObj["subscribe"] = true;
    closeAppObj["appType"] = "web";

    if (!Call<WebAppManagerServiceLuna,
              &WebAppManagerServiceLuna::GetCloseAppIdCallback>(
            "luna://com.webos.memorymanager/getCloseAppId", closeAppObj,
            this)) {
      LOG_WARNING(MSGID_MEM_MGR_API_CALL_FAIL, 0,
                  "Failed to get close application identifier");
    }

    Json::Value threshold_changed;
    threshold_changed["subscribe"] = true;
    threshold_changed["category"] = "/com/webos/memory";
    threshold_changed["method"] = "thresholdChanged";
    if (!Call<WebAppManagerServiceLuna,
              &WebAppManagerServiceLuna::ThresholdChangedCallback>(
            "luna://com.palm.bus/signal/addmatch", threshold_changed, this)) {
      LOG_WARNING(MSGID_SIGNAL_REGISTRATION_FAIL, 0,
                  "Failed to register a client for thresholdChanged");
    }
  }
}

void WebAppManagerServiceLuna::GetCloseAppIdCallback(const Json::Value& reply) {
  if (!reply.isObject() || !reply["pid"].isUInt() ||
      !reply["instanceId"].isString()) {
    LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", kErrInvalidParam.c_str());
    return;
  }

  std::string appId = reply["id"].asString();
  std::string instanceId = reply["instanceId"].asString();

  if (!appId.empty() && !instanceId.empty())
    WebAppManagerService::SetForceCloseApp(appId.c_str(), instanceId.c_str());
}

void WebAppManagerServiceLuna::ThresholdChangedCallback(
    const Json::Value& reply) {
  if (!reply.isObject() || !reply["current"].isString()) {
    LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", kErrInvalidParam.c_str());
    return;
  }

  std::string currentLevel = reply["current"].asString();
  if (currentLevel.empty()) {
    LOG_DEBUG("thresholdChanged without level");
    return;
  }
  LOG_INFO(MSGID_NOTIFY_MEMORY_STATE, 1, PMLOGKS("State", currentLevel.c_str()),
           "");

  webos::WebViewBase::MemoryPressureLevel level;
  if (currentLevel.compare("medium") == 0)
    level = webos::WebViewBase::MEMORY_PRESSURE_LOW;
  else if (currentLevel.compare("critical") == 0 ||
           currentLevel.compare("low") == 0)
    level = webos::WebViewBase::MEMORY_PRESSURE_CRITICAL;
  else
    level = webos::WebViewBase::MEMORY_PRESSURE_NONE;
  WebAppManagerService::NotifyMemoryPressure(level);
}

void WebAppManagerServiceLuna::ApplicationManagerConnectCallback(
    const Json::Value& reply) {
  if (!reply.isObject()) {
    LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", kErrInvalidParam.c_str());
    return;
  }

  if (reply["connected"] == true) {
    Json::Value params;
    params["subscribe"] = true;

    if (!Call<WebAppManagerServiceLuna,
              &WebAppManagerServiceLuna::GetAppStatusCallback>(
            "luna://com.webos.applicationManager/listApps", params, this)) {
      LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0,
                  "Failed to get an application list");
    }

    params["extraInfo"] = true;
    if (!Call<WebAppManagerServiceLuna,
              &WebAppManagerServiceLuna::GetForegroundAppInfoCallback>(
            "luna://com.webos.applicationManager/getForegroundAppInfo", params,
            this)) {
      LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0,
                  "Failed to get foreground application Information");
    }
  }
}

void WebAppManagerServiceLuna::GetAppStatusCallback(const Json::Value& reply) {
  if (!reply.isObject() || !reply["app"].isObject() ||
      !reply["change"].isString()) {
    LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", kErrInvalidParam.c_str());
    return;
  }

  std::string change_kind = reply["change"].asString();
  Json::Value app_object = reply["app"];

  if (change_kind.compare("removed") == 0) {
    std::string app_id =
        app_object["id"].isString() ? app_object["id"].asString() : "";
    LOG_INFO(MSGID_WAM_DEBUG, 0, "Application removed %s", app_id.c_str());
    WebAppManagerService::OnAppRemoved(app_id);
  }
  if (change_kind.compare("added") == 0) {
    std::string app_id =
        app_object["id"].isString() ? app_object["id"].asString() : "";
    LOG_INFO(MSGID_WAM_DEBUG, 0, "Application installed %s", app_id.c_str());
    WebAppManagerService::OnAppInstalled(app_id);
  }
  if (change_kind.compare("removed") == 0 ||
      change_kind.compare("updated") == 0) {
    std::string app_base_path = app_object["folderPath"].isString()
                                    ? app_object["folderPath"].asString()
                                    : "";
    bool is_custom_plugin = app_object["customPlugin"] == true;

    if (is_custom_plugin) {
      WebAppManagerService::KillCustomPluginProcess(app_base_path.c_str());
    }
  }
}

void WebAppManagerServiceLuna::GetForegroundAppInfoCallback(
    const Json::Value& reply) {
  if (!reply.isObject()) {
    LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", kErrInvalidParam.c_str());
    return;
  }

  if (cleared_cache_)
    cleared_cache_ = false;

  if (reply["returnValue"] == true) {
    if (reply.isMember("appId") && reply["appId"].isString()) {
      std::string appId = reply["appId"].asString();
      webos::Runtime::GetInstance()->SetIsForegroundAppEnyo(
          WebAppManagerService::IsEnyoApp(appId.c_str()));
    }
  }
}

void WebAppManagerServiceLuna::BootdConnectCallback(const Json::Value& reply) {
  if (!reply.isObject()) {
    LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", kErrInvalidParam.c_str());
    return;
  }

  if (reply["connected"] == true) {
    Json::Value subscribe;
    subscribe["subscribe"] = true;
    if (!LS2_CALL(GetBootStatusCallback,
                  "luna://com.webos.bootManager/getBootStatus", subscribe)) {
      LOG_WARNING(MSGID_BOOTD_SUBSCRIBE_FAIL, 0,
                  "Failed to subscribe to bootManager");
    }
  }
}

void WebAppManagerServiceLuna::GetBootStatusCallback(const Json::Value& reply) {
  if (!reply.isObject() || !reply["signals"].isObject()) {
    LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", kErrInvalidParam.c_str());
    return;
  }

  boot_done_ = reply["signals"]["boot-done"] == true;
}

void WebAppManagerServiceLuna::CloseApp(const std::string& id) {
  Json::Value json;
  json["instanceId"] = id;

  if (!LS2_CALL(CloseAppCallback, "luna://com.webos.applicationManager/close",
                json))
    LOG_WARNING(MSGID_CLOSE_CALL_FAIL, 0,
                "Failed to send closeByAppId command to SAM");
}

void WebAppManagerServiceLuna::CloseAppCallback(const Json::Value& reply) {
  // TODO: check reply and close app again.
}

Json::Value WebAppManagerServiceLuna::webProcessCreated(
    const Json::Value& request,
    bool subscribed) {
  Json::Value reply;

  if (!request.isObject()) {
    reply["returnValue"] = false;
    reply["errorCode"] = kErrCodeWebProcessCreatedInvalidParam;
    reply["errorText"] = kErrInvalidParam;
    return reply;
  }

  std::string app_id =
      request["appId"].isString() ? request["appId"].asString() : "";
  if (!app_id.empty()) {
    std::string instance_id = request["instanceId"].isString()
                                  ? request["instanceId"].asString()
                                  : "";
    int pid = WebAppManagerService::GetWebProcessId(app_id.c_str(),
                                                    instance_id.c_str());
    reply["id"] = app_id;
    reply["instanceId"] = instance_id;

    if (pid) {
      reply["webprocessid"] = pid;
      reply["returnValue"] = true;
    } else {
      reply["returnValue"] = false;
      reply["errorText"] = "process is not running";
    }
  } else if (subscribed) {
    reply["returnValue"] = true;
  } else {
    reply["returnValue"] = false;
    reply["errorText"] = "parameter error";
  }

  return reply;
}

void WebAppManagerServiceLuna::NetworkConnectionStatusCallback(
    const Json::Value& reply) {
  if (!reply.isObject()) {
    LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", kErrInvalidParam.c_str());
    return;
  }

  if (reply["connected"] == true) {
    LOG_DEBUG("connectionmanager is connected");
    Json::Value subscribe;
    subscribe["subscribe"] = true;
    if (!LS2_CALL(GetNetworkConnectionStatusCallback,
                  "luna://com.palm.connectionmanager/getStatus", subscribe)) {
      LOG_WARNING(MSGID_LS2_CALL_FAIL, 0,
                  "Fail to subscribe to connection manager");
    }
  }
}

void WebAppManagerServiceLuna::GetNetworkConnectionStatusCallback(
    const Json::Value& reply) {
  // luna-send -f -n 1 luna://com.webos.service.connectionmanager/getstatus
  // '{"subscribe": true}'
  WebAppManagerService::UpdateNetworkStatus(reply);
}
