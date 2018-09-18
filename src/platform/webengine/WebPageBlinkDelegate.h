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

#ifndef WEBPAGE_BLINK_DELEGATE_H_
#define WEBPAGE_BLINK_DELEGATE_H_

#include <QString>
#include <QStringList>

class WebPageBlinkDelegate {
public:
    virtual ~WebPageBlinkDelegate() {}
    virtual void close() = 0;
    virtual bool decidePolicyForResponse(bool isMainFrame, int statusCode,
        const std::string& url, const std::string& statusText)
        = 0;
    virtual bool acceptsVideoCapture() = 0;
    virtual bool acceptsAudioCapture() = 0;
    virtual void didFirstFrameFocused() = 0;
    virtual void handleBrowserControlCommand(const QString& command, const QStringList& arguments) = 0;
    virtual void handleBrowserControlFunction(const QString& command, const QStringList& arguments, QString* result) = 0;
    virtual void loadFinished(const std::string& url) = 0;
    virtual void loadFailed(const std::string& url, int errCode, const std::string& errDesc) = 0;
    virtual void loadStopped(const std::string& url) = 0;
    virtual void loadStarted() = 0;
    virtual void loadVisuallyCommitted() = 0;
    virtual void renderProcessCreated(int pid) = 0;
    virtual void renderProcessCrashed() = 0;
    virtual void titleChanged(const std::string& title) = 0;
    virtual void navigationHistoryChanged() = 0;
    virtual void didHistoryBackOnTopPage() {}
    virtual void didClearWindowObject() {}
    virtual void didDropAllPeerConnections() {}
    virtual bool allowMouseOnOffEvent() const = 0;
};

#endif //WEBPAGE_BLINK_DELEGATE_H_
