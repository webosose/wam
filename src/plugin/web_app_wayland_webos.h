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

#ifndef PLUGIN_WEB_APP_WAYLAND_WEBOS_H_
#define PLUGIN_WEB_APP_WAYLAND_WEBOS_H_

#include <string>

#include "web_app_wayland.h"

class ApplicationDescription;

class WebAppWaylandWebOS : public WebAppWayland {
 public:
  explicit WebAppWaylandWebOS(const std::string& win_type,
                              const ApplicationDescription& desc);
};

#endif  // PLUGIN_WEB_APP_WAYLAND_WEBOS_H_
