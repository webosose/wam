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

#include "Url.h"

#include <glib.h>

namespace wam {

Url::Url(const std::string& uri) {
  ParseUri(uri);
}

void Url::SetQuery(const UrlQuery& query) {
  query_.clear();
  for (const auto& q : query) {
    gchar* escaped_key = g_uri_escape_string(q.first.c_str(), nullptr, true);
    gchar* escaped_val = g_uri_escape_string(q.second.c_str(), nullptr, true);
    query_ += query_.empty() ? '?' : '&';
    query_ += std::string(escaped_key) + "=" + escaped_val;
    g_free(escaped_key);
    g_free(escaped_val);
  }
}

std::string Url::ToString() const {
  return uri_ + query_ + fragment_;
}

std::string Url::ToLocalFile() const {
  g_autofree gchar* cpath = g_filename_from_uri(uri_.c_str(), nullptr, nullptr);
  return cpath ? std::string(cpath) : std::string();
}

Url Url::FromLocalFile(const std::string& path) {
  g_autofree gchar* cpath = g_filename_to_uri(path.c_str(), nullptr, nullptr);
  return cpath ? Url(cpath) : Url("");
}

bool Url::IsLocalFile() const {
  return scheme_ == "file";
}

std::string Url::FileName() const {
  if (!IsLocalFile())
    return "";
  std::string local = ToLocalFile();
  auto found = local.find_last_of("/");
  if (found == std::string::npos)
    return local;
  return local.substr(found + 1, local.size() - found);
}

void Url::ParseUri(const std::string& uri) {
  auto scheme_delimeter = uri.find(':');
  if (scheme_delimeter != std::string::npos)
    scheme_ = uri.substr(0, scheme_delimeter);

  auto authority_start = uri.find("//");
  if (authority_start != std::string::npos)
    authority_start += 2;
  auto authority_end = uri.find_first_of("/?#", authority_start);

  if (authority_start != std::string::npos) {
    auto host_start = authority_start;
    auto user_info_end = uri.find('@', authority_start);
    if (user_info_end != std::string::npos)
      host_start = user_info_end + 1;
    auto host_end = uri.find_first_of(":/?#", host_start);
    if (host_start != std::string::npos)
      host_ = uri.substr(host_start, host_end - host_start);
    if (uri[host_end] == ':')
      port_ = uri.substr(host_end + 1, authority_end - host_end - 1);
  }

  auto path_end = uri.find_first_of("?#", authority_end);
  // Save part of the original uri without query and fragment.
  uri_ = uri.substr(0, path_end);

  if (authority_start == std::string::npos) {
    path_ = uri.substr(scheme_delimeter + 1, uri.size() - scheme_delimeter);
  } else if (authority_end != std::string::npos) {
    if (uri[authority_end] == '/')
      path_ = uri.substr(authority_end, path_end - authority_end);

    auto query_start = uri.find("?", authority_end);
    if (query_start != std::string::npos) {
      auto query_end = uri.find("#", query_start);
      query_ = uri.substr(query_start, query_end - query_start);
    }

    auto fragment_start = uri.find("#", authority_end);
    if (fragment_start != std::string::npos) {
      fragment_ = uri.substr(fragment_start, uri.size() - fragment_start);
    }
  }
}

}  // namespace wam
