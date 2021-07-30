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

#include "web_view_mock_impl.h"

#include <gmock/gmock.h>

void WebViewMockImpl::SetOnInitActions() {
  ON_CALL(*this, SetDelegate(testing::_))
      .WillByDefault(testing::Invoke([&](WebPageBlinkDelegate* delegate) {
        web_wiew_delegate_ = delegate;
      }));
}

void WebViewMockImpl::SetOnLoadURLActions() {
  ON_CALL(*this, GetUrl()).WillByDefault(testing::ReturnRef(view_url_));
  ON_CALL(*this, LoadUrl(testing::_))
      .WillByDefault(testing::Invoke([&](const std::string& url) {
        view_url_ = url;
        if (!web_wiew_delegate_) {
          return;
        }
        web_wiew_delegate_->LoadStarted();
        web_wiew_delegate_->LoadProgressChanged(100.0);
        web_wiew_delegate_->LoadVisuallyCommitted();
        web_wiew_delegate_->LoadFinished(url);
      }));
}

WebPageBlinkDelegate* WebViewMockImpl::GetWebViewDelegate() {
  return web_wiew_delegate_;
};
