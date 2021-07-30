//  (c) 2021 LG Electronics, Inc.
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

#ifndef TESTS_MOCKS_WEB_VIEW_MOCK_H_
#define TESTS_MOCKS_WEB_VIEW_MOCK_H_

#include <gmock/gmock.h>

#include "web_view.h"

class WebViewMock : public WebView {
 public:
  WebViewMock() = default;
  ~WebViewMock() override = default;

  MOCK_METHOD(void, AddUserScript, (const std::string&), (override));
  MOCK_METHOD(void, ClearUserScripts, (), (override));
  MOCK_METHOD(void, ExecuteUserScripts, (), (override));
  MOCK_METHOD(void, SetDelegate, (WebPageBlinkDelegate*), (override));
  MOCK_METHOD(WebPageBlinkDelegate*, Delegate, (), (override));
  MOCK_METHOD(int, Progress, (), (override));
  MOCK_METHOD(void,
              Initialize,
              (const std::string&,
               const std::string&,
               const std::string&,
               const std::string&,
               const std::string&,
               bool),
              (override));
  MOCK_METHOD(void, AddUserStyleSheet, (const std::string&), (override));
  MOCK_METHOD(std::string, DefaultUserAgent, (), (const, override));
  MOCK_METHOD(std::string, UserAgent, (), (const, override));
  MOCK_METHOD(void, LoadUrl, (const std::string&), (override));
  MOCK_METHOD(void, StopLoading, (), (override));
  MOCK_METHOD(void, LoadExtension, (const std::string&), (override));
  MOCK_METHOD(void, ClearExtensions, (), (override));
  MOCK_METHOD(void, EnableInspectablePage, (), (override));
  MOCK_METHOD(void, DisableInspectablePage, (), (override));
  MOCK_METHOD(void, SetInspectable, (bool), (override));
  MOCK_METHOD(void, AddAvailablePluginDir, (const std::string&), (override));
  MOCK_METHOD(void, AddCustomPluginDir, (const std::string&), (override));
  MOCK_METHOD(void, SetUserAgent, (const std::string&), (override));
  MOCK_METHOD(void, SetBackgroundColor, (int, int, int, int), (override));
  MOCK_METHOD(void, SetShouldSuppressDialogs, (bool), (override));
  MOCK_METHOD(void, SetUseAccessibility, (bool), (override));
  MOCK_METHOD(void, SetActiveOnNonBlankPaint, (bool), (override));
  MOCK_METHOD(void, SetViewportSize, (int, int), (override));
  MOCK_METHOD(void,
              NotifyMemoryPressure,
              (webos::WebViewBase::MemoryPressureLevel),
              (override));
  MOCK_METHOD(void, SetVisible, (bool), (override));
  MOCK_METHOD(void, SetPrerenderState, (), (override));
  MOCK_METHOD(void,
              SetVisibilityState,
              (webos::WebViewBase::WebPageVisibilityState),
              (override));
  MOCK_METHOD(void, DeleteWebStorages, (const std::string&), (override));
  MOCK_METHOD(std::string, DocumentTitle, (), (const, override));
  MOCK_METHOD(void, SuspendWebPageDOM, (), (override));
  MOCK_METHOD(void, ResumeWebPageDOM, (), (override));
  MOCK_METHOD(void, SuspendWebPageMedia, (), (override));
  MOCK_METHOD(void, ResumeWebPageMedia, (), (override));
  MOCK_METHOD(void, SuspendPaintingAndSetVisibilityHidden, (), (override));
  MOCK_METHOD(void, ResumePaintingAndSetVisibilityVisible, (), (override));
  MOCK_METHOD(void, CommitLoadVisually, (), (override));
  MOCK_METHOD(void, RunJavaScript, (const std::string&), (override));
  MOCK_METHOD(void, RunJavaScriptInAllFrames, (const std::string&), (override));
  MOCK_METHOD(void, Reload, (), (override));
  MOCK_METHOD(int, RenderProcessPid, (), (const, override));
  MOCK_METHOD(bool, IsDrmEncrypted, (const std::string&), (override));
  MOCK_METHOD(std::string, DecryptDrm, (const std::string&), (override));
  MOCK_METHOD(void, SetFocus, (bool), (override));
  MOCK_METHOD(double, GetZoomFactor, (), (override));
  MOCK_METHOD(void, SetZoomFactor, (double), (override));
  MOCK_METHOD(void, SetDoNotTrack, (bool), (override));
  MOCK_METHOD(void, ForwardWebOSEvent, (WebOSEvent*), (override));
  MOCK_METHOD(bool, CanGoBack, (), (const, override));
  MOCK_METHOD(void, GoBack, (), (override));
  MOCK_METHOD(bool, IsInputMethodActive, (), (override));
  MOCK_METHOD(void, SetAdditionalContentsScale, (float, float), (override));
  MOCK_METHOD(void, SetHardwareResolution, (int, int), (override));
  MOCK_METHOD(void, SetEnableHtmlSystemKeyboardAttr, (bool), (override));
  MOCK_METHOD(void, RequestInjectionLoading, (const std::string&), (override));
  MOCK_METHOD(void,
              DropAllPeerConnections,
              (webos::DropPeerConnectionReason),
              (override));
  MOCK_METHOD(void, ActivateRendererCompositor, (), (override));
  MOCK_METHOD(void, DeactivateRendererCompositor, (), (override));
  MOCK_METHOD(const std::string&, GetUrl, (), (override));
  MOCK_METHOD(void, UpdatePreferences, (), (override));
  MOCK_METHOD(void, ResetStateToMarkNextPaint, (), (override));
  MOCK_METHOD(void, SetAllowRunningInsecureContent, (bool), (override));
  MOCK_METHOD(void, SetAllowScriptsToCloseWindows, (bool), (override));
  MOCK_METHOD(void, SetAllowUniversalAccessFromFileUrls, (bool), (override));
  MOCK_METHOD(void, SetRequestQuotaEnabled, (bool), (override));
  MOCK_METHOD(void, SetSuppressesIncrementalRendering, (bool), (override));
  MOCK_METHOD(void, SetDisallowScrollbarsInMainFrame, (bool), (override));
  MOCK_METHOD(void, SetDisallowScrollingInMainFrame, (bool), (override));
  MOCK_METHOD(void, SetJavascriptCanOpenWindows, (bool), (override));
  MOCK_METHOD(void, SetSpatialNavigationEnabled, (bool), (override));
  MOCK_METHOD(void, SetSupportsMultipleWindows, (bool), (override));
  MOCK_METHOD(void, SetCSSNavigationEnabled, (bool), (override));
  MOCK_METHOD(void, SetV8DateUseSystemLocaloffset, (bool), (override));
  MOCK_METHOD(void, SetAllowLocalResourceLoad, (bool), (override));
  MOCK_METHOD(void, SetLocalStorageEnabled, (bool), (override));
  MOCK_METHOD(void, SetDatabaseIdentifier, (const std::string&), (override));
  MOCK_METHOD(void, SetWebSecurityEnabled, (bool), (override));
  MOCK_METHOD(void, SetKeepAliveWebApp, (bool), (override));
  MOCK_METHOD(void, SetAdditionalFontFamilyEnabled, (bool), (override));
  MOCK_METHOD(void, SetNetworkStableTimeout, (double), (override));
  MOCK_METHOD(void, SetAllowFakeBoldText, (bool), (override));
  MOCK_METHOD(void, SetAppId, (const std::string&), (override));
  MOCK_METHOD(void, SetSecurityOrigin, (const std::string&), (override));
  MOCK_METHOD(void, SetAcceptLanguages, (const std::string&), (override));
  MOCK_METHOD(void, SetBoardType, (const std::string&), (override));
  MOCK_METHOD(void, SetMediaCodecCapability, (const std::string&), (override));
  MOCK_METHOD(void, SetMediaPreferences, (const std::string&), (override));
  MOCK_METHOD(void, SetSearchKeywordForCustomPlayer, (bool), (override));
  MOCK_METHOD(void, SetUseUnlimitedMediaPolicy, (bool), (override));
  MOCK_METHOD(void, SetAudioGuidanceOn, (bool), (override));
  MOCK_METHOD(void, SetStandardFontFamily, (const std::string&), (override));
  MOCK_METHOD(void, SetFixedFontFamily, (const std::string&), (override));
  MOCK_METHOD(void, SetSerifFontFamily, (const std::string&), (override));
  MOCK_METHOD(void, SetSansSerifFontFamily, (const std::string&), (override));
  MOCK_METHOD(void, SetCursiveFontFamily, (const std::string&), (override));
  MOCK_METHOD(void, SetFantasyFontFamily, (const std::string&), (override));
  MOCK_METHOD(void,
              LoadAdditionalFont,
              (const std::string&, const std::string&),
              (override));
  MOCK_METHOD(void, SetUseLaunchOptimization, (bool, int), (override));
  MOCK_METHOD(void, SetUseEnyoOptimization, (bool), (override));
  MOCK_METHOD(void, SetAppPreloadHint, (bool), (override));
  MOCK_METHOD(void, SetTransparentBackground, (bool), (override));
  MOCK_METHOD(void, SetBackHistoryAPIDisabled, (const bool), (override));
  MOCK_METHOD(content::WebContents*, GetWebContents, (), (override));
};

using NiceWebViewMock = testing::NiceMock<WebViewMock>;
using StrictWebViewMock = testing::StrictMock<WebViewMock>;

#endif  // TESTS_MOCKS_WEB_VIEW_MOCK_H_
