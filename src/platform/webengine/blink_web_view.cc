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

#include "blink_web_view.h"

#include "log_manager.h"
#include "web_page_blink_delegate.h"

BlinkWebView::BlinkWebView(bool do_initialize)
    : WebViewBase::WebViewBase(),
      delegate_(nullptr),
      progress_(0),
      user_script_executed_(false) {}

void BlinkWebView::AddUserScript(const std::string& script) {
  user_scripts_.push_back(script);
}

void BlinkWebView::ClearUserScripts() {
  user_scripts_.clear();
}

void BlinkWebView::ExecuteUserScripts() {
  if (user_script_executed_)
    return;

  for (std::vector<std::string>::const_iterator it = user_scripts_.begin();
       it != user_scripts_.end(); ++it)
    RunJavaScript(*it);

  user_script_executed_ = true;
}

void BlinkWebView::SetDelegate(WebPageBlinkDelegate* delegate) {
  delegate_ = delegate;
}

void BlinkWebView::HandleBrowserControlCommand(
    const std::string& command,
    const std::vector<std::string>& arguments) {
  if (delegate_) {
    delegate_->HandleBrowserControlCommand(command, arguments);
  }
}

void BlinkWebView::HandleBrowserControlFunction(
    const std::string& command,
    const std::vector<std::string>& arguments,
    std::string* result) {
  if (delegate_) {
    delegate_->HandleBrowserControlFunction(command, arguments, result);
  }
}

void BlinkWebView::OnLoadProgressChanged(double progress) {
  if (!delegate_)
    return;

  progress_ = static_cast<int>(progress * 100);
  delegate_->LoadProgressChanged(progress);
}

void BlinkWebView::Close() {
  if (!delegate_)
    return;

  delegate_->Close();
}

bool BlinkWebView::DecidePolicyForResponse(bool is_main_frame,
                                           int status_code,
                                           const std::string& url,
                                           const std::string& status_text) {
  if (!delegate_)
    return false;

  return delegate_->DecidePolicyForResponse(is_main_frame, status_code, url,
                                            status_text);
}

bool BlinkWebView::AcceptsVideoCapture() {
  if (!delegate_)
    return false;

  return delegate_->AcceptsVideoCapture();
}

bool BlinkWebView::AcceptsAudioCapture() {
  if (!delegate_)
    return false;

  return delegate_->AcceptsAudioCapture();
}

void BlinkWebView::DidDropAllPeerConnections(
    webos::DropPeerConnectionReason reason) {
  if (!delegate_)
    return;

  delegate_->DidDropAllPeerConnections();
  LOG_DEBUG("Dropped all WebRTC peer connections with reason %d", reason);
}

void BlinkWebView::DidSwapCompositorFrame() {
  if (delegate_)
    delegate_->DidSwapCompositorFrame();
}

void BlinkWebView::DidFirstFrameFocused() {
  if (!delegate_)
    return;

  delegate_->DidFirstFrameFocused();
}

void BlinkWebView::DocumentLoadFinished() {
  ExecuteUserScripts();
}

void BlinkWebView::LoadVisuallyCommitted() {
  if (!delegate_)
    return;

  delegate_->LoadVisuallyCommitted();
}

void BlinkWebView::LoadFinished(const std::string& url) {
  if (!delegate_)
    return;

  delegate_->LoadFinished(url);
}

void BlinkWebView::LoadFailed(const std::string& url,
                              int err_code,
                              const std::string& err_desc) {
  if (!delegate_)
    return;

  delegate_->LoadFailed(url, err_code, err_desc);
}

void BlinkWebView::LoadAborted(const std::string& url) {
  if (!delegate_)
    return;

  delegate_->LoadAborted(url);
}

void BlinkWebView::LoadStarted() {
  if (!delegate_)
    return;

  delegate_->LoadStarted();
}

void BlinkWebView::LoadStopped() {
  if (!delegate_)
    return;

  delegate_->LoadStopped();
}

void BlinkWebView::DidStartNavigation(const std::string& url,
                                      bool is_in_main_frame) {
  if (!delegate_)
    return;

  user_script_executed_ = false;

  delegate_->DidStartNavigation(url, is_in_main_frame);
}

void BlinkWebView::DidFinishNavigation(const std::string& url,
                                       bool is_in_main_frame) {
  if (!delegate_)
    return;

  delegate_->DidFinishNavigation(url, is_in_main_frame);
}

void BlinkWebView::RenderProcessCreated(int pid) {
  if (!delegate_)
    return;

  delegate_->RenderProcessCreated(pid);
}

void BlinkWebView::RenderProcessGone() {
  if (!delegate_)
    return;

  delegate_->RenderProcessCrashed();
}

void BlinkWebView::TitleChanged(const std::string& title) {
  if (!delegate_)
    return;

  delegate_->TitleChanged(title);
}

void BlinkWebView::NavigationHistoryChanged() {
  if (!delegate_)
    return;

  delegate_->NavigationHistoryChanged();
}

bool BlinkWebView::AllowMouseOnOffEvent() const {
  if (!delegate_)
    return false;

  return delegate_->AllowMouseOnOffEvent();
}

void BlinkWebView::DidResumeDOM() {
  if (delegate_)
    delegate_->DidResumeDOM();
}

void BlinkWebView::DidErrorPageLoadedFromNetErrorHelper() {
  if (!delegate_)
    return;
  return delegate_->DidErrorPageLoadedFromNetErrorHelper();
}
