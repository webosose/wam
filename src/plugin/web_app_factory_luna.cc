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

#include "web_app_factory_luna.h"

#include <string>

#include "log_manager.h"
#include "plugin_interface.h"
#include "util/url.h"
#include "web_app_base.h"
#include "web_app_wayland.h"
#include "web_app_wayland_webos.h"
#include "web_page_base.h"
#include "web_page_blink.h"
#include "window_types.h"

const char* kPluginApplicationType = "default";

WebAppFactoryInterface* CreateInstance() {
  return new WebAppFactoryLuna();
}

void DeleteInstance(WebAppFactoryInterface* interface) {
  delete interface;
}

WebAppBase* WebAppFactoryLuna::CreateWebApp(
    const std::string& win_type,
    std::shared_ptr<ApplicationDescription> desc) {
  WebAppBase* app = nullptr;

  if (win_type == kWtCard || win_type == kWtPopup || win_type == kWtMinimal ||
      win_type == kWtFloating || win_type == kWtSystemUi) {
    app = new WebAppWaylandWebOS(win_type, desc);
  } else if (win_type == kWtOverlay || win_type == kWtNone) {
    app = new WebAppWayland(win_type, 0, 0, desc->GetDisplayAffinity());
  } else {
    LOG_WARNING(MSGID_BAD_WINDOW_TYPE, 1,
                PMLOGKS("WINDOW_TYPE", win_type.c_str()), "");
  }
  return app;
}

WebAppBase* WebAppFactoryLuna::CreateWebApp(
    const std::string& win_type,
    WebPageBase* page,
    std::shared_ptr<ApplicationDescription> desc) {
  return CreateWebApp(win_type, desc);
}

WebPageBase* WebAppFactoryLuna::CreateWebPage(
    const wam::Url& url,
    std::shared_ptr<ApplicationDescription> desc,
    const std::string& launch_params) {
  return new WebPageBlink(url, desc, launch_params);
}
