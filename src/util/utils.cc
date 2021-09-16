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

#include "utils.h"

#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <limits>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <glib.h>
#include <json/json.h>
#include <sys/stat.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "bcp47.h"
namespace util {

static std::string GetString(const char* value) {
  return value ? std::string(value) : std::string();
}

std::vector<std::string> GetErrorPagePaths(
    const std::string& error_page_location,
    const std::string& language) {
  if (error_page_location.empty())
    return std::vector<std::string>();

  namespace fs = boost::filesystem;

  fs::path page_location(error_page_location);
  fs::path filename = page_location.filename();
  fs::path search_path = fs::canonical(page_location.parent_path());
  auto bcp47_pieces = BCP47::FromString(language);

  // search order:
  // searchPath/resources/<language>/<script>/<region>/html/fileName
  // searchPath/resources/<language>/<region>/html/fileName
  // searchPath/resources/<language>/html/fileName
  // searchPath/resources/html/fileName
  // searchPath/fileName

  std::vector<std::string> result;
  result.reserve(5);
  if (bcp47_pieces) {
    if (bcp47_pieces->HasScript()) {
      std::stringstream ss;
      ss << search_path.string() << "/resources/";
      ss << bcp47_pieces->Language() << "/";
      ss << bcp47_pieces->Script();

      if (bcp47_pieces->HasRegion())
        ss << "/" << bcp47_pieces->Region();

      ss << "/html/" << filename.string();
      result.emplace_back(ss.str());
    }
    if (bcp47_pieces->HasRegion()) {
      std::stringstream ss;
      ss << search_path.string() << "/resources/";
      ss << bcp47_pieces->Language() << "/";
      ss << bcp47_pieces->Region() << "/html/";
      ss << filename.string();
      result.emplace_back(ss.str());
    }
    std::stringstream ss;
    ss << search_path.string() << "/resources/";
    ss << bcp47_pieces->Language() << "/html/";
    ss << filename.string();
    result.emplace_back(ss.str());
  }
  result.emplace_back(search_path.string() + "/resources/html/" +
                      filename.string());
  result.emplace_back(page_location.string());

  return result;
}

std::string GetHostname(const std::string& url) {
  if (url.empty())
    return std::string();

  // source https://datatracker.ietf.org/doc/html/rfc3986#appendix-B

  std::regex rfc3986_regex(
      R"(^(([^:\/?#]+):)?(\/\/([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)");
  std::regex authority_regex(R"(^(?:[\w\:]+[@])?([\w.]+)(?:[:])?(?:[0-9]+)?)");
  std::smatch matches;

  if (!std::regex_match(url, matches, rfc3986_regex))
    return std::string();

  std::string authority = matches[4];
  if (!std::regex_match(authority, matches, authority_regex))
    return std::string();

  return matches[1];
}

bool DoesPathExist(const std::string& path) {
  if (path.empty())
    return false;

  struct stat st;
  if (stat(path.c_str(), &st))
    return false;

  return st.st_mode & S_IFDIR || st.st_mode & S_IFREG;
}

std::string ReadFile(const std::string& path) {
  if (!DoesPathExist(path))
    return std::string();

  std::ifstream file(path);
  return std::string(std::istreambuf_iterator<char>(file),
                     std::istreambuf_iterator<char>());
}

std::string UriToLocal(const std::string& uri) {
  g_autofree gchar* cpath = g_filename_from_uri(uri.c_str(), nullptr, nullptr);
  return GetString(cpath);
}

std::string LocalToUri(const std::string& uri) {
  g_autofree gchar* cpath = g_filename_to_uri(uri.c_str(), nullptr, nullptr);
  return GetString(cpath);
}

std::string GetEnvVar(const char* env) {
  return GetString(getenv(env));
}

// STRING
bool StrToInt(const std::string& str, int& num) {
  char* endptr = nullptr;
  errno = 0;
  long long value = strtoll(str.c_str(), &endptr, 10);
  if (endptr == str) {
    return false;
  }

  if (value > std::numeric_limits<int>::max() ||
      value < std::numeric_limits<int>::min() || errno == ERANGE) {
    return false;
  }

  if (errno) {
    return false;
  }

  num = value;
  return true;
}

int StrToIntWithDefault(const std::string& str, int default_value) {
  int converted_value;
  return StrToInt(str, converted_value) ? converted_value : default_value;
}

std::vector<std::string> SplitString(const std::string& str, char delimiter) {
  std::vector<std::string> resList;
  std::stringstream ss(str);
  std::string s;

  while (std::getline(ss, s, delimiter)) {
    resList.push_back(s);
  }

  return resList;
}

std::string TrimString(const std::string& str) {
  std::string trimmed(str);
  boost::trim_right(trimmed);
  boost::trim_left(trimmed);

  return trimmed;
}

void ReplaceSubstr(std::string& in,
                   const std::string& to_search,
                   const std::string& replace_str /* ="" */) {
  size_t pos = in.find(to_search);
  while (pos != std::string::npos) {
    in.replace(pos, to_search.size(), replace_str);
    pos = in.find(to_search, pos + replace_str.size());
  }
}

// JSON
bool StringToJson(const std::string& str, Json::Value& value) {
  Json::CharReaderBuilder builder;
  Json::CharReaderBuilder::strictMode(&builder.settings_);
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

  return reader->parse(str.c_str(), str.c_str() + str.size(), &value, nullptr);
}

Json::Value StringToJson(const std::string& str) {
  Json::Value result;
  return StringToJson(str, result) ? result : Json::Value(Json::nullValue);
}

std::string JsonToString(const Json::Value& value) {
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "    ";
  builder["enableYAMLCompatibility"] = true;

  return Json::writeString(builder, value);
}

}  // namespace util
