// Copyright (c) 2013-2018 LG Electronics, Inc.
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

#ifndef WebAppManagerTracer_h
#define WebAppManagerTracer_h

#ifdef HAS_LTTNG

#include "pmtrace_webappmanager3_provider.h"

/* PMTRACE_LOG is for free form tracing. Provide a string
   which uniquely identifies your trace point. */
#define PMTRACE(label) \
    tracepoint(pmtrace_webappmanager3, message, label)

/* PMTRACE_BEFORE / AFTER is for tracing a time duration
 * which is not contained within a scope (curly braces) or function,
 * or in C code where there is no mechanism to automatically detect
 * exiting a scope or function.
 */
#define PMTRACE_BEFORE(label) \
    tracepoint(pmtrace_webappmanager3, before, label)
#define PMTRACE_AFTER(label) \
    tracepoint(pmtrace_webappmanager3, after, label)

/* PMTRACE_SCOPE* is for tracing a the duration of a scope.  In
 * C++ code use PMTRACE_SCOPE only, in C code use the
 * ENTRY/EXIT macros and be careful to catch all exit cases.
 */
#define PMTRACE_SCOPE_ENTRY(label) \
    tracepoint(pmtrace_webappmanager3, scope_entry, label)
#define PMTRACE_SCOPE_EXIT(label) \
    tracepoint(pmtrace_webappmanager3, scope_exit, label)
#define PMTRACE_SCOPE(label) \
    PmTraceScope traceScope(label)

/* PMTRACE_FUNCTION* is for tracing a the duration of a scope.
 * In C++ code use PMTRACE_FUNCTION only, in C code use the
 * ENTRY/EXIT macros and be careful to catch all exit cases.
 */
#define PMTRACE_FUNCTION_ENTRY(label) \
    tracepoint(pmtrace_webappmanager3, function_entry, label)
#define PMTRACE_FUNCTION_EXIT(label) \
    tracepoint(pmtrace_webappmanager3, function_exit, label)
#define PMTRACE_FUNCTION \
    PmTraceFunction traceFunction(const_cast<char*>(Q_FUNC_INFO))

class PmTraceScope {
public:
    PmTraceScope(char* label)
        : scopeLabel(label)
    {
        PMTRACE_SCOPE_ENTRY(scopeLabel);
    }

    ~PmTraceScope()
    {
        PMTRACE_SCOPE_EXIT(scopeLabel);
    }

private:
    char* scopeLabel;

    // Prevent heap allocation
    void operator delete(void*);
    void* operator new(size_t);
    PmTraceScope(const PmTraceScope&);
    PmTraceScope& operator=(const PmTraceScope&);
};

class PmTraceFunction {
public:
    PmTraceFunction(char* label)
        : fnLabel(label)
    {
        PMTRACE_FUNCTION_ENTRY(fnLabel);
    }

    ~PmTraceFunction()
    {
        PMTRACE_FUNCTION_EXIT(fnLabel);
    }

private:
    char* fnLabel;

    // Prevent heap allocation
    void operator delete(void*);
    void* operator new(size_t);
    PmTraceFunction(const PmTraceFunction&);
    PmTraceFunction& operator=(const PmTraceFunction&);
};

#else // HAS_LTNG

#define PMTRACE(label)
#define PMTRACE_BEFORE(label)
#define PMTRACE_AFTER(label)
#define PMTRACE_SCOPE_ENTRY(label)
#define PMTRACE_SCOPE_EXIT(label)
#define PMTRACE_SCOPE(label)
#define PMTRACE_FUNCTION_ENTRY(label)
#define PMTRACE_FUNCTION_EXIT(label)
#define PMTRACE_FUNCTION

#endif // HAS_LTTNG

#endif // WebAppManagerTracer_h
