// Copyright (c) 2016-2021 LG Electronics, Inc.
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

#ifndef PLATFORM_WEBENGINE_WEB_PAGE_BLINK_DELEGATE_H_
#define PLATFORM_WEBENGINE_WEB_PAGE_BLINK_DELEGATE_H_

#include <string>
#include <vector>

class WebPageBlinkDelegate {
 public:
  virtual ~WebPageBlinkDelegate() {}
  virtual void Close() = 0;
  virtual bool DecidePolicyForResponse(bool is_main_frame,
                                       int status_code,
                                       const std::string& url,
                                       const std::string& status_text) = 0;
  virtual bool AcceptsVideoCapture() = 0;
  virtual bool AcceptsAudioCapture() = 0;
  virtual void DidFirstFrameFocused() = 0;
  virtual void HandleBrowserControlCommand(
      const std::string& command,
      const std::vector<std::string>& arguments) = 0;
  virtual void HandleBrowserControlFunction(
      const std::string& command,
      const std::vector<std::string>& arguments,
      std::string* result) = 0;
  virtual void LoadFinished(const std::string& url) = 0;
  virtual void LoadFailed(const std::string& url,
                          int errCode,
                          const std::string& errDesc) = 0;
  virtual void LoadStopped() = 0;
  virtual void LoadAborted(const std::string& url) = 0;
  virtual void LoadStarted() {}
  virtual void LoadProgressChanged(double progress) = 0;
  virtual void DidStartNavigation(const std::string& url,
                                  bool is_in_main_frame) {}
  virtual void DidFinishNavigation(const std::string& url,
                                   bool is_in_main_frame) {}
  virtual void LoadVisuallyCommitted() = 0;
  virtual void RenderProcessCreated(int pid) = 0;
  virtual void RenderProcessCrashed() = 0;
  virtual void TitleChanged(const std::string& title) = 0;
  virtual void NavigationHistoryChanged() = 0;
  virtual void DidHistoryBackOnTopPage() {}
  virtual void DidClearWindowObject() {}
  virtual void DidDropAllPeerConnections() {}
  virtual void DidSwapCompositorFrame() {}
  virtual bool AllowMouseOnOffEvent() const = 0;
  virtual void DidResumeDOM() {}
  virtual void DidErrorPageLoadedFromNetErrorHelper() = 0;
};

#endif  // PLATFORM_WEBENGINE_WEB_PAGE_BLINK_DELEGATE_H_
