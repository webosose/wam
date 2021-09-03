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

#include "log_manager.h"

static bool debug_events_enable = false;
static bool debug_bundle_messages_enable = false;
static bool debug_mouse_move_enable = false;

void LogManager::SetLogControl(const std::string& keys,
                               const std::string& value) {
  LOG_DEBUG("[LogManager::setLogControl] keys : %s, value : %s", keys.c_str(),
            value.c_str());

  if (keys == "all") {
    if (value == "on") {
      debug_events_enable = true;
      debug_bundle_messages_enable = true;
    } else if (value == "off") {
      debug_events_enable = false;
      debug_bundle_messages_enable = false;
    }
  } else if (keys == "event") {
    if (value == "on")
      debug_events_enable = true;
    else if (value == "off")
      debug_events_enable = false;
  } else if (keys == "bundleMessage") {
    if (value == "on")
      debug_bundle_messages_enable = true;
    else if (value == "off")
      debug_bundle_messages_enable = false;
  } else if (keys == "mouseMove") {
    if (value == "on")
      debug_mouse_move_enable = true;
    else if (value == "off")
      debug_mouse_move_enable = false;
  }
}

bool LogManager::GetDebugEventsEnabled() {
  return debug_events_enable;
}

bool LogManager::GetDebugBundleMessagesEnabled() {
  return debug_bundle_messages_enable;
}

bool LogManager::GetDebugMouseMoveEnabled() {
  return debug_mouse_move_enable;
}
