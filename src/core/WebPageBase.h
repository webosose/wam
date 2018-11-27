// Copyright (c) 2013-2018 LG Electronics, Inc.
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

#ifndef WEBPAGEBASE_H
#define WEBPAGEBASE_H

#include <string>

#include "ObserverList.h"
#include "Url.h"

#include "webos/webview_base.h"

class ApplicationDescription;
class WebAppBase;
class WebAppManagerConfig;
class WebPageObserver;
class WebProcessManager;

/**
 * Common base class for UI independent
 * web page functionality
 */
class WebPageBase {
public:
    // Originally, webview_base.h, WebPageVisibilityState.h, PageVisibilityState.h
    // we can use enum of webview_base.h directly but this is WebPageBase in core
    // shouldn't depend on web engine
    enum WebPageVisibilityState {
        WebPageVisibilityStateVisible,
        WebPageVisibilityStateHidden,
        WebPageVisibilityStateLaunching,
        WebPageVisibilityStatePrerender,
        WebPageVisibilityStateLast = WebPageVisibilityStatePrerender
    };

    WebPageBase();
    WebPageBase(const Url& url, ApplicationDescription* desc, const std::string& params);
    virtual ~WebPageBase();

    // WebPageBase
    virtual void init() = 0;
    virtual void* getWebContents() = 0;
    virtual void setLaunchParams(const std::string& params);
    virtual void notifyMemoryPressure(webos::WebViewBase::MemoryPressureLevel level) {}
    virtual std::string getIdentifier() const;
    virtual Url url() const = 0; /* return current url */
    virtual Url defaultUrl() const { return m_defaultUrl; } /* return default url */
    virtual void setDefaultUrl(const Url& url) { m_defaultUrl = url; } /* just set default url */
    virtual void replaceBaseUrl(const Url& newUrl) = 0;
    virtual void loadUrl(const std::string& url) = 0;
    virtual int progress() const = 0;
    virtual bool hasBeenShown() const = 0;
    virtual void setPageProperties() = 0;
    virtual void setPreferredLanguages(const std::string& language) = 0;
    virtual std::string defaultFont();
    virtual void setDefaultFont(const std::string& font) = 0;
    virtual void cleanResources();
    virtual void reloadDefaultPage() = 0;
    virtual void reload() = 0;
    virtual void setVisibilityState(WebPageVisibilityState visibilityState) = 0;
    virtual void setFocus(bool focus) = 0;
    virtual std::string title() = 0;
    virtual bool canGoBack() = 0;
    virtual void closeVkb() = 0;
    virtual bool isKeyboardVisible() const { return false; }
    virtual void keyboardVisibilityChanged(bool visible) {}
    virtual void updatePageSettings() = 0;
    virtual void handleDeviceInfoChanged(const std::string& deviceInfo) = 0;
    virtual bool relaunch(const std::string& args, const std::string& launchingAppId);
    virtual void evaluateJavaScript(const std::string& jsCode) = 0;
    virtual void evaluateJavaScriptInAllFrames(const std::string& jsCode, const char* method = "") = 0;
    virtual void setForceActivateVtg(bool enabled) = 0;
    virtual uint32_t getWebProcessProxyID() = 0;
    virtual uint32_t getWebProcessPID() const = 0;
    virtual void createPalmSystem(WebAppBase* app) = 0;
    virtual void setUseLaunchOptimization(bool enabled) {}
    virtual void setUseSystemAppOptimization(bool enabled) {}
    virtual void setUseAccessibility(bool enabled) {}
    virtual void setBlockWriteDiskcache(bool blocked) {}
    virtual void suspendWebPageAll() = 0;
    virtual void resumeWebPageAll() = 0;
    virtual void suspendWebPageMedia() = 0;
    virtual void resumeWebPageMedia() = 0;
    virtual void resumeWebPagePaintingAndJSExecution() = 0;
    virtual bool isRegisteredCloseCallback() { return false; }
    virtual void executeCloseCallback(bool forced) {}
    virtual void reloadExtensionData() {}
    virtual bool isLoadErrorPageFinish() { return m_isLoadErrorPageFinish; }
    virtual bool isLoadErrorPageStart() { return m_isLoadErrorPageStart; }
    virtual void updateIsLoadErrorPageFinish();
    virtual void updateDatabaseIdentifier() {}
    virtual void deleteWebStorages(const std::string& identifier) {}
    virtual void setInspectorEnable() {}
    virtual void setKeepAliveWebApp(bool keepAlive) {}
    virtual void setContentsScale() {}
    virtual void setCustomUserScript();
    virtual void forwardEvent(void* event) = 0;
    virtual void setAudioGuidanceOn(bool on) {}
    virtual void resetStateToMarkNextPaintForContainer() {}
    virtual bool isInputMethodActive() const { return false; }

    std::string launchParams() const;
    void setApplicationDescription(ApplicationDescription* desc);
    void load();
    void setEnableBackgroundRun(bool enable) { m_enableBackgroundRun = enable; }
    void sendLocaleChangeEvent(const std::string& language);
    void setCleaningResources(bool cleaningResources) { m_cleaningResources = cleaningResources; }
    bool cleaningResources() const { return m_cleaningResources; }
    bool doHostedWebAppRelaunch(const std::string& launchParams);
    void sendRelaunchEvent();
    void setAppId(const std::string& appId) { m_appId = appId; }
    const std::string& appId() const { return m_appId; }
    ApplicationDescription* getAppDescription() { return m_appDesc; }

    void setClosing(bool status) { m_isClosing = status; }
    bool isClosing() { return m_isClosing; }

    void setIsPreload(bool isPreload) { m_isPreload = isPreload; }
    bool isPreload() const { return m_isPreload; }

    void addObserver(WebPageObserver* observer);
    void removeObserver(WebPageObserver* observer);

protected:
    // WebPageBase
    virtual void cleanResourcesFinished();
    virtual void handleForceDeleteWebPage();
    virtual void loadDefaultUrl() = 0;
    virtual void addUserScript(const std::string& script) = 0;
    virtual void addUserScriptUrl(const Url& url) = 0;
    virtual int suspendDelay();
    virtual bool hasLoadErrorPolicy(bool isHttpResponseError, int errorCode);
    virtual void loadErrorPage(int errorCode) = 0;
    virtual void recreateWebView() = 0;
    virtual void setVisible(bool visible) {}
    virtual bool doDeeplinking(const std::string& launchParams);
    virtual void suspendWebPagePaintingAndJSExecution() = 0;

    void handleLoadStarted();
    void handleLoadFinished();
    void handleLoadFailed(int errorCode);
    bool getDeviceInfo(const std::string& name, std::string& value);
    bool getSystemLanguage(std::string& value);
    int currentUiWidth();
    int currentUiHeight();
    WebProcessManager* getWebProcessManager();
    WebAppManagerConfig* getWebAppManagerConfig();
    bool processCrashed();
    std::string telluriumNubPath();

    void applyPolicyForUrlResponse(bool isMainFrame, const std::string& url, int statusCode);
    void postRunningAppList();
    void postWebProcessCreated(uint32_t pid);
    bool isAccessibilityEnabled() const;

    ApplicationDescription* m_appDesc;
    std::string m_appId;
    bool m_suspendAtLoad;
    bool m_isClosing;
    bool m_isLoadErrorPageFinish;
    bool m_isLoadErrorPageStart;
    bool m_enableBackgroundRun;
    Url m_defaultUrl;
    std::string m_launchParams;
    std::string m_loadErrorPolicy;
    ObserverList<WebPageObserver> m_observers;

private:
    void setBackgroundColorOfBody(const std::string& color);
    void setupLaunchEvent();

    bool m_cleaningResources;
    bool m_isPreload;
};

#endif // WEBPAGEBASE_H
