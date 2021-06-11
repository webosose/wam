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

#ifndef WEBVIEWMOCKIMPL_H
#define WEBVIEWMOCKIMPL_H

#include <string>

#include "WebPageBlinkDelegate.h"
#include "WebViewMock.h"

class WebViewMockImpl : public WebViewMock {
 public:
  WebViewMockImpl() = default;
  ~WebViewMockImpl() override = default;
  void SetOnInitActions();
  void SetOnLoadURLActions();
  WebPageBlinkDelegate* GetWebViewDelegate();

 private:
  WebPageBlinkDelegate* web_wiew_delegate_ = nullptr;
  std::string view_url_;
};

using NiceWebViewMockImpl = testing::NiceMock<WebViewMockImpl>;
using StrictWebViewMockImpl = testing::StrictMock<WebViewMockImpl>;

#endif  // WEBVIEWMOCKIMPL_H
