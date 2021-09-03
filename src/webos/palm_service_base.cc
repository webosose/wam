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

#include "palm_service_base.h"

#include "log_manager.h"
#include "utils.h"

PalmServiceBase::PalmServiceBase() : service_handle_(0) {}

PalmServiceBase::~PalmServiceBase() {
  StopService();
}

bool PalmServiceBase::StartService() {
  service_name_ = ServiceName();

  LSErrorSafe ls_error;

  if (!LSRegister(ServiceName(), &service_handle_, &ls_error)) {
    LOG_ERROR(MSGID_REG_LS2_FAIL, 2, PMLOGKS("SERVICE", ServiceName()),
              PMLOGKS("ERROR", ls_error.message), "");
    return false;
  }

  if (!LSRegisterCategory(service_handle_, Category(), Methods(),
                          NULL,  // LSSignal - ?
                          NULL, &ls_error)) {
    LOG_ERROR(MSGID_REG_LS2_CAT_FAIL, 2, PMLOGKS("SERVICE", ServiceName()),
              PMLOGKS("ERROR", ls_error.message), "");
    StopService();
    return false;
  }

  if (!LSCategorySetData(service_handle_, Category(), this, &ls_error)) {
    LOG_ERROR(MSGID_REG_LS2_CAT_FAIL, 2, PMLOGKS("SERVICE", ServiceName()),
              PMLOGKS("ERROR", ls_error.message), "");
    StopService();
    return false;
  }

  if (!LSGmainAttach(service_handle_, MainLoop(), &ls_error)) {
    LOG_ERROR(MSGID_REG_LS2_ATTACH_FAIL, 2, PMLOGKS("SERVICE", ServiceName()),
              PMLOGKS("ERROR", ls_error.message), "");
    StopService();
    return false;
  }
  LOG_DEBUG("Successfully registered %s on service bus", ServiceName());

  DidConnect();

  return true;
}

bool PalmServiceBase::StopService() {
  if (!service_handle_)
    return true;

  LSErrorSafe lsError;
  if (!LSUnregister(service_handle_, &lsError)) {
    service_handle_ = 0;
    return true;
  }

  LOG_WARNING(MSGID_UNREG_LS2_FAIL, 2,
              PMLOGKS("SERVICE", service_name_.c_str()),
              PMLOGKS("ERROR", lsError.message), "");

  return false;
}

bool PalmServiceBase::Call(LSHandle* handle,
                           const char* what,
                           Json::Value parameters,
                           const char* applicationId = 0,
                           LSCalloutContext* context = 0) {
  std::string parameters_str = util::JsonToString(parameters);
  if (!parameters.isObject()) {
    LOG_WARNING(MSGID_LS2_CALL_FAIL, 2, PMLOGKS("SERVICE", ServiceName()),
                PMLOGKFV("parameters", "%s", parameters_str.c_str()), "");
    return false;
  }

  LSErrorSafe ls_error;
  bool call_ret;
  if (parameters["subscribe"] == true || parameters["watch"] == true) {
    if (context) {
      call_ret = LSCallFromApplication(
          handle, what, parameters_str.c_str(), applicationId,
          LSCallbackHandler::Callback, static_cast<LSCallbackHandler*>(context),
          &context->token_, &ls_error);
      context->service_ = handle;
    } else {
      // caller does not care about reply from call
      call_ret = LSCallFromApplication(handle, what, parameters_str.c_str(),
                                       applicationId, 0, 0, 0, &ls_error);
    }
  } else {
    if (context) {
      call_ret = LSCallFromApplicationOneReply(
          handle, what, parameters_str.c_str(), applicationId,
          LSCallbackHandler::Callback, static_cast<LSCallbackHandler*>(context),
          &context->token_, &ls_error);
      context->service_ = handle;
    } else {
      // caller does not care about reply from call
      call_ret =
          LSCallFromApplicationOneReply(handle, what, parameters_str.c_str(),
                                        applicationId, 0, 0, 0, &ls_error);
    }
  }
  if (!call_ret) {
    LOG_WARNING(MSGID_LS2_CALL_FAIL, 2, PMLOGKS("SERVICE", ServiceName()),
                PMLOGKS("ERROR", ls_error.message), "");
    return false;
  }
  return true;
}

GMainLoop* PalmServiceBase::MainLoop() const {
  static GMainLoop* s_main_loop = nullptr;
  if (!s_main_loop)
    s_main_loop = g_main_loop_new(nullptr, false);
  return s_main_loop;
}

bool LSCalloutContext::Cancel() {
  if (token_ == LSMESSAGE_TOKEN_INVALID || service_ == 0) {
    LOG_WARNING(MSGID_LS2_CANCEL_NOT_ACTIVE, 0,
                "callout context not cancelled: not active");
    return false;
  }

  LSErrorSafe ls_error;

  if (!LSCallCancel(service_, token_, &ls_error)) {
    LOG_WARNING(MSGID_LS2_CANCEL_FAIL, 1, PMLOGKS("ERROR", ls_error.message),
                "Failed to cancel service call");
    return false;
  }

  token_ = LSMESSAGE_TOKEN_INVALID;
  return true;
}
