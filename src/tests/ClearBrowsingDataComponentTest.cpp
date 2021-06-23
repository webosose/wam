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
#include <gmock/gmock.h>

#include <QJsonDocument>
#include <QJsonObject>

#include "BaseMockInitializer.h"
#include "WebAppManagerServiceLuna.h"
#include "webos/webview_profile.h"

TEST(ClearBrowsingData, invalidValue) {
    const char json_parameter[] = R"({
        "types":[1]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    ASSERT_TRUE(reply.contains("errorText"));
    ASSERT_TRUE(reply.contains("errorCode"));
    EXPECT_FALSE(reply["returnValue"].toBool());
    EXPECT_EQ(ERR_CODE_CLEAR_DATA_BRAWSING_INVALID_VALUE, reply["errorCode"].toInt());
}

TEST(ClearBrowsingData, emptyArray) {
    const char json_parameter[] = R"({
        "types":[]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    ASSERT_TRUE(reply.contains("errorText"));
    ASSERT_TRUE(reply.contains("errorCode"));
    EXPECT_FALSE(reply["returnValue"].toBool());
    EXPECT_EQ(ERR_CODE_CLEAR_DATA_BRAWSING_EMPTY_ARRAY, reply["errorCode"].toInt());
}

TEST(ClearBrowsingData, unknownData) {
    const char json_parameter[] = R"({
        "types":["unknown"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    ASSERT_TRUE(reply.contains("errorText"));
    ASSERT_TRUE(reply.contains("errorCode"));
    EXPECT_FALSE(reply["returnValue"].toBool());
    EXPECT_EQ(ERR_CODE_CLEAR_DATA_BRAWSING_UNKNOWN_DATA, reply["errorCode"].toInt());
}

TEST(ClearBrowsingData, all) {
    const char json_parameter[] = R"({
        "types":["all"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(ClearBrowsingData, appcache) {
    const char json_parameter[] = R"({
        "types":["appcache"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(ClearBrowsingData, cache) {
    const char json_parameter[] = R"({
        "types":["cache"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(ClearBrowsingData, codecache) {
    const char json_parameter[] = R"({
        "types":["codecache"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(ClearBrowsingData, cookies) {
    const char json_parameter[] = R"({
        "types":["codecache"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(ClearBrowsingData, fileSystems) {
    const char json_parameter[] = R"({
        "types":["fileSystems"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(ClearBrowsingData, localStorage) {
    const char json_parameter[] = R"({
        "types":["localStorage"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(ClearBrowsingData, channelIDs) {
    const char json_parameter[] = R"({
        "types":["channelIDs"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(ClearBrowsingData, serviceWorkers) {
    const char json_parameter[] = R"({
        "types":["serviceWorkers"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(ClearBrowsingData, cacheStorage) {
    const char json_parameter[] = R"({
        "types":["cacheStorage"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["returnValue"].toBool());
}

TEST(ClearBrowsingData, webSQL) {
    const char json_parameter[] = R"({
        "types":["webSQL"]
    })";

    BaseMockInitializer<> mockInitializer;
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(json_parameter).toUtf8(), &parseError);
    ASSERT_EQ(parseError.error, QJsonParseError::NoError);

    QJsonObject reply = WebAppManagerServiceLuna::instance()->clearBrowsingData(doc.object());
    ASSERT_TRUE(reply.contains("returnValue"));
    EXPECT_TRUE(reply["returnValue"].toBool());
}
