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
#include <json/json.h>

#include "network_status.h"

TEST(NetworkStatusTest, NetworkStatusTestConnection) {
  Json::Value json_information;
  json_information["netmask"] = "255.255.255.0";
  json_information["dns1"] = "192.168.0.1";
  json_information["dns2"] = "192.168.0.2";
  json_information["ipAddress"] = "192.168.0.3";
  json_information["method"] = "direct";
  json_information["state"] = "online";
  json_information["gateway"] = "192.168.0.1";
  json_information["interfaceName"] = "eth1";
  json_information["onInternet"] = "yes";

  Json::Value wired_status;
  wired_status["returnValue"] = true;
  wired_status["isInternetConnectionAvailable"] = true;
  wired_status["wired"] = json_information;

  Json::Value wifi_status;
  wifi_status["returnValue"] = true;
  wifi_status["isInternetConnectionAvailable"] = true;
  wifi_status["wifi"] = json_information;

  Json::Value wifidirect_status;
  wifidirect_status["returnValue"] = true;
  wifidirect_status["isInternetConnectionAvailable"] = true;
  wifidirect_status["wifiDirect"] = json_information;

  NetworkStatus status;
  status.FromJsonObject(wired_status);
  EXPECT_TRUE(status.Type() == "wired");
  EXPECT_TRUE(status.IsInternetConnectionAvailable());
  NetworkStatus::Information object_info = status.GetInformation();
  EXPECT_TRUE(object_info.Netmask() == "255.255.255.0");
  EXPECT_TRUE(object_info.Dns1() == "192.168.0.1");
  EXPECT_TRUE(object_info.Dns2() == "192.168.0.2");
  EXPECT_TRUE(object_info.IpAddress() == "192.168.0.3");
  EXPECT_TRUE(object_info.Method() == "direct");
  EXPECT_TRUE(object_info.State() == "online");
  EXPECT_TRUE(object_info.Gateway() == "192.168.0.1");
  EXPECT_TRUE(object_info.InterfaceName() == "eth1");
  EXPECT_TRUE(object_info.OnInternet() == "yes");

  status.FromJsonObject(wifi_status);
  EXPECT_TRUE(status.Type() == "wifi");

  status.FromJsonObject(wifidirect_status);
  EXPECT_TRUE(status.Type() == "wifiDirect");
}

TEST(NetworkStatusTest, NetworkStatusTestNoConnection) {
  Json::Value json_status;
  json_status["returnValue"] = true;
  json_status["isInternetConnectionAvailable"] = false;
  NetworkStatus status;
  status.FromJsonObject(json_status);
  EXPECT_FALSE(status.IsInternetConnectionAvailable());
}
