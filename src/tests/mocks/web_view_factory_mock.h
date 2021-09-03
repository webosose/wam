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

#ifndef TESTS_MOCKS_WEB_VIEW_FACTORY_MOCK_H_
#define TESTS_MOCKS_WEB_VIEW_FACTORY_MOCK_H_

#include "web_view_factory.h"

class WebView;

class WebViewFactoryMock : public WebViewFactory {
 public:
  WebViewFactoryMock();
  ~WebViewFactoryMock() override;

  WebView* CreateWebView() override;
  void SetWebView(WebView* web_view);

 private:
  WebView* web_view_;
};

#endif  // TESTS_MOCKS_WEB_VIEW_FACTORY_MOCK_H_
