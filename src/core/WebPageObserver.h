// Copyright (c) 2016-2018 LG Electronics, Inc.
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

#ifndef WEBPAGEOBSERVER_H
#define WEBPAGEOBSERVER_H

class WebPageBase;

class WebPageObserver {
public:
    virtual void titleChanged() {}
    virtual void firstFrameVisuallyCommitted() {}
    virtual void navigationHistoryChanged() {}

protected:
    WebPageObserver(WebPageBase* page);
    WebPageObserver();

    virtual ~WebPageObserver() {}

    void observe(WebPageBase* page);
    void unobserve(WebPageBase* page);

private:
    WebPageBase* m_page;
};

#endif /* WEBPAGEOBSERVER_H */
