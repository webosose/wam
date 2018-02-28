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

#ifndef TIMER_H
#define TIMER_H

int timeout_cb(void* data);
int timeout_cb_destroy(void* data);

typedef struct _GTimer GTimer;

class Timer {
public:
    Timer(bool isRepeating)
        : m_sourceId(0)
        , m_isRunning(false)
        , m_isRepeating(isRepeating)
    {
    }
    virtual ~Timer() {}

    // Timer
    virtual void handleCallback() = 0;
    virtual void start(int delayInMilliSeconds, bool willDestroy = false);

    bool isRunning() { return m_isRunning; }
    bool isRepeating() { return m_isRepeating; }
    void stop();

protected:
    void running(bool isRunning) { m_isRunning = isRunning; }

private:
    int m_sourceId;
    bool m_isRunning;
    bool m_isRepeating;
};

template <class Receiver, bool kIsRepeating>
class BaseTimer : public Timer {
public:
    typedef void (Receiver::*ReceiverMethod)();

    BaseTimer()
        : Timer(kIsRepeating)
    {
    }

    ~BaseTimer() override
    {
        if (isRunning())
            stop();
    }

    void handleCallback() override
    {
        running(kIsRepeating);
        (m_receiver->*m_method)();
    }

    void start(int delayInMilliSeconds, Receiver* receiver, ReceiverMethod method, bool willDestroy = false)
    {
        m_receiver = receiver;
        m_method = method;
        Timer::start(delayInMilliSeconds, willDestroy);
    }

private:
    Receiver* m_receiver;
    ReceiverMethod m_method;
};

template <class Receiver>
class OneShotTimer : public BaseTimer<Receiver, false> {
};

template <class Receiver>
class RepeatingTimer : public BaseTimer<Receiver, true> {
};

template <class Receiver>
class SingleShotTimer : public BaseTimer<Receiver, false> {
public:
    typedef void (Receiver::*ReceiverMethod)();

    static void singleShot(int delayInMilliSeconds, Receiver* receiver, ReceiverMethod method)
    {
        SingleShotTimer<Receiver>* timer = new SingleShotTimer<Receiver>;
        timer->start(delayInMilliSeconds, receiver, method, true);
    }

private:
    SingleShotTimer() {}
};

class ElapsedTimer {
public:
    ElapsedTimer();
    ~ElapsedTimer();
    bool isRunning() const;
    void start();
    void stop();
    int elapsed_ms() const;
    int elapsed_us() const;

private:
    bool m_isRunning;
    GTimer* m_timer;
};

#endif /* TIMER_H */
