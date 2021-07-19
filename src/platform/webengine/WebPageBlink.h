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

#ifndef WEBPAGEBLINK_H
#define WEBPAGEBLINK_H

#include <memory>
#include <string>
#include <vector>

#include "Timer.h"
#include "WebPageBase.h"
#include "WebPageBlinkDelegate.h"

#include "webos/webview_base.h"

namespace wam {
    class Url;
}

class WebAppBase;
class WebView;
class WebPageBlinkPrivate;
class WebPageBlinkObserver;
class WebViewFactory;

class WebPageBlink : public WebPageBase, public WebPageBlinkDelegate {
public:
    WebPageBlink(const wam::Url& url,
                 std::shared_ptr<ApplicationDescription> desc,
                 const std::string& launchParams,
                 std::unique_ptr<WebViewFactory> factory);
    WebPageBlink(const wam::Url& url, std::shared_ptr<ApplicationDescription> desc, const std::string& launchParams);
    ~WebPageBlink() override;

    void setObserver(WebPageBlinkObserver* observer);

    // WebPageBase
    void init() override;
    void* getWebContents() override;
    void setLaunchParams(const std::string& params) override;
    void notifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level) override;
    wam::Url url() const override;
    void loadUrl(const std::string& url) override;
    int progress() const override;
    bool hasBeenShown() const override;
    void setPageProperties() override;
    void setPreferredLanguages(const std::string& language) override;
    void setDefaultFont(const std::string& font) override;
    void reloadDefaultPage() override;
    void reload() override;
    void setVisibilityState(WebPageVisibilityState visibilityState) override;
    void setFocus(bool focus) override;
    std::string title() override;
    bool canGoBack() override;
    void closeVkb() override;
    bool isInputMethodActive() const override;
    void keyboardVisibilityChanged(bool visible) override;
    void handleDeviceInfoChanged(const std::string& deviceInfo) override;
    void evaluateJavaScript(const std::string& jsCode) override;
    void evaluateJavaScriptInAllFrames(const std::string& jsCode, const char* method = "") override;
    uint32_t getWebProcessProxyID() override;
    uint32_t getWebProcessPID() const override { return renderProcessPid(); }
    void createPalmSystem(WebAppBase* app) override;
    void setUseLaunchOptimization(bool enabled, int delayMs = 0);
    void setUseSystemAppOptimization(bool enabled) override;
    void setUseAccessibility(bool enabled) override;
    void setAppPreloadHint(bool is_preload) override;
    void suspendWebPageAll() override;
    void resumeWebPageAll() override;
    void suspendWebPageMedia() override;
    void resumeWebPageMedia() override;
    void resumeWebPagePaintingAndJSExecution() override;
    bool isRegisteredCloseCallback() override { return m_hasCloseCallback; }
    void reloadExtensionData() override;
    void updateIsLoadErrorPageFinish() override;
    void updateDatabaseIdentifier() override;
    void deleteWebStorages(const std::string& identfier) override;
    void setInspectorEnable() override;
    void setKeepAliveWebApp(bool keepAlive) override;

    // WebPageBlink
    virtual void loadExtension();
    virtual void clearExtensions();
    virtual void setViewportSize();
    virtual void setHasOnCloseCallback(bool hasCloseCallback);
    virtual void executeCloseCallback(bool forced);
    virtual void didRunCloseCallback();
    virtual void cleanResources();

    // WebPageBlinkDelegate
    void close() override;
    bool decidePolicyForResponse(bool isMainFrame, int statusCode,
        const std::string& url, const std::string& statusText) override;
    bool acceptsVideoCapture() override;
    bool acceptsAudioCapture() override;
    void didFirstFrameFocused() override;
    void didDropAllPeerConnections() override;
    void loadFinished(const std::string& url) override;
    void loadFailed(const std::string& url, int errCode, const std::string& errDesc) override;
    void loadStopped() override;
    void loadAborted(const std::string& url) override;
    void loadProgressChanged(double progress) override;
    void didStartNavigation(const std::string& url, bool isInMainFrame) override;
    void didFinishNavigation(const std::string& url, bool isInMainFrame) override;
    void loadVisuallyCommitted() override;
    void renderProcessCreated(int pid) override;
    void renderProcessCrashed() override;
    void titleChanged(const std::string& title) override;
    void navigationHistoryChanged() override;
    void didErrorPageLoadedFromNetErrorHelper() override;

    void updateExtensionData(const std::string& key, const std::string& value);
    void setLoadErrorPolicy(const std::string& policy);
    void setTrustLevel(const std::string& trustLevel) { m_trustLevel = trustLevel; }
    std::string trustLevel() const { return m_trustLevel; }
    std::string defaultTrustLevel() const;
    std::string escapeData(const std::string& value);
    int renderProcessPid() const;
    static void setFileAccessBlocked(bool blocked);
    void updateBoardType();
    void updateMediaCodecCapability();
    double devicePixelRatio();
    void setAdditionalContentsScale(float scaleX, float scaleY);
    void setSupportDolbyHDRContents();
    void updateHardwareResolution();

    void forwardEvent(void* event) override;
    bool allowMouseOnOffEvent() const override;
    void didSwapCompositorFrame();
    void activateRendererCompositor() override;
    void deactivateRendererCompositor() override;

    void didResumeDOM() override;

    // Timer callback
    void timeoutCloseCallback();

    void setAudioGuidanceOn(bool on) override;
    void updateBackHistoryAPIDisabled();

protected:
    WebView* pageView() const;

    // WebPageBase
    virtual void loadDefaultUrl();
    virtual void loadErrorPage(int errorCode);
    virtual WebView* createPageView();
    virtual void setupStaticUserScripts();
    virtual void addUserScript(const std::string& script);
    virtual void addUserScriptUrl(const wam::Url& url);
    virtual void recreateWebView();
    virtual void setVisible(bool visible);
    virtual bool shouldStopJSOnSuspend() const { return true; }

    bool inspectable();

    // WebPageDelegate
    void handleBrowserControlCommand(const std::string& command, const std::vector<std::string>& arguments) override;
    void handleBrowserControlFunction(const std::string& command, const std::vector<std::string>& arguments, std::string* result) override;

    std::string handleBrowserControlMessage(const std::string& command, const std::vector<std::string>& arguments);

    void suspendWebPagePaintingAndJSExecution() override;

private:
    void setCustomPluginIfNeeded();
    void setDisallowScrolling(bool disallow);
    std::vector<std::string> getErrorPagePath(const std::string& errorpage);

private:
    WebPageBlinkPrivate* d;

    bool m_isPaused;
    bool m_isSuspended;
    bool m_hasCustomPolicyForResponse;
    bool m_hasBeenShown;
    OneShotTimer<WebPageBlink> m_domSuspendTimer;
    std::string m_customPluginPath;
    double m_vkbHeight;
    bool m_vkbWasOverlap;
    bool m_hasCloseCallback;
    OneShotTimer<WebPageBlink> m_closeCallbackTimer;
    std::string m_trustLevel;
    std::string m_loadFailedHostname;
    std::string m_loadingUrl;
    int m_customSuspendDOMTime;

    WebPageBlinkObserver *m_observer;

    std::unique_ptr<WebViewFactory> m_factory;
};

#endif /* WEBPAGEBLINK_H */
