// Copyright (c) 2014-2018 LG Electronics, Inc.
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

#include "palm_system_webos.h"

#include <memory>

#include <json/json.h>
#include <sys/stat.h>

#include "application_description.h"
#include "log_manager.h"
#include "utils.h"
#include "web_app_base.h"
#include "web_app_wayland.h"
#include "web_page_base.h"

PalmSystemWebOS::PalmSystemWebOS(WebAppBase* app)
    : app_(static_cast<WebAppWayland*>(app)), launch_params_() {}

void PalmSystemWebOS::SetLaunchParams(const std::string& params) {
  Json::Value jsonDoc = Json::nullValue;

  const bool result = util::StringToJson(params, jsonDoc);

  if (!result || jsonDoc.isNull())
    launch_params_.erase();
  else
    launch_params_ = params;
}

bool PalmSystemWebOS::IsActivated() const {
  return app_->IsFocused();
}

bool PalmSystemWebOS::IsKeyboardVisible() const {
  return app_->IsKeyboardVisible();
}

bool PalmSystemWebOS::IsMinimal() const {
  return util::DoesPathExist("/var/luna/preferences/ran-firstuse");
}

int PalmSystemWebOS::ActivityId() const {
  if (!app_->Page())
    return -1;

  return 0;
}

void PalmSystemWebOS::Activate() {
  ApplicationDescription* appDesc = app_->GetAppDescription();
  if (appDesc && !appDesc->HandlesRelaunch())
    return;

  // ask compositor to raise window. Compositor should raise us, then
  // give us focus, so we shouldn't have to specifically request focus.
  LOG_DEBUG(
      "[%s] called webOSSystem.activate() from the app, call raise() to make "
      "full screen",
      app_->AppId().c_str());

  app_->Raise();
}

void PalmSystemWebOS::Deactivate() {
  LOG_DEBUG(
      "[%s] called webOSSystem.deactivate() from the app, call goBackground() "
      "to go background",
      app_->AppId().c_str());
  app_->GoBackground();
}

void PalmSystemWebOS::StagePreparing() {
  app_->StagePreparing();
}

void PalmSystemWebOS::StageReady() {
  app_->StageReady();
}

void PalmSystemWebOS::Hide() {
  app_->Hide();
}

void PalmSystemWebOS::SetInputRegion(const std::string& params) {
  // this function is not related to windowGroup anymore
  Json::Value jsonDoc;
  const bool result = util::StringToJson(params, jsonDoc);
  if (result)
    app_->SetInputRegion(jsonDoc);
  else
    LOG_ERROR(MSGID_TYPE_ERROR, 0, "[%s] setInputRegion failed, params='%s'",
              app_->AppId().c_str(), params.c_str());
}

void PalmSystemWebOS::SetGroupClientEnvironment(GroupClientCallKey callKey,
                                                const std::string& params) {
  ApplicationDescription* app_desc = app_ ? app_->GetAppDescription() : 0;
  if (app_desc) {
    ApplicationDescription::WindowGroupInfo group_info =
        app_desc->GetWindowGroupInfo();
    if (!group_info.name.empty() && !group_info.is_owner) {
      switch (callKey) {
        case kKeyMask: {
          Json::Value json_doc;
          const bool result = util::StringToJson(params, json_doc);
          if (result)
            app_->SetKeyMask(json_doc);
          else
            LOG_ERROR(MSGID_TYPE_ERROR, 0,
                      "[%s] failed to get key mask from params='%s'",
                      app_->AppId().c_str(), params.c_str());
        } break;
        case kFocusOwner:
          app_->FocusOwner();
          break;
        case kFocusLayer:
          app_->FocusLayer();
          break;
        default:
          break;
      }
    }
  }
}

void PalmSystemWebOS::SetKeepAlive(bool keep) {
  // TODO: Need to check that this app is keepalive app.
  app_->SetKeepAlive(keep);
}

void PalmSystemWebOS::LogMsgWithClock(const std::string& msg_id,
                                      const std::string& perf_type,
                                      const std::string& perf_group) {
  LOG_INFO_WITH_CLOCK_TO_CUSTOM_CONTEXT(
      app_->Page()->AppId().c_str(), msg_id.size() ? msg_id.c_str() : nullptr,
      2, PMLOGKS("PerfType", perf_type.size() ? perf_type.c_str() : "empty"),
      PMLOGKS("PerfGroup", perf_group.size() ? perf_group.c_str() : "empty"),
      "");
}

void PalmSystemWebOS::LogMsgString(int32_t level,
                                   const std::string& msg_id,
                                   const std::string& kvpairs,
                                   const std::string& message) {
  LOG_STRING(app_->Page()->AppId().c_str(), level,
             msg_id.size() ? msg_id.c_str() : nullptr,
             kvpairs.size() ? kvpairs.c_str() : nullptr,
             message.size() ? message.c_str() : nullptr);
}

bool PalmSystemWebOS::CursorVisibility() {
  return app_->CursorVisibility();
}

void PalmSystemWebOS::UpdateLaunchParams(const std::string& launch_params) {
  app_->Page()->SetLaunchParams(launch_params);
}
