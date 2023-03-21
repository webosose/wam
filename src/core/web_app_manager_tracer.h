// Copyright (c) 2013-2021 LG Electronics, Inc.
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

#ifndef CORE_WEB_APP_MANAGER_TRACER_H_
#define CORE_WEB_APP_MANAGER_TRACER_H_

#ifdef HAS_LTTNG

#include "pmtrace_provider_lib_wrapper.h"

/* PMTRACE_LOG is for free form tracing. Provide a string
   which uniquely identifies your trace point. */
#define PMTRACE(label) pmtrace::TraceMessage(label)

/* PMTRACE_BEFORE / AFTER is for tracing a time duration
 * which is not contained within a scope (curly braces) or function,
 * or in C code where there is no mechanism to automatically detect
 * exiting a scope or function.
 */
#define PMTRACE_BEFORE(label) pmtrace::TraceBefore(label)
#define PMTRACE_AFTER(label) pmtrace::TraceAfter(label)

/* PMTRACE_SCOPE* is for tracing a the duration of a scope.  In
 * C++ code use PMTRACE_SCOPE only, in C code use the
 * ENTRY/EXIT macros and be careful to catch all exit cases.
 */
#define PMTRACE_SCOPE_ENTRY(label) pmtrace::TraceScopeEntry(label)
#define PMTRACE_SCOPE_EXIT(label) pmtrace::TraceScopeExit(label)
#define PMTRACE_SCOPE(label) PmTraceScope trace_scope(label)

/* PMTRACE_FUNCTION* is for tracing a the duration of a scope.
 * In C++ code use PMTRACE_FUNCTION only, in C code use the
 * ENTRY/EXIT macros and be careful to catch all exit cases.
 */
#define PMTRACE_FUNCTION_ENTRY(label) pmtrace::TraceFunctionEntry(label)
#define PMTRACE_FUNCTION_EXIT(label) pmtrace::TraceFunctionExit(label)
#define PMTRACE_FUNCTION PmTraceFunction trace_function(__FILE__, __FUNCTION__)

class PmTraceScope {
 public:
  PmTraceScope(const std::string& label) : scope_label_(label) {
    PMTRACE_SCOPE_ENTRY(scope_label_.c_str());
  }

  ~PmTraceScope() { PMTRACE_SCOPE_EXIT(scope_label_.c_str()); }

  // Prevent heap allocation
  void operator delete(void*) = delete;
  void* operator new(size_t) = delete;
  PmTraceScope(const PmTraceScope&) = delete;
  PmTraceScope& operator=(const PmTraceScope&) = delete;

 private:
  std::string scope_label_;
};

class PmTraceFunction {
 public:
  PmTraceFunction(const std::string& label) : fn_label_(label) {
    PMTRACE_FUNCTION_ENTRY(fn_label_.c_str());
  }

  PmTraceFunction(const std::string& file, const std::string& name)
      : fn_label_(file + "::" + name) {
    PMTRACE_FUNCTION_ENTRY(fn_label_.c_str());
  }

  ~PmTraceFunction() { PMTRACE_FUNCTION_EXIT(fn_label_.c_str()); }

  // Prevent heap allocation
  void operator delete(void*) = delete;
  void* operator new(size_t) = delete;
  PmTraceFunction(const PmTraceFunction&) = delete;
  PmTraceFunction& operator=(const PmTraceFunction&) = delete;

 private:
  std::string fn_label_;
};

#else  // HAS_LTNG

#define PMTRACE(label)
#define PMTRACE_BEFORE(label)
#define PMTRACE_AFTER(label)
#define PMTRACE_SCOPE_ENTRY(label)
#define PMTRACE_SCOPE_EXIT(label)
#define PMTRACE_SCOPE(label)
#define PMTRACE_FUNCTION_ENTRY(label)
#define PMTRACE_FUNCTION_EXIT(label)
#define PMTRACE_FUNCTION

#endif  // HAS_LTTNG

#endif  // CORE_WEB_APP_MANAGER_TRACER_H_
