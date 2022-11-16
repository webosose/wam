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

#include "notification_platform_bridge.h"

#include <codecvt>
#include <locale>

#include <json/json.h>

#include "util/utils.h"
#include "web_app_luna.h"

NotificationPlatformBridge::NotificationPlatformBridge() = default;

NotificationPlatformBridge::~NotificationPlatformBridge() = default;

void NotificationPlatformBridge::Display(
    const neva_app_runtime::Notification& notification) {
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;

  WebAppLuna toast_luna(notification.AppId());

  Json::Value toast_params;
  toast_params["message"] = convert.to_bytes(notification.Message());
  toast_params["opacity"] = 0.0;
  toast_params["persistent"] = true;
  toast_params["sourceId"] = notification.AppId();

  toast_luna.Call("luna://com.webos.notification/createToast",
                  util::JsonToString(toast_params).c_str());

  WebAppLuna alert_luna("");

  const std::vector<neva_app_runtime::ButtonInfo>& notificationButtons =
      notification.Buttons();

  Json::Value alert_buttons;
  Json::Value button;

  button["label"] = "OK";

  alert_buttons.append(button);

  if (notificationButtons.size() == 0) {
    Json::Value click_button;
    Json::Value params;

    params["type"] = "notificationclick";
    params["appId"] = notification.AppId();
    params["notificationId"] = notification.Id();
    params["origin"] = notification.Origin();

    click_button["label"] = "Shortcut";
    click_button["onclick"] =
        "luna://com.webos.service.webappmanager/fireNotificationEvent";
    click_button["params"] = params;

    alert_buttons.append(click_button);
  } else {
    Json::Value click_button;
    Json::Value params;

    params["type"] = "notificationclick";
    params["appId"] = notification.AppId();
    params["notificationId"] = notification.Id();
    params["origin"] = notification.Origin();
    params["actionIndex"] = 0;

    click_button["label"] = notificationButtons[0].title;
    click_button["onclick"] =
        "luna://com.webos.service.webappmanager/fireNotificationEvent";
    click_button["params"] = params;

    alert_buttons.append(click_button);
  }

  Json::Value alert_params;
  alert_params["buttons"] = alert_buttons;
  alert_params["message"] = convert.to_bytes(notification.Message());
  alert_params["title"] = convert.to_bytes(notification.Title());

  alert_luna.Call("luna://com.webos.notification/createAlert",
                  util::JsonToString(alert_params).c_str());
}

void NotificationPlatformBridge::Close(const std::string& notificationId) {}

void NotificationPlatformBridge::GetDisplayed(
    neva_app_runtime::GetDisplayedNotificationsCallback callback) const {}

void NotificationPlatformBridge::SetReadyCallback(
    neva_app_runtime::NotificationPlatformBridge::
        NotificationBridgeReadyCallback callback) {}
