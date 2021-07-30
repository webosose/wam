// Copyright (c) 2008-2021 LG Electronics, Inc.
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

#include "application_description.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <limits>
#include <sstream>

#include <json/json.h>

#include "log_manager.h"
#include "utils.h"

bool ApplicationDescription::CheckTrustLevel(std::string trust_level) {
  if (trust_level.empty())
    return false;
  if (trust_level.compare("default") == 0)
    return true;
  if (trust_level.compare("trusted") == 0)
    return true;
  return false;
}

ApplicationDescription::ApplicationDescription()
    : transparency_(false),
      window_class_value_(kWindowClassNormal),
      handles_relaunch_(false),
      inspectable_(true),
      custom_plugin_(false),
      back_history_api_disabled_(false),
      width_override_(0),
      height_override_(0),
      do_not_track_(false),
      handle_exit_key_(false),
      enable_background_run_(false),
      allow_video_capture_(false),
      allow_audio_capture_(false),
      supports_audio_guidance_(false),
      use_native_scroll_(false),
      use_prerendering_(false),
      network_stable_timeout_(std::numeric_limits<double>::quiet_NaN()),
      disallow_scrolling_in_main_frame_(true),
      delay_ms_for_lanch_optimization_(0),
      use_unlimited_media_policy_(false),
      display_affinity_(kUndefinedDisplayId),
      use_virtual_keyboard_(true),
      custom_suspend_dom_time_(0) {}

const ApplicationDescription::WindowGroupInfo
ApplicationDescription::GetWindowGroupInfo() {
  ApplicationDescription::WindowGroupInfo info;

  if (!group_window_desc_.empty()) {
    Json::Value json = util::StringToJson(group_window_desc_);

    if (json.isObject()) {
      auto name = json["name"];
      if (name.isString()) {
        info.name = name.asString();
      }

      auto is_owner = json["owner"];
      if (is_owner.isBool())
        info.is_owner = is_owner.asBool();
    }
  }

  return info;
}

const ApplicationDescription::WindowOwnerInfo
ApplicationDescription::GetWindowOwnerInfo() {
  ApplicationDescription::WindowOwnerInfo info;
  if (!group_window_desc_.empty()) {
    Json::Value json = util::StringToJson(group_window_desc_);

    auto owner_info = json["ownerInfo"];
    if (owner_info.isObject()) {
      if (owner_info["allowAnonymous"].isBool())
        info.allow_anonymous = owner_info["allowAnonymous"].asBool();

      auto layers = owner_info["layers"];
      if (layers.isArray()) {
        for (const auto& layer : layers) {
          auto name = layer["name"];
          auto z = layer["z"];
          if (name.isString() && z.isInt()) {
            info.layers.emplace(name.asString(), z.asInt());
          }
        }
      }
    }
  }
  return info;
}

const ApplicationDescription::WindowClientInfo
ApplicationDescription::GetWindowClientInfo() {
  ApplicationDescription::WindowClientInfo info;
  if (!group_window_desc_.empty()) {
    Json::Value json = util::StringToJson(group_window_desc_);

    auto client_info = json["clientInfo"];
    if (client_info.isObject()) {
      auto layer = client_info["layer"];
      if (layer.isString())
        info.layer = layer.asString();

      auto hint = client_info["hint"];
      if (hint.isString())
        info.hint = hint.asString();
    }
  }
  return info;
}

std::unique_ptr<ApplicationDescription> ApplicationDescription::FromJsonString(
    const char* json_str) {
  Json::Value json_obj = util::StringToJson(json_str);
  if (!json_obj.isObject()) {
    LOG_WARNING(MSGID_APP_DESC_PARSE_FAIL, 1, PMLOGKFV("JSON", "%s", json_str),
                "Failed to parse JSON string");
    return nullptr;
  }

  auto app_desc =
      std::unique_ptr<ApplicationDescription>(new ApplicationDescription());

  app_desc->transparency_ = json_obj["transparent"].asBool();
  auto vendor_extension =
      json_obj.get("vendorExtension", Json::Value(Json::objectValue));
  app_desc->vendor_extension_ = util::JsonToString(vendor_extension);
  app_desc->trust_level_ = json_obj["trustLevel"].asString();
  app_desc->sub_type_ = json_obj["subType"].asString();
  app_desc->deep_linking_params_ = json_obj["deeplinkingParams"].asString();
  app_desc->handles_relaunch_ = json_obj["handlesRelaunch"].asBool();
  app_desc->default_window_type_ = json_obj["defaultWindowType"].asString();
  app_desc->inspectable_ = json_obj["inspectable"].asBool();
  app_desc->enyo_bundle_version_ = json_obj["enyoBundleVersion"].asString();
  app_desc->enyo_version_ = json_obj["enyoVersion"].asString();
  app_desc->version_ = json_obj["version"].asString();
  app_desc->custom_plugin_ = json_obj["customPlugin"].asBool();
  app_desc->back_history_api_disabled_ =
      json_obj["disableBackHistoryAPI"].asBool();
  auto group_window_desc =
      json_obj.get("windowGroup", Json::Value(Json::objectValue));
  app_desc->group_window_desc_ = util::JsonToString(group_window_desc);

  auto supported_versions = json_obj["supportedEnyoBundleVersions"];
  if (supported_versions.isArray()) {
    for (const Json::Value& version : supported_versions)
      app_desc->supported_enyo_bundle_versions_.insert(version.asString());
  }

  app_desc->id_ = json_obj["id"].asString();
  app_desc->entry_point_ = json_obj["main"].asString();
  app_desc->icon_ = json_obj["icon"].asString();
  app_desc->folder_path_ = json_obj["folderPath"].asString();
  app_desc->requested_window_orientation_ =
      json_obj["requestedWindowOrientation"].asString();
  app_desc->title_ = json_obj["title"].asString();
  app_desc->do_not_track_ = json_obj["doNotTrack"].asBool();
  app_desc->handle_exit_key_ = json_obj["handleExitKey"].asBool();
  app_desc->enable_background_run_ = json_obj["enableBackgroundRun"].asBool();
  app_desc->allow_video_capture_ = json_obj["allowVideoCapture"].asBool();
  app_desc->allow_audio_capture_ = json_obj["allowAudioCapture"].asBool();

  auto enable_keyboard = json_obj["enableKeyboard"];
  app_desc->use_virtual_keyboard_ =
      enable_keyboard.isBool() && enable_keyboard.asBool();

  auto use_prerendering = json_obj["usePrerendering"];
  app_desc->use_prerendering_ =
      use_prerendering.isBool() && use_prerendering.asBool();

  auto disallow_scrolling = json_obj["disallowScrollingInMainFrame"];
  app_desc->disallow_scrolling_in_main_frame_ =
      disallow_scrolling.isBool() && disallow_scrolling.asBool();

  auto media_extension =
      json_obj.get("mediaExtension", Json::Value(Json::objectValue));
  app_desc->media_preferences_ = util::JsonToString(media_extension);

  // Handle accessibility, supportsAudioGuidance
  auto accessibility = json_obj["accessibility"];
  if (accessibility.isObject()) {
    auto audio_guidance = accessibility["supportsAudioGuidance"];
    app_desc->supports_audio_guidance_ =
        audio_guidance.isBool() && audio_guidance.asBool();
  }

  // Handle v8 snapshot file
  auto v8_snapshot_file = json_obj["v8SnapshotFile"];
  if (v8_snapshot_file.isString()) {
    std::string snapshot_file = v8_snapshot_file.asString();
    if (snapshot_file.length() > 0) {
      if (snapshot_file.at(0) == '/')
        app_desc->v8_snapshot_path_ = snapshot_file;
      else
        app_desc->v8_snapshot_path_ =
            app_desc->folder_path_ + "/" + snapshot_file;
    }
  }

  // Handle v8 extra flags
  auto v8_extra_flags = json_obj["v8ExtraFlags"];
  if (v8_extra_flags.isString())
    app_desc->v8_extra_flags_ = v8_extra_flags.asString();

  // Handle resolution
  auto resolution = json_obj["resolution"];
  if (resolution.isString()) {
    std::string override_resolution = json_obj["resolution"].asString();
    auto res_list = util::SplitString(override_resolution, 'x');
    if (res_list.size() == 2) {
      util::StrToInt(res_list.at(0), app_desc->width_override_);
      util::StrToInt(res_list.at(1), app_desc->height_override_);
    }
    if (app_desc->width_override_ < 0 || app_desc->height_override_ < 0) {
      app_desc->width_override_ = 0;
      app_desc->height_override_ = 0;
    }
  }

  auto location_hint = json_obj["locationHint"];
  if (location_hint.isString())
    app_desc->location_hint_ = location_hint.asString();

  // Handle keyFilterTable
  // Key code is changed only for facebooklogin WebApp
  auto key_filter_table = json_obj["keyFilterTable"];
  if (key_filter_table.isArray()) {
    for (const auto& k : key_filter_table) {
      if (!k.isObject())
        continue;
      int from = k["from"].asInt();
      int to = k["to"].asInt();
      int modifier = k["modifier"].asInt();
      app_desc->key_filter_table_[from] = std::make_pair(to, modifier);
    }
  }

  // Handle trustLevel
  if (!app_desc->CheckTrustLevel(app_desc->trust_level_))
    app_desc->trust_level_ = std::string("default");

  // Handle webAppType
  if (app_desc->sub_type_.empty()) {
    app_desc->sub_type_ = std::string("default");
  }

  // Handle hidden property of window class
  // Convert a json object for window class to an enum value
  // and store it instead of the json object.
  // The format of window class in the appinfo.json is as below.
  //
  // class : { "hidden" : boolean }
  //
  WindowClass class_value = kWindowClassNormal;
  auto clazz = json_obj["class"];
  if (clazz.isObject()) {
    if (clazz["hidden"].isBool() && clazz["hidden"].asBool())
      class_value = kWindowClassHidden;
  }
  app_desc->window_class_value_ = class_value;

  // Handle folderPath
  if (!app_desc->folder_path_.empty()) {
    std::string temp_path =
        app_desc->folder_path_ + "/" + app_desc->entry_point_;
    struct stat stat_ent_pt;
    if (!stat(temp_path.c_str(), &stat_ent_pt)) {
      app_desc->entry_point_ = "file://" + temp_path;
    }
    temp_path.clear();
    temp_path = app_desc->folder_path_ + "/" + app_desc->icon_;
    if (!stat(temp_path.c_str(), &stat_ent_pt)) {
      app_desc->icon_ = temp_path;
    }
  }
  app_desc->use_native_scroll_ = json_obj["useNativeScroll"].isBool() &&
                                 json_obj["useNativeScroll"].asBool();

  // Set network stable timeout
  if (json_obj.isMember("networkStableTimeout")) {
    auto network_stable_timeout = json_obj["networkStableTimeout"];
    if (!network_stable_timeout.isDouble()) {
      LOG_ERROR(MSGID_TYPE_ERROR, 2, PMLOGKS("APP_ID", app_desc->Id().c_str()),
                PMLOGKFV("DATA_TYPE", "%d", network_stable_timeout.type()),
                "Invaild JsonValue type");
    } else {
      app_desc->network_stable_timeout_ = network_stable_timeout.asDouble();
    }
  }

  // Set delay millisecond for launch optimization
  auto delay_ms_for_launch_optimization =
      json_obj["delayMsForLaunchOptimization"];
  if (delay_ms_for_launch_optimization.isInt()) {
    int delay_ms = delay_ms_for_launch_optimization.asInt();
    app_desc->delay_ms_for_lanch_optimization_ = (delay_ms >= 0) ? delay_ms : 0;
  }

  auto use_unlimited_media_policy = json_obj["useUnlimitedMediaPolicy"];
  if (use_unlimited_media_policy.isBool()) {
    app_desc->use_unlimited_media_policy_ = use_unlimited_media_policy.asBool();
  }

  auto suspend_dom_time = json_obj["suspendDOMTime"];
  if (suspend_dom_time.isInt())
    app_desc->custom_suspend_dom_time_ = suspend_dom_time.asInt();

  return app_desc;
}

void ApplicationDescription::SetMediaPreferences(const std::string& pref) {
  media_preferences_ = pref;
}
