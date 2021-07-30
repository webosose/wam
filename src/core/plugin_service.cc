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

#include "plugin_service.h"

#include "web_app_base.h"
#include "web_app_manager.h"

WebAppBase* PlugInService::FindAppById(const std::string& app_id) {
  return WebAppManager::Instance()->FindAppById(app_id.c_str());
}

WebAppBase* PlugInService::FindAppByInstanceId(const std::string& instance_id) {
  return WebAppManager::Instance()->FindAppByInstanceId(instance_id);
}

std::string PlugInService::GetActiveInstanceId() {
  return WebAppManager::Instance()->GetActiveInstanceId();
}

std::list<const WebAppBase*> PlugInService::RunningApps() {
  return WebAppManager::Instance()->RunningApps();
}
