// Copyright (c) 2016-2021 LG Electronics, Inc.
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

#ifndef CORE_WEB_PAGE_OBSERVER_H_
#define CORE_WEB_PAGE_OBSERVER_H_

class WebPageBase;

class WebPageObserver {
 public:
  virtual void CloseCallbackExecuted() {}
  virtual void ClosingAppProcessDidCrashed() {}
  virtual void DidDispatchUnload() {}
  virtual void FirstFrameVisuallyCommitted() {}
  virtual void NavigationHistoryChanged() {}
  virtual void TimeoutExecuteCloseCallback() {}
  virtual void TitleChanged() {}
  virtual void WebPageClosePageRequested() {}
  virtual void WebPageLoadFailed(int errorCode) {}
  virtual void WebPageLoadFinished() {}
  virtual void WebViewRecreated() {}

 protected:
  WebPageObserver(WebPageBase* page);
  WebPageObserver();

  virtual ~WebPageObserver() {}

  void Observe(WebPageBase* page);
  void Unobserve(WebPageBase* page);

 private:
  WebPageBase* page_;
};

#endif  // CORE_WEB_PAGE_OBSERVER_H_
