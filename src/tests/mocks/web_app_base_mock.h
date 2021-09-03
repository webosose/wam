//  (c) 2021 LG Electronics, Inc.
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

#ifndef TESTS_MOCKS_WEB_APP_BASE_MOCK_H_
#define TESTS_MOCKS_WEB_APP_BASE_MOCK_H_

#include <gmock/gmock.h>

#include "web_app_base.h"

class WebAppBaseMock : public WebAppBase {
 public:
  WebAppBaseMock() = default;
  ~WebAppBaseMock() override = default;

  void Attach(WebPageBase*) override {}
  WebPageBase* Detach() override { return nullptr; }
  void SuspendAppRendering() override {}
  void ResumeAppRendering() override {}
  bool IsFocused() const override { return false; }
  void Resize(int width, int height) override {}
  bool IsActivated() const override { return false; }
  bool IsMinimized() override { return false; }
  bool IsNormal() override { return false; }
  void OnStageActivated() override {}
  void OnStageDeactivated() override {}
  void StartLaunchTimer() override {}
  void SetHiddenWindow(bool hidden) override {}
  void ConfigureWindow(const std::string& type) override {}
  void SetKeepAlive(bool keep_alive) override {}
  bool IsWindowed() const override { return false; }
  void Relaunch(const std::string& args,
                const std::string& launching_app_id) override {}
  void SetWindowProperty(const std::string& name,
                         const std::string& value) override {}
  void PlatformBack() override {}
  void SetCursor(const std::string& cursor_arg,
                 int hotspot_x,
                 int hotspot_y) override {}
  void SetInputRegion(const Json::Value& json_doc) override {}
  void SetKeyMask(const Json::Value& json_doc) override {}
  void Hide(bool forced_hide = false) override {}
  void Focus() override {}
  void Unfocus() override {}
  void SetOpacity(float opacity) override {}
  void SetAppDescription(std::shared_ptr<ApplicationDescription>) override {}
  void SetPreferredLanguages(const std::string& language) override {}
  void StagePreparing() override {}
  void StageReady() override {}
  void Raise() override {}
  void GoBackground() override {}
  void DoPendingRelaunch() override {}
  void DeleteSurfaceGroup() override {}
  void KeyboardVisibilityChanged(bool visible, int height) override {}
  void DoClose() override {}
  void SetUseVirtualKeyboard(const bool enable) override {}
  bool IsKeyboardVisible() override { return false; }
  bool HideWindow() override { return false; }
  void DoAttach() override {}
  void WebPageLoadFailed(int error_code) override {}
};

#endif  // TESTS_MOCKS_WEB_APP_BASE_MOCK_H_
