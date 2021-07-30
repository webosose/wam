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

#ifndef UTIL_URL_H_
#define UTIL_URL_H_

#include <string>
#include <utility>
#include <vector>

namespace wam {

class Url {
 public:
  typedef std::vector<std::pair<std::string, std::string>> UrlQuery;
  Url(const std::string& uri);
  ~Url() = default;

  std::string Scheme() const { return scheme_; }
  std::string Host() const { return host_; }
  std::string Port() const { return port_; }
  std::string Path() const { return path_; }
  std::string Query() const { return query_; }
  std::string Fragment() const { return fragment_; }

  void SetQuery(const UrlQuery& query);
  std::string ToString() const;
  std::string ToLocalFile() const;
  static Url FromLocalFile(const std::string& path);
  bool IsLocalFile() const;
  std::string FileName() const;

 private:
  void ParseUri(const std::string& uri);
  std::string uri_;
  std::string scheme_;
  std::string host_;
  std::string port_;
  std::string path_;
  std::string query_;
  std::string fragment_;
};

}  // namespace wam

#endif  // UTIL_URL_H_
