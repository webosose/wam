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

#include "timer.h"

#include <glib.h>

static int TimeoutCallback(void* data) {
  Timer* timer = (Timer*)data;
  bool isRepeating = timer->IsRepeating();
  timer->HandleCallback();
  return isRepeating;
}

static int TimeoutCallbackDestroy(void* data) {
  Timer* timer = (Timer*)data;
  timer->HandleCallback();
  delete timer;
  return 0;
}

void Timer::Start(int delay_in_milli_seconds, bool will_destroy) {
  is_running_ = true;
  if (!will_destroy)
    source_id_ = g_timeout_add(delay_in_milli_seconds, TimeoutCallback, this);
  else
    source_id_ =
        g_timeout_add(delay_in_milli_seconds, TimeoutCallbackDestroy, this);
}

void Timer::Stop() {
  is_running_ = false;
  if (source_id_) {
    g_source_remove(source_id_);
    source_id_ = 0;
  }
}

ElapsedTimer::ElapsedTimer() : is_running_(false), timer_(g_timer_new()) {}

ElapsedTimer::~ElapsedTimer() {
  g_timer_destroy(timer_);
}

bool ElapsedTimer::IsRunning() const {
  return is_running_;
}

void ElapsedTimer::Start() {
  g_timer_start(timer_);
  is_running_ = true;
}

void ElapsedTimer::Stop() {
  g_timer_stop(timer_);
  is_running_ = false;
}

int ElapsedTimer::ElapsedMs() const {
  return static_cast<int>(g_timer_elapsed(timer_, NULL) * 1000);
}

int ElapsedTimer::ElapsedUs() const {
  return static_cast<int>(g_timer_elapsed(timer_, NULL) * 1000000);
}
