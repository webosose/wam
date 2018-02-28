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

#include "PalmServiceBase.h"
#include "LogManager.h"

PalmServiceBase::PalmServiceBase()
    : m_serviceHandle(0)
    , m_serviceHandlePublic(0)
    , m_serviceHandlePrivate(0)
{
}

PalmServiceBase::~PalmServiceBase()
{
    stopService();
}

bool PalmServiceBase::startService()
{
    LSErrorSafe lsError;

    if (!LSRegisterPalmService(serviceName(), &m_serviceHandle, &lsError) ) {
        LOG_ERROR(MSGID_REG_LS2_FAIL, 2,
                  PMLOGKS("SERVICE", serviceName()),
                  PMLOGKS("ERROR", lsError.message), "");
       return false;
    }

    if(!LSPalmServiceRegisterCategory(m_serviceHandle, category(),
            publicMethods(),
            privateMethods(),
            NULL, //LSSignal - ?
            this, //user data - used to call into class instance again
            &lsError)) {
        LOG_ERROR(MSGID_REG_LS2_CAT_FAIL, 2,
                  PMLOGKS("SERVICE", serviceName()),
                  PMLOGKS("ERROR", lsError.message), "");
       return false;
    }

    m_serviceHandlePublic =  LSPalmServiceGetPublicConnection(m_serviceHandle);
    m_serviceHandlePrivate = LSPalmServiceGetPrivateConnection(m_serviceHandle);

    if (!LSGmainAttachPalmService(m_serviceHandle, mainLoop(), &lsError)) {
        LOG_ERROR(MSGID_REG_LS2_ATTACH_FAIL, 2,
                  PMLOGKS("SERVICE", serviceName()),
                  PMLOGKS("ERROR", lsError.message), "");
        return false;
    }
    LOG_DEBUG("Successfully registered %s on service bus", serviceName());

    didConnect();

    return true;
}

void PalmServiceBase::stopService()
{
    LSErrorSafe lsError;
    if (!LSUnregisterPalmService(m_serviceHandle, &lsError) ) {
        LOG_WARNING(MSGID_UNREG_LS2_FAIL, 2,
                    PMLOGKS("SERVICE", serviceName()),
                    PMLOGKS("ERROR", lsError.message), "");
       return;
    }

}

bool PalmServiceBase::call(
        LSHandle* handle,
        const char* what,
        QJsonObject parameters,
        const char* applicationId = 0,
        LSCalloutContext* context = 0)
{
    LSErrorSafe lsError;
    bool callRet;
    if(parameters.value("subscribe").toBool() || parameters.value("watch").toBool()) {
        if(context) {
            callRet = LSCallFromApplication(handle,
                    what,
                    QJsonDocument(parameters).toJson().data(),
                    applicationId,
                    LSCallbackHandler::callback,
                    static_cast<LSCallbackHandler*>(context),
                    &context->m_token,
                    &lsError);
            context->m_service = handle;
        } else {
            //caller does not care about reply from call
            callRet = LSCallFromApplication(handle,
                    what,
                    QJsonDocument(parameters).toJson().data(),
                    applicationId,
                    0, 0, 0,
                    &lsError);
        }
    } else {
        if(context) {
            callRet = LSCallFromApplicationOneReply(handle,
                    what,
                    QJsonDocument(parameters).toJson().data(),
                    applicationId,
                    LSCallbackHandler::callback,
                    static_cast<LSCallbackHandler*>(context),
                    &context->m_token,
                    &lsError);
            context->m_service = handle;
        } else {
            //caller does not care about reply from call
            callRet = LSCallFromApplicationOneReply(handle,
                    what,
                    QJsonDocument(parameters).toJson().data(),
                    applicationId,
                    0, 0, 0,
                    &lsError);
        }
    }
    if(!callRet) {
        LOG_WARNING(MSGID_LS2_CALL_FAIL, 2,
                    PMLOGKS("SERVICE", serviceName()),
                    PMLOGKS("ERROR", lsError.message), "");
        return false;
    }
    return true;
}

GMainLoop* PalmServiceBase::mainLoop() const {
  static GMainLoop* s_mainLoop = NULL;
  if (!s_mainLoop)
      s_mainLoop = g_main_loop_new(NULL, false);
  return s_mainLoop;
}

bool LSCalloutContext::cancel() {
        if (m_token == LSMESSAGE_TOKEN_INVALID || m_service == 0) {
            LOG_WARNING(MSGID_LS2_CANCEL_NOT_ACTIVE, 0, "callout context not cancelled: not active");
            return false;
        }

        LSErrorSafe lsError;

        if (!LSCallCancel(m_service, m_token, &lsError)) {
            LOG_WARNING(MSGID_LS2_CANCEL_FAIL, 1,
                        PMLOGKS("ERROR", lsError.message), "Failed to cancel service call");
            return false;
        }

        m_token = LSMESSAGE_TOKEN_INVALID;
        return true;
}



