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

#include <string>

#include <gtest/gtest.h>

#include "url.h"

namespace {

const char* kAboutBlank = "about:blank";
const char* kHttpsSimpleUri = "https://www.google.com/";
const char* kHttpsWithPathUri =
    "https://google.com/notexist/virtual/path/index.php";
const char* kHttpsWithQueryAndFragmentUri =
    "https://google.com/notexist/virtual/path?test=value#somefragment";
const char* kHttpsNoPathUri = "https://google.com?test=value#somefragment";
const char* kHttpsWithPortUri =
    "https://google.com:8080/notexist/virtual/path?test=value#somefragment";
const char* kHttpsWithUSerInfoUri =
    "https://user:pass@google.com:8080/notexist/virtual/"
    "path?test=value#somefragment";
const char* kFileUri = "file:///usr/opt/webos%20test%23%3F%25/test.txt";
const char* kFileWithHost = "file://server/foo.html";
const char* kFileWithQueryAndFragment = "file:///foo.html?f=v#fragment";
const char* kFileName = "/usr/opt/webos test#?%/test.txt";

}  // namespace

TEST(UrlTest, Scheme) {
  wam::Url https_url(kHttpsSimpleUri);
  EXPECT_EQ("https", https_url.Scheme());

  wam::Url file_url(kFileWithHost);
  EXPECT_EQ("file", file_url.Scheme());

  wam::Url about_blank(kAboutBlank);
  EXPECT_EQ("about", about_blank.Scheme());
}

TEST(UrlTest, Host) {
  wam::Url https_url(kHttpsWithPathUri);
  EXPECT_EQ("google.com", https_url.Host());

  wam::Url https_user_info_url(kHttpsWithUSerInfoUri);
  EXPECT_EQ("google.com", https_user_info_url.Host());

  wam::Url simple_uri(kHttpsSimpleUri);
  EXPECT_EQ("www.google.com", simple_uri.Host());

  wam::Url file_with_host_url(kFileWithHost);
  EXPECT_EQ("server", file_with_host_url.Host());

  wam::Url file_url(kFileUri);
  EXPECT_EQ("", file_url.Host());
}

TEST(UrlTest, Port) {
  wam::Url https_with_port_url(kHttpsWithPortUri);
  EXPECT_EQ("8080", https_with_port_url.Port());

  wam::Url https_url(kHttpsWithQueryAndFragmentUri);
  EXPECT_EQ("", https_url.Port());
}

TEST(UrlTest, Path) {
  wam::Url https_url(kHttpsWithPathUri);
  EXPECT_EQ("/notexist/virtual/path/index.php", https_url.Path());

  wam::Url https_short_path_url(kHttpsSimpleUri);
  EXPECT_EQ("/", https_short_path_url.Path());

  wam::Url file_with_host_url(kFileWithHost);
  EXPECT_EQ("/foo.html", file_with_host_url.Path());

  wam::Url https_no_path_url(kHttpsNoPathUri);
  EXPECT_EQ("", https_no_path_url.Path());

  wam::Url about_blank(kAboutBlank);
  EXPECT_EQ("blank", about_blank.Path());
}

TEST(UrlTest, Query) {
  wam::Url https_url(kHttpsWithQueryAndFragmentUri);
  EXPECT_EQ("?test=value", https_url.Query());

  wam::Url file_url(kFileWithQueryAndFragment);
  EXPECT_EQ("?f=v", file_url.Query());
}

TEST(UrlTest, SetQuery) {
  wam::Url https_url(kHttpsWithQueryAndFragmentUri);
  wam::Url::UrlQuery new_query;
  new_query.emplace_back("first", "value1");
  new_query.emplace_back("second", "value2");
  https_url.SetQuery(new_query);
  EXPECT_EQ("?first=value1&second=value2", https_url.Query());
  EXPECT_EQ(
      "https://google.com/notexist/virtual/"
      "path?first=value1&second=value2#somefragment",
      https_url.ToString());
}

TEST(UrlTest, Fragment) {
  wam::Url https_url(kHttpsWithQueryAndFragmentUri);
  EXPECT_EQ("#somefragment", https_url.Fragment());

  wam::Url file_url(kFileWithQueryAndFragment);
  EXPECT_EQ("#fragment", file_url.Fragment());
}

TEST(UrlTest, UrlToString) {
  wam::Url https_url(kHttpsWithQueryAndFragmentUri);
  EXPECT_EQ(kHttpsWithQueryAndFragmentUri, https_url.ToString());
}

TEST(UrlTest, FromLocalFile) {
  wam::Url url = wam::Url::FromLocalFile(kFileName);
  EXPECT_EQ(kFileUri, url.ToString());
}

TEST(UrlTest, ToLocalFile) {
  wam::Url url(kFileUri);
  EXPECT_EQ(kFileName, url.ToLocalFile());
}

TEST(UrlTest, IsLocalFile) {
  wam::Url https_url(kHttpsSimpleUri);
  EXPECT_FALSE(https_url.IsLocalFile());

  wam::Url file_url(kFileUri);
  EXPECT_TRUE(file_url.IsLocalFile());
}

TEST(UrlTest, FileName) {
  wam::Url https_url(kHttpsSimpleUri);
  EXPECT_EQ(https_url.FileName(), "");

  wam::Url file_url(kFileUri);
  EXPECT_EQ(file_url.FileName(), "test.txt");

  wam::Url file_with_host_url(kFileWithHost);
  EXPECT_EQ(file_with_host_url.FileName(), "foo.html");
}
