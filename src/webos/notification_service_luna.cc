// Copyright (c) 2023 LG Electronics, Inc.
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

#include "notification_service_luna.h"

namespace {

Json::Value Callback(const std::string& notification_id,
                     std::map<std::string, LSCalloutContext>& contexts,
                     std::map<std::string, std::string>& ids,
                     const std::string& name,
                     const Json::Value& reply) {
  if (reply.isObject() && reply["returnValue"].isBool() &&
      reply["returnValue"].asBool() && reply[name].isString()) {
    ids[notification_id] = reply[name].asString();
  }
  contexts.erase(notification_id);
  return Json::Value();
}

std::map<std::string, LSCalloutContext> toast_contexts;
std::map<std::string, LSCalloutContext> alert_contexts;
std::map<std::string, std::string> toast_ids;
std::map<std::string, std::string> alert_ids;

}  // namespace

// static
NotificationService* NotificationService::Instance() {
  return NotificationServiceLuna::Instance();
}

// static
NotificationServiceLuna* NotificationServiceLuna::Instance() {
  static NotificationServiceLuna* instance = new NotificationServiceLuna();
  return instance;
}

NotificationServiceLuna::NotificationServiceLuna() {
  StartService();
}

void NotificationServiceLuna::DidConnect() {}

LSMethod* NotificationServiceLuna::Methods() const {
  return nullptr;
}

const char* NotificationServiceLuna::ServiceName() const {
  return "com.webos.notification.client";
}

bool NotificationServiceLuna::Display(const NotificationData& notification) {
  // Close the previous notification with the same tag, if possible.
  Close(notification.id);
  return CreateAlert(notification);
}

bool NotificationServiceLuna::CreateToast(
    const NotificationData& notification) {
  // Transparent toast is created to save the notification in the database.
  Json::Value toast_params;
  toast_params["message"] = notification.message;
  toast_params["opacity"] = 0.0;
  toast_params["persistent"] = true;
  toast_params["sourceId"] = notification.app_id;

  toast_contexts.emplace(
      notification.id,
      LSCalloutContext(std::bind(Callback, notification.id,
                                 std::ref(toast_contexts), std::ref(toast_ids),
                                 std::cref("toastId"), std::placeholders::_1)));
  return Call("luna://com.webos.notification/createToast", toast_params,
              notification.app_id.c_str(), &toast_contexts.at(notification.id));
}

bool NotificationServiceLuna::CreateAlert(
    const NotificationData& notification) {
  Json::Value alert_buttons;

  Json::Value button;
  button["label"] = "OK";
  alert_buttons.append(button);

  Json::Value click_button;
  Json::Value params;
  params["type"] = "notificationclick";
  params["appId"] = notification.app_id;
  params["notificationId"] = notification.id;
  params["origin"] = notification.origin;
  if (!notification.buttons.empty()) {
    params["actionIndex"] = 0;
    click_button["label"] = notification.buttons[0].title;
  } else {
    click_button["label"] = "Shortcut";
  }
  click_button["onclick"] =
      "luna://com.webos.service.webappmanager/fireNotificationEvent";
  click_button["params"] = params;
  alert_buttons.append(click_button);

  Json::Value alert_params;
  alert_params["buttons"] = alert_buttons;
  alert_params["message"] = notification.message;
  alert_params["title"] = notification.title;
  if (!notification.icon.empty()) {
    alert_params["iconUrl"] = notification.icon;
  }

  alert_contexts.emplace(
      notification.id,
      LSCalloutContext(std::bind(Callback, notification.id,
                                 std::ref(alert_contexts), std::ref(alert_ids),
                                 std::cref("alertId"), std::placeholders::_1)));
  return Call("luna://com.webos.notification/createAlert", alert_params,
              nullptr, &alert_contexts.at(notification.id));
}

bool NotificationServiceLuna::Close(const std::string& notification_id) {
  return Close(notification_id, toast_ids, "toastId",
               "luna://com.webos.notification/closeToast") &&
         Close(notification_id, alert_ids, "alertId",
               "luna://com.webos.notification/closeAlert");
}

bool NotificationServiceLuna::Close(const std::string& notification_id,
                                    std::map<std::string, std::string>& map,
                                    const std::string& name,
                                    const std::string& uri) {
  // TODO: Use std::map::contains when C++20 is everywhere.
  if (map.find(notification_id) == map.end()) {
    return false;
  }

  std::string id = map.at(notification_id);
  map.erase(notification_id);

  Json::Value close_params;
  close_params[name] = id;

  return Call(uri.c_str(), close_params);
}
