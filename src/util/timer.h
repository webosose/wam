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

#ifndef UTIL_TIMER_H_
#define UTIL_TIMER_H_

typedef struct _GTimer GTimer;

class Timer {
 public:
  explicit Timer(bool is_repeating) : is_repeating_(is_repeating) {}
  virtual ~Timer() = default;

  // Timer
  virtual void HandleCallback() = 0;
  virtual void Start(int delay_in_milli_seconds, bool will_destroy = false);

  bool IsRunning() { return is_running_; }
  bool IsRepeating() { return is_repeating_; }
  void Stop();

 protected:
  void Running(bool is_running) { is_running_ = is_running; }

 private:
  int source_id_ = 0;
  bool is_running_ = false;
  bool is_repeating_;
};

template <class Receiver, bool kIsRepeating>
class BaseTimer : public Timer {
 public:
  typedef void (Receiver::*ReceiverMethod)();

  BaseTimer() : Timer(kIsRepeating) {}

  ~BaseTimer() override {
    if (IsRunning()) {
      Stop();
    }
  }

  void HandleCallback() override {
    Running(kIsRepeating);
    (receiver_->*method_)();
  }

  void StartWithReceiver(int delay_in_milli_seconds,
                         Receiver* receiver,
                         ReceiverMethod method,
                         bool will_destroy = false) {
    receiver_ = receiver;
    method_ = method;
    Start(delay_in_milli_seconds, will_destroy);
  }

 private:
  Receiver* receiver_ = nullptr;
  ReceiverMethod method_ = nullptr;
};

template <class Receiver>
class OneShotTimer : public BaseTimer<Receiver, false> {};

template <class Receiver>
class RepeatingTimer : public BaseTimer<Receiver, true> {};

template <class Receiver>
class SingleShotTimer : public BaseTimer<Receiver, false> {
 public:
  typedef void (Receiver::*ReceiverMethod)();

  static void SingleShot(int delay_in_milli_seconds,
                         Receiver* receiver,
                         ReceiverMethod method) {
    SingleShotTimer<Receiver>* timer = new SingleShotTimer<Receiver>;
    timer->StartWithReceiver(delay_in_milli_seconds, receiver, method, true);
  }

 private:
  SingleShotTimer() = default;
};

class ElapsedTimer {
 public:
  ElapsedTimer();
  ElapsedTimer(const ElapsedTimer&) = delete;
  ElapsedTimer& operator=(const ElapsedTimer&) = delete;
  ~ElapsedTimer();

  bool IsRunning() const;
  void Start();
  void Stop();
  int ElapsedMs() const;
  int ElapsedUs() const;

 private:
  bool is_running_ = false;
  GTimer* timer_;
};

#endif  // UTIL_TIMER_H_
