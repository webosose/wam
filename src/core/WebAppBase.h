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

#ifndef WEBAPPBASE_H
#define WEBAPPBASE_H

#include <memory>
#include <string>

#include "WebAppManager.h"
#include "WebPageObserver.h"

class ApplicationDescription;
class WebAppBasePrivate;
class WebPageBase;

namespace Json {
class Value;
};

class WebAppBase : public WebPageObserver {
public:
    enum PreloadState {
        NONE_PRELOAD = 0,
        FULL_PRELOAD = 1,
        SEMI_FULL_PRELOAD = 2,
        PARTIAL_PRELOAD = 3,
        MINIMAL_PRELOAD = 4
    };

    WebAppBase();
    ~WebAppBase() override;

    virtual void attach(WebPageBase*);
    virtual WebPageBase* detach();
    virtual void suspendAppRendering() = 0;
    virtual void resumeAppRendering() = 0;
    virtual bool isFocused() const = 0;
    virtual void resize(int width, int height) = 0;
    virtual bool isActivated() const = 0;
    virtual bool isMinimized() = 0;
    virtual bool isNormal() = 0;
    virtual void onStageActivated() = 0;
    virtual void onStageDeactivated() = 0;
    virtual void startLaunchTimer() {}
    virtual void setHiddenWindow(bool hidden);
    virtual void configureWindow(const std::string& type) = 0;
    virtual void setKeepAlive(bool keepAlive);
    virtual bool isWindowed() const;
    virtual void relaunch(const std::string& args, const std::string& launchingAppId);
    virtual void setWindowProperty(const std::string& name, const std::string& value) = 0;
    virtual void platformBack() = 0;
    virtual void setCursor(const std::string& cursorArg, int hotspot_x, int hotspot_y) = 0;
    virtual void setInputRegion(const Json::Value& jsonDoc) = 0;
    virtual void setKeyMask(const Json::Value& jsonDoc) = 0;
    virtual void hide(bool forcedHide = false) = 0;
    virtual void focus() = 0;
    virtual void unfocus() = 0;
    virtual void setOpacity(float opacity) = 0;
    virtual void setAppDescription(std::shared_ptr<ApplicationDescription>);
    virtual void setPreferredLanguages(const std::string& language);
    virtual void stagePreparing();
    virtual void stageReady();
    virtual void raise() = 0;
    virtual void goBackground() = 0;
    virtual void doPendingRelaunch();
    virtual void deleteSurfaceGroup() = 0;
    virtual void keyboardVisibilityChanged(bool visible, int height);
    virtual void doClose() = 0;
    virtual void setUseVirtualKeyboard(const bool enable) = 0;
    virtual bool isKeyboardVisible() { return false; }
    static void onCursorVisibilityChanged(const std::string& jsscript);
    virtual bool hideWindow() = 0;

    bool getCrashState();
    void setCrashState(bool state);
    bool getHiddenWindow();
    bool keepAlive() const;
    void setForceClose();
    bool forceClose();
    WebPageBase* page() const;
    void handleWebAppMessage(WebAppManager::WebAppMessageType type, const std::string& message);
    void setAppId(const std::string& appId);
    void setLaunchingAppId(const std::string& appId);
    std::string appId() const;
    std::string launchingAppId() const;
    void setInstanceId(const std::string& instanceId);
    std::string instanceId() const;
    std::string url() const;

    ApplicationDescription* getAppDescription() const;

    void setAppProperties(const std::string& properties);

    void setNeedReload(bool status) { m_needReload = status; }
    bool needReload() { return m_needReload; }

    static int currentUiWidth();
    static int currentUiHeight();

    void cleanResources();
    void executeCloseCallback();
    void dispatchUnload();

    void setUseAccessibility(bool enabled);
    void serviceCall(const std::string& url, const std::string& payload, const std::string& appId);

    void setPreloadState(const std::string& properties);
    void clearPreloadState();
    PreloadState preloadState() { return m_preloadState; }

    bool isClosing() const;
    bool isCheckLaunchTimeEnabled();
    void setClosePageRequested(bool requested) { m_closePageRequested = requested; }
    bool closePageRequested() { return m_closePageRequested; }

protected:
    virtual void doAttach() = 0;
    virtual void showWindow();

    void setUiSize(int width, int height);
    void setActiveInstanceId(const std::string& id);
    void forceCloseAppInternal();
    void closeAppInternal();
    void closeWebApp();

    // WebPageObserver
    void closeCallbackExecuted() override;
    void closingAppProcessDidCrashed() override;
    void didDispatchUnload() override;
    void timeoutExecuteCloseCallback() override;
    void webPageClosePageRequested() override;
    void webPageLoadFinished() override;

protected:
    PreloadState m_preloadState;
    bool m_addedToWindowMgr;
    std::string m_inProgressRelaunchParams;
    std::string m_inProgressRelaunchLaunchingAppId;
    float m_scaleFactor;

private:
    WebAppBasePrivate* d;
    bool m_needReload;
    bool m_crashed;
    bool m_hiddenWindow;
    bool m_closePageRequested; // window.close() is called once then have to drop further requests
};
#endif // WEBAPPBASE_H
