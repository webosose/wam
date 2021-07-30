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

#ifndef UTIL_BCP47_H_
#define UTIL_BCP47_H_

#include <memory>
#include <string>

class BCP47 {
 public:
  static std::unique_ptr<BCP47> FromString(const std::string& bcp47_string);

  bool HasLanguage() const;
  bool HasScript() const;
  bool HasRegion() const;

  std::string Language() const;
  std::string Script() const;
  std::string Region() const;

  ~BCP47();

 private:
  BCP47();

  std::string language_;
  std::string script_;
  std::string region_;
};

#endif  //  UTIL_BCP47_H_
