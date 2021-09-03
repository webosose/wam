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

#ifndef WEBOS_DEVICE_INFO_IMPL_H_
#define WEBOS_DEVICE_INFO_IMPL_H_

#include "device_info.h"

class DeviceInfoImpl : public DeviceInfo {
 public:
  DeviceInfoImpl();

  void Initialize() override;

 private:
  int screen_width_;
  int screen_height_;

  float screen_density_;

  std::string model_name_;
  std::string platform_version_;

  // platform versions are <major>.<minor>.<dot>
  int version_major_;
  int version_minor_;
  int version_dot_;

  bool support_3d_;
  std::string ota_id_;
  std::string hardware_version_;
  std::string firmware_version_;

  bool GetInfoFromLunaPrefs(const char* key, std::string& value) const;
  void InitDisplayInfo();
  void InitPlatformInfo();
  void GatherInfo();
};

#endif  // WEBOS_DEVICE_INFO_IMPL_H_
