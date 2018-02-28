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

#ifndef BLINKWEBVIEW_H
#define BLINKWEBVIEW_H

#include "webos/webview_base.h"
#include <QString>
#include <vector>

class WebPageBlinkDelegate;

class BlinkWebView : public webos::WebViewBase {
public:
    // TODO need to refactor both constructors (here & pluggables)
    BlinkWebView(bool doInitialize = true);
    BlinkWebView(const QString& group)
        : BlinkWebView()
    {
    }

    void addUserScript(const std::string& script);
    void clearUserScripts();
    void executeUserScripts();
    void setDelegate(WebPageBlinkDelegate* delegate);
    WebPageBlinkDelegate* delegate() { return m_delegate; }
    int progress() { return m_progress; }

    // webos::WebViewBase(indirectly from webos::WebViewDelegate)
    void OnLoadProgressChanged(double progress) override;
    void DidFirstFrameFocused() override;
    void TitleChanged(const std::string& title);
    void NavigationHistoryChanged();
    void Close() override;
    bool DecidePolicyForResponse(bool isMainFrame, int statusCode,
        const std::string& url, const std::string& statusText) override;
    bool AcceptsVideoCapture() override;
    bool AcceptsAudioCapture() override;
    void LoadStarted() override;
    void LoadFinished(const std::string& url) override;
    void LoadFailed(const std::string& url, int errCode, const std::string& errDesc) override;
    void LoadStopped(const std::string& url) override;
    void DocumentLoadFinished() override;
    void RenderProcessCreated(int pid) override;
    void RenderProcessGone() override;
    void DidHistoryBackOnTopPage() {}
    void DidClearWindowObject() {}
    void DidDropAllPeerConnections(webos::DropPeerConnectionReason reason) override;
    bool AllowMouseOnOffEvent() const override;
    void HandleBrowserControlCommand(const std::string& command, const std::vector<std::string>& arguments) override;
    void HandleBrowserControlFunction(const std::string& command, const std::vector<std::string>& arguments, std::string* result) override;
    void LoadVisuallyCommitted() override;

private:
    WebPageBlinkDelegate* m_delegate;
    int m_progress;

    bool m_userScriptExecuted;
    std::vector<std::string> m_userScripts;
};

#endif
