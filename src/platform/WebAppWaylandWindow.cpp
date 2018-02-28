// Copyright (c) 2008-2018 LG Electronics, Inc.
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

#include "ApplicationDescription.h"
#include "LogManager.h"
#include "WebAppWayland.h"
#include "WebAppWaylandWindow.h"

WebAppWaylandWindow* WebAppWaylandWindow::s_instance = 0;

WebAppWaylandWindow* WebAppWaylandWindow::take()
{
    WebAppWaylandWindow* window;

    if (!s_instance)
        s_instance = new WebAppWaylandWindow();

    if (!s_instance) {
        LOG_CRITICAL(MSGID_TAKE_FAIL, 0, "Failed to take WebAppWaylandWindow");
        return NULL;
    }

    window = s_instance;
    s_instance = NULL;
    return window;
}

void WebAppWaylandWindow::prepareRenderingContext()
{
    /* This shouldn't be destroyed */
    (void*)createWindow();
}

void WebAppWaylandWindow::prepare()
{
    if (s_instance)
        return;

    // TODO: Need to make sure preparing window is helpful
    WebAppWaylandWindow* window = createWindow();
    if (!window)
        return;

    s_instance = window;
}

WebAppWaylandWindow* WebAppWaylandWindow::createWindow() {
    WebAppWaylandWindow *window = new WebAppWaylandWindow();
    if (!window) {
        LOG_CRITICAL(MSGID_PREPARE_FAIL, 0, "Failed to prepare WindowedWebAppWindow");
        return 0;
    }
    window->Resize(1,1);
    return window;
}

WebAppWaylandWindow::WebAppWaylandWindow()
    : m_webApp(0)
    , m_cursorVisible(false)
    , m_xinputActivated(false)
    , m_lastMouseEvent(WebOSMouseEvent(WebOSEvent::None, -1., -1.))
{
    m_cursorEnabled = (qgetenv("ENABLE_CURSOR_BY_DEFAULT") == "1") ? true : false;;
}

void WebAppWaylandWindow::hide()
{
    LOG_INFO(MSGID_WAM_DEBUG, 1, PMLOGKS("APP_ID", qPrintable(m_webApp->appId())), "WebAppWaylandWindow::hide(); call onStageDeactivated");
    onStageDeactivated();
    WebAppWindowBase::Hide();
}

void WebAppWaylandWindow::show()
{
    LOG_INFO(MSGID_WAM_DEBUG, 1, PMLOGKS("APP_ID", qPrintable(m_webApp->appId())), "WebAppWaylandWindow::show(); call onStageActivated");
    onStageActivated();
    WebAppWindowBase::Show();
}

void WebAppWaylandWindow::platformBack()
{
    LOG_INFO(MSGID_WAM_DEBUG, 1, PMLOGKS("APP_ID", qPrintable(m_webApp->appId())), "WebAppWaylandWindow::platformBack(); generate RECENT key");
}

void WebAppWaylandWindow::setCursor(const QString & cursorArg, int hotspot_x, int hotspot_y)
{
    webos::CustomCursorType type = webos::CUSTOM_CURSOR_NOT_USE;
    if (cursorArg.isEmpty() || !cursorArg.compare("default"))
        LOG_DEBUG("[%s] %s; arg: %s; Restore Cursor to webos::CUSTOM_CURSOR_NOT_USE", qPrintable(m_webApp->appId()), __PRETTY_FUNCTION__, cursorArg.toUtf8().data());
    else if (!cursorArg.compare("blank")) {
        LOG_DEBUG("[%s] %s; arg: %s; Set Cursor to webos::CUSTOM_CURSOR_BLANK", qPrintable(m_webApp->appId()), __PRETTY_FUNCTION__, cursorArg.toUtf8().data());
        type = webos::CUSTOM_CURSOR_BLANK;
    } else {
        LOG_DEBUG("[%s] %s; Custom Cursor file path : %s, hotspot_x : %d, hotspot_y : %d", __PRETTY_FUNCTION__, qPrintable(m_webApp->appId()), cursorArg.toUtf8().data(), hotspot_x, hotspot_y);
        type = webos::CUSTOM_CURSOR_PATH;
    }

    SetCustomCursor(type, cursorArg.toUtf8().data(), hotspot_x, hotspot_y);

    if (type == webos::CUSTOM_CURSOR_BLANK)
        m_cursorEnabled = false;
    else
        m_cursorEnabled = true; // all mouse event will be filtered
}

void WebAppWaylandWindow::attachWebContents(void* webContents)
{
    WebAppWindowBase::AttachWebContents(webContents);
}

bool WebAppWaylandWindow::event(WebOSEvent* event)
{
    if (!m_webApp)
        return true;

    logEventDebugging(event);

    // TODO: Implement each event handler and
    // remove above event() function used for qtwebengine.
    switch (event->GetType())
    {
        case WebOSEvent::Close:
            LOG_INFO(MSGID_WINDOW_CLOSED, 1, PMLOGKS("APP_ID", qPrintable(m_webApp->appId())), "");
            m_webApp->doClose();
            return true;
        case WebOSEvent::WindowStateChange:
            if (GetWindowHostState() == webos::NATIVE_WINDOW_MINIMIZED) {
                LOG_INFO(MSGID_WINDOW_STATECHANGE, 1, PMLOGKS("APP_ID", qPrintable(m_webApp->appId())), "WebOSEvent::WindowStateChange; Minimize; m_lastMouseEvent's type : %s", m_lastMouseEvent.GetType() == WebOSEvent::MouseButtonPress ? "Press; Generate MouseButtonRelease event" : "Release");
                if (m_lastMouseEvent.GetType() == WebOSEvent::MouseButtonPress) {
                    m_lastMouseEvent.SetType(WebOSEvent::MouseButtonRelease);
                    m_webApp->forwardWebOSEvent(&m_lastMouseEvent);
                }
            }
            onWindowStateChangeEvent();
            break;
        case WebOSEvent::WindowStateAboutToChange:
            m_webApp->stateAboutToChange(GetWindowHostStateAboutToChange());
            return true;
        case WebOSEvent::Swap:
            if (m_webApp->isCheckLaunchTimeEnabled())
                m_webApp->onDelegateWindowFrameSwapped();
            break;
        case WebOSEvent::KeyPress:
            break;
        case WebOSEvent::KeyRelease:
            break;
        case WebOSEvent::MouseButtonPress:
            m_lastMouseEvent.SetType(WebOSEvent::MouseButtonPress);
            m_lastMouseEvent.SetFlags(event->GetFlags());
            return onCursorVisibileChangeEvent(event);
        case WebOSEvent::MouseButtonRelease:
            m_lastMouseEvent.SetType(WebOSEvent::MouseButtonRelease);
        case WebOSEvent::MouseMove:
            return onCursorVisibileChangeEvent(event);
        case WebOSEvent::Wheel:
            if (!m_cursorEnabled) {
                // if magic is disabled, then all mouse event should be filtered
                // but this wheel event is not related to cursor visibility
                return true;
            }
            break;
        case WebOSEvent::Enter:
            m_webApp->sendWebOSMouseEvent("Enter");
            break;
        case WebOSEvent::Leave:
            m_webApp->sendWebOSMouseEvent("Leave");
            break;
        case WebOSEvent::FocusIn:
            m_webApp->focus();
            LOG_INFO_WITH_CLOCK(MSGID_WINDOW_FOCUSIN, 3,
                    PMLOGKS("PerfType", "AppLaunch"),
                    PMLOGKS("PerfGroup", qPrintable(m_webApp->appId())),
                    PMLOGKS("APP_ID", qPrintable(m_webApp->appId())), "");
            break;
        case WebOSEvent::FocusOut:
            LOG_INFO(MSGID_WINDOW_FOCUSOUT, 1, PMLOGKS("APP_ID", qPrintable(m_webApp->appId())), "m_lastMouseEvent's type : %s", m_lastMouseEvent.GetType() == WebOSEvent::MouseButtonPress ? "Press; Generate MouseButtonRelease event" : "Release");

            // Cherry-pick http://wall.lge.com:8110/#/c/89417/ partially.
            // The FocusAboutToChange event is specific to Qt and it is for the
            // lost of keyboard focus. So, it could be handled in the same way
            // by using FocusOut.
            if (m_lastMouseEvent.GetType() == WebOSEvent::MouseButtonPress) {
                m_lastMouseEvent.SetType(WebOSEvent::MouseButtonRelease);
                m_webApp->forwardWebOSEvent(&m_lastMouseEvent);
            }

            m_webApp->unfocus();
            break;
        case WebOSEvent::InputPanelVisible:
            {
                float height = static_cast<WebOSVirtualKeyboardEvent*>(event)->GetHeight();
                if (static_cast<WebOSVirtualKeyboardEvent*>(event)->GetVisible()) {
                    m_webApp->keyboardVisibilityChanged(true, height);
                } else {
                    m_webApp->keyboardVisibilityChanged(false, height);
                }
            }
            break;
        default:
            break;
    }

    return WebAppWindowDelegate::event(event);
}

void WebAppWaylandWindow::onStageActivated()
{
    if (!m_webApp)
        return;

    m_webApp->onStageActivated();
}

void WebAppWaylandWindow::onStageDeactivated()
{
    if (!m_webApp)
        return;

    m_webApp->onStageDeactivated();
}

void WebAppWaylandWindow::onWindowStateChangeEvent()
{
    if (m_webApp->isClosing()) {
        LOG_INFO(MSGID_WINDOW_STATE_CHANGED, 1, PMLOGKS("APP_ID", qPrintable(m_webApp->appId())), "In Closing; return;");
        return;
    }

    webos::NativeWindowState state = GetWindowHostState();
    switch (state)
    {
        case webos::NATIVE_WINDOW_DEFAULT:
        case webos::NATIVE_WINDOW_MAXIMIZED:
        case webos::NATIVE_WINDOW_FULLSCREEN:
            LOG_INFO(MSGID_WINDOW_STATE_CHANGED, 1, PMLOGKS("APP_ID", qPrintable(m_webApp->appId())), "To FullScreen; call onStageActivated");
            m_webApp->applyInputRegion();
            onStageActivated();
            break;
        case webos::NATIVE_WINDOW_MINIMIZED:
            LOG_INFO(MSGID_WINDOW_STATE_CHANGED, 1, PMLOGKS("APP_ID", qPrintable(m_webApp->appId())), "To Minimized; call onStageDeactivated");
            onStageDeactivated();
            break;
        default:
            LOG_INFO(MSGID_WINDOW_STATE_CHANGED, 2, PMLOGKS("APP_ID", qPrintable(m_webApp->appId())), PMLOGKFV("HOST_STATE", "%d", state), "Unknown state. Do not calling nothing anymore.");
            break;
    }
}

bool WebAppWaylandWindow::onCursorVisibileChangeEvent(WebOSEvent* e)
{
    if (!m_cursorEnabled) {
        if (cursorVisible())
            setCursorVisible(false);
        return true;
    }

    // This event is not handled, so keep the event being dispatched.
    return false;
}

unsigned int WebAppWaylandWindow::CheckKeyFilterTable(unsigned keycode, unsigned* modifier)
{
    QMap<int, QPair<int, int>> table = m_webApp->getAppDescription()->keyFilterTable();

    if (table.empty())
        return 0;

    if (!table.contains(keycode))
        return 0;

    *modifier = table[keycode].second;

    return table[keycode].first;
}

void WebAppWaylandWindow::logEventDebugging(WebOSEvent* event)
{
    if (LogManager::getDebugMouseMoveEnabled()) {
       if (event->GetType() == WebOSEvent::MouseMove) {
           if (m_cursorEnabled) {
               // log all mouse move events
               LOG_INFO(MSGID_MOUSE_MOVE_EVENT, 3,
                    PMLOGKS("APP_ID", qPrintable(m_webApp->appId())),
                    PMLOGKFV("X", "%.f", static_cast<WebOSMouseEvent*>(event)->GetX()),
                    PMLOGKFV("Y", "%.f", static_cast<WebOSMouseEvent*>(event)->GetY()), "");
            }
            else {
                LOG_INFO(MSGID_MOUSE_MOVE_EVENT, 1,
                    PMLOGKS("APP_ID", qPrintable(m_webApp->appId())),
                    "Mouse event should be Disabled by blank cursor");
            }
        }
    }

    if (LogManager::getDebugEventsEnabled()) {
        if (event->GetType() == WebOSEvent::KeyPress || event->GetType() == WebOSEvent::KeyRelease) {
            // remote key event
            LOG_INFO(MSGID_KEY_EVENT, 3,
                PMLOGKS("APP_ID", qPrintable(m_webApp->appId())),
                PMLOGKFV("VALUE_HEX", "%x", static_cast<WebOSKeyEvent*>(event)->GetCode()),
                PMLOGKS("STATUS", event->GetType() == WebOSEvent::KeyPress ? "KeyPress" : "KeyRelease"), "");
        }
        else if (event->GetType() == WebOSEvent::MouseButtonPress || event->GetType() == WebOSEvent::MouseButtonRelease) {
            if (!m_cursorEnabled) {
                LOG_INFO(MSGID_MOUSE_BUTTON_EVENT, 1,
                    PMLOGKS("APP_ID", qPrintable(m_webApp->appId())),
                    "Mouse event should be Disabled by blank cursor");
            }
            else {
                // mouse button event
                LOG_INFO(MSGID_MOUSE_BUTTON_EVENT, 3,
                    PMLOGKS("APP_ID", qPrintable(m_webApp->appId())),
                    PMLOGKFV("VALUE", "%d", (int)static_cast<WebOSMouseEvent*>(event)->GetButton()),
                    PMLOGKS("STATUS", event->GetType() == WebOSEvent::MouseButtonPress ? "MouseButtonPress" : "MouseButtonRelease"), "");
            }
        }
        else if (event->GetType() != WebOSEvent::MouseMove) {
            // log all window event except mouseMove
            // to print mouseMove event, set mouseMove : true
            LOG_INFO(MSGID_WINDOW_EVENT, 2,
                PMLOGKS("APP_ID", qPrintable(m_webApp->appId())),
                PMLOGKFV("TYPE", "%d", event->GetType()), "");
        }
    }
}

void WebAppWaylandWindow::sendKeyCode(int keyCode)
{
    if (!m_xinputActivated) {
        XInputActivate();
        m_xinputActivated = true;
    }
    XInputInvokeAction(keyCode);
}
