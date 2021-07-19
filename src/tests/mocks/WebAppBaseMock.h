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

#ifndef WEBAPPBASEMOCK_H
#define WEBAPPBASEMOCK_H

#include <gmock/gmock.h>

#include "WebAppBase.h"

class WebAppBaseMock : public WebAppBase {
 public:
  WebAppBaseMock() = default;
  ~WebAppBaseMock() override = default;

  void attach(WebPageBase*) override {}
  WebPageBase* detach() override { return nullptr; }
  void suspendAppRendering() override {}
  void resumeAppRendering() override {}
  bool isFocused() const override { return false; }
  void resize(int width, int height) override {}
  bool isActivated() const override { return false; }
  bool isMinimized() override { return false; }
  bool isNormal() override { return false; }
  void onStageActivated() override {}
  void onStageDeactivated() override {}
  void startLaunchTimer() override {}
  void setHiddenWindow(bool hidden) override {}
  void configureWindow(const std::string& type) override {}
  void setKeepAlive(bool keepAlive) override {}
  bool isWindowed() const override { return false; }
  void relaunch(const std::string& args, const std::string& launchingAppId) override {}
  void setWindowProperty(const std::string& name, const std::string& value) override {}
  void platformBack() override {}
  void setCursor(const std::string& cursorArg,
                 int hotspot_x,
                 int hotspot_y) override {}
  void setInputRegion(const Json::Value& jsonDoc) override {}
  void setKeyMask(const Json::Value& jsonDoc) override {}
  void hide(bool forcedHide = false) override {}
  void focus() override {}
  void unfocus() override {}
  void setOpacity(float opacity) override {}
  void setAppDescription(std::shared_ptr<ApplicationDescription>) override {}
  void setPreferredLanguages(const std::string& language) override {}
  void stagePreparing() override {}
  void stageReady() override {}
  void raise() override {}
  void goBackground() override {}
  void doPendingRelaunch() override {}
  void deleteSurfaceGroup() override {}
  void keyboardVisibilityChanged(bool visible, int height) override {}
  void doClose() override {}
  void setUseVirtualKeyboard(const bool enable) override {}
  bool isKeyboardVisible() override { return false; }
  bool hideWindow() override { return false; }
  void doAttach() override {}
  void webPageLoadFailed(int errorCode) override {}
};

#endif  // WEBAPPBASEMOCK_H
