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

#ifndef CORE_NOTIFICATION_SERVICE_H_
#define CORE_NOTIFICATION_SERVICE_H_

#include <string>
#include <vector>

struct NotificationData {
  struct ButtonInfo {
    // TODO: Remove the constructor when C++20 is everywhere.
    ButtonInfo(const std::string& title, const std::string icon_path)
        : title(title), icon_path(icon_path) {}
    std::string title;
    std::string icon_path;
  };
  std::string id;
  std::string origin;
  std::string title;
  std::string message;
  std::string app_id;
  std::string icon;
  std::vector<ButtonInfo> buttons;
};

class NotificationService {
 public:
  static NotificationService* Instance();

  virtual bool Display(const NotificationData& notification) = 0;
};

#endif  // CORE_NOTIFICATION_SERVICE_H_
