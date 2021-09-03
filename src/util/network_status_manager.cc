// Copyright (c) 2008-2018 LG Electronics, Inc.
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

#include "network_status_manager.h"

#include "log_manager.h"

void NetworkStatusManager::UpdateNetworkStatus(const NetworkStatus& status) {
  if (current_.Type() != status.Type())
    AppendLogList(status.Type(), current_.Type(), status.Type());

  CheckInformationChange(status.GetInformation());
  if (log_list_.size() > 0) {  // one more information was changed
    AppendLogList("date", current_.SavedDate(), status.SavedDate());
    PrintLog();
    current_ = status;
  }
}

void NetworkStatusManager::CheckInformationChange(
    const NetworkStatus::Information& info) {
  if (current_.GetInformation().IpAddress() != info.IpAddress())
    AppendLogList("ipAddress", current_.GetInformation().IpAddress(),
                  info.IpAddress());
  if (current_.GetInformation().Dns1() != info.Dns1())
    AppendLogList("dns1", current_.GetInformation().Dns1(), info.Dns1());
  if (current_.GetInformation().Dns2() != info.Dns2())
    AppendLogList("dns2", current_.GetInformation().Dns2(), info.Dns2());
  if (current_.GetInformation().Method() != info.Method())
    AppendLogList("method", current_.GetInformation().Method(), info.Method());
  if (current_.GetInformation().State() != info.State())
    AppendLogList("state", current_.GetInformation().State(), info.State());
  if (current_.GetInformation().Gateway() != info.Gateway())
    AppendLogList("gateway", current_.GetInformation().Gateway(),
                  info.Gateway());
  if (current_.GetInformation().InterfaceName() != info.InterfaceName())
    AppendLogList("interfaceName", current_.GetInformation().InterfaceName(),
                  info.InterfaceName());
  if (current_.GetInformation().OnInternet() != info.OnInternet())
    AppendLogList("onInternet", current_.GetInformation().OnInternet(),
                  info.OnInternet());
}

void NetworkStatusManager::AppendLogList(const std::string& key,
                                         const std::string& previous,
                                         const std::string& current) {
  log_list_.emplace(std::piecewise_construct, std::forward_as_tuple(key),
                    std::forward_as_tuple(previous, current));
}

void NetworkStatusManager::PrintLog() {
  for (const auto& line : log_list_) {
    LOG_INFO(MSGID_NETWORKSTATUS_INFO, 3, PMLOGKS("CHANGE", line.first.c_str()),
             PMLOGKS("Previous", line.second.first.c_str()),
             PMLOGKS("Current", line.second.second.c_str()), "");
  }
  log_list_.clear();
}
