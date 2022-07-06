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

#ifndef CORE_APPLICATION_DESCRIPTION_H_
#define CORE_APPLICATION_DESCRIPTION_H_

#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "display_id.h"

class ApplicationDescription {
 public:
  enum WindowClass { kWindowClassNormal = 0x00, kWindowClassHidden = 0x01 };

  ApplicationDescription();
  virtual ~ApplicationDescription() {}

  const std::string& Id() const { return id_; }
  const std::string& Title() const { return title_; }
  const std::string& EntryPoint() const { return entry_point_; }
  const std::string& Icon() const { return icon_; }

  bool IsTransparent() const { return transparency_; }

  bool HandlesDeeplinking() const { return !deep_linking_params_.empty(); }

  bool HandlesRelaunch() const { return handles_relaunch_; }

  const std::string& VendorExtension() const { return vendor_extension_; }

  WindowClass WindowClassValue() const { return window_class_value_; }

  const std::string& TrustLevel() const { return trust_level_; }

  const std::string& SubType() const { return sub_type_; }

  const std::string& FolderPath() const { return folder_path_; }

  const std::string& DefaultWindowType() const { return default_window_type_; }

  const std::string& EnyoBundleVersion() const { return enyo_bundle_version_; }

  const std::set<std::string>& SupportedEnyoBundleVersions() const {
    return supported_enyo_bundle_versions_;
  }

  const std::string& EnyoVersion() const { return enyo_version_; }

  const std::string& Version() const { return version_; }

  const std::string& GroupWindowDesc() const { return group_window_desc_; }

  const std::string& V8SnapshotPath() const { return v8_snapshot_path_; }

  const std::string& V8ExtraFlags() const { return v8_extra_flags_; }

  static std::unique_ptr<ApplicationDescription> FromJsonString(
      const char* json_str);

  bool IsInspectable() const { return inspectable_; }
  bool UseCustomPlugin() const { return custom_plugin_; }
  bool UseNativeScroll() const { return use_native_scroll_; }
  bool UsePrerendering() const { return use_prerendering_; }

  bool DoNotTrack() const { return do_not_track_; }

  bool BackHistoryAPIDisabled() const { return back_history_api_disabled_; }
  void SetBackHistoryAPIDisabled(bool disabled) {
    back_history_api_disabled_ = disabled;
  }

  int WidthOverride() const { return width_override_; }
  int HeightOverride() const { return height_override_; }

  bool HandleExitKey() const { return handle_exit_key_; }
  bool SupportsAudioGuidance() const { return supports_audio_guidance_; }
  bool IsEnableBackgroundRun() const { return enable_background_run_; }
  bool AllowVideoCapture() const { return allow_video_capture_; }
  bool AllowAudioCapture() const { return allow_audio_capture_; }

  virtual bool UseVirtualKeyboard() const { return use_virtual_keyboard_; }
  // Key code is changed only for facebooklogin WebApp
  const std::unordered_map<int, std::pair<int, int>>& KeyFilterTable() const {
    return key_filter_table_;
  }

  double NetworkStableTimeout() const { return network_stable_timeout_; }
  bool DisallowScrollingInMainFrame() const {
    return disallow_scrolling_in_main_frame_;
  }
  int DelayMsForLaunchOptimization() const {
    return delay_ms_for_lanch_optimization_;
  }
  bool UseUnlimitedMediaPolicy() const { return use_unlimited_media_policy_; }
  const std::string& LocationHint() const { return location_hint_; }

  struct WindowOwnerInfo {
    WindowOwnerInfo() : allow_anonymous(false) {}

    bool allow_anonymous;
    std::unordered_map<std::string, int> layers;
  };

  struct WindowClientInfo {
    std::string layer;
    std::string hint;
  };

  struct WindowGroupInfo {
    WindowGroupInfo() : is_owner(false) {}

    std::string name;
    bool is_owner;
  };

  const WindowGroupInfo GetWindowGroupInfo();
  const WindowOwnerInfo GetWindowOwnerInfo();
  const WindowClientInfo GetWindowClientInfo();

  // To support multi display
  DisplayId GetDisplayAffinity() { return display_affinity_; }
  void SetDisplayAffinity(DisplayId display) { display_affinity_ = display; }
  int CustomSuspendDOMTime() const { return custom_suspend_dom_time_; }
  std::string MediaPreferences() const { return media_preferences_; }
  void SetMediaPreferences(const std::string& pref);

 private:
  bool CheckTrustLevel(std::string trust_level);

  std::string id_;
  std::string title_;
  std::string entry_point_;
  std::string icon_;
  std::string requested_window_orientation_;

  bool transparency_;
  std::string vendor_extension_;
  WindowClass window_class_value_;
  std::string trust_level_;
  std::string sub_type_;
  std::string deep_linking_params_;
  bool handles_relaunch_;
  std::string folder_path_;
  std::string default_window_type_;
  std::string enyo_bundle_version_;
  std::set<std::string> supported_enyo_bundle_versions_;
  std::string enyo_version_;
  std::string version_;
  std::string v8_snapshot_path_;
  std::string v8_extra_flags_;
  bool inspectable_;
  bool custom_plugin_;
  bool back_history_api_disabled_;
  int width_override_;
  int height_override_;
  std::unordered_map<int, std::pair<int, int>> key_filter_table_;
  std::string group_window_desc_;
  bool do_not_track_;
  bool handle_exit_key_;
  bool enable_background_run_;
  bool allow_video_capture_;
  bool allow_audio_capture_;
  bool supports_audio_guidance_;
  bool use_native_scroll_;
  bool use_prerendering_;
  double network_stable_timeout_;
  bool disallow_scrolling_in_main_frame_;
  int delay_ms_for_lanch_optimization_;
  bool use_unlimited_media_policy_;
  int display_affinity_;
  std::string location_hint_;
  bool use_virtual_keyboard_;
  int custom_suspend_dom_time_;
  std::string media_preferences_;
};

#endif  // CORE_APPLICATION_DESCRIPTION_H_
