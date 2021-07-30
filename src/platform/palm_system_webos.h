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

#ifndef PLATFORM_PALM_SYSTEM_WEBOS_H_
#define PLATFORM_PALM_SYSTEM_WEBOS_H_

#include <string>

#include "palm_system_base.h"

class WebAppBase;
class WebAppWayland;

class PalmSystemWebOS : public PalmSystemBase {
 public:
  PalmSystemWebOS(WebAppBase* app);

  virtual void SetCountry() {}
  virtual void SetFolderPath(const std::string& params) {}
  virtual void SetLaunchParams(const std::string& params);

 protected:
  enum GroupClientCallKey { kKeyMask = 1, kFocusOwner, kFocusLayer };

  virtual std::string Identifier() const = 0;
  virtual std::string LaunchParams() const { return launch_params_; }
  virtual std::string Version() const { return std::string(); }

  virtual std::string ScreenOrientation() const { return std::string("up"); }
  virtual std::string WindowOrientation() const { return std::string("free"); }

  virtual bool IsActivated() const;
  virtual bool IsKeyboardVisible() const;
  virtual bool IsMinimal() const;
  virtual int ActivityId() const;
  virtual void Activate();
  virtual void Deactivate();
  virtual void StagePreparing();
  virtual void StageReady();

  virtual void Show() {}
  virtual void Hide();

  virtual void SetKeepAlive(bool keep);
  virtual void SetLoadErrorPolicy(const std::string& params) {}
  virtual void SetInputRegion(const std::string& params);
  virtual void SetGroupClientEnvironment(GroupClientCallKey callKey,
                                         const std::string& params);

  virtual void LogMsgWithClock(const std::string& msg_id,
                               const std::string& perf_type,
                               const std::string& perf_group);
  virtual void LogMsgString(int32_t level,
                            const std::string& msg_id,
                            const std::string& kv_pairs,
                            const std::string& message);
  virtual bool CursorVisibility();
  virtual void UpdateLaunchParams(const std::string& launch_params);

 protected:
  WebAppWayland* app_;
  std::string launch_params_;
};

#endif  // PLATFORM_PALM_SYSTEM_WEBOS_H_
