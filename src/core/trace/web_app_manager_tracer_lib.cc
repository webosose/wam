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

#include "web_app_manager_tracer_lib.h"

#include "pmtrace_webappmanager3_provider.h"

void TraceMessage(const char* label) {
  tracepoint(pmtrace_webappmanager3, message, (char*)label);
}

void TraceBefore(const char* label) {
  tracepoint(pmtrace_webappmanager3, before, (char*)label);
}

void TraceAfter(const char* label) {
  tracepoint(pmtrace_webappmanager3, after, (char*)label);
}

void TraceScopeEntry(const char* label) {
  tracepoint(pmtrace_webappmanager3, scope_entry, (char*)label);
}

void TraceScopeExit(const char* label) {
  tracepoint(pmtrace_webappmanager3, scope_exit, (char*)label);
}

void TraceFunctionEntry(const char* label) {
  tracepoint(pmtrace_webappmanager3, function_entry, (char*)label);
}

void TraceFunctionExit(const char* label) {
  tracepoint(pmtrace_webappmanager3, function_exit, (char*)label);
}

void TraceItem(const char* name, const char* value) {
  tracepoint(pmtrace_webappmanager3, item, (char*)name, (char*)value);
}

void TracePosition(const char* label, int posX, int posY) {
  tracepoint(pmtrace_webappmanager3, position, (char*)label, posX, posY);
}
