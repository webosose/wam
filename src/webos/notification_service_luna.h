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

#ifndef WEBOS_NOTIFICATION_SERVICE_LUNA_H_
#define WEBOS_NOTIFICATION_SERVICE_LUNA_H_

#include "notification_service.h"
#include "palm_service_base.h"

class NotificationServiceLuna : public PalmServiceBase,
                                public NotificationService {
 public:
  static NotificationServiceLuna* Instance();

  NotificationServiceLuna(const NotificationServiceLuna&) = delete;
  NotificationServiceLuna& operator=(const NotificationServiceLuna&) = delete;

  // PalmServiceBase
  void DidConnect() override;
  LSMethod* Methods() const override;
  const char* ServiceName() const override;

  // NotificationService
  bool Display(const NotificationData& notification) override;
  bool Close(const std::string& notification_id) override;

 private:
  NotificationServiceLuna();
  ~NotificationServiceLuna() override = default;

  bool CreateToast(const NotificationData& notification);
  bool CreateAlert(const NotificationData& notification);
  bool Close(const std::string& notification_id,
             std::map<std::string, std::string>& map,
             const std::string& name,
             const std::string& uri);
};

#endif  // WEBOS_NOTIFICATION_SERVICE_LUNA_H_
