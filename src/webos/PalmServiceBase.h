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

#ifndef PalmServiceBase_H
#define PalmServiceBase_H

#include <glib.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <luna-service2/lunaservice.h>

class LSHandle;
class LSMessage;
class LSPalmService;
class PalmServiceBase;

/*
 * helper class that makes LSError clean up after itself.
 * significantly simplifies LS call sequences
 */

class LSErrorSafe : public LSError {
public:
    LSErrorSafe()
    {
        LSErrorInit(this);
    };
    ~LSErrorSafe()
    {
        LSErrorFree(this);
    }
};

/*
 * This class allows us to call into LS2 and have the reply be forwarded to a
 * Qt slot or Q_INVOKABLE function of some object of the signature
 * QJsonObject handlerFunc(QJsonObject payload)
 *
 * */
class LSCallbackHandler : public QObject {
    Q_OBJECT
public:
    LSCallbackHandler(QObject* receiver, const char* slot)
        : m_receiver(receiver)
        , m_slot(slot)
    {
    }

    virtual ~LSCallbackHandler() {}

protected:
    QJsonObject called(QJsonObject payload)
    {
        QJsonObject retVal;
        QMetaObject::invokeMethod(m_receiver, m_slot,
            Q_RETURN_ARG(QJsonObject, retVal),
            Q_ARG(QJsonObject, payload));
        return retVal;
    }

    static bool callback(LSHandle* handle, LSMessage* message, void* user_data)
    {
        LSErrorSafe lsError;

        if (!message) {
            if (!LSMessageReply(handle, message, "{\"returnValue\": false}", &lsError))
                return false;
            return true;
        }

        QJsonObject request = QJsonDocument::fromJson(LSMessageGetPayload(message)).object();
        QJsonObject reply;

        reply = static_cast<LSCallbackHandler*>(user_data)->called(request);

        if (!reply.isEmpty())
            return LSMessageReply(handle, message, QJsonDocument(reply).toJson().data(), &lsError);
        else
            return true;
    }

    QObject* m_receiver;
    const char* m_slot;
};

/**
 * A class to make a LS2 call cancellable by providing storage for a token
 */
class LSCalloutContext : public LSCallbackHandler {
    friend class PalmServiceBase;

public:
    LSCalloutContext(QObject* receiver, const char* slot)
        : LSCallbackHandler(receiver, slot)
        , m_service(0)
        , m_token(LSMESSAGE_TOKEN_INVALID){};

    ~LSCalloutContext()
    {
        cancel();
    };

    bool cancel();

private:
    LSHandle* m_service;
    LSMessageToken m_token;
};

/**
 * a function template that wraps a given function expecting and returning QJsonDocuments
 * in a static function that is compatible with the LunaService callback signature.
 * usage:
 *
 * static LSMethod WebAppManagerService::s_publicMethods[] = {
 *     {"launchUrl", bus_callback_qjson<WebAppManagerService, &WebAppManagerService::launchUrl>},
 *     { 0, 0 }
 * };
 *
 * */

template <class CLASS, QJsonObject (CLASS::*FUNCTION)(QJsonObject)>
static bool bus_callback_qjson(LSHandle* handle, LSMessage* message, void* user_data)
{
    LSErrorSafe lsError;

    if (!message) {
        if (!LSMessageReply(handle, message, "{\"returnValue\": false}", &lsError))
            return false;
        return true;
    }

    QJsonObject request = QJsonDocument::fromJson(LSMessageGetPayload(message)).object();
    QJsonObject reply;

    reply = (static_cast<CLASS*>(user_data)->*FUNCTION)(request);

    if (!LSMessageReply(handle, message, QJsonDocument(reply).toJson().data(), &lsError))
        return false;

    return true;
};

template <class CLASS, QJsonObject (CLASS::*FUNCTION)(QJsonObject, bool subscribed)>
static bool bus_subscription_callback_qjson(LSHandle* handle, LSMessage* message, void* user_data)
{
    LSErrorSafe lsError;

    if (!message) {
        if (!LSMessageReply(handle, message, "{\"returnValue\": false}", &lsError))
            return false;
        return true;
    }

    bool subscribed = false;
    if (LSMessageIsSubscription(message)) {
        if (!LSSubscriptionProcess(handle, message, &subscribed, &lsError))
            return false;
    }

    QJsonObject request = QJsonDocument::fromJson(LSMessageGetPayload(message)).object();
    QJsonObject reply;

    reply = (static_cast<CLASS*>(user_data)->*FUNCTION)(request, subscribed);

    if (subscribed)
        reply["subscribed"] = true;

    if (!LSMessageReply(handle, message, QJsonDocument(reply).toJson().data(), &lsError))
        return false;

    return true;
};

/*
 * same as above, but for a void function handling the reply
 */
template <class CLASS, void (CLASS::*FUNCTION)(QJsonObject)>
static bool bus_callback_qjson(LSHandle* handle, LSMessage* message, void* user_data)
{
    QJsonObject reply;
    if (message) {
        reply = QJsonDocument::fromJson(LSMessageGetPayload(message)).object();
    }

    (static_cast<CLASS*>(user_data)->*FUNCTION)(reply);

    return true;
};

class PalmServiceBase {
public:
    PalmServiceBase();
    virtual ~PalmServiceBase();

    bool startService();
    void stopService();
    /*
 *  Methods to issue calls to the LS2 bus, optional parameters are a callback context
 *  and applicationId
 **/
    inline bool callPrivate(const char* what,
        QJsonObject parameters,
        const char* applicationId = 0,
        LSCalloutContext* context = 0)
    {
        return call(m_serviceHandlePrivate, what, parameters, applicationId, context);
    };

    inline bool callPublic(const char* what,
        QJsonObject parameters,
        const char* applicationId = 0,
        LSCalloutContext* context = 0)
    {
        return call(m_serviceHandlePublic, what, parameters, applicationId, context);
    };

    /*
 * methods to post subscription updates TODO make subscriptions represented through objects
 **/
    bool postSubscriptionPrivate(const char* subscription, QJsonObject reply)
    {
        LSErrorSafe lsError;
        return LSSubscriptionPost(
            m_serviceHandlePrivate,
            category(),
            subscription,
            QJsonDocument(reply).toJson().data(),
            &lsError);
    }

    bool postSubscriptionPublic(const char* subscription, QJsonObject reply)
    {
        LSErrorSafe lsError;
        return LSSubscriptionPost(
            m_serviceHandlePublic,
            category(),
            subscription,
            QJsonDocument(reply).toJson().data(),
            &lsError);
    }

    virtual void didConnect() = 0;

protected:
    /*
     * helper methods for simple calls that come back into methods using a bit of template magic.
     * necessary since LS2 does not allow to specify user_data individually for each method published
     * so distinguishing between method invocations from the bus has to happen through
     * individual callback handlers. Using these template, we create a new static callback function
     * for each HANDLER_CLASS::CALLBACK_METHOD to forward the call to.
     * */
    template <class HANDLER_CLASS, void (HANDLER_CLASS::*CALLBACK_METHOD)(QJsonObject)>
    bool callPrivate(const char* what,
        QJsonObject parameters,
        HANDLER_CLASS* callback_receiver)
    {
        LSErrorSafe lsError;
        bool err = false;
        if (parameters.value("subscribe").toBool() || parameters.value("watch").toBool()) {
            err = LSCall(m_serviceHandlePrivate, what,
                QJsonDocument(parameters).toJson().data(),
                bus_callback_qjson<HANDLER_CLASS, CALLBACK_METHOD>,
                callback_receiver, NULL, &lsError);
        } else {
            err = LSCallOneReply(m_serviceHandlePrivate,
                what,
                QJsonDocument(parameters).toJson().data(),
                bus_callback_qjson<HANDLER_CLASS, CALLBACK_METHOD>,
                callback_receiver, NULL, &lsError);
        }
        if (!err) {
            qWarning("Failed to call in %s Service: %s", serviceName(), lsError.message);
            return false;
        }
        return true;
    };

    template <class HANDLER_CLASS, void (HANDLER_CLASS::*CALLBACK_METHOD)(QJsonObject)>
    bool callPublic(const char* what,
        QJsonObject parameters,
        HANDLER_CLASS* callback_receiver)
    {
        LSErrorSafe lsError;
        bool err = false;
        if (parameters.value("subscribe").toBool() || parameters.value("watch").toBool()) {
            err = LSCall(m_serviceHandlePublic, what,
                QJsonDocument(parameters).toJson().data(),
                bus_callback_qjson<HANDLER_CLASS, CALLBACK_METHOD>,
                callback_receiver, NULL, &lsError);
        } else {
            err = LSCallOneReply(m_serviceHandlePrivate,
                what,
                QJsonDocument(parameters).toJson().data(),
                bus_callback_qjson<HANDLER_CLASS, CALLBACK_METHOD>,
                callback_receiver, NULL, &lsError);
        }
        if (!err) {
            qWarning("Failed to call in %s Service: %s", serviceName(), lsError.message);
            return false;
        }
        return true;
    };

    virtual LSMethod* privateMethods() const = 0;
    virtual LSMethod* publicMethods() const = 0;
    virtual const char* serviceName() const = 0;
    virtual const char* category() const { return "/"; };
    virtual GMainLoop* mainLoop() const;
    LSPalmService* m_serviceHandle;
    LSHandle* m_serviceHandlePublic;
    LSHandle* m_serviceHandlePrivate;

private:
    static bool serviceConnectCallback(LSHandle* sh, LSMessage* message, void* ctx);
    bool call(LSHandle* service,
        const char* what,
        QJsonObject parameters,
        const char* applicationId,
        LSCalloutContext* context);
};

#endif /* PalmServiceBase_H */
