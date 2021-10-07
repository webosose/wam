// Copyright (c) 2021 LG Electronics, Inc.
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

#include "pmtrace_provider_lib_wrapper.h"

#include <dlfcn.h>

#include <cstddef>
#include <functional>
#include <memory>

#include "log_manager.h"

namespace {
const char kTraceAfter[] = "TraceAfter";
const char kTraceBefore[] = "TraceBefore";
const char kTraceItem[] = "TraceItem";
const char kTraceFunctionEntry[] = "TraceFunctionEntry";
const char kTraceFunctionExit[] = "TraceFunctionExit";
const char kTraceMessage[] = "TraceMessage";
const char kTracePosition[] = "TracePosition";
const char kTraceScopeEntry[] = "TraceScopeEntry";
const char kTraceScopeExit[] = "TraceScopeExit";

using LibFunctypeTraceLabel = void* (*)(const char*);
using FunctypeTraceLabel = std::function<void(const char*)>;

using LibFunctypeTraceItem = void* (*)(const char*, const char*);
using FunctypeTraceItem = std::function<void(const char*, const char*)>;

using LibFunctypeTracePosition = void* (*)(const char*, int, int);
using FunctypeTracePosition = std::function<void(const char*, int, int)>;

template <typename T, typename F>
bool TryGetFunction(void* lib_handle, const char* func_name, F& func_handle) {
  if (lib_handle)
    func_handle = reinterpret_cast<T>(dlsym(lib_handle, func_name));
  if (!func_handle)
    LOG_ERROR(MSGID_DL_ERROR, 0, "Function \"%s\" address obtain error: %s",
              func_name, dlerror());

  return func_handle != nullptr;
}

class PmtraceProviderLibWrapper {
 public:
  static PmtraceProviderLibWrapper* Instance() {
    // not a leak -- static variable initializations are only ever done once
    static std::unique_ptr<PmtraceProviderLibWrapper> instance(
        new PmtraceProviderLibWrapper());
    return instance.get();
  }

  ~PmtraceProviderLibWrapper() {
    if (lib_handle_) {
      dlclose(lib_handle_);
      lib_handle_ = nullptr;
    }
  }

  void DoTraceMessage(const char* label) {
    if (CheckIsReady())
      func_trace_message_(label);
  }

  void DoTraceBefore(const char* label) {
    if (CheckIsReady())
      func_trace_before_(label);
  }

  void DoTraceAfter(const char* label) {
    if (CheckIsReady())
      func_trace_after_(label);
  }

  void DoTraceScopeEntry(const char* label) {
    if (CheckIsReady())
      func_trace_scope_entry_(label);
  }

  void DoTraceScopeExit(const char* label) {
    if (CheckIsReady())
      func_trace_scope_exit_(label);
  }

  void DoTraceFunctionEntry(const char* label) {
    if (CheckIsReady())
      func_trace_function_entry_(label);
  }

  void DoTraceFunctionExit(const char* label) {
    if (CheckIsReady())
      func_trace_function_exit_(label);
  }

  void DoTraceItem(const char* name, const char* value) {
    if (CheckIsReady())
      func_trace_item_(name, value);
  }

  void DoTracePosition(const char* label, int posX, int posY) {
    if (CheckIsReady())
      func_trace_position_(label, posX, posY);
  }

 private:
  PmtraceProviderLibWrapper()
      : is_ready(false),
        lib_handle_(nullptr),
        func_trace_message_(nullptr),
        func_trace_before_(nullptr),
        func_trace_after_(nullptr),
        func_trace_scope_entry_(nullptr),
        func_trace_scope_exit_(nullptr),
        func_trace_function_entry_(nullptr),
        func_trace_function_exit_(nullptr),
        func_trace_item_(nullptr),
        func_trace_position_(nullptr){};

  bool CheckIsReady() {
    if (!is_ready && !lib_handle_) {
      lib_handle_ = dlopen(TRACE_PROVIDER_LIB_PATH, RTLD_LAZY);
      if (!lib_handle_) {
        LOG_ERROR(MSGID_DL_ERROR, 0, "The provider library loading error: %s",
                  dlerror());
      } else {
        is_ready =
            TryGetFunction<LibFunctypeTraceLabel>(lib_handle_, kTraceMessage,
                                                  func_trace_message_) &&
            TryGetFunction<LibFunctypeTraceLabel>(lib_handle_, kTraceBefore,
                                                  func_trace_before_) &&
            TryGetFunction<LibFunctypeTraceLabel>(lib_handle_, kTraceAfter,
                                                  func_trace_after_) &&
            TryGetFunction<LibFunctypeTraceLabel>(lib_handle_, kTraceScopeEntry,
                                                  func_trace_scope_entry_) &&
            TryGetFunction<LibFunctypeTraceLabel>(lib_handle_, kTraceScopeExit,
                                                  func_trace_scope_exit_) &&
            TryGetFunction<LibFunctypeTraceLabel>(
                lib_handle_, kTraceFunctionEntry, func_trace_function_entry_) &&
            TryGetFunction<LibFunctypeTraceLabel>(
                lib_handle_, kTraceFunctionExit, func_trace_function_exit_) &&
            TryGetFunction<LibFunctypeTraceItem>(lib_handle_, kTraceItem,
                                                 func_trace_item_) &&
            TryGetFunction<LibFunctypeTracePosition>(
                lib_handle_, kTracePosition, func_trace_position_);
      }
    }

    return is_ready;
  }

  bool is_ready;
  void* lib_handle_;
  FunctypeTraceLabel func_trace_message_;
  FunctypeTraceLabel func_trace_before_;
  FunctypeTraceLabel func_trace_after_;
  FunctypeTraceLabel func_trace_scope_entry_;
  FunctypeTraceLabel func_trace_scope_exit_;
  FunctypeTraceLabel func_trace_function_entry_;
  FunctypeTraceLabel func_trace_function_exit_;
  FunctypeTraceItem func_trace_item_;
  FunctypeTracePosition func_trace_position_;
};  // class PmtraceProviderLibWrapper
}  // namespace

void pmtrace::TraceMessage(const char* label) {
  PmtraceProviderLibWrapper::Instance()->DoTraceMessage(label);
}

void pmtrace::TraceBefore(const char* label) {
  PmtraceProviderLibWrapper::Instance()->DoTraceBefore(label);
}

void pmtrace::TraceAfter(const char* label) {
  PmtraceProviderLibWrapper::Instance()->DoTraceAfter(label);
}

void pmtrace::TraceScopeEntry(const char* label) {
  PmtraceProviderLibWrapper::Instance()->DoTraceScopeEntry(label);
}

void pmtrace::TraceScopeExit(const char* label) {
  PmtraceProviderLibWrapper::Instance()->DoTraceScopeExit(label);
}

void pmtrace::TraceFunctionEntry(const char* label) {
  PmtraceProviderLibWrapper::Instance()->DoTraceFunctionEntry(label);
}

void pmtrace::TraceFunctionExit(const char* label) {
  PmtraceProviderLibWrapper::Instance()->DoTraceFunctionExit(label);
}

void pmtrace::TraceItem(const char* name, const char* value) {
  PmtraceProviderLibWrapper::Instance()->DoTraceItem(name, value);
}

void pmtrace::TracePosition(const char* label, int posX, int posY) {
  PmtraceProviderLibWrapper::Instance()->DoTracePosition(label, posX, posY);
}
