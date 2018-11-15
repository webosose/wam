// Copyright (c) 2008-2018 LG Electronics, Inc.
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


#include <limits>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ApplicationDescription.h"
#include "JsonHelper.h"
#include "LogManager.h"

bool ApplicationDescription::checkTrustLevel(std::string trustLevel)
{
    if (trustLevel.empty())
        return false;
    if (trustLevel.compare("default") == 0)
        return true;
    if (trustLevel.compare("trusted") == 0)
        return true;
    return false;
}

ApplicationDescription::ApplicationDescription()
    : m_transparency(false)
    , m_windowClassValue(WindowClass_Normal)
    , m_handlesRelaunch(false)
    , m_inspectable(true)
    , m_customPlugin(false)
    , m_backHistoryAPIDisabled(false)
    , m_widthOverride(0)
    , m_heightOverride(0)
    , m_doNotTrack(false)
    , m_handleExitKey(false)
    , m_allowVideoCapture(false)
    , m_allowAudioCapture(false)
    , m_supportsAudioGuidance(false)
    , m_useNativeScroll(false)
    , m_usePrerendering(false)
    , m_networkStableTimeout(std::numeric_limits<double>::quiet_NaN())
    , m_disallowScrollingInMainFrame(true)
    , m_delayMsForLanchOptimization(0)
{
}

const ApplicationDescription::WindowGroupInfo ApplicationDescription::getWindowGroupInfo()
{
    ApplicationDescription::WindowGroupInfo info;

    if (!m_groupWindowDesc.empty()) {
        Json::Value json;
        readJsonFromString(m_groupWindowDesc, json);

        if (json.isObject()) {
            auto name = json["name"];
            if (name.isString())
                info.name = QString::fromStdString(name.asString());

            auto isOwner = json["owner"];
            if (isOwner.isBool())
                info.isOwner = isOwner.asBool();
        }
    }

    return info;
}

const ApplicationDescription::WindowOwnerInfo ApplicationDescription::getWindowOwnerInfo()
{
    ApplicationDescription::WindowOwnerInfo info;
    if (!m_groupWindowDesc.empty()) {
        Json::Value json;
        readJsonFromString(m_groupWindowDesc, json);

        auto ownerInfo = json["ownerInfo"];
        if (ownerInfo.isObject()) {
            if (ownerInfo["allowAnonymous"].isBool())
                info.allowAnonymous = ownerInfo["allowAnonymous"].asBool();

            auto layers = ownerInfo["layers"];
            if (layers.isArray()) {
                for (const auto &layer : layers) {
                    auto name = layer["name"];
                    auto zstr = layer["z"];
                    if (name.isString() && zstr.isString()) {
                        int z = std::stoi(zstr.asString());
                        info.layers.insert(QString::fromStdString(name.asString()), z);
                    }
                }
            }
        }

    }
    return info;
}

const ApplicationDescription::WindowClientInfo ApplicationDescription::getWindowClientInfo()
{
    ApplicationDescription::WindowClientInfo info;
    if (!m_groupWindowDesc.empty()) {
        Json::Value json;
        readJsonFromString(m_groupWindowDesc, json);

        auto clientInfo = json["clientInfo"];
        if (clientInfo.isObject()) {
            auto layer = clientInfo["layer"];
            if (layer.isString())
                info.layer = QString::fromStdString(layer.asString());

            auto hint = clientInfo["hint"];
            if (hint.isString())
                info.hint = QString::fromStdString(hint.asString());
        }
    }
    return info;
}

std::unique_ptr<ApplicationDescription> ApplicationDescription::fromJsonString(const char* jsonStr)
{
    Json::Value jsonObj;
    if (!readJsonFromString(jsonStr, jsonObj)) {
        LOG_WARNING(MSGID_APP_DESC_PARSE_FAIL, 1,
                    PMLOGKFV("JSON", "%s", jsonStr), "Failed to parse JSON string");
        return nullptr;
    }

    auto appDesc = std::unique_ptr<ApplicationDescription>(new ApplicationDescription());

    appDesc->m_surfaceId = jsonObj["surfaceId"].asInt();
    appDesc->m_transparency = jsonObj["transparent"].asBool();
    appDesc->m_trustLevel = jsonObj["trustLevel"].asString();
    appDesc->m_subType = jsonObj["subType"].asString();
    appDesc->m_deeplinkingParams = jsonObj["deeplinkingParams"].asString();
    appDesc->m_handlesRelaunch = jsonObj["handlesRelaunch"].asBool();
    appDesc->m_defaultWindowType = jsonObj["defaultWindowType"].asString();
    appDesc->m_inspectable = jsonObj["inspectable"].asBool();
    appDesc->m_containerJS = jsonObj["containerJS"].asString();
    appDesc->m_containerCSS = jsonObj["containerCSS"].asString();
    appDesc->m_enyoBundleVersion = jsonObj["enyoBundleVersion"].asString();
    appDesc->m_enyoVersion = jsonObj["enyoVersion"].asString();
    appDesc->m_version = jsonObj["version"].asString();
    appDesc->m_customPlugin = jsonObj["customPlugin"].asBool();
    appDesc->m_backHistoryAPIDisabled = jsonObj["disableBackHistoryAPI"].asBool();

    auto vendorExtension = jsonObj.get("vendorExtension", Json::Value(Json::objectValue));
    auto groupWindowDesc = jsonObj.get("windowGroup", Json::Value(Json::objectValue));
    dumpJsonToString(vendorExtension, appDesc->m_vendorExtension);
    dumpJsonToString(groupWindowDesc, appDesc->m_groupWindowDesc);

    auto supportedVersions = jsonObj["supportedEnyoBundleVersions"];
    if (supportedVersions.isArray()) {
        for (const Json::Value &version : supportedVersions)
            appDesc->m_supportedEnyoBundleVersions.append(QString::fromStdString(version.asString()));
    }

    appDesc->m_id = jsonObj["id"].asString();
    appDesc->m_entryPoint= jsonObj["main"].asString();
    appDesc->m_icon = jsonObj["icon"].asString();
    appDesc->m_folderPath = jsonObj["folderPath"].asString();
    appDesc->m_requestedWindowOrientation = jsonObj["requestedWindowOrientation"].asString();
    appDesc->m_title = jsonObj["title"].asString();
    appDesc->m_doNotTrack = jsonObj["doNotTrack"].asBool();
    appDesc->m_handleExitKey = jsonObj["handleExitKey"].asBool();
    appDesc->m_enableBackgroundRun = jsonObj["enableBackgroundRun"].asBool();
    appDesc->m_allowVideoCapture = jsonObj["allowVideoCapture"].asBool();
    appDesc->m_allowAudioCapture = jsonObj["allowAudioCapture"].asBool();

    auto usePrerendering = jsonObj["usePrerendering"];
    appDesc->m_usePrerendering = usePrerendering.isBool() && usePrerendering.asBool();
    auto disallowScrolling = jsonObj["disallowScrollingInMainFrame"];
    appDesc->m_disallowScrollingInMainFrame = disallowScrolling.isBool() && disallowScrolling.asBool();

    // Handle accessibility, supportsAudioGuidance
    auto accessibility = jsonObj["accessibility"];
    if (accessibility.isObject()) {
        auto audioGuidance = accessibility["supportsAudioGuidance"];
        appDesc->m_supportsAudioGuidance = audioGuidance.isBool() && audioGuidance.asBool();
    }

    // Handle v8 snapshot file
    auto v8SnapshotFile = jsonObj["v8SnapshotFile"];
    if (v8SnapshotFile.isString()) {
        std::string snapshotFile = v8SnapshotFile.asString();
        if (snapshotFile.length() > 0) {
            if (snapshotFile.at(0) == '/')
                appDesc->m_v8SnapshotPath = snapshotFile;
            else
                appDesc->m_v8SnapshotPath = appDesc->m_folderPath + "/" + snapshotFile;
        }
    }

    // Handle v8 extra flags
    auto v8ExtraFlags = jsonObj["v8ExtraFlags"];
    if (v8ExtraFlags.isString())
        appDesc->m_v8ExtraFlags = v8ExtraFlags.asString();

    // Handle resolution
    auto resolution = jsonObj["resolution"];
    if (resolution.isString()) {
        QString overrideResolution = QString::fromStdString(jsonObj["resolution"].asString());
        QStringList resList(overrideResolution.split("x", QString::KeepEmptyParts, Qt::CaseInsensitive));
        if(resList.size() == 2) {
            appDesc->m_widthOverride = resList.at(0).toInt();
            appDesc->m_heightOverride = resList.at(1).toInt();
        }
        if(appDesc->m_widthOverride < 0 || appDesc->m_heightOverride < 0) {
            appDesc->m_widthOverride = 0;
            appDesc->m_heightOverride = 0;
        }
    }

    // Handle keyFilterTable
    //Key code is changed only for facebooklogin WebApp
    auto keyFilterTable = jsonObj["keyFilterTable"];
    if (keyFilterTable.isArray()) {
        for (const auto &k : keyFilterTable) {
            if (!k.isObject())
                continue;
            int from = std::stoi(k["from"].asString());
            int to = std::stoi(k["to"].asString());
            int mod = std::stoi(k["mod"].asString());
            appDesc->m_keyFilterTable[from] = qMakePair(to, mod);
        }
    }

    // Handle trustLevel
    if (!appDesc->checkTrustLevel(appDesc->m_trustLevel))
        appDesc->m_trustLevel = std::string("default");

    // Handle webAppType
    if (appDesc->m_subType.empty()) {
        appDesc->m_subType = std::string("default");
    }

    // Handle hidden property of window class
    // Convert a json object for window class to an enum value
    // and store it instead of the json object.
    // The format of window class in the appinfo.json is as below.
    //
    // class : { "hidden" : boolean }
    //
    WindowClass classValue = WindowClass_Normal;
    auto clazz = jsonObj["class"];
    if (clazz.isObject()) {
        if (clazz["hidden"].isBool() && clazz["hidden"].asBool())
            classValue = WindowClass_Hidden;
    }
    appDesc->m_windowClassValue = classValue;

    // Handle folderPath
    if(!appDesc->m_folderPath.empty()) {
        std::string tempPath = appDesc->m_folderPath + "/" + appDesc->m_entryPoint;
        struct stat statEntPt;
        if(!stat(tempPath.c_str(), &statEntPt)) {
            appDesc->m_entryPoint = "file://" + tempPath;
        }
        tempPath.clear();
        tempPath = appDesc->m_folderPath + "/" + appDesc->m_icon;
        if(!stat(tempPath.c_str(), &statEntPt)) {
            appDesc->m_icon = tempPath;
        }
    }
    appDesc->m_useNativeScroll = jsonObj["useNativeScroll"].isBool() && jsonObj["useNativeScroll"].asBool();

    // Set network stable timeout
    auto networkStableTimeout = jsonObj["networkStableTimeout"];
    if (!networkStableTimeout.isDouble())
        LOG_ERROR(MSGID_TYPE_ERROR, 2, PMLOGKS("APP_ID", appDesc->id().c_str()),
            PMLOGKFV("DATA_TYPE", "%d", networkStableTimeout.type()),  "Invaild JsonValue type");
    else
        appDesc->m_networkStableTimeout = networkStableTimeout.asDouble();

    // Set delay millisecond for launch optimization
    auto delayMsForLaunchOptimization = jsonObj["delayMsForLaunchOptimization"];
    if (delayMsForLaunchOptimization.isInt()) {
        int delayMs = delayMsForLaunchOptimization.asInt();
        appDesc->m_delayMsForLanchOptimization = (delayMs >= 0) ? delayMs : 0;
    }

    return appDesc;
}
