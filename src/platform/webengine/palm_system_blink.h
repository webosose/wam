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

#ifndef PLATFORM_WEBENGINE_PALM_SYSTEM_BLINK_H_
#define PLATFORM_WEBENGINE_PALM_SYSTEM_BLINK_H_

#include <string>
#include <vector>

#include "palm_system_webos.h"

namespace Json {
class Value;
};

class PalmSystemBlink : public PalmSystemWebOS {
 public:
  PalmSystemBlink(WebAppBase* app);

  std::string HandleBrowserControlMessage(
      const std::string& command,
      const std::vector<std::string>& arguments);

  // PalmSystemWebOS
  void SetCountry() override;
  void SetLaunchParams(const std::string& params) override;

  virtual void SetLocale(const std::string& params);
  virtual double DevicePixelRatio();
  void ResetInitialized() { initialized_ = false; }
  bool IsInitialized() { return initialized_; }

 protected:
  // PalmSystemWebOS
  Json::Value Initialize();
  std::string Identifier() const override;
  void SetLoadErrorPolicy(const std::string& params) override;

  virtual std::string TrustLevel() const;
  virtual void OnCloseNotify(const std::string& params);

 private:
  bool initialized_;
};

#endif  // PLATFORM_WEBENGINE_PALM_SYSTEM_BLINK_H_
