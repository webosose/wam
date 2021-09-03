// Copyright (c) 2021 LG Electronics, Inc.
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

#include "web_app_window_impl.h"

#include "web_app_wayland_window.h"

WebAppWindowImpl::WebAppWindowImpl(std::unique_ptr<WebAppWaylandWindow> window)
    : window_(std::move(window)) {}

WebAppWindowImpl::~WebAppWindowImpl() = default;

int WebAppWindowImpl::DisplayWidth() {
  return window_ ? window_->DisplayWidth() : 0;
}

int WebAppWindowImpl::DisplayHeight() {
  return window_ ? window_->DisplayHeight() : 0;
}

void WebAppWindowImpl::InitWindow(int width, int height) {
  if (window_)
    window_->InitWindow(width, height);
}

void WebAppWindowImpl::SetLocationHint(
    webos::WebAppWindowBase::LocationHint value) {
  if (window_)
    window_->SetLocationHint(value);
}

webos::NativeWindowState WebAppWindowImpl::GetWindowHostState() const {
  return window_ ? window_->GetWindowHostState() : webos::NATIVE_WINDOW_DEFAULT;
}

void WebAppWindowImpl::CreateWindowGroup(
    const webos::WindowGroupConfiguration& config) {
  if (window_)
    window_->CreateWindowGroup(config);
}

void WebAppWindowImpl::AttachToWindowGroup(const std::string& name,
                                           const std::string& layer) {
  if (window_)
    window_->AttachToWindowGroup(name, layer);
}

bool WebAppWindowImpl::IsKeyboardVisible() {
  return window_ ? window_->IsKeyboardVisible() : false;
}

void WebAppWindowImpl::SetKeyMask(webos::WebOSKeyMask key_mask) {
  if (window_)
    window_->SetKeyMask(key_mask);
}

void WebAppWindowImpl::SetKeyMask(webos::WebOSKeyMask key_mask, bool set) {
  if (window_)
    window_->SetKeyMask(key_mask, set);
}

void WebAppWindowImpl::SetInputRegion(const std::vector<gfx::Rect>& region) {
  if (window_)
    window_->SetInputRegion(region);
}

void WebAppWindowImpl::SetWindowProperty(const std::string& name,
                                         const std::string& value) {
  if (window_)
    window_->SetWindowProperty(name, value);
}

void WebAppWindowImpl::Resize(int width, int height) {
  if (window_)
    window_->Resize(width, height);
}

void WebAppWindowImpl::FocusWindowGroupOwner() {
  if (window_)
    window_->FocusWindowGroupOwner();
}

void WebAppWindowImpl::FocusWindowGroupLayer() {
  if (window_)
    window_->FocusWindowGroupLayer();
}

void WebAppWindowImpl::SetOpacity(float opacity) {
  if (window_)
    window_->SetOpacity(opacity);
}

void WebAppWindowImpl::SetWindowHostState(webos::NativeWindowState state) {
  if (window_)
    window_->SetWindowHostState(state);
}

void WebAppWindowImpl::RecreatedWebContents() {
  if (window_)
    window_->RecreatedWebContents();
}

void WebAppWindowImpl::DetachWindowGroup() {
  if (window_)
    window_->DetachWindowGroup();
}

void WebAppWindowImpl::SetUseVirtualKeyboard(bool enable) {
  if (window_)
    window_->SetUseVirtualKeyboard(enable);
}

void WebAppWindowImpl::SetWebApp(WebAppWayland* w) {
  if (window_)
    window_->SetWebApp(w);
}

void WebAppWindowImpl::Hide() {
  if (window_)
    window_->HideWindow();
}

void WebAppWindowImpl::Show() {
  if (window_)
    window_->ShowWindow();
}

void WebAppWindowImpl::PlatformBack() {
  if (window_)
    window_->PlatformBack();
}

void WebAppWindowImpl::SetCursor(const std::string& cursor_arg,
                                 int hotspot_x,
                                 int hotspot_y) {
  if (window_)
    window_->SetCursor(cursor_arg, hotspot_x, hotspot_y);
}

void WebAppWindowImpl::AttachWebContents(void* web_contents) {
  if (window_)
    window_->AttachWebContentsToWindow(web_contents);
}

bool WebAppWindowImpl::Event(WebOSEvent* event) {
  return window_ ? window_->event(event) : false;
}
