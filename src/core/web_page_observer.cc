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

#include "web_page_observer.h"

#include "web_page_base.h"

WebPageObserver::WebPageObserver(WebPageBase* page) {
  Observe(page);
}

WebPageObserver::WebPageObserver() = default;

void WebPageObserver::Observe(WebPageBase* page) {
  if (page_ == page) {
    return;
  }
  page_ = page;
  if (page_) {
    page_->AddObserver(this);
  }
}

void WebPageObserver::Unobserve(WebPageBase* page) {
  if (page_ != page) {
    return;
  }
  if (page_) {
    page_->RemoveObserver(this);
  }
  page_ = nullptr;
}
