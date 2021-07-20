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

#ifndef BASECHECK_H
#define BASECHECK_H

#ifdef NDEBUG
// Supress Werror=unused-variable in release build.
#define UTIL_ASSERT(c) ((void)(c))
#else
#include <cassert>
#define UTIL_ASSERT(c) (assert(c))
#endif

#endif // BASECHECK_H
