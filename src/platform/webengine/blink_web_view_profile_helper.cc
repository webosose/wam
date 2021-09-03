// Copyright (c) 2017-2021 LG Electronics, Inc.
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

#include <string.h>

#include "webos/webview_profile.h"

#include "blink_web_view_profile_helper.h"

void BlinkWebViewProfileHelper::ClearBrowsingData(
    const int remove_browsing_data_mask,
    webos::WebViewProfile* profile) {
  if (profile)
    profile->RemoveBrowsingData(remove_browsing_data_mask);
  else
    BlinkWebViewProfileHelper::ClearDefaultBrowsingData(
        remove_browsing_data_mask);
}

void BlinkWebViewProfileHelper::ClearDefaultBrowsingData(
    const int remove_browsing_data_mask) {
  webos::WebViewProfile::GetDefaultProfile()->RemoveBrowsingData(
      remove_browsing_data_mask);
}

int BlinkWebViewProfileHelper::MaskForBrowsingDataType(const char* type) {
  if (strcmp(type, browsing_data_types::kAll) == 0)
    return webos::WebViewProfile::REMOVE_ALL;
  if (strcmp(type, browsing_data_types::kAppCache) == 0)
    return webos::WebViewProfile::REMOVE_APPCACHE;
  if (strcmp(type, browsing_data_types::kCache) == 0)
    return webos::WebViewProfile::REMOVE_CACHE;
  if (strcmp(type, browsing_data_types::kCodeCache) == 0)
    return webos::WebViewProfile::REMOVE_CODE_CACHE;
  if (strcmp(type, browsing_data_types::kCookies) == 0) {
    return webos::WebViewProfile::REMOVE_COOKIES |
           webos::WebViewProfile::REMOVE_WEBRTC_IDENTITY;
  }
  if (strcmp(type, browsing_data_types::kFileSystems) == 0)
    return webos::WebViewProfile::REMOVE_FILE_SYSTEMS;
  if (strcmp(type, browsing_data_types::kIndexedDB) == 0)
    return webos::WebViewProfile::REMOVE_INDEXEDDB;
  if (strcmp(type, browsing_data_types::kLocalStorage) == 0)
    return webos::WebViewProfile::REMOVE_LOCAL_STORAGE;
  if (strcmp(type, browsing_data_types::kChannelIDs) == 0)
    return webos::WebViewProfile::REMOVE_CHANNEL_IDS;
  if (strcmp(type, browsing_data_types::kServiceWorkers) == 0)
    return webos::WebViewProfile::REMOVE_SERVICE_WORKERS;
  if (strcmp(type, browsing_data_types::kCacheStorage) == 0)
    return webos::WebViewProfile::REMOVE_CACHE_STORAGE;
  if (strcmp(type, browsing_data_types::kWebSQL) == 0)
    return webos::WebViewProfile::REMOVE_WEBSQL;

  return 0;
}
