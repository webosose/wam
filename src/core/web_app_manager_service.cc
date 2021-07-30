// Copyright (c) 2008-2021 LG Electronics, Inc.
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

#include "web_app_manager_service.h"

#include <json/value.h>

#include "log_manager.h"
#include "web_app_base.h"

WebAppManagerService::WebAppManagerService() {}

std::string WebAppManagerService::OnLaunch(const std::string& app_desc_string,
                                           const std::string& params,
                                           const std::string& launching_app_id,
                                           int& err_code,
                                           std::string& err_msg) {
  return WebAppManager::Instance()->Launch(app_desc_string, params,
                                           launching_app_id, err_code, err_msg);
}

bool WebAppManagerService::OnKillApp(const std::string& app_id,
                                     const std::string& instance_id,
                                     bool force) {
  return WebAppManager::Instance()->OnKillApp(app_id, instance_id, force);
}

bool WebAppManagerService::OnPauseApp(const std::string& instance_id) {
  return WebAppManager::Instance()->OnPauseApp(instance_id);
}

Json::Value WebAppManagerService::OnLogControl(const std::string& keys,
                                               const std::string& value) {
  LogManager::SetLogControl(keys, value);

  Json::Value reply;

  reply["event"] = LogManager::GetDebugEventsEnabled();
  reply["bundleMessage"] = LogManager::GetDebugBundleMessagesEnabled();
  reply["mouseMove"] = LogManager::GetDebugMouseMoveEnabled();
  reply["returnValue"] = true;

  return reply;
}

bool WebAppManagerService::OnCloseAllApps(uint32_t pid) {
  LOG_INFO(MSGID_LUNA_API, 2, PMLOGKS("API", "closeAllApps"),
           PMLOGKFV("PID", "%d", pid), "");
  return WebAppManager::Instance()->CloseAllApps(pid);
}

bool WebAppManagerService::IsDiscardCodeCacheRequired() {
  return WebAppManager::Instance()->IsDiscardCodeCacheRequired();
}

void WebAppManagerService::OnDiscardCodeCache(uint32_t pid) {
  LOG_INFO(MSGID_LUNA_API, 2, PMLOGKS("API", "discardCodeCache"),
           PMLOGKFV("PID", "%d", pid), "");
  WebAppManager::Instance()->DiscardCodeCache(pid);
}

bool WebAppManagerService::OnPurgeSurfacePool(uint32_t pid) {
  return WebAppManager::Instance()->PurgeSurfacePool(pid);
}

Json::Value WebAppManagerService::GetWebProcessProfiling() {
  return WebAppManager::Instance()->GetWebProcessProfiling();
}

void WebAppManagerService::OnClearBrowsingData(
    const int remove_browsing_data_mask) {
  WebAppManager::Instance()->ClearBrowsingData(remove_browsing_data_mask);
}

void WebAppManagerService::OnAppInstalled(const std::string& app_id) {
  WebAppManager::Instance()->AppInstalled(app_id);
}

void WebAppManagerService::OnAppRemoved(const std::string& app_id) {
  WebAppManager::Instance()->AppRemoved(app_id);
}

void WebAppManagerService::SetDeviceInfo(const std::string& name,
                                         const std::string& value) {
  WebAppManager::Instance()->SetDeviceInfo(name, value);
}

void WebAppManagerService::SetUiSize(int width, int height) {
  WebAppManager::Instance()->SetUiSize(width, height);
}

void WebAppManagerService::SetSystemLanguage(const std::string& language) {
  WebAppManager::Instance()->SetSystemLanguage(language);
}

std::string WebAppManagerService::GetSystemLanguage() {
  std::string language;
  WebAppManager::Instance()->GetSystemLanguage(language);
  return language;
}

void WebAppManagerService::SetForceCloseApp(const std::string& appId,
                                            const std::string& instance_id) {
  WebAppManager::Instance()->SetForceCloseApp(appId, instance_id);
}

void WebAppManagerService::DeleteStorageData(const std::string& identifier) {
  WebAppManager::Instance()->DeleteStorageData(identifier);
}

void WebAppManagerService::KillCustomPluginProcess(
    const std::string& app_base_path) {
  WebAppManager::Instance()->KillCustomPluginProcess(app_base_path);
}

void WebAppManagerService::RequestKillWebProcess(uint32_t pid) {
  WebAppManager::Instance()->RequestKillWebProcess(pid);
}

std::list<const WebAppBase*> WebAppManagerService::RunningApps() {
  return WebAppManager::Instance()->RunningApps();
}

std::list<const WebAppBase*> WebAppManagerService::RunningApps(uint32_t pid) {
  return WebAppManager::Instance()->RunningApps(pid);
}

std::vector<ApplicationInfo> WebAppManagerService::List(
    bool include_system_apps) {
  return WebAppManager::Instance()->List(include_system_apps);
}

void WebAppManagerService::SetAccessibilityEnabled(bool enable) {
  WebAppManager::Instance()->SetAccessibilityEnabled(enable);
}

uint32_t WebAppManagerService::GetWebProcessId(const std::string& app_id,
                                               const std::string& instance_id) {
  return WebAppManager::Instance()->GetWebProcessId(app_id, instance_id);
}

void WebAppManagerService::UpdateNetworkStatus(const Json::Value& object) {
  WebAppManager::Instance()->UpdateNetworkStatus(object);
}

void WebAppManagerService::NotifyMemoryPressure(
    webos::WebViewBase::MemoryPressureLevel level) {
  WebAppManager::Instance()->NotifyMemoryPressure(level);
}

bool WebAppManagerService::IsEnyoApp(const std::string& app_id) {
  return WebAppManager::Instance()->IsEnyoApp(app_id);
}

int WebAppManagerService::MaskForBrowsingDataType(const char* type) {
  return WebAppManager::Instance()->MaskForBrowsingDataType(type);
}
