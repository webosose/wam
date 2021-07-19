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

#include "ApplicationDescription.h"

namespace {

    const char *applicationDescriptionJson = R"({
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

} // namespace

class ApplicationDescriptionTest : public ::testing::Test {
public:
    ApplicationDescriptionTest()
        : m_applicationDescription(ApplicationDescription::fromJsonString(applicationDescriptionJson))
    {
    }

    ~ApplicationDescriptionTest() override = default;

    void SetUp() override
    {
        ASSERT_TRUE(m_applicationDescription.get());
    }

    std::unique_ptr<ApplicationDescription> m_applicationDescription;
};

TEST_F(ApplicationDescriptionTest, checkGetIsTransparent)
{
    EXPECT_FALSE(m_applicationDescription.get()->isTransparent());
}

TEST_F(ApplicationDescriptionTest, checkGetHandlesRelaunch)
{
    EXPECT_FALSE(m_applicationDescription.get()->handlesRelaunch());
}

TEST_F(ApplicationDescriptionTest, checkGetIsInspectable)
{
    EXPECT_FALSE(m_applicationDescription.get()->isInspectable());
}

TEST_F(ApplicationDescriptionTest, checkGetUseCustomPlugin)
{
    EXPECT_FALSE(m_applicationDescription.get()->useCustomPlugin());
}

TEST_F(ApplicationDescriptionTest, checkGetBackHistoryAPIDisabled)
{
    EXPECT_FALSE(m_applicationDescription.get()->backHistoryAPIDisabled());
}

TEST_F(ApplicationDescriptionTest, checkGetDoNotTrack)
{
    EXPECT_TRUE(m_applicationDescription.get()->doNotTrack());
}

TEST_F(ApplicationDescriptionTest, checkGetHandleExitKey)
{
    EXPECT_TRUE(m_applicationDescription.get()->handleExitKey());
}

TEST_F(ApplicationDescriptionTest, checkGetIsEnableBackgroundRun)
{
    EXPECT_FALSE(m_applicationDescription.get()->isEnableBackgroundRun());
}

TEST_F(ApplicationDescriptionTest, checkGetAllowVideoCapture)
{
    EXPECT_TRUE(m_applicationDescription.get()->allowVideoCapture());
}

TEST_F(ApplicationDescriptionTest, checkGetAllowAudioCapture)
{
    EXPECT_TRUE(m_applicationDescription.get()->allowAudioCapture());
}

TEST_F(ApplicationDescriptionTest, checkGetUseVirtualKeyboard)
{
    EXPECT_FALSE(m_applicationDescription.get()->useVirtualKeyboard());
}

TEST_F(ApplicationDescriptionTest, checkGetUsePrerendering)
{
    EXPECT_TRUE(m_applicationDescription.get()->usePrerendering());
}

TEST_F(ApplicationDescriptionTest, checkGetDisallowScrollingInMainFrame)
{
    EXPECT_FALSE(m_applicationDescription.get()->disallowScrollingInMainFrame());
}

TEST_F(ApplicationDescriptionTest, checkGetHandlesDeeplinking)
{
    EXPECT_FALSE(m_applicationDescription.get()->handlesDeeplinking());
}

TEST_F(ApplicationDescriptionTest, checkGetUseUnlimitedMediaPolicy)
{
    EXPECT_TRUE(m_applicationDescription.get()->useUnlimitedMediaPolicy());
}

TEST_F(ApplicationDescriptionTest, checkGetUseNativeScroll)
{
    EXPECT_TRUE(m_applicationDescription.get()->useNativeScroll());
}

TEST_F(ApplicationDescriptionTest, checkGetTrustLevel)
{
    EXPECT_STREQ("default", m_applicationDescription.get()->trustLevel().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetSubType)
{
    EXPECT_STREQ("default", m_applicationDescription.get()->subType().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetDefaultWindowType)
{
    EXPECT_STREQ("card", m_applicationDescription.get()->defaultWindowType().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetEnyoBundleVersion)
{
    EXPECT_STREQ("Version 2.0.1", m_applicationDescription.get()->enyoBundleVersion().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetEnyoVersion)
{
    EXPECT_STREQ("", m_applicationDescription.get()->enyoVersion().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetVersion)
{
    EXPECT_STREQ("1.0.1", m_applicationDescription.get()->version().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetId)
{
    EXPECT_STREQ("bareapp", m_applicationDescription.get()->id().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetEntryPoint)
{
    struct stat info;
    if(stat("/usr/palm/applications/bareapp/index.html", &info))
        EXPECT_STREQ("index.html", m_applicationDescription.get()->entryPoint().c_str());
    else
        EXPECT_STREQ("file:///usr/palm/applications/bareapp/index.html",
                     m_applicationDescription.get()->entryPoint().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetIcon)
{
    struct stat info;
    if(stat("/usr/palm/applications/bareapp/icon.png", &info))
        EXPECT_STREQ("icon.png", m_applicationDescription.get()->icon().c_str());
    else
        EXPECT_STREQ("/usr/palm/applications/bareapp/icon.png",
                     m_applicationDescription.get()->icon().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetFolderPath)
{
    EXPECT_STREQ("/usr/palm/applications/bareapp", m_applicationDescription.get()->folderPath().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetTitle)
{
    EXPECT_STREQ("Bare App", m_applicationDescription.get()->title().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetLocationHint)
{
    EXPECT_STREQ("Location Hint", m_applicationDescription.get()->locationHint().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetV8ExtraFlags)
{
    EXPECT_STREQ("", m_applicationDescription.get()->v8ExtraFlags().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetV8SnapshotPath)
{
    EXPECT_STREQ("/usr/palm/applications/bareapp/v8SnapshotFileName.ext",
                 m_applicationDescription.get()->v8SnapshotPath().c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetWidthOverride)
{
    EXPECT_EQ(800, m_applicationDescription.get()->widthOverride());
}

TEST_F(ApplicationDescriptionTest, checkGetHeightOverride)
{
    EXPECT_EQ(600, m_applicationDescription.get()->heightOverride());
}

TEST_F(ApplicationDescriptionTest, checkGetDelayMsForLaunchOptimization)
{
    EXPECT_EQ(25, m_applicationDescription.get()->delayMsForLaunchOptimization());
}

TEST_F(ApplicationDescriptionTest, checkGetCustomSuspendDomTime)
{
    EXPECT_EQ(300, m_applicationDescription.get()->customSuspendDOMTime());
}

TEST_F(ApplicationDescriptionTest, checkGetNetworkStableTimeout)
{
    EXPECT_DOUBLE_EQ(12345.6789, m_applicationDescription.get()->networkStableTimeout());
}

TEST_F(ApplicationDescriptionTest, checkGetSupportsAudioGuidance)
{
    EXPECT_TRUE(m_applicationDescription.get()->supportsAudioGuidance());
}

TEST_F(ApplicationDescriptionTest, checkGetWindowClassValue)
{
    EXPECT_TRUE(ApplicationDescription::WindowClass_Hidden ==
                m_applicationDescription.get()->windowClassValue());
}

TEST_F(ApplicationDescriptionTest, checkGetWindowGroupInfo)
{
    const ApplicationDescription::WindowGroupInfo groupInfo =
        m_applicationDescription.get()->getWindowGroupInfo();

    EXPECT_STREQ("Window group name", groupInfo.name.c_str());
    EXPECT_TRUE(groupInfo.isOwner);
}

TEST_F(ApplicationDescriptionTest, checkGetWindowClientInfo)
{
    const ApplicationDescription::WindowClientInfo clientInfo =
        m_applicationDescription.get()->getWindowClientInfo();

    EXPECT_STREQ("Client layer name", clientInfo.layer.c_str());
    EXPECT_STREQ("Client layer hint", clientInfo.hint.c_str());
}

TEST_F(ApplicationDescriptionTest, checkGetWindowOwnertInfo)
{
    const ApplicationDescription::WindowOwnerInfo ownerInfo =
        m_applicationDescription.get()->getWindowOwnerInfo();

    EXPECT_FALSE(ownerInfo.allowAnonymous);
    ASSERT_EQ(1, ownerInfo.layers.size());
    auto layer = ownerInfo.layers.begin();
    EXPECT_STREQ("Owner layer name", layer->first.c_str());
    EXPECT_EQ(111, layer->second);
}

TEST_F(ApplicationDescriptionTest, checkGetSupportedEnyoBundleVersions)
{
    std::set<std::string> expectedVersions = {
        "Version 1.0.1",
        "Version 2.0.1",
        "Version 3.0.1"};
    const auto actualVersions = m_applicationDescription.get()->supportedEnyoBundleVersions();

    EXPECT_EQ(expectedVersions, actualVersions);
}

TEST_F(ApplicationDescriptionTest, checkGetKeyFilterTable)
{
    std::unordered_map<int, std::pair<int, int>> expectedTable = {
        {1, {2, 3}},
        {4, {5, 6}},
        {7, {8, 9}}};
    const auto actualTable = m_applicationDescription.get()->keyFilterTable();

    EXPECT_EQ(expectedTable, actualTable);
}
