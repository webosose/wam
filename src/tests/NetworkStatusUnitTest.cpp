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

#include "NetworkStatus.h"

TEST(NetworkStatusTest, NetworkStatusTestConnection) {
  QJsonObject json_information{
      {"netmask", "255.255.255.0"}, {"dns1", "192.168.0.1"},
      {"dns2", "192.168.0.2"},      {"ipAddress", "192.168.0.3"},
      {"method", "direct"},         {"state", "online"},
      {"gateway", "192.168.0.1"},   {"interfaceName", "eth1"},
      {"onInternet", "yes"},
  };

  QJsonObject wired_status{{"returnValue", true},
                           {"isInternetConnectionAvailable", true}};
  wired_status.insert("wired", json_information);

  QJsonObject wifi_status{{"returnValue", true},
                          {"isInternetConnectionAvailable", true}};
  wifi_status.insert("wifi", json_information);

  QJsonObject wifidirect_status{{"returnValue", true},
                                {"isInternetConnectionAvailable", true}};
  wifidirect_status.insert("wifiDirect", json_information);

  NetworkStatus status;
  status.fromJsonObject(wired_status);
  EXPECT_TRUE(status.type() == "wired");
  EXPECT_TRUE(status.isInternetConnectionAvailable());
  NetworkStatus::Information object_info = status.information();
  EXPECT_TRUE(object_info.netmask() == "255.255.255.0");
  EXPECT_TRUE(object_info.dns1() == "192.168.0.1");
  EXPECT_TRUE(object_info.dns2() == "192.168.0.2");
  EXPECT_TRUE(object_info.ipAddress() == "192.168.0.3");
  EXPECT_TRUE(object_info.method() == "direct");
  EXPECT_TRUE(object_info.state() == "online");
  EXPECT_TRUE(object_info.gateway() == "192.168.0.1");
  EXPECT_TRUE(object_info.interfaceName() == "eth1");
  EXPECT_TRUE(object_info.onInternet() == "yes");

  status.fromJsonObject(wifi_status);
  EXPECT_TRUE(status.type() == "wifi");

  status.fromJsonObject(wifidirect_status);
  EXPECT_TRUE(status.type() == "wifiDirect");
}

TEST(NetworkStatusTest, NetworkStatusTestNoConnection) {
  QJsonObject json_status{{"returnValue", true},
                          {"isInternetConnectionAvailable", false}};
  NetworkStatus status;
  status.fromJsonObject(json_status);
  EXPECT_FALSE(status.isInternetConnectionAvailable());
}
