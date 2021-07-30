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

#ifndef WEBOS_PLUGIN_SERVICE_LUNA_H_
#define WEBOS_PLUGIN_SERVICE_LUNA_H_

#include "palm_service_base.h"
#include "plugin_service.h"

class WebAppBase;

class PlugInServiceLuna : public PalmServiceBase, public PlugInService {
 public:
  // PlugInService
  void StartService() override;
};

#endif  // WEBOS_PLUGIN_SERVICE_LUNA_H_
