//  (c) 2021 LG Electronics, Inc.
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

#include <gtest/gtest.h>
#include <QJsonDocument>
#include <QJsonObject>
#include "WebAppManagerServiceLuna.h"

TEST(LogControl, SetEventOn) {
    const char json_parameters[] = R"({
        "keys":"event",
        "value":"on"
    })";

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameters).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->logControl(doc.object());

    ASSERT_TRUE(reply.contains("event"));
    ASSERT_TRUE(reply.contains("bundleMessage"));
    ASSERT_TRUE(reply.contains("mouseMove"));
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["event"].toBool());
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(LogControl, SetEventOff) {
    const char json_parameters[] = R"({
        "keys":"event",
        "value":"off"
    })";

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameters).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->logControl(doc.object());

    ASSERT_TRUE(reply.contains("event"));
    ASSERT_TRUE(reply.contains("bundleMessage"));
    ASSERT_TRUE(reply.contains("mouseMove"));
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_FALSE(reply["event"].toBool());
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(LogControl, SetBundleMessageOn) {
    const char json_parameters[] = R"({
        "keys":"bundleMessage",
        "value":"on"
    })";

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameters).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->logControl(doc.object());

    ASSERT_TRUE(reply.contains("event"));
    ASSERT_TRUE(reply.contains("bundleMessage"));
    ASSERT_TRUE(reply.contains("mouseMove"));
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["bundleMessage"].toBool());
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(LogControl, SetBundleMessageOff) {
    const char json_parameters[] = R"({
        "keys":"bundleMessage",
        "value":"off"
    })";

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameters).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->logControl(doc.object());

    ASSERT_TRUE(reply.contains("event"));
    ASSERT_TRUE(reply.contains("bundleMessage"));
    ASSERT_TRUE(reply.contains("mouseMove"));
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_FALSE(reply["bundleMessage"].toBool());
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(LogControl, SetMouseMoveOn) {
    const char json_parameters[] = R"({
        "keys":"mouseMove",
        "value":"on"
    })";

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameters).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->logControl(doc.object());

    ASSERT_TRUE(reply.contains("event"));
    ASSERT_TRUE(reply.contains("bundleMessage"));
    ASSERT_TRUE(reply.contains("mouseMove"));
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["mouseMove"].toBool());
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(LogControl, SetMouseMoveOff) {
    const char json_parameters[] = R"({
        "keys":"mouseMove",
        "value":"off"
    })";

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameters).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->logControl(doc.object());

    ASSERT_TRUE(reply.contains("event"));
    ASSERT_TRUE(reply.contains("bundleMessage"));
    ASSERT_TRUE(reply.contains("mouseMove"));
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_FALSE(reply["mouseMove"].toBool());
    EXPECT_TRUE(reply["returnValue"].toBool());
}
