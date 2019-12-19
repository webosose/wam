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
{
}

PalmServiceBase::~PalmServiceBase()
{
    stopService();
}

bool PalmServiceBase::startService()
{
    m_serviceName = serviceName();

    LSErrorSafe lsError;

    if (!LSRegister(serviceName(), &m_serviceHandle, &lsError)) {
        LOG_ERROR(MSGID_REG_LS2_FAIL, 2,
                  PMLOGKS("SERVICE", serviceName()),
                  PMLOGKS("ERROR", lsError.message), "");
       return false;
    }

    if (!LSRegisterCategory(m_serviceHandle, category(),
            methods(),
            NULL, //LSSignal - ?
            NULL,
            &lsError)) {
        LOG_ERROR(MSGID_REG_LS2_CAT_FAIL, 2,
                  PMLOGKS("SERVICE", serviceName()),
                  PMLOGKS("ERROR", lsError.message), "");
        stopService();
        return false;
    }

    if (!LSCategorySetData(m_serviceHandle, category(), this, &lsError)) {
        LOG_ERROR(MSGID_REG_LS2_CAT_FAIL, 2,
            PMLOGKS("SERVICE", serviceName()),
            PMLOGKS("ERROR", lsError.message), "");
        stopService();
        return false;
    }

    if (!LSGmainAttach(m_serviceHandle, mainLoop(), &lsError)) {
        LOG_ERROR(MSGID_REG_LS2_ATTACH_FAIL, 2,
                  PMLOGKS("SERVICE", serviceName()),
                  PMLOGKS("ERROR", lsError.message), "");
        stopService();
        return false;
    }
    LOG_DEBUG("Successfully registered %s on service bus", serviceName());

    didConnect();

    return true;
}

bool PalmServiceBase::stopService()
{
    if (!m_serviceHandle)
        return true;

    LSErrorSafe lsError;
    if (!LSUnregister(m_serviceHandle, &lsError)) {
        m_serviceHandle = 0;
        return true;
    }

    LOG_WARNING(MSGID_UNREG_LS2_FAIL, 2,
            PMLOGKS("SERVICE", m_serviceName.c_str()),
            PMLOGKS("ERROR", lsError.message), "");

    return false;
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



