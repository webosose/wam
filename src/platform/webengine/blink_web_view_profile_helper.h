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

#ifndef PLATFORM_WEBENGINE_BLINK_WEB_VIEW_PROFILE_HELPER_H_
#define PLATFORM_WEBENGINE_BLINK_WEB_VIEW_PROFILE_HELPER_H_

namespace webos {
class WebViewProfile;
}

namespace browsing_data_types {
const char kAll[] = "all";
const char kAppCache[] = "appcache";
const char kCache[] = "cache";
const char kCodeCache[] = "codecache";
const char kCookies[] = "cookies";
const char kFileSystems[] = "fileSystems";
const char kIndexedDB[] = "indexedDB";
const char kLocalStorage[] = "localStorage";
const char kChannelIDs[] = "channelIDs";
const char kServiceWorkers[] = "serviceWorkers";
const char kCacheStorage[] = "cacheStorage";
const char kWebSQL[] = "webSQL";
}  // namespace browsing_data_types

class BlinkWebViewProfileHelper {
 public:
  BlinkWebViewProfileHelper() {}
  virtual ~BlinkWebViewProfileHelper() {}

  static void ClearBrowsingData(const int remove_browsing_data_mask,
                                webos::WebViewProfile* profile = nullptr);
  static void ClearDefaultBrowsingData(const int remove_browsing_data_mask);
  static int MaskForBrowsingDataType(const char* key);
};

#endif  // PLATFORM_WEBENGINE_BLINK_WEB_VIEW_PROFILE_HELPER_H_
