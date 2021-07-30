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

#ifndef PLATFORM_WEBENGINE_BLINK_WEB_VIEW_H_
#define PLATFORM_WEBENGINE_BLINK_WEB_VIEW_H_

#include <string>
#include <vector>

#include "webos/webview_base.h"

class WebPageBlinkDelegate;

class BlinkWebView : public webos::WebViewBase {
 public:
  // TODO need to refactor both constructors (here & pluggables)
  BlinkWebView(bool doInitialize = true);
  BlinkWebView(const std::string& group) : BlinkWebView() {}

  void AddUserScript(const std::string& script);
  void ClearUserScripts();
  void ExecuteUserScripts();
  void SetDelegate(WebPageBlinkDelegate* delegate);
  WebPageBlinkDelegate* Delegate() { return delegate_; }
  int Progress() { return progress_; }

  // webos::WebViewBase(indirectly from webos::WebViewDelegate)
  void OnLoadProgressChanged(double progress) override;
  void DidFirstFrameFocused() override;
  void TitleChanged(const std::string& title);
  void NavigationHistoryChanged();
  void Close() override;
  bool DecidePolicyForResponse(bool is_main_frame,
                               int status_code,
                               const std::string& url,
                               const std::string& status_text) override;
  bool AcceptsVideoCapture() override;
  bool AcceptsAudioCapture() override;
  void LoadAborted(const std::string& url) override;
  void LoadStarted() override;
  void LoadFinished(const std::string& url) override;
  void LoadFailed(const std::string& url,
                  int err_code,
                  const std::string& err_desc) override;
  void LoadStopped() override;
  void DocumentLoadFinished() override;
  void DidStartNavigation(const std::string& url,
                          bool is_in_main_frame) override;
  void DidFinishNavigation(const std::string& url,
                           bool is_in_main_frame) override;
  void RenderProcessCreated(int pid) override;
  void RenderProcessGone() override;
  void DidHistoryBackOnTopPage() {}
  void DidClearWindowObject() {}
  virtual void DidDropAllPeerConnections(
      webos::DropPeerConnectionReason reason);
  void DidSwapCompositorFrame();
  virtual bool AllowMouseOnOffEvent() const;
  void HandleBrowserControlCommand(
      const std::string& command,
      const std::vector<std::string>& arguments) override;
  void HandleBrowserControlFunction(const std::string& command,
                                    const std::vector<std::string>& arguments,
                                    std::string* result) override;
  void LoadVisuallyCommitted() override;
  void DidResumeDOM() override;
  void DidErrorPageLoadedFromNetErrorHelper() override;

 private:
  WebPageBlinkDelegate* delegate_;
  int progress_;

  bool user_script_executed_;
  std::vector<std::string> user_scripts_;
};

#endif  // PLATFORM_WEBENGINE_BLINK_WEB_VIEW_H_
