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

#include "log_manager_pmlog.h"

PmLogContext GetCustomPmLogContext(const char* context_id) {
  PmLogContext context;
  PmLogGetContext(context_id, &context);
  return context;
}

extern PmLogContext GetWAMPmLogContext() {
  static PmLogContext wam_log_context;
  // static variables are always initialized to nullptr if not specified
  if (nullptr == wam_log_context) {
    PmLogGetContext(LOG_CONTEXT, &wam_log_context);
  }
  return wam_log_context;
}
