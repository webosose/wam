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
#include "util/log_manager.h"

PermissionPrompt::PermissionPrompt(
    neva_app_runtime::PermissionPrompt::Delegate* delegate)
    : delegate_(delegate) {
  Close();
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
