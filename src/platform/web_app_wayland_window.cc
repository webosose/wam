// Copyright (c) 2008-2019 LG Electronics, Inc.
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

#include "web_app_wayland_window.h"
#include "application_description.h"
#include "log_manager.h"
#include "utils.h"
#include "web_app_wayland.h"

WebAppWaylandWindow* WebAppWaylandWindow::instance_ = nullptr;

WebAppWaylandWindow* WebAppWaylandWindow::Take() {
  WebAppWaylandWindow* window;

  if (!instance_) {
    instance_ = new WebAppWaylandWindow();
    if (!instance_) {
      LOG_CRITICAL(MSGID_TAKE_FAIL, 0, "Failed to take WebAppWaylandWindow");
      return nullptr;
    }
  }

  window = instance_;
  instance_ = nullptr;
  return window;
}

void WebAppWaylandWindow::Prepare() {
  if (instance_)
    return;

  // TODO: Need to make sure preparing window is helpful
  WebAppWaylandWindow* window = CreateWindow();
  if (!window)
    return;

  instance_ = window;
}

WebAppWaylandWindow* WebAppWaylandWindow::CreateWindow() {
  WebAppWaylandWindow* window = new WebAppWaylandWindow();
  if (!window) {
    LOG_CRITICAL(MSGID_PREPARE_FAIL, 0,
                 "Failed to prepare WindowedWebAppWindow");
    return 0;
  }
  window->Resize(1, 1);
  return window;
}

WebAppWaylandWindow::WebAppWaylandWindow()
    : web_app_(0),
      cursor_visible_(false),
      xinput_activated_(false),
      last_mouse_event_(WebOSMouseEvent(WebOSEvent::None, -1., -1.)) {
  cursor_enabled_ = util::GetEnvVar("ENABLE_CURSOR_BY_DEFAULT") == "1";
}

void WebAppWaylandWindow::HideWindow() {
  LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", web_app_->AppId().c_str()),
           PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()),
           "WebAppWaylandWindow::hide()");
  WebAppWindowBase::Hide();
}

void WebAppWaylandWindow::ShowWindow() {
  WebAppWindowBase::Show();
}

void WebAppWaylandWindow::PlatformBack() {
  LOG_INFO(MSGID_WAM_DEBUG, 2, PMLOGKS("APP_ID", web_app_->AppId().c_str()),
           PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()),
           "WebAppWaylandWindow::platformBack(); generate RECENT key");
}

void WebAppWaylandWindow::SetCursor(const std::string& cursor_arg,
                                    int hotspot_x,
                                    int hotspot_y) {
  const std::string& arg = cursor_arg;
  webos::CustomCursorType type = webos::CUSTOM_CURSOR_NOT_USE;
  if (arg.empty() || arg != "default")
    LOG_DEBUG(
        "[%s] %s; arg: %s; Restore Cursor to webos::CUSTOM_CURSOR_NOT_USE",
        web_app_->AppId().c_str(), __PRETTY_FUNCTION__, arg.c_str());
  else if (arg != "blank") {
    LOG_DEBUG("[%s] %s; arg: %s; Set Cursor to webos::CUSTOM_CURSOR_BLANK",
              web_app_->AppId().c_str(), __PRETTY_FUNCTION__, arg.c_str());
    type = webos::CUSTOM_CURSOR_BLANK;
  } else {
    LOG_DEBUG(
        "[%s] %s; Custom Cursor file path : %s, hotspot_x : %d, hotspot_y : %d",
        __PRETTY_FUNCTION__, web_app_->AppId().c_str(), arg.c_str(), hotspot_x,
        hotspot_y);
    type = webos::CUSTOM_CURSOR_PATH;
  }

  SetCustomCursor(type, arg, hotspot_x, hotspot_y);

  if (type == webos::CUSTOM_CURSOR_BLANK)
    cursor_enabled_ = false;
  else
    cursor_enabled_ = true;  // all mouse event will be filtered
}

void WebAppWaylandWindow::AttachWebContentsToWindow(void* web_contents) {
  WebAppWindowBase::AttachWebContents(web_contents);
}

bool WebAppWaylandWindow::event(WebOSEvent* event) {
  if (!web_app_)
    return true;

  LogEventDebugging(event);

  // TODO: Implement each event handler and
  // remove above event() function used for qtwebengine.
  switch (event->GetType()) {
    case WebOSEvent::Close:
      LOG_INFO(MSGID_WINDOW_CLOSED, 2,
               PMLOGKS("APP_ID", web_app_->AppId().c_str()),
               PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()), "");
      web_app_->DoClose();
      return true;
    case WebOSEvent::WindowStateChange:
      if (GetWindowHostState() == webos::NATIVE_WINDOW_MINIMIZED) {
        LOG_INFO(MSGID_WINDOW_STATECHANGE, 2,
                 PMLOGKS("APP_ID", web_app_->AppId().c_str()),
                 PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()),
                 "WebOSEvent::WindowStateChange; Minimize; last_mouse_event_'s "
                 "type : %s",
                 last_mouse_event_.GetType() == WebOSEvent::MouseButtonPress
                     ? "Press; Generate MouseButtonRelease event"
                     : "Release");
        if (last_mouse_event_.GetType() == WebOSEvent::MouseButtonPress) {
          last_mouse_event_.SetType(WebOSEvent::MouseButtonRelease);
          web_app_->ForwardWebOSEvent(&last_mouse_event_);
        }
      }
      web_app_->StateChanged(GetWindowHostState());
      break;
    case WebOSEvent::WindowStateAboutToChange:
      web_app_->StateAboutToChange(GetWindowHostStateAboutToChange());
      return true;
    case WebOSEvent::Swap:
      if (web_app_->IsCheckLaunchTimeEnabled())
        web_app_->OnDelegateWindowFrameSwapped();
      break;
    case WebOSEvent::KeyPress:
      break;
    case WebOSEvent::KeyRelease:
      break;
    case WebOSEvent::MouseButtonPress:
      last_mouse_event_.SetType(WebOSEvent::MouseButtonPress);
      last_mouse_event_.SetFlags(event->GetFlags());
      return OnCursorVisibileChangeEvent(event);
    case WebOSEvent::MouseButtonRelease:
      last_mouse_event_.SetType(WebOSEvent::MouseButtonRelease);
    case WebOSEvent::MouseMove:
      return OnCursorVisibileChangeEvent(event);
    case WebOSEvent::Wheel:
      if (!cursor_enabled_) {
        // if magic is disabled, then all mouse event should be filtered
        // but this wheel event is not related to cursor visibility
        return true;
      }
      break;
    case WebOSEvent::Enter:
      web_app_->SendWebOSMouseEvent("Enter");
      break;
    case WebOSEvent::Leave:
      web_app_->SendWebOSMouseEvent("Leave");
      break;
    case WebOSEvent::FocusIn:
      web_app_->Focus();
      LOG_INFO_WITH_CLOCK(
          MSGID_WINDOW_FOCUSIN, 4, PMLOGKS("PerfType", "AppLaunch"),
          PMLOGKS("PerfGroup", web_app_->AppId().c_str()),
          PMLOGKS("APP_ID", web_app_->AppId().c_str()),
          PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()), "");
      break;
    case WebOSEvent::FocusOut:
      LOG_INFO(MSGID_WINDOW_FOCUSOUT, 2,
               PMLOGKS("APP_ID", web_app_->AppId().c_str()),
               PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()),
               "last_mouse_event_'s type : %s",
               last_mouse_event_.GetType() == WebOSEvent::MouseButtonPress
                   ? "Press; Generate MouseButtonRelease event"
                   : "Release");

      // Cherry-pick http://wall.lge.com:8110/#/c/89417/ partially.
      // The FocusAboutToChange event is specific to Qt and it is for the
      // lost of keyboard focus. So, it could be handled in the same way
      // by using FocusOut.
      if (last_mouse_event_.GetType() == WebOSEvent::MouseButtonPress) {
        last_mouse_event_.SetType(WebOSEvent::MouseButtonRelease);
        web_app_->ForwardWebOSEvent(&last_mouse_event_);
      }

      web_app_->Unfocus();
      break;
    case WebOSEvent::InputPanelVisible: {
      float height =
          static_cast<WebOSVirtualKeyboardEvent*>(event)->GetHeight();
      if (static_cast<WebOSVirtualKeyboardEvent*>(event)->GetVisible()) {
        web_app_->KeyboardVisibilityChanged(true, height);
      } else {
        web_app_->KeyboardVisibilityChanged(false, height);
      }
    } break;
    default:
      break;
  }

  return WebAppWindowDelegate::event(event);
}

bool WebAppWaylandWindow::OnCursorVisibileChangeEvent(WebOSEvent* e) {
  if (!cursor_enabled_) {
    if (CursorVisible())
      SetCursorVisible(false);
    return true;
  }

  // This event is not handled, so keep the event being dispatched.
  return false;
}

unsigned int WebAppWaylandWindow::CheckKeyFilterTable(unsigned keycode,
                                                      unsigned* modifier) {
  auto table = web_app_->GetAppDescription()->KeyFilterTable();

  if (table.empty())
    return 0;

  auto found = table.find(keycode);
  if (found == table.end())
    return 0;

  *modifier = found->second.second;

  return found->second.first;
}

void WebAppWaylandWindow::LogEventDebugging(WebOSEvent* event) {
  if (LogManager::GetDebugMouseMoveEnabled()) {
    if (event->GetType() == WebOSEvent::MouseMove) {
      if (cursor_enabled_) {
        // log all mouse move events
        LOG_INFO(
            MSGID_MOUSE_MOVE_EVENT, 4,
            PMLOGKS("APP_ID", web_app_->AppId().c_str()),
            PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()),
            PMLOGKFV("X", "%.f", static_cast<WebOSMouseEvent*>(event)->GetX()),
            PMLOGKFV("Y", "%.f", static_cast<WebOSMouseEvent*>(event)->GetY()),
            "");
      } else {
        LOG_INFO(MSGID_MOUSE_MOVE_EVENT, 2,
                 PMLOGKS("APP_ID", web_app_->AppId().c_str()),
                 PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()),
                 "Mouse event should be Disabled by blank cursor");
      }
    }
  }

  if (LogManager::GetDebugEventsEnabled()) {
    if (event->GetType() == WebOSEvent::KeyPress ||
        event->GetType() == WebOSEvent::KeyRelease) {
      // remote key event
      LOG_INFO(MSGID_KEY_EVENT, 4, PMLOGKS("APP_ID", web_app_->AppId().c_str()),
               PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()),
               PMLOGKFV("VALUE_HEX", "%x",
                        static_cast<WebOSKeyEvent*>(event)->GetCode()),
               PMLOGKS("STATUS", event->GetType() == WebOSEvent::KeyPress
                                     ? "KeyPress"
                                     : "KeyRelease"),
               "");
    } else if (event->GetType() == WebOSEvent::MouseButtonPress ||
               event->GetType() == WebOSEvent::MouseButtonRelease) {
      if (!cursor_enabled_) {
        LOG_INFO(MSGID_MOUSE_BUTTON_EVENT, 2,
                 PMLOGKS("APP_ID", web_app_->AppId().c_str()),
                 PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()),
                 "Mouse event should be Disabled by blank cursor");
      } else {
        // mouse button event
        float scale = 1.0;
        int height = web_app_->GetAppDescription()->HeightOverride();
        if (height)
          scale = (float)DisplayHeight() /
                  web_app_->GetAppDescription()->HeightOverride();
        LOG_INFO(
            MSGID_MOUSE_BUTTON_EVENT, 6,
            PMLOGKS("APP_ID", web_app_->AppId().c_str()),
            PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()),
            PMLOGKFV("VALUE", "%d",
                     (int)static_cast<WebOSMouseEvent*>(event)->GetButton()),
            PMLOGKS("STATUS", event->GetType() == WebOSEvent::MouseButtonPress
                                  ? "MouseButtonPress"
                                  : "MouseButtonRelease"),
            PMLOGKFV("X", "%.f",
                     static_cast<WebOSMouseEvent*>(event)->GetX() * scale),
            PMLOGKFV("Y", "%.f",
                     static_cast<WebOSMouseEvent*>(event)->GetY() * scale),
            "");
      }
    } else if (event->GetType() == WebOSEvent::InputPanelVisible) {
      LOG_INFO(MSGID_VKB_EVENT, 4, PMLOGKS("APP_ID", web_app_->AppId().c_str()),
               PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()),
               PMLOGKS("STATUS", "InputPanelVisible"),
               PMLOGKS("Visible", static_cast<WebOSVirtualKeyboardEvent*>(event)
                                              ->GetVisible() == true
                                      ? "true"
                                      : "false"),
               "");
    } else if (event->GetType() != WebOSEvent::MouseMove) {
      // log all window event except mouseMove
      // to print mouseMove event, set mouseMove : true
      LOG_INFO(MSGID_WINDOW_EVENT, 3,
               PMLOGKS("APP_ID", web_app_->AppId().c_str()),
               PMLOGKS("INSTANCE_ID", web_app_->InstanceId().c_str()),
               PMLOGKFV("TYPE", "%d", event->GetType()), "");
    }
  }
}

void WebAppWaylandWindow::SendKeyCode(int key_code) {
  if (!xinput_activated_) {
    XInputActivate();
    xinput_activated_ = true;
  }
  XInputInvokeAction(key_code);
}
