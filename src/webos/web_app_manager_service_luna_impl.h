// Copyright (c) 2015-2021 LG Electronics, Inc.
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

#ifndef WEBOS_WEB_APP_MANAGER_SERVICE_LUNA_IMPL_H_
#define WEBOS_WEB_APP_MANAGER_SERVICE_LUNA_IMPL_H_

#include "web_app_manager_service_luna.h"

namespace Json {
class Value;
};

class WebAppManagerServiceLunaImpl : public WebAppManagerServiceLuna {
 public:
  // Overridden from WebAppManagerServiceLuna
  void SystemServiceConnectCallback(const Json::Value& reply);
  Json::Value setInspectorEnable(const Json::Value& request) override;

  void GetSystemOptionCallback(const Json::Value& reply);
};

#endif  // WEBOS_WEB_APP_MANAGER_SERVICE_LUNA_IMPL_H_
