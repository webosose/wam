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

#ifndef WEBPAGEBLINK_H
#define WEBPAGEBLINK_H

#include <QtCore/QUrl>

#include "Timer.h"
#include "WebPageBase.h"
#include "WebPageBlinkDelegate.h"

#include "webos/webview_base.h"

class WebAppBase;
class BlinkWebView;
class WebPageBlinkPrivate;

class WebPageBlink : public WebPageBase, public WebPageBlinkDelegate {
    Q_OBJECT
public:
    enum FontRenderParams {
        HINTING_NONE = 0,
        HINTING_SLIGHT = 1,
        HINTING_MEDIUM = 2,
        HINTING_FULL = 3
    };

    WebPageBlink(const QUrl& url, ApplicationDescription* desc, const QString& launchParams);
    ~WebPageBlink() override;

    // WebPageBase
    void init() override;
    void* getWebContents() override;
    void setLaunchParams(const QString& params) override;
    void notifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level) override;
    QUrl url() const override;
    void replaceBaseUrl(QUrl newUrl) override;
    void loadUrl(const std::string& url) override;
    int progress() const override;
    bool hasBeenShown() const override;
    void setPageProperties() override;
    void setPreferredLanguages(const QString& language) override;
    void setDefaultFont(const QString& font) override;
    void reloadDefaultPage() override;
    void reload() override;
    void setVisibilityState(WebPageVisibilityState visibilityState) override;
    void setFocus(bool focus) override;
    QString title() override;
    bool canGoBack() override;
    void closeVkb() override;
    bool isKeyboardVisible() const override;
    bool isInputMethodActive() const override;
    void keyboardVisibilityChanged(bool visible) override;
    void updatePageSettings() override;
    void handleDeviceInfoChanged(const QString& deviceInfo) override;
    void evaluateJavaScript(const QString& jsCode) override;
    void evaluateJavaScriptInAllFrames(const QString& jsCode, const char* method = "") override;
    void setForceActivateVtg(bool enabled) override;
    uint32_t getWebProcessProxyID() override;
    uint32_t getWebProcessPID() const override { return renderProcessPid(); }
    void createPalmSystem(WebAppBase* app) override;
    void setUseLaunchOptimization(bool enabled) override;
    void setUseSystemAppOptimization(bool enabled) override;
    void setUseAccessibility(bool enabled) override;
    void setBlockWriteDiskcache(bool blocked) override;
    void suspendWebPageAll() override;
    void resumeWebPageAll() override;
    void suspendWebPageMedia() override;
    void resumeWebPageMedia() override;
    void resumeWebPagePaintingAndJSExecution() override;
    bool isRegisteredCloseCallback() override { return m_hasCloseCallback; }
    void reloadExtensionData() override;
    void updateIsLoadErrorPageFinish() override;
    void updateDatabaseIdentifier() override;
    void deleteWebStorages(const QString& identfier) override;
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
    void loadStopped(const std::string& url) override;
    void loadStarted() override;
    void loadVisuallyCommitted() override;
    void renderProcessCreated(int pid) override;
    void renderProcessCrashed() override;
    void titleChanged(const std::string& title) override;
    void navigationHistoryChanged() override;

    void updateExtensionData(const QString& key, const QString& value);
    void setLoadErrorPolicy(const QString& policy);
    void setTrustLevel(const QString& trustLevel) { m_trustLevel = trustLevel; }
    QString trustLevel() const { return m_trustLevel; }
    QString defaultTrustLevel() const;
    QString escapeData(const QString& value);
    int renderProcessPid() const;
    static void setFileAccessBlocked(bool blocked);
    void updateBoardType();
    void updateMediaCodecCapability();
    double devicePixelRatio();
    void setFontHinting(FontRenderParams hinting);
    void setSupportDolbyHDRContents();
    void updateHardwareResolution();

    void forwardEvent(void* event) override;
    bool allowMouseOnOffEvent() const override;

    // Timer callback
    void timeoutCloseCallback();

    void setAudioGuidanceOn(bool on) override;
    void resetStateToMarkNextPaintForContainer() override;
    void updateBackHistoryAPIDisabled();

Q_SIGNALS:
    void webPageClosePageRequested();
    void webPageTitleChanged();
    void webViewRecreated();
    void moveInputRegion(qreal);

protected:
    BlinkWebView* pageView() const;

    // WebPageBase
    virtual void loadDefaultUrl();
    virtual void loadErrorPage(int errorCode);
    virtual BlinkWebView* createPageView();
    virtual void setupStaticUserScripts();
    virtual void addUserScript(const QString& script);
    virtual void addUserScriptUrl(const QUrl& url);
    virtual void recreateWebView();
    virtual void setVisible(bool visible);
    virtual bool shouldStopJSOnSuspend() const { return true; }

    bool inspectable();

    // WebPageDelegate
    void handleBrowserControlCommand(const QString& command, const QStringList& arguments) override;
    void handleBrowserControlFunction(const QString& command, const QStringList& arguments, QString* result) override;

    QString handleBrowserControlMessage(const QString& message, const QStringList& params);

protected Q_SLOTS:
    virtual void didFinishLaunchingSlot();
    virtual void suspendWebPagePaintingAndJSExecution();

private:
    void setCustomPluginIfNeeded();
    void setDisallowScrolling(bool disallow);

private:
    WebPageBlinkPrivate* d;

    bool m_isPaused;
    bool m_isSuspended;
    bool m_hasCustomPolicyForResponse;
    bool m_hasBeenShown;
    OneShotTimer<WebPageBlink> m_domSuspendTimer;
    QString m_customPluginPath;
    qreal m_vkbHeight;
    bool m_vkbWasOverlap;
    bool m_hasCloseCallback;
    OneShotTimer<WebPageBlink> m_closeCallbackTimer;
    QString m_trustLevel;
    QString m_loadFailedHostname;
};

#endif /* WEBPAGEBLINK_H */
