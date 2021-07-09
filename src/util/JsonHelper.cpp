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

#include "JsonHelper.h"

#include <json/json.h>

//TODO: remove header when QT less implementation will be completed.
#include <QJsonDocument>

namespace util {

bool JsonValueFromString(const std::string& str, Json::Value& value) {
  Json::CharReaderBuilder builder;
  Json::CharReaderBuilder::strictMode(&builder.settings_);
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  return reader->parse(str.c_str(), str.c_str() + str.size(), &value, nullptr);
}

std::string StringFromJsonValue(const Json::Value& value) {
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "    ";
  builder["enableYAMLCompatibility"] = true;
  return Json::writeString(builder, value);
}

// TODO: remove this methods when QT less implementation will be completed.
QJsonObject QJsonObjectFromJsonValue(const Json::Value& value) {
  std::string str = StringFromJsonValue(value);
  return QJsonDocument::fromJson(str.c_str()).object();
}

bool JsonValueFromQJsonObject(const QJsonObject& object, Json::Value& value) {
  QJsonDocument doc(object);
  std::string json_string = doc.toJson(QJsonDocument::Compact).data();
  return JsonValueFromString(json_string, value);
}

}  // namespace util
