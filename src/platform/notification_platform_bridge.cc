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

#include "notification_service_luna.h"

NotificationPlatformBridge::NotificationPlatformBridge() = default;

NotificationPlatformBridge::~NotificationPlatformBridge() = default;

void NotificationPlatformBridge::Display(
    const neva_app_runtime::Notification& notification) {
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;

  NotificationData data;
  data.app_id = notification.AppId();
  for (const neva_app_runtime::ButtonInfo& button : notification.Buttons()) {
    data.buttons.emplace_back(button.title, button.icon_path);
  }
  data.message = convert.to_bytes(notification.Message());
  data.id = notification.Id();
  data.origin = notification.Origin();
  data.title = convert.to_bytes(notification.Title());

  NotificationService::Instance()->Display(data);
}

void NotificationPlatformBridge::Close(const std::string& notificationId) {}

void NotificationPlatformBridge::GetDisplayed(
    neva_app_runtime::GetDisplayedNotificationsCallback callback) const {}

void NotificationPlatformBridge::SetReadyCallback(
    neva_app_runtime::NotificationPlatformBridge::
        NotificationBridgeReadyCallback callback) {}
