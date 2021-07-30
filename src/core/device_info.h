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

#ifndef CORE_DEVICE_INFO_H_
#define CORE_DEVICE_INFO_H_

#include <string>
#include <unordered_map>

class DeviceInfo {
 public:
  virtual ~DeviceInfo() {}

  virtual void Initialize(){};
  virtual void Terminate(){};

  virtual bool GetDisplayWidth(int& value) const;
  virtual void SetDisplayWidth(int value);

  virtual bool GetDisplayHeight(int& value) const;
  virtual void SetDisplayHeight(int value);

  virtual bool GetSystemLanguage(std::string& value) const;
  virtual void SetSystemLanguage(const std::string& value);

  virtual bool GetDeviceInfo(const std::string& name, std::string& value) const;
  virtual void SetDeviceInfo(const std::string& name, const std::string& value);

 private:
  std::unordered_map<std::string, std::string> device_info_;
};

#endif  // CORE_DEVICE_INFO_H_
