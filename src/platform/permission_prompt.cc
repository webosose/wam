// Copyright 2022 LG Electronics, Inc.
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

#include "platform/permission_prompt.h"

#include <string>

#include "core/application_description.h"
#include "core/web_app_base.h"
#include "core/web_app_manager.h"
#include "util/log_manager.h"

namespace {
const char* PermissionRequestTypeToString(PermissionRequest::RequestType type) {
  switch (type) {
    case PermissionRequest::RequestType::kNotifications:
      return "notifications";
    default:
      return nullptr;
  }
}
}  // namespace

PermissionPrompt::PermissionPrompt(
    neva_app_runtime::PermissionPrompt::Delegate* delegate)
    : delegate_(delegate) {
  SetDecisions();
}

PermissionPrompt::~PermissionPrompt() = default;

void PermissionPrompt::Show() {
  LOG_DEBUG("PermissionPrompt::Show");
  // TODO: A permission prompt displays when web app asks for permission if
  // needed.
  delegate_->Accept();
}

void PermissionPrompt::Close() {
  LOG_DEBUG("PermissionPrompt::Close");
  delegate_->Closing();
}

void PermissionPrompt::SetDecisions() {
  LOG_DEBUG("PermissionPrompt::SetDecisions");
  for (const PermissionRequest* request : delegate_->Requests()) {
    PermissionRequest::RequestType type = request->GetRequestType();
    switch (type) {
      case PermissionRequest::RequestType::kCameraStream:
      case PermissionRequest::RequestType::kMicStream:
      case PermissionRequest::RequestType::kNotifications: {
        bool status = GetPermissionStatusFromAppDesc(type);
        if (status)
          delegate_->Accept();
        else
          delegate_->Deny();
      } break;
      default:
        LOG_ERROR(MSGID_ERROR_ERROR, 0,
                  "There is no matching permission type.");
    }
  }
}

bool PermissionPrompt::GetPermissionStatusFromAppDesc(
    PermissionRequest::RequestType type) {
  const std::string app_id = delegate_->GetAppId();
  WebAppBase* app = WebAppManager::Instance()->FindAppById(app_id);
  ApplicationDescription* app_desc = app->GetAppDescription();

  bool status = false;
  switch (type) {
    case PermissionRequest::RequestType::kCameraStream: {
      status = app_desc->AllowVideoCapture();
    } break;
    case PermissionRequest::RequestType::kMicStream: {
      status = app_desc->AllowAudioCapture();
    } break;
    default: {
      auto& permissions = app_desc->WebAppPermissions();
      const char* str_type = PermissionRequestTypeToString(type);
      if (str_type != nullptr) {
        status = (permissions.find(str_type) != permissions.end());
      }
    } break;
  }
  LOG_INFO(MSGID_SET_PERMISSION, 2, PMLOGKS("APP_ID", app_id.c_str()),
           PMLOGKS("PERMISSION_STATUS", (status ? "granted" : "denied")), "");
  return status;
}
