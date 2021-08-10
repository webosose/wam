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

// NOTICE: the next ApplicationDescription fields are read from JSON
// but never used (at least in WebOS-OSE version of WAM):
// * ApplicationDescription::m_requestedWindowOrientation
//   (is read from "requestedWindowOrientation" tag)
// * ApplicationDescription::m_vendorExtension
//   (is read from "vendorExtension" tag)
//   is accessible via std::string& vendorExtension(), which is never called

#include <gtest/gtest.h>

#include "application_description.h"

namespace {

const char* kApplicationDescriptionJson = R"({
        "transparent":false,
        "handlesRelaunch":false,
        "inspectable":false,
        "customPlugin":false,
        "disableBackHistoryAPI":false,
        "doNotTrack":true,
        "handleExitKey":true,
        "enableBackgroundRun":false,
        "allowVideoCapture":true,
        "allowAudioCapture":true,
        "enableKeyboard":false,
        "usePrerendering":true,
        "disallowScrollingInMainFrame":false,
        "useUnlimitedMediaPolicy":true,
        "useNativeScroll":true,
        "trustLevel":"default",
        "subType":"default",
        "deeplinkingParams":"",
        "defaultWindowType":"card",
        "enyoBundleVersion":"Version 2.0.1",
        "enyoVersion":"",
        "version":"1.0.1",
        "id":"bareapp",
        "main":"index.html",
        "icon":"icon.png",
        "folderPath":"/usr/palm/applications/bareapp",
        "title": "Bare App",
        "locationHint": "Location Hint",
        "v8ExtraFlags": "",
        "resolution":"800x600",
        "v8SnapshotFile":"v8SnapshotFileName.ext",
        "delayMsForLaunchOptimization":25,
        "suspendDOMTime":300,
        "accessibility":{
        "supportsAudioGuidance":true
        },
        "class":{
            "hidden":true
        },
        "keyFilterTable": [
            {
                "from": 1,
                "to": 2,
                "modifier": 3
            },
            {
                "from": 4,
                "to": 5,
                "modifier": 6
            },
            {
                "from": 7,
                "to": 8,
                "modifier": 9
            }
        ],
        "networkStableTimeout": 12345.6789,
        "supportedEnyoBundleVersions": [
            "Version 1.0.1",
            "Version 2.0.1",
            "Version 3.0.1"
        ],
        "windowGroup":{
            "name":"Window group name",
            "owner":true,
            "ownerInfo":{
                "allowAnonymous":false,
                "layers":[
                    {"name":"Owner layer name", "z":111}
                ]
            },
            "clientInfo":{
                "layer":"Client layer name",
                "hint":"Client layer hint"
            }
        }
    })";

}  // namespace

class ApplicationDescriptionTest : public ::testing::Test {
 public:
  ApplicationDescriptionTest()
      : application_description_(ApplicationDescription::FromJsonString(
            kApplicationDescriptionJson)) {}

  ~ApplicationDescriptionTest() override = default;

  void SetUp() override { ASSERT_TRUE(application_description_.get()); }

  std::unique_ptr<ApplicationDescription> application_description_;
};

TEST_F(ApplicationDescriptionTest, checkGetIsTransparent) {
  EXPECT_FALSE(application_description_->IsTransparent());
}

TEST_F(ApplicationDescriptionTest, checkGetHandlesRelaunch) {
  EXPECT_FALSE(application_description_->HandlesRelaunch());
}

TEST_F(ApplicationDescriptionTest, checkGetIsInspectable) {
  EXPECT_FALSE(application_description_->IsInspectable());
}

TEST_F(ApplicationDescriptionTest, checkGetUseCustomPlugin) {
  EXPECT_FALSE(application_description_->UseCustomPlugin());
}

TEST_F(ApplicationDescriptionTest, checkGetBackHistoryAPIDisabled) {
  EXPECT_FALSE(application_description_->BackHistoryAPIDisabled());
}

TEST_F(ApplicationDescriptionTest, checkGetDoNotTrack) {
  EXPECT_TRUE(application_description_->DoNotTrack());
}

TEST_F(ApplicationDescriptionTest, checkGetHandleExitKey) {
  EXPECT_TRUE(application_description_->HandleExitKey());
}

TEST_F(ApplicationDescriptionTest, checkGetIsEnableBackgroundRun) {
  EXPECT_FALSE(application_description_->IsEnableBackgroundRun());
}

TEST_F(ApplicationDescriptionTest, checkGetAllowVideoCapture) {
  EXPECT_TRUE(application_description_->AllowVideoCapture());
}

TEST_F(ApplicationDescriptionTest, checkGetAllowAudioCapture) {
  EXPECT_TRUE(application_description_->AllowAudioCapture());
}

TEST_F(ApplicationDescriptionTest, checkGetUseVirtualKeyboard) {
  EXPECT_FALSE(application_description_->UseVirtualKeyboard());
}

TEST_F(ApplicationDescriptionTest, checkGetUsePrerendering) {
  EXPECT_TRUE(application_description_->UsePrerendering());
}

TEST_F(ApplicationDescriptionTest, checkGetDisallowScrollingInMainFrame) {
  EXPECT_FALSE(application_description_->DisallowScrollingInMainFrame());
  EXPECT_TRUE(ApplicationDescription::FromJsonString(
                  "{\"disallowScrollingInMainFrame\":true}")
                  ->DisallowScrollingInMainFrame());
}

TEST_F(ApplicationDescriptionTest, checkMissedDisallowScrollingInMainFrame) {
  EXPECT_TRUE(ApplicationDescription::FromJsonString("{}")
                  ->DisallowScrollingInMainFrame());
}

TEST_F(ApplicationDescriptionTest, checkGetHandlesDeeplinking) {
  EXPECT_FALSE(application_description_->HandlesDeeplinking());
}

TEST_F(ApplicationDescriptionTest, checkGetUseUnlimitedMediaPolicy) {
  EXPECT_TRUE(application_description_->UseUnlimitedMediaPolicy());
}

TEST_F(ApplicationDescriptionTest, checkGetUseNativeScroll) {
  EXPECT_TRUE(application_description_->UseNativeScroll());
}

TEST_F(ApplicationDescriptionTest, checkGetTrustLevel) {
  EXPECT_STREQ("default", application_description_->TrustLevel().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetSubType) {
  EXPECT_STREQ("default", application_description_->SubType().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetDefaultWindowType) {
  EXPECT_STREQ("card", application_description_->DefaultWindowType().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetEnyoBundleVersion) {
  EXPECT_STREQ("Version 2.0.1",
               application_description_->EnyoBundleVersion().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetEnyoVersion) {
  EXPECT_STREQ("", application_description_->EnyoVersion().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetVersion) {
  EXPECT_STREQ("1.0.1", application_description_->Version().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetId) {
  EXPECT_STREQ("bareapp", application_description_->Id().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetEntryPoint) {
  struct stat info;
  if (stat("/usr/palm/applications/bareapp/index.html", &info))
    EXPECT_STREQ("index.html", application_description_->EntryPoint().c_str());
  else
    EXPECT_STREQ("file:///usr/palm/applications/bareapp/index.html",
                 application_description_->EntryPoint().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetIcon) {
  struct stat info;
  if (stat("/usr/palm/applications/bareapp/icon.png", &info))
    EXPECT_STREQ("icon.png", application_description_->Icon().c_str());
  else
    EXPECT_STREQ("/usr/palm/applications/bareapp/icon.png",
                 application_description_->Icon().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetFolderPath) {
  EXPECT_STREQ("/usr/palm/applications/bareapp",
               application_description_->FolderPath().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetTitle) {
  EXPECT_STREQ("Bare App", application_description_->Title().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetLocationHint) {
  EXPECT_STREQ("Location Hint",
               application_description_->LocationHint().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetV8ExtraFlags) {
  EXPECT_STREQ("", application_description_->V8ExtraFlags().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetV8SnapshotPath) {
  EXPECT_STREQ("/usr/palm/applications/bareapp/v8SnapshotFileName.ext",
               application_description_->V8SnapshotPath().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetWidthOverride) {
  EXPECT_EQ(800, application_description_->WidthOverride());
}

TEST_F(ApplicationDescriptionTest, checkGetHeightOverride) {
  EXPECT_EQ(600, application_description_->HeightOverride());
}

TEST_F(ApplicationDescriptionTest, checkGetDelayMsForLaunchOptimization) {
  EXPECT_EQ(25, application_description_->DelayMsForLaunchOptimization());
}

TEST_F(ApplicationDescriptionTest, checkGetCustomSuspendDomTime) {
  EXPECT_EQ(300, application_description_->CustomSuspendDOMTime());
}

TEST_F(ApplicationDescriptionTest, checkGetNetworkStableTimeout) {
  EXPECT_DOUBLE_EQ(12345.6789,
                   application_description_->NetworkStableTimeout());
}

TEST_F(ApplicationDescriptionTest, checkGetSupportsAudioGuidance) {
  EXPECT_TRUE(application_description_->SupportsAudioGuidance());
}

TEST_F(ApplicationDescriptionTest, checkGetWindowClassValue) {
  EXPECT_TRUE(ApplicationDescription::kWindowClassHidden ==
              application_description_->WindowClassValue());
}

TEST_F(ApplicationDescriptionTest, checkGetWindowGroupInfo) {
  const ApplicationDescription::WindowGroupInfo group_info =
      application_description_->GetWindowGroupInfo();

  EXPECT_STREQ("Window group name", group_info.name.c_str());
  EXPECT_TRUE(group_info.is_owner);
}

TEST_F(ApplicationDescriptionTest, checkGetWindowClientInfo) {
  const ApplicationDescription::WindowClientInfo client_info =
      application_description_->GetWindowClientInfo();

  EXPECT_STREQ("Client layer name", client_info.layer.c_str());
  EXPECT_STREQ("Client layer hint", client_info.hint.c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetWindowOwnertInfo) {
  const ApplicationDescription::WindowOwnerInfo owner_info =
      application_description_->GetWindowOwnerInfo();

  EXPECT_FALSE(owner_info.allow_anonymous);
  ASSERT_EQ(1, owner_info.layers.size());
  auto layer = owner_info.layers.begin();
  EXPECT_STREQ("Owner layer name", layer->first.c_str());
  EXPECT_EQ(111, layer->second);
}

TEST_F(ApplicationDescriptionTest, checkGetSupportedEnyoBundleVersions) {
  std::set<std::string> expected_versions = {"Version 1.0.1", "Version 2.0.1",
                                             "Version 3.0.1"};
  const auto actual_versions =
      application_description_->SupportedEnyoBundleVersions();

  EXPECT_EQ(expected_versions, actual_versions);
}

TEST_F(ApplicationDescriptionTest, checkGetKeyFilterTable) {
  std::unordered_map<int, std::pair<int, int>> expected_table = {
      {1, {2, 3}}, {4, {5, 6}}, {7, {8, 9}}};
  const auto actual_table = application_description_->KeyFilterTable();

  EXPECT_EQ(expected_table, actual_table);
}
