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

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QJsonValue>
#include <QVariant>

#include "ApplicationDescription.h"
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
{
}

const ApplicationDescription::WindowGroupInfo ApplicationDescription::getWindowGroupInfo()
{
    ApplicationDescription::WindowGroupInfo info;

    if (!m_groupWindowDesc.empty()) {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray(m_groupWindowDesc.c_str()));
        QJsonObject jsonObject = jsonDoc.object();

        if (!jsonObject.value("name").isUndefined())
            info.name = jsonObject.value("name").toString();
        if (!jsonObject.value("owner").isUndefined())
            info.isOwner = jsonObject.value("owner").toBool();
    }

    return info;
}

const ApplicationDescription::WindowOwnerInfo ApplicationDescription::getWindowOwnerInfo()
{
    ApplicationDescription::WindowOwnerInfo info;
    if (!m_groupWindowDesc.empty()) {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray(m_groupWindowDesc.c_str()));
        QJsonObject jsonObject = jsonDoc.object();

        if (!jsonObject.value("ownerInfo").isUndefined()) {
            QJsonObject ownerJsonObject = jsonObject.value("ownerInfo").toObject();
            if (!ownerJsonObject.value("allowAnonymous").isUndefined())
                info.allowAnonymous =ownerJsonObject.value("allowAnonymous").toBool();

            if (!ownerJsonObject.value("layers").isUndefined()) {
                QJsonArray ownerJsonArray = ownerJsonObject.value("layers").toArray();

                for (int i=0; i<ownerJsonArray.size(); i++) {
                    QVariantMap map = ownerJsonArray[i].toObject().toVariantMap();
                    if (!map.empty())
                        info.layers.insert(map["name"].toString(), map["z"].toString().toInt());
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
        QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray(m_groupWindowDesc.c_str()));
        QJsonObject jsonObject = jsonDoc.object();

        if (!jsonObject.value("clientInfo").isUndefined()) {
            QJsonObject clientJsonObject = jsonObject.value("clientInfo").toObject();
            if (!clientJsonObject.value("layer").isUndefined())
                info.layer = clientJsonObject.value("layer").toString();

            if (!clientJsonObject.value("hint").isUndefined())
                info.hint = clientJsonObject.value("hint").toString();
        }
    }
    return info;
}

ApplicationDescription* ApplicationDescription::fromJsonString(const char* jsonStr)
{
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(QByteArray(jsonStr), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        LOG_WARNING(MSGID_APP_DESC_PARSE_FAIL, 1,
                    PMLOGKFV("JSON", "%s", jsonStr), "Failed to parse JSON string");
        return 0;
    }
    QJsonObject jsonObj = jsonDoc.object();

    ApplicationDescription* appDesc = new ApplicationDescription();

    appDesc->m_transparency = jsonObj["transparent"].toBool();
    appDesc->m_vendorExtension = QJsonDocument(jsonObj["vendorExtension"].toObject()).toJson().data();
    appDesc->m_trustLevel = jsonObj["trustLevel"].toString().toStdString();
    appDesc->m_subType = jsonObj["subType"].toString().toStdString();
    appDesc->m_deeplinkingParams = jsonObj["deeplinkingParams"].toString().toStdString();
    appDesc->m_handlesRelaunch = jsonObj["handlesRelaunch"].toBool();
    appDesc->m_defaultWindowType = jsonObj["defaultWindowType"].toString().toStdString();
    appDesc->m_inspectable = jsonObj["inspectable"].toBool();
    appDesc->m_containerJS = jsonObj["containerJS"].toString().toStdString();
    appDesc->m_containerCSS = jsonObj["containerCSS"].toString().toStdString();
    appDesc->m_enyoBundleVersion = jsonObj["enyoBundleVersion"].toString().toStdString();
    appDesc->m_enyoVersion = jsonObj["enyoVersion"].toString().toStdString();
    appDesc->m_version = jsonObj["version"].toString().toStdString();
    appDesc->m_customPlugin = jsonObj["customPlugin"].toBool();
    appDesc->m_backHistoryAPIDisabled = jsonObj["disableBackHistoryAPI"].toBool();
    appDesc->m_groupWindowDesc = QJsonDocument(jsonObj["windowGroup"].toObject()).toJson().data();

    if (jsonObj.contains("supportedEnyoBundleVersions")) {
        QJsonArray versions = jsonObj["supportedEnyoBundleVersions"].toArray();
        for (int i=0; i < versions.size(); i++)
            appDesc->m_supportedEnyoBundleVersions.append(versions[i].toString());
    }

    appDesc->m_id = jsonObj["id"].toString().toStdString();
    appDesc->m_entryPoint= jsonObj["main"].toString().toStdString();
    appDesc->m_icon = jsonObj["icon"].toString().toStdString();
    appDesc->m_folderPath = jsonObj["folderPath"].toString().toStdString();
    appDesc->m_requestedWindowOrientation = jsonObj["requestedWindowOrientation"].toString().toStdString();
    appDesc->m_title = jsonObj["title"].toString().toStdString();
    appDesc->m_doNotTrack = jsonObj["doNotTrack"].toBool();
    appDesc->m_handleExitKey = jsonObj["handleExitKey"].toBool();
    appDesc->m_enableBackgroundRun = jsonObj["enableBackgroundRun"].toBool();
    appDesc->m_allowVideoCapture = jsonObj["allowVideoCapture"].toBool();
    appDesc->m_allowAudioCapture = jsonObj["allowAudioCapture"].toBool();
    appDesc->m_usePrerendering = jsonObj.contains("usePrerendering") && jsonObj["usePrerendering"].toBool();
    appDesc->m_disallowScrollingInMainFrame = !jsonObj.contains("disallowScrollingInMainFrame") || jsonObj["disallowScrollingInMainFrame"].toBool();

    // Handle accessibility, supportsAudioGuidance
    if (!jsonObj.value("accessibility").isUndefined() && jsonObj.value("accessibility").isObject()) {
        QJsonObject accessibilityObj = jsonObj["accessibility"].toObject();
        if (!accessibilityObj.value("supportsAudioGuidance").isUndefined())
            appDesc->m_supportsAudioGuidance = accessibilityObj["supportsAudioGuidance"].toBool();
    }

    // Handle v8 snapshot file
    if (!jsonObj.value("v8SnapshotFile").isUndefined()) {
        std::string snapshotFile = jsonObj["v8SnapshotFile"].toString().toStdString();
        if (snapshotFile.length() > 0) {
            if (snapshotFile.at(0) == '/')
                appDesc->m_v8SnapshotPath = snapshotFile;
            else
                appDesc->m_v8SnapshotPath = appDesc->m_folderPath + "/" + snapshotFile;
        }
    }

    // Handle v8 extra flags
    if (!jsonObj.value("v8ExtraFlags").isUndefined())
        appDesc->m_v8ExtraFlags = jsonObj["v8ExtraFlags"].toString().toStdString();

    // Handle resolution
    if (!jsonObj.value("resolution").isUndefined()) {
        QString overrideResolution = jsonObj["resolution"].toString();
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
    if (!jsonObj.value("keyFilterTable").isUndefined()) {
        QJsonArray keyFilterTable = jsonObj["keyFilterTable"].toArray();
        for (int i=0 ; i < keyFilterTable.size() ; i++) {
            QVariantMap map = keyFilterTable[i].toObject().toVariantMap();
            if (!map.empty())
                appDesc->m_keyFilterTable[map["from"].toString().toInt()] = qMakePair(map["to"].toString().toInt(), map["modifier"].toString().toInt());
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
    if (!jsonObj.value("class").isUndefined() && jsonObj.value("class").isObject()) {
        QJsonObject classObj = jsonObj["class"].toObject();
        if (classObj["hidden"].toBool())
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
    appDesc->m_useNativeScroll = jsonObj.contains("useNativeScroll") && jsonObj["useNativeScroll"].toBool();

    // Set network stable timeout
    if(jsonObj.contains("networkStableTimeout")) {
        if (jsonObj["networkStableTimeout"].type() != QJsonValue::Double)
            LOG_ERROR(MSGID_TYPE_ERROR, 2, PMLOGKS("APP_ID", appDesc->id().c_str()),
                PMLOGKFV("DATA_TYPE", "%d", jsonObj["networkStableTimeout"].type()),  "Invaild QJsonValue type");
        else
            appDesc->m_networkStableTimeout = jsonObj["networkStableTimeout"].toDouble();
    }

    return appDesc;
}
