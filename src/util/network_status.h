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

#ifndef UTIL_NETWORK_STATUS_H_
#define UTIL_NETWORK_STATUS_H_

#include <string>

namespace Json {
class Value;
};

class NetworkStatus {
 public:
  NetworkStatus();

  class Information {
   public:
    void FromJsonObject(const Json::Value& info);
    std::string Netmask() const { return netmask_; }
    std::string Dns1() const { return dns1_; }
    std::string Dns2() const { return dns2_; }
    std::string IpAddress() const { return ip_address_; }
    std::string Method() const { return method_; }
    std::string State() const { return state_; }
    std::string Gateway() const { return gateway_; }
    std::string InterfaceName() const { return interface_name_; }
    std::string OnInternet() const { return on_internet_; }

   private:
    std::string netmask_;
    std::string dns1_;
    std::string dns2_;
    std::string ip_address_;
    std::string method_;
    std::string state_;
    std::string gateway_;
    std::string interface_name_;
    std::string on_internet_;
  };

  void FromJsonObject(const Json::Value& object);
  std::string Type() const { return type_; }
  Information GetInformation() const { return information_; }
  std::string SavedDate() const { return saved_date_; }
  bool IsInternetConnectionAvailable() {
    return is_internet_connection_available_;
  }

 private:
  std::string type_;
  Information information_;
  bool is_internet_connection_available_;
  bool return_value_;
  std::string saved_date_;
};

#endif  // UTIL_NETWORK_STATUS_H_
