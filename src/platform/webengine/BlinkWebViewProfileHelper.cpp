// Copyright (c) 2017-2018 LG Electronics, Inc.
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


#include "BlinkWebViewProfileHelper.h"
#include "webos/webview_profile.h"
#include <string.h>

void BlinkWebViewProfileHelper::clearBrowsingData(const int removeBrowsingDataMask,
        webos::WebViewProfile *profile)
{
    if (profile)
        profile->RemoveBrowsingData(removeBrowsingDataMask);
    else
        BlinkWebViewProfileHelper::clearDefaultBrowsingData(removeBrowsingDataMask);
}

void BlinkWebViewProfileHelper::clearDefaultBrowsingData(const int removeBrowsingDataMask)
{
    webos::WebViewProfile::GetDefaultProfile()->RemoveBrowsingData(removeBrowsingDataMask);
}

int BlinkWebViewProfileHelper::maskForBrowsingDataType(const char* type) {

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
