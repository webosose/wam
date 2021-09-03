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

#include "json/json.h"

#include "application_description.h"
#include "log_manager.h"
#include "palm_system_blink.h"
#include "utils.h"
#include "web_app_base.h"
#include "web_app_wayland.h"
#include "web_page_blink.h"

namespace {

const char* toStr(const bool value) {
  return value ? "true" : "false";
}

}  // namespace

PalmSystemBlink::PalmSystemBlink(WebAppBase* app)
    : PalmSystemWebOS(app), initialized_(false) {}

std::string PalmSystemBlink::HandleBrowserControlMessage(
    const std::string& command,
    const std::vector<std::string>& arguments) {
  if (command == "initialize") {
    return util::JsonToString(Initialize());
  } else if (command == "country") {
    return Country();
  } else if (command == "locale") {
    return Locale();
  } else if (command == "localeRegion") {
    return LocaleRegion();
  } else if (command == "isMinimal") {
    return toStr(IsMinimal());
  } else if (command == "identifier") {
    return Identifier();
  } else if (command == "screenOrientation") {
    return ScreenOrientation();
  } else if (command == "currentCountryGroup") {
    return GetDeviceInfo("CountryGroup");
  } else if (command == "stageReady") {
    StageReady();
  } else if (command == "activate") {
    LOG_INFO(MSGID_PALMSYSTEM, 3, PMLOGKS("APP_ID", app_->AppId().c_str()),
             PMLOGKS("INSTANCE_ID", app_->InstanceId().c_str()),
             PMLOGKFV("PID", "%d", app_->Page()->GetWebProcessPID()),
             "webOSSystem.activate()");
    Activate();
  } else if (command == "deactivate") {
    LOG_INFO(MSGID_PALMSYSTEM, 3, PMLOGKS("APP_ID", app_->AppId().c_str()),
             PMLOGKS("INSTANCE_ID", app_->InstanceId().c_str()),
             PMLOGKFV("PID", "%d", app_->Page()->GetWebProcessPID()),
             "webOSSystem.deactivate()");
    Deactivate();
  } else if (command == "isActivated") {
    return toStr(IsActivated());
  } else if (command == "isKeyboardVisible") {
    return toStr(IsKeyboardVisible());
  } else if (command == "getIdentifier" || command == "identifier") {
    return Identifier();
  } else if (command == "launchParams") {
    LOG_INFO(MSGID_PALMSYSTEM, 3, PMLOGKS("APP_ID", app_->AppId().c_str()),
             PMLOGKS("INSTANCE_ID", app_->InstanceId().c_str()),
             PMLOGKFV("PID", "%d", app_->Page()->GetWebProcessPID()),
             "webOSSystem.launchParams Updated by app; %s",
             arguments[0].c_str());
    UpdateLaunchParams(arguments[0]);
  } else if (command == "screenOrientation") {
    return ScreenOrientation();
  } else if (command == "keepAlive") {
    if (arguments.size() > 0)
      SetKeepAlive(arguments[0] == "true");
  } else if (command == "PmLogInfoWithClock") {
    if (arguments.size() == 3)
      LogMsgWithClock(arguments[0], arguments[1], arguments[2]);
  } else if (command == "PmLogString") {
    if (arguments.size() > 3) {
      int32_t v1;
      if (util::StrToInt(arguments[0], v1))
        LogMsgString(v1, arguments[1], arguments[2], arguments[3]);
    }
  } else if (command == "setWindowProperty") {
    if (arguments.size() > 1) {
      LOG_INFO(MSGID_PALMSYSTEM, 3, PMLOGKS("APP_ID", app_->AppId().c_str()),
               PMLOGKS("INSTANCE_ID", app_->InstanceId().c_str()),
               PMLOGKFV("PID", "%d", app_->Page()->GetWebProcessPID()),
               "webOSSystem.window.setProperty('%s', '%s')",
               arguments[0].c_str(), arguments[1].c_str());
      app_->SetWindowProperty(arguments[0], arguments[1]);
    }
  } else if (command == "platformBack") {
    LOG_INFO(MSGID_PALMSYSTEM, 3, PMLOGKS("APP_ID", app_->AppId().c_str()),
             PMLOGKS("INSTANCE_ID", app_->InstanceId().c_str()),
             PMLOGKFV("PID", "%d", app_->Page()->GetWebProcessPID()),
             "webOSSystem.platformBack()");
    app_->PlatformBack();
  } else if (command == "setCursor") {
    if (arguments.size() == 3) {
      std::string v1 = arguments[0];
      int32_t v2, v3;
      const bool v2_conversion = util::StrToInt(arguments[1], v2);
      const bool v3_conversion = util::StrToInt(arguments[2], v3);
      if (v2_conversion && v3_conversion)
        app_->SetCursor(v1, v2, v3);
    }
  } else if (command == "setInputRegion") {
    std::string data;
    for (const auto& argument : arguments) {
      data.append(argument);
    }
    SetInputRegion(data);
  } else if (command == "setKeyMask") {
    std::string data;
    for (const auto& argument : arguments) {
      data.append(argument);
    }
    SetGroupClientEnvironment(kKeyMask, data);
  } else if (command == "focusOwner") {
    SetGroupClientEnvironment(kFocusOwner, nullptr);
  } else if (command == "focusLayer") {
    SetGroupClientEnvironment(kFocusLayer, nullptr);
  } else if (command == "hide") {
    Hide();
  } else if (command == "setLoadErrorPolicy") {
    if (arguments.size() > 0) {
      LOG_INFO(MSGID_PALMSYSTEM, 3, PMLOGKS("APP_ID", app_->AppId().c_str()),
               PMLOGKS("INSTANCE_ID", app_->InstanceId().c_str()),
               PMLOGKFV("PID", "%d", app_->Page()->GetWebProcessPID()),
               "webOSSystem.setLoadErrorPolicy(%s)", arguments[0].c_str());
      SetLoadErrorPolicy(arguments[0]);
    }
  } else if (command == "onCloseNotify") {
    if (arguments.size() > 0) {
      LOG_INFO(MSGID_PALMSYSTEM, 3, PMLOGKS("APP_ID", app_->AppId().c_str()),
               PMLOGKS("INSTANCE_ID", app_->InstanceId().c_str()),
               PMLOGKFV("PID", "%d", app_->Page()->GetWebProcessPID()),
               "webOSSystem.onCloseNotify(%s)", arguments[0].c_str());
      OnCloseNotify(arguments[0]);
    }
  } else if (command == "cursorVisibility") {
    return toStr(CursorVisibility());
  } else if (command == "serviceCall") {
    if (app_->Page()->IsClosing()) {
      LOG_INFO(MSGID_PALMSYSTEM, 3, PMLOGKS("APP_ID", app_->AppId().c_str()),
               PMLOGKS("INSTANCE_ID", app_->InstanceId().c_str()),
               PMLOGKFV("PID", "%d", app_->Page()->GetWebProcessPID()),
               "webOSSystem.serviceCall(%s, %s)", arguments[0].c_str(),
               arguments[1].c_str());
      app_->ServiceCall(arguments[0], arguments[1], app_->AppId());
    } else {
      LOG_WARNING(
          MSGID_SERVICE_CALL_FAIL, 3, PMLOGKS("APP_ID", app_->AppId().c_str()),
          PMLOGKS("INSTANCE_ID", app_->InstanceId().c_str()),
          PMLOGKS("URL", arguments[0].c_str()), "Page is NOT in closing");
    }
  }

  return std::string();
}

void PalmSystemBlink::SetCountry() {
  static_cast<WebPageBlink*>(app_->Page())
      ->UpdateExtensionData("country", Country());
}

void PalmSystemBlink::SetLaunchParams(const std::string& params) {
  PalmSystemWebOS::SetLaunchParams(params);
  static_cast<WebPageBlink*>(app_->Page())
      ->UpdateExtensionData("launchParams", LaunchParams());
}

void PalmSystemBlink::SetLocale(const std::string& params) {
  static_cast<WebPageBlink*>(app_->Page())
      ->UpdateExtensionData("locale", params);
}

std::string PalmSystemBlink::Identifier() const {
  if (!app_->Page())
    return std::string();

  return static_cast<WebPageBlink*>(app_->Page())->GetIdentifier();
}

void PalmSystemBlink::SetLoadErrorPolicy(const std::string& params) {
  static_cast<WebPageBlink*>(app_->Page())->SetLoadErrorPolicy(params);
}

std::string PalmSystemBlink::TrustLevel() const {
  return static_cast<WebPageBlink*>(app_->Page())->TrustLevel();
}

void PalmSystemBlink::OnCloseNotify(const std::string& params) {
  if (params == "didSetOnCloseCallback")
    static_cast<WebPageBlink*>(app_->Page())->SetHasOnCloseCallback(true);
  else if (params == "didClearOnCloseCallback")
    static_cast<WebPageBlink*>(app_->Page())->SetHasOnCloseCallback(false);
  else if (params == "didRunOnCloseCallback")
    static_cast<WebPageBlink*>(app_->Page())->DidRunCloseCallback();
}

double PalmSystemBlink::DevicePixelRatio() {
  return static_cast<WebPageBlink*>(app_->Page())->DevicePixelRatio();
}

Json::Value PalmSystemBlink::Initialize() {
  initialized_ = true;

  Json::Value data;
  data["launchParams"] = LaunchParams();
  data["country"] = Country();
  data["tvSystemName"] = GetDeviceInfo("TvSystemName");
  data["currentCountryGroup"] = GetDeviceInfo("CountryGroup");
  data["locale"] = Locale();
  data["localeRegion"] = LocaleRegion();
  data["isMinimal"] = IsMinimal();
  data["identifier"] = Identifier();
  data["screenOrientation"] = ScreenOrientation();
  data["deviceInfo"] = GetDeviceInfo("TvDeviceInfo");
  data["activityId"] = (double)ActivityId();
  data["phoneRegion"] = PhoneRegion();
  data["folderPath"] = app_->GetAppDescription()->FolderPath();

  data["devicePixelRatio"] = DevicePixelRatio();
  data["trustLevel"] = TrustLevel();
  return data;
}
