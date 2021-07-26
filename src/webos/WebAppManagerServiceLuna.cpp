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


#include "WebAppManagerServiceLuna.h"

#include <string>
#include <vector>

#include <json/json.h>

#include "LogManager.h"
#include "Utils.h"
#include "webos/public/runtime.h"
#include "webos/webview_base.h"

// just to save some typing, the template filled out with the name of this class
#define QCB(FUNC) bus_callback_json<WebAppManagerServiceLuna, &WebAppManagerServiceLuna::FUNC>
#define QCB_subscription(FUNC) bus_subscription_callback_json<WebAppManagerServiceLuna, &WebAppManagerServiceLuna::FUNC>
#define LS2_METHOD_ENTRY(FUNC) {#FUNC, QCB(FUNC)}
#define LS2_SUBSCRIPTION_ENTRY(FUNC) {#FUNC, QCB_subscription(FUNC)}

#define GET_LS2_SERVER_STATUS(FUNC, PARAMS) call<WebAppManagerServiceLuna, &WebAppManagerServiceLuna::FUNC>("luna://com.palm.lunabus/signal/registerServerStatus", PARAMS, this)
#define LS2_CALL(FUNC, SERVICE, PARAMS) call<WebAppManagerServiceLuna, &WebAppManagerServiceLuna::FUNC>(SERVICE, PARAMS, this)

static void logJsonTruncated(const char* funcName, const Json::Value& request)
{
    std::string requestBuffer = util::jsonToString(request);
    const size_t chunkSize = 255;
    const size_t chunksCount = requestBuffer.size() % chunkSize ? (requestBuffer.size() / chunkSize) + 1 : requestBuffer.size() / chunkSize;
    for (size_t i = 0, part = 1; i < requestBuffer.size(); i += chunkSize, part++) {
        const auto& chunk = requestBuffer.substr(i, chunkSize);
        LOG_INFO(MSGID_WAM_DEBUG, 0, ">>>>>>> WebAppManagerServiceLuna::%s [%u/%u] request:\"%s\"", funcName, part, chunksCount, chunk.c_str());
    }
}

LSMethod WebAppManagerServiceLuna::s_methods[] = {
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
    { 0, 0 }
};

WebAppManagerServiceLuna::WebAppManagerServiceLuna()
    : m_clearedCache(false)
    , m_bootDone(false)
    , m_debugLevel("release")
{
}

WebAppManagerServiceLuna::~WebAppManagerServiceLuna()
{
}

bool WebAppManagerServiceLuna::startService()
{
    return PalmServiceBase::startService();
}

Json::Value WebAppManagerServiceLuna::launchApp(const Json::Value& requestJson)
{
    logJsonTruncated(__func__, requestJson);
    int errCode;
    std::string errMsg;
    Json::Value reply;

    if (!requestJson.isObject()
       || (!requestJson.isMember("appDesc") || !requestJson["appDesc"].isObject() || !requestJson["appDesc"]["id"].isString())
       || (requestJson.isMember("parameters") && !requestJson["parameters"].isObject())
       || (requestJson.isMember("launchingAppId") && !requestJson["launchingAppId"].isString())
       || (requestJson.isMember("launchingProcId") && !requestJson["launchingProcId"].isString())
       || (!requestJson.isMember("instanceId") || !requestJson["instanceId"].isString())) {
        reply["returnValue"] = false;
        reply["errorCode"] = ERR_CODE_LAUNCHAPP_MISS_PARAM;
        reply["errorText"] = err_missParam;
        return reply;
    }

    Json::Value jsonParams = requestJson["parameters"];
    if (requestJson.isMember("launchHidden") && requestJson["launchHidden"] == true) {
        jsonParams["launchedHidden"] = true;
    }

    // if "preload" parameter is not "full" or "partial" or "minimal", there is no preload parameter.
    if (requestJson.isMember("preload") && requestJson["preload"].isString()) {
        jsonParams["preload"] = requestJson["preload"];
    }

    if (requestJson.isMember("keepAlive") && requestJson["keepAlive"] == true) {
        jsonParams["keepAlive"] = true;
    }

    std::string instanceId = requestJson["instanceId"].asString();
    if (!isValidInstanceId(instanceId)) {
        reply["returnValue"] = false;
        reply["errorCode"] = ERR_CODE_LAUNCHAPP_MISS_PARAM;
        reply["errorText"] = err_missParam;
        return reply;
    }
    jsonParams["instanceId"] = instanceId;

    std::string strParams = util::jsonToString(jsonParams);

    std::string appId = requestJson["appDesc"]["id"].asString();
    LOG_INFO_WITH_CLOCK(MSGID_APPLAUNCH_START, 4,
                        PMLOGKS("PerfType","AppLaunch"),
                        PMLOGKS("PerfGroup", appId.c_str()),
                        PMLOGKS("APP_ID", appId.c_str()),
                        PMLOGKS("INSTANCE_ID", instanceId.c_str()),
                        "params : %s", strParams.c_str());

    std::string strAppDesc = util::jsonToString(requestJson["appDesc"]);
    instanceId = WebAppManagerService::onLaunch(
                    strAppDesc,
                    strParams,
                    requestJson["launchingAppId"].asString(),
                    errCode, errMsg);

    if (instanceId.empty()) {
        reply["returnValue"] = false;
        reply["errorCode"] = errCode;
        reply["errorText"] = errMsg;
    } else {
        reply["returnValue"] = true;
        reply["appId"] = requestJson["appDesc"]["id"].asString();
        reply["instanceId"] = instanceId;
    }
    return reply;
}

bool WebAppManagerServiceLuna::isValidInstanceId(const std::string& instanceId)
{
    return instanceId.find_first_not_of("\f\n\r\v") != std::string::npos;
}

Json::Value WebAppManagerServiceLuna::killApp(const Json::Value& requestJson)
{
    logJsonTruncated(__func__, requestJson);
    Json::Value reply;

    if (!requestJson.isObject()
       || (requestJson.isMember("instanceId") && !requestJson["instanceId"].isString())
       || (requestJson.isMember("appId") && !requestJson["appId"].isString())
       || (requestJson.isMember("reason") && !requestJson["reason"].isString())) {
        reply["returnValue"] = false;
        reply["errorCode"] = ERR_CODE_KILL_APP_INVALID_PARAM;
        reply["errorText"] = err_invalidParam;
        return reply;
    }

    bool instances;
    std::string instanceId = requestJson["instanceId"].asString();
    std::string appId = requestJson["appId"].asString();
    std::string reason;

    if (requestJson.isMember("reason"))
        reason = requestJson["reason"].asString();

    LOG_INFO(MSGID_LUNA_API, 3, PMLOGKS("APP_ID", appId.c_str()), PMLOGKS("INSTANCE_ID", instanceId.c_str()), PMLOGKS("API", "killApp"), "reason : %s", reason.c_str());

    bool memoryReclaim = reason.compare("memoryReclaim") == 0;
    instances = WebAppManagerService::onKillApp(appId, instanceId, memoryReclaim);

    if (instances) {
        reply["appId"] = appId;
        reply["instanceId"] = instanceId;
        reply["returnValue"] = true;
    } else {
        reply["returnValue"] = false;
        reply["errorCode"] = ERR_CODE_NO_RUNNING_APP;
        reply["errorText"] = err_noRunningApp;
    }
    return reply;
}

Json::Value WebAppManagerServiceLuna::pauseApp(const Json::Value& requestJson)
{
    logJsonTruncated(__func__, requestJson);
    Json::Value reply;

    if (!requestJson.isObject()
       || (!requestJson.isMember("instanceId") || !requestJson["instanceId"].isString())) {
        reply["returnValue"] = false;
        reply["errorCode"] = ERR_CODE_PAUSE_APP_INVALID_PARAM;
        reply["errorText"] = err_invalidParam;
        return reply;
    }

    std::string id = requestJson["instanceId"].asString();

    LOG_INFO(MSGID_LUNA_API, 2, PMLOGKS("INSTANCE_ID", id.c_str()), PMLOGKS("API", "pauseApp"), "");

    if (WebAppManagerService::onPauseApp(id))
    {
        reply["returnValue"] = true;
        reply["appId"] = requestJson["appId"].asString();
        reply["instanceId"] = requestJson["instanceId"].asString();
    }
    else
    {
        reply["returnValue"] = false;
        reply["errorCode"] = ERR_CODE_NO_RUNNING_APP;
        reply["errorText"] = err_noRunningApp;
    }
    return reply;
}

Json::Value WebAppManagerServiceLuna::setInspectorEnable(const Json::Value& requestJson)
{
    logJsonTruncated(__func__, requestJson);
    LOG_DEBUG("WebAppManagerService::setInspectorEnable");
    Json::Value reply;
    std::string errorMessage("Not supported on this platform");

    LOG_DEBUG("errorMessage : %s", errorMessage.c_str());
    reply["errorMessage"] = errorMessage;
    reply["returnValue"] = false;
    return reply;
}


Json::Value WebAppManagerServiceLuna::closeAllApps(const Json::Value& requestJson)
{
    logJsonTruncated(__func__, requestJson);
    bool val = WebAppManagerService::onCloseAllApps();

    Json::Value reply;
    reply["returnValue"] = val;
    return reply;
}

Json::Value WebAppManagerServiceLuna::logControl(const Json::Value& requestJson)
{
    logJsonTruncated(__func__, requestJson);

    if (!requestJson.isObject()
       || (!requestJson.isMember("keys") || !requestJson["keys"].isString())
       || (!requestJson.isMember("value") || !requestJson["value"].isString())) {
        Json::Value reply;
        reply["returnValue"] = false;
        reply["errorCode"] = ERR_CODE_LOG_CONTROL_INVALID_PARAM;
        reply["errorText"] = err_invalidParam;
        return reply;
    }

    return WebAppManagerService::onLogControl(requestJson["keys"].asString(),
                                              requestJson["value"].asString());
}

Json::Value WebAppManagerServiceLuna::discardCodeCache(const Json::Value& requestJson)
{
    logJsonTruncated(__func__, requestJson);
    Json::Value reply;
    
    if (!requestJson.isObject()) {
        Json::Value reply;
        reply["returnValue"] = false;
        reply["errorCode"] = ERR_CODE_DISCARD_CODE_CACHE_INVALID_PARAM;
        reply["errorText"] = err_invalidParam;
        return reply;
    }

    bool forcedClearCache = false;
    uint32_t pid = 0;
    std::list<const WebAppBase*> running;

    if (!WebAppManagerService::isDiscardCodeCacheRequired()) {
        reply["returnValue"] = true;
        return reply;
    }

    if (requestJson.isMember("force"))
        forcedClearCache = requestJson["force"] == true;

    if (requestJson.isMember("pid") && requestJson["pid"].isUInt())
        pid = requestJson["pid"].asUInt();

    if (!pid)
        running = WebAppManagerService::runningApps();
    else
        running = WebAppManagerService::runningApps(pid);

    if (running.size() != 0 && !forcedClearCache) {
        reply["returnValue"] = false;
        return reply;
    }

    if (!WebAppManagerService::onCloseAllApps(pid)) {
        reply["returnValue"] = false;
        return reply;
    }

    m_clearedCache = true;
    WebAppManagerService::onDiscardCodeCache(pid);
    if (forcedClearCache)
        WebAppManagerService::onPurgeSurfacePool(pid);
    reply["returnValue"] = true;
    return reply;
}

Json::Value WebAppManagerServiceLuna::getWebProcessSize(const Json::Value& requestJson)
{
    logJsonTruncated(__func__, requestJson);
    return WebAppManagerService::getWebProcessProfiling();
}

Json::Value WebAppManagerServiceLuna::listRunningApps(const Json::Value& requestJson, bool subscribed)
{
    logJsonTruncated(__func__, requestJson);
    bool includeSysApps = requestJson["includeSysApps"] == true;

    std::vector<ApplicationInfo> apps = WebAppManagerService::list(includeSysApps);

    Json::Value reply;
    Json::Value runningApps;
    for (auto it = apps.begin(); it != apps.end(); ++it) {
        Json::Value app;
        app["id"] = it->appId;
        app["instanceId"] = it->instanceId;
        app["webprocessid"] = std::to_string(it->pid);
        runningApps.append(app);
    }
    reply["running"] = runningApps;
    reply["returnValue"] = true;
    return reply;
}

Json::Value WebAppManagerServiceLuna::clearBrowsingData(const Json::Value& requestJson)
{
    logJsonTruncated(__func__, requestJson);
    Json::Value reply;
    
    if (!requestJson.isObject()) {
        Json::Value reply;
        reply["returnValue"] = false;
        reply["errorCode"] = ERR_CODE_CLEAR_BROWSING_DATA_INVALID_PARAM;
        reply["errorText"] = err_invalidParam;
        return reply;
    }

    Json::Value value = requestJson["types"];
    bool returnValue = true;
    int removeBrowsingDataMask = 0;


    switch (value.type()) {
        case Json::ValueType::nullValue:
            removeBrowsingDataMask = WebAppManagerService::maskForBrowsingDataType("all");
            break;
        case Json::ValueType::arrayValue: {
            if (value.size() < 1) {
                reply["errorCode"] = ERR_CODE_CLEAR_DATA_BRAWSING_EMPTY_ARRAY;
                reply["errorText"] = err_emptyArray;
                returnValue = false;
                break;
            }

            for (Json::Value::ArrayIndex i = 0; i < value.size(); ++i) {
                if (!value[i].isString()) {
                    std::stringstream errorText;
                    errorText << err_invalidValue.c_str() << " (" << err_onlyAllowedForString << ")";
                    reply["errorCode"] = ERR_CODE_CLEAR_DATA_BRAWSING_INVALID_VALUE;
                    reply["errorText"] = errorText.str();
                    returnValue = false;
                    break;
                }

                int mask = WebAppManagerService::maskForBrowsingDataType(
                        value[i].asString().c_str());
                if (mask == 0) {
                    std::stringstream errorText;
                    errorText << err_unknownData.c_str() << " (" << value[i].asString().c_str() << ")";
                    reply["errorCode"] = ERR_CODE_CLEAR_DATA_BRAWSING_UNKNOWN_DATA;
                    reply["errorText"] = errorText.str();
                    returnValue = false;
                    break;
                }

                removeBrowsingDataMask |= mask;
            }
            break;
        }
        default:
            reply["errorCode"] = ERR_CODE_CLEAR_DATA_BRAWSING_INVALID_VALUE;
            reply["errorText"] = err_invalidValue;
            returnValue = false;
    }

    LOG_DEBUG("removeBrowsingDataMask: %d", removeBrowsingDataMask);

    if (returnValue)
        WebAppManagerService::onClearBrowsingData(removeBrowsingDataMask);

    reply["returnValue"] = returnValue;
    return reply;
}

void WebAppManagerServiceLuna::didConnect()
{
    Json::Value params;
    params["subscribe"] = true;

    params["serviceName"] = std::string("com.webos.settingsservice");
    if (!GET_LS2_SERVER_STATUS(systemServiceConnectCallback, params)) {
        LOG_WARNING(MSGID_SERVICE_CONNECT_FAIL, 0, "Failed to connect to settingsservice");
    }

    params["serviceName"] = std::string("com.webos.memorymanager");
    if (!GET_LS2_SERVER_STATUS(memoryManagerConnectCallback, params)) {
        LOG_WARNING(MSGID_MEMORY_CONNECT_FAIL, 0, "Failed to connect to memory manager");
    }

    params["serviceName"] = std::string("com.webos.applicationManager");
    if (!GET_LS2_SERVER_STATUS(applicationManagerConnectCallback, params)) {
        LOG_WARNING(MSGID_APPMANAGER_CONNECT_FAIL, 0, "Failed to connect to application manager");
    }

    params["serviceName"] = std::string("com.webos.bootManager");
    if (!GET_LS2_SERVER_STATUS(bootdConnectCallback, params)) {
        LOG_WARNING(MSGID_BOOTD_CONNECT_FAIL, 0, "Failed to connect to bootd");
    }

    params["serviceName"] = std::string("com.webos.service.connectionmanager");
    if (!GET_LS2_SERVER_STATUS(networkConnectionStatusCallback, params)) {
        LOG_WARNING(MSGID_NETWORK_CONNECT_FAIL, 0, "Failed to connect to connectionmanager");
    }
}

void WebAppManagerServiceLuna::systemServiceConnectCallback(const Json::Value& replyJson)
{
    if (!replyJson.isObject()) {
        LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", err_invalidParam.c_str());
        return;
    }

    if (replyJson["connected"] == true) {
        Json::Value localeParams;
        localeParams["subscribe"] = true;
        Json::Value localeList;
        localeList.append(std::string("localeInfo"));
        localeParams["keys"] = localeList;
        LS2_CALL(getSystemLocalePreferencesCallback,
            "luna://com.webos.settingsservice/getSystemSettings", localeParams);
    }
}

void WebAppManagerServiceLuna::getSystemLocalePreferencesCallback(const Json::Value& replyJson)
{
    if (!replyJson.isObject() || !replyJson["settings"].isObject()) {
        LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", err_invalidParam.c_str());
        return;
    }

    Json::Value localeInfo = replyJson["settings"]["localeInfo"];

    //LocaleInfo(language, etc) is empty when service is crashed
    //The right value will be notified again when service is restarted
    if (!localeInfo.isObject() || localeInfo.empty()
       || !localeInfo["locales"].isObject() || !localeInfo["locales"]["UI"].isString()) {
        std::string doc = util::jsonToString(replyJson);
        LOG_WARNING(MSGID_RECEIVED_INVALID_SETTINGS, 1,
            PMLOGKFV("MSG", "%s", doc.c_str()),
            "");
        return;

    }

    std::string language(localeInfo["locales"]["UI"].asString());

    LOG_INFO(MSGID_SETTING_SERVICE, 1,
        PMLOGKS("LANGUAGE", language.empty() ? "None" : language.c_str()),
        "");

    if (language.empty())
        return;

    if (language.compare(WebAppManagerService::getSystemLanguage()) == 0)
        return;

    WebAppManagerService::setSystemLanguage(language.c_str());
}

void WebAppManagerServiceLuna::memoryManagerConnectCallback(const Json::Value& replyJson)
{
    if (!replyJson.isObject()) {
        LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", err_invalidParam.c_str());
        return;
    }

    if (replyJson["connected"] == true) {
        Json::Value closeAppObj;
        closeAppObj["subscribe"] = true;
        closeAppObj["appType"] = "web";

        if (!call<WebAppManagerServiceLuna, &WebAppManagerServiceLuna::getCloseAppIdCallback>(
                "luna://com.webos.memorymanager/getCloseAppId", closeAppObj, this)) {
            LOG_WARNING(MSGID_MEM_MGR_API_CALL_FAIL, 0, "Failed to get close application identifier");
        }

        Json::Value thresholdChanged;
        thresholdChanged["subscribe"] = true;
        thresholdChanged["category"] = "/com/webos/memory";
        thresholdChanged["method"] = "thresholdChanged";
        if (!call<WebAppManagerServiceLuna, &WebAppManagerServiceLuna::thresholdChangedCallback>(
                "luna://com.palm.bus/signal/addmatch", thresholdChanged, this)) {
            LOG_WARNING(MSGID_SIGNAL_REGISTRATION_FAIL, 0, "Failed to register a client for thresholdChanged");
        }
    }
}

void WebAppManagerServiceLuna::getCloseAppIdCallback(const Json::Value& replyJson)
{
    if (!replyJson.isObject() || !replyJson["pid"].isUInt() || !replyJson["instanceId"].isString()) {
        LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", err_invalidParam.c_str());
        return;
    }

    std::string appId = replyJson["id"].asString();
    std::string instanceId = replyJson["instanceId"].asString();

   if(!appId.empty() && !instanceId.empty())
        WebAppManagerService::setForceCloseApp(appId.c_str(), instanceId.c_str());
}

void WebAppManagerServiceLuna::thresholdChangedCallback(const Json::Value& replyJson)
{
    if (!replyJson.isObject() || !replyJson["current"].isString()) {
        LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", err_invalidParam.c_str());
        return;
    }

    std::string currentLevel = replyJson["current"].asString();
    if (currentLevel.empty()) {
        LOG_DEBUG("thresholdChanged without level");
        return;
    }
    LOG_INFO(MSGID_NOTIFY_MEMORY_STATE, 1, PMLOGKS("State", currentLevel.c_str()), "");

    webos::WebViewBase::MemoryPressureLevel level;
    if (currentLevel.compare("medium") == 0)
        level = webos::WebViewBase::MEMORY_PRESSURE_LOW;
    else if (currentLevel.compare("critical") == 0 || currentLevel.compare("low") == 0)
        level = webos::WebViewBase::MEMORY_PRESSURE_CRITICAL;
    else
        level = webos::WebViewBase::MEMORY_PRESSURE_NONE;
    WebAppManagerService::notifyMemoryPressure(level);
}

void WebAppManagerServiceLuna::applicationManagerConnectCallback(const Json::Value& replyJson)
{
    if (!replyJson.isObject()) {
        LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", err_invalidParam.c_str());
        return;
    }

    if (replyJson["connected"] == true) {
        Json::Value params;
        params["subscribe"] = true;

        if (!call<WebAppManagerServiceLuna, &WebAppManagerServiceLuna::getAppStatusCallback>(
                "luna://com.webos.applicationManager/listApps", params, this)) {
            LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "Failed to get an application list");
        }

        params["extraInfo"] = true;
        if (!call<WebAppManagerServiceLuna, &WebAppManagerServiceLuna::getForegroundAppInfoCallback>(
                "luna://com.webos.applicationManager/getForegroundAppInfo", params, this)) {
            LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "Failed to get foreground application Information");
        }
    }
}

void WebAppManagerServiceLuna::getAppStatusCallback(const Json::Value& replyJson)
{
    if (!replyJson.isObject() || !replyJson["app"].isObject()
       || !replyJson["change"].isString()) {
        LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", err_invalidParam.c_str());
        return;
    }

    std::string changeKind = replyJson["change"].asString();
    Json::Value appObject = replyJson["app"];

    if (changeKind.compare("removed") == 0) {
        std::string appId = appObject["id"].isString() ? appObject["id"].asString() : "";
        LOG_INFO(MSGID_WAM_DEBUG, 0, "Application removed %s", appId.c_str());
        WebAppManagerService::onAppRemoved(appId);
    }
    if (changeKind.compare("added") == 0) {
        std::string appId = appObject["id"].isString() ? appObject["id"].asString() : "";
        LOG_INFO(MSGID_WAM_DEBUG, 0, "Application installed %s", appId.c_str());
        WebAppManagerService::onAppInstalled(appId);
    }
    if (changeKind.compare("removed") == 0 ||
        changeKind.compare("updated") == 0) {
        std::string appBasePath = appObject["folderPath"].isString() ? appObject["folderPath"].asString() : "";
        bool isCustomPlugin = appObject["customPlugin"] == true;

        if (isCustomPlugin) {
            WebAppManagerService::killCustomPluginProcess(appBasePath.c_str());
        }
    }
}


void WebAppManagerServiceLuna::getForegroundAppInfoCallback(const Json::Value& replyJson)
{
    if (!replyJson.isObject()) {
        LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", err_invalidParam.c_str());
        return;
    }

    if (m_clearedCache)
        m_clearedCache = false;

    if (replyJson["returnValue"] == true) {
        if (replyJson.isMember("appId") && replyJson["appId"].isString()) {
            std::string appId = replyJson["appId"].asString();
            webos::Runtime::GetInstance()->SetIsForegroundAppEnyo(
                WebAppManagerService::isEnyoApp(appId.c_str()));
        }
    }
}

void WebAppManagerServiceLuna::bootdConnectCallback(const Json::Value& replyJson)
{
    if (!replyJson.isObject()) {
        LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", err_invalidParam.c_str());
        return;
    }

    if (replyJson["connected"] == true) {
        Json::Value subscribe;
        subscribe["subscribe"] = true;
        if (!LS2_CALL(getBootStatusCallback, "luna://com.webos.bootManager/getBootStatus", subscribe)) {
            LOG_WARNING(MSGID_BOOTD_SUBSCRIBE_FAIL, 0, "Failed to subscribe to bootManager");
        }
    }
}

void WebAppManagerServiceLuna::getBootStatusCallback(const Json::Value& replyJson)
{
    if (!replyJson.isObject() || !replyJson["signals"].isObject()) {
        LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", err_invalidParam.c_str());
        return;
    }

    m_bootDone = replyJson["signals"]["boot-done"] == true;
}

void WebAppManagerServiceLuna::closeApp(const std::string& id)
{
    Json::Value json;
    json["instanceId"] = id;

    if (!LS2_CALL(closeAppCallback, "luna://com.webos.applicationManager/close", json))
        LOG_WARNING(MSGID_CLOSE_CALL_FAIL, 0, "Failed to send closeByAppId command to SAM");
}

void WebAppManagerServiceLuna::closeAppCallback(const Json::Value& reply)
{
    // TODO: check reply and close app again.
}

Json::Value WebAppManagerServiceLuna::webProcessCreated(const Json::Value& requestJson, bool subscribed)
{
    Json::Value reply;

    if (!requestJson.isObject()) {
        reply["returnValue"] = false;
        reply["errorCode"] = ERR_CODE_WEB_PROCESS_CREATED_INVALID_PARAM;
        reply["errorText"] = err_invalidParam;
        return reply;
    }

    std::string appId = requestJson["appId"].isString() ? requestJson["appId"].asString() : "";
    if (!appId.empty())
    {
        std::string instanceId = requestJson["instanceId"].isString() ? requestJson["instanceId"].asString() : "";
        int pid = WebAppManagerService::getWebProcessId(appId.c_str(), instanceId.c_str());
        reply["id"] = appId;
        reply["instanceId"] = instanceId;

        if (pid) {
            reply["webprocessid"] = pid;
            reply["returnValue"] = true;
        }
        else {
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

void WebAppManagerServiceLuna::networkConnectionStatusCallback(const Json::Value& reply)
{
    if (!reply.isObject()) {
        LOG_WARNING(MSGID_APP_MGR_API_CALL_FAIL, 0, "%s", err_invalidParam.c_str());
        return;
    }

    if (reply["connected"] == true) {
        LOG_DEBUG("connectionmanager is connected");
        Json::Value subscribe;
        subscribe["subscribe"] = true;
        if (!LS2_CALL(getNetworkConnectionStatusCallback, "luna://com.palm.connectionmanager/getStatus", subscribe)) {
            LOG_WARNING(MSGID_LS2_CALL_FAIL, 0, "Fail to subscribe to connection manager");
        }
    }
}

void WebAppManagerServiceLuna::getNetworkConnectionStatusCallback(const Json::Value& reply)
{
    // luna-send -f -n 1 luna://com.webos.service.connectionmanager/getstatus '{"subscribe": true}'
    WebAppManagerService::updateNetworkStatus(reply);
}
