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

#ifndef WEBAPPWAYLANDWINDOW_H
#define WEBAPPWAYLANDWINDOW_H

#include "webos/webapp_window_base.h"

class WebAppWayland;

class WebAppWaylandWindow : public webos::WebAppWindowBase {
public:
    WebAppWaylandWindow();
    virtual ~WebAppWaylandWindow() {}
    static WebAppWaylandWindow* take();
    static void prepareRenderingContext();
    static void prepare();

    inline const WebAppWayland* webApp() const { return m_webApp; }
    inline void setWebApp(WebAppWayland* w) { m_webApp = w; }

    virtual void hide();
    virtual void show();

    void platformBack();
    void setCursor(const QString& cursorArg, int hotspot_x = -1, int hotspot_y = -1);

    void attachWebContents(void* webContents);

    void sendKeyCode(int keyCode);

    // webos::WebAppWindowBase
    bool event(WebOSEvent* event) override;
    unsigned int CheckKeyFilterTable(unsigned keycode, unsigned* modifier) override;

protected:
    bool cursorVisible() { return m_cursorVisible; }
    void setCursorVisible(bool visible) { m_cursorVisible = visible; }
    bool cursorEnabled() { return m_cursorEnabled; }
    void setCursorEnabled(bool enabled) { m_cursorEnabled = enabled; }

    void onStageActivated();
    void onStageDeactivated();

private:
    void onWindowStateChangeEvent();
    bool onCursorVisibileChangeEvent(WebOSEvent* e);
    static WebAppWaylandWindow* createWindow();
    void logEventDebugging(WebOSEvent* event);

private:
    static WebAppWaylandWindow* s_instance;

    bool m_cursorEnabled;

    WebAppWayland* m_webApp;
    bool m_cursorVisible;
    bool m_xinputActivated;

    WebOSMouseEvent m_lastMouseEvent;
};

#endif
