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

#ifndef WEBOS_PALM_SERVICE_BASE_H_
#define WEBOS_PALM_SERVICE_BASE_H_

#include <functional>

#include <glib.h>
#include <json/json.h>
#include <luna-service2/lunaservice.h>

#include "log_manager.h"
#include "utils.h"

class LSHandle;
class LSMessage;
class PalmServiceBase;

/*
 * helper class that makes LSError clean up after itself.
 * significantly simplifies LS call sequences
 */

class LSErrorSafe : public LSError {
 public:
  LSErrorSafe() { LSErrorInit(this); };
  ~LSErrorSafe() { LSErrorFree(this); }
};

/*
 * This class allows us to call into LS2 and have the reply be forwarded to a
 * Qt slot or Q_INVOKABLE function of some object of the signature
 * Json::Value handlerFunc(Json::Value payload)
 *
 * */
class LSCallbackHandler {
 public:
  LSCallbackHandler(std::function<Json::Value(const Json::Value&)>& func)
      : func_(func) {}

  virtual ~LSCallbackHandler() {}

 protected:
  Json::Value Called(Json::Value payload) { return func_(payload); }

  static bool Callback(LSHandle* handle, LSMessage* message, void* user_data) {
    LSErrorSafe lsError;

    if (!message) {
      if (!LSMessageReply(handle, message, "{\"returnValue\": false}",
                          &lsError))
        return false;
      return true;
    }

    Json::Value request;
    if (!util::StringToJson(LSMessageGetPayload(message), request)) {
      if (!LSMessageReply(handle, message, "{\"returnValue\": false}",
                          &lsError))
        return false;
      return true;
    }

    Json::Value reply;

    reply = static_cast<LSCallbackHandler*>(user_data)->Called(request);

    if (!reply.isNull())
      return LSMessageReply(handle, message, util::JsonToString(reply).c_str(),
                            &lsError);
    else
      return true;
  }

  std::function<Json::Value(const Json::Value&)> func_;
};

/**
 * A class to make a LS2 call cancellable by providing storage for a token
 */
class LSCalloutContext : public LSCallbackHandler {
  friend class PalmServiceBase;

 public:
  LSCalloutContext(std::function<Json::Value(const Json::Value&)> func)
      : LSCallbackHandler(func), service_(0), token_(LSMESSAGE_TOKEN_INVALID){};

  ~LSCalloutContext() { Cancel(); };

  bool Cancel();

 private:
  LSHandle* service_;
  LSMessageToken token_;
};

/**
 * a function template that wraps a given function expecting and returning
 * Json::Value in a static function that is compatible with the LunaService
 * callback signature. usage:
 *
 * static LSMethod WebAppManagerService::s_methods[] = {
 *     {"launchUrl", bus_callback_json<WebAppManagerService,
 * &WebAppManagerService::launchUrl>}, { 0, 0 }
 * };
 *
 * */

template <class CLASS, Json::Value (CLASS::*FUNCTION)(const Json::Value&)>
static bool bus_callback_json(LSHandle* handle,
                              LSMessage* message,
                              void* user_data) {
  LSErrorSafe ls_error;

  if (!message) {
    if (!LSMessageReply(handle, message, "{\"returnValue\": false}", &ls_error))
      return false;
    return true;
  }

  Json::Value request;
  if (!util::StringToJson(LSMessageGetPayload(message), request)) {
    LOG_WARNING(MSGID_LUNA_API, 0, "Failed to parse request message.");
    return false;
  }
  Json::Value reply;

  reply = (static_cast<CLASS*>(user_data)->*FUNCTION)(request);

  if (!LSMessageReply(handle, message, util::JsonToString(reply).c_str(),
                      &ls_error))
    return false;

  return true;
};

template <class CLASS,
          Json::Value (CLASS::*FUNCTION)(const Json::Value&, bool subscribed)>
static bool bus_subscription_callback_json(LSHandle* handle,
                                           LSMessage* message,
                                           void* user_data) {
  LSErrorSafe ls_error;

  if (!message) {
    if (!LSMessageReply(handle, message, "{\"returnValue\": false}", &ls_error))
      return false;
    return true;
  }

  bool subscribed = false;
  if (LSMessageIsSubscription(message)) {
    if (!LSSubscriptionProcess(handle, message, &subscribed, &ls_error))
      return false;
  }

  Json::Value request;
  if (!util::StringToJson(LSMessageGetPayload(message), request)) {
    LOG_WARNING(MSGID_LUNA_API, 0, "Failed to parse request message.");
    return false;
  }
  Json::Value reply;

  reply = (static_cast<CLASS*>(user_data)->*FUNCTION)(request, subscribed);

  if (subscribed)
    reply["subscribed"] = true;

  if (!LSMessageReply(handle, message, util::JsonToString(reply).c_str(),
                      &ls_error))
    return false;

  return true;
};

/*
 * same as above, but for a void function handling the reply
 */
template <class CLASS, void (CLASS::*FUNCTION)(const Json::Value&)>
static bool bus_callback_json(LSHandle* handle,
                              LSMessage* message,
                              void* user_data) {
  Json::Value reply;
  if (message) {
    if (!util::StringToJson(LSMessageGetPayload(message), reply))
      LOG_WARNING(MSGID_LUNA_API, 0, "Failed to parse reply message.");
  }

  (static_cast<CLASS*>(user_data)->*FUNCTION)(reply);

  return true;
};

class PalmServiceBase {
 public:
  PalmServiceBase();
  virtual ~PalmServiceBase();

  bool StartService();
  bool StopService();

  /*
   *  Methods to issue calls to the LS2 bus, optional parameters are a callback
   *context and application_id
   **/
  inline bool Call(const char* what,
                   Json::Value parameters,
                   const char* application_id = 0,
                   LSCalloutContext* context = 0) {
    return Call(service_handle_, what, parameters, application_id, context);
  };

  /*
   * methods to post subscription updates TODO make subscriptions represented
   *through objects
   **/
  bool PostSubscription(const char* subscription, Json::Value reply) {
    LSErrorSafe ls_error;
    return LSSubscriptionPost(service_handle_, Category(), subscription,
                              util::JsonToString(reply).c_str(), &ls_error);
  }

  virtual void DidConnect() = 0;

 protected:
  /*
   * helper methods for simple calls that come back into methods using a bit of
   * template magic. necessary since LS2 does not allow to specify user_data
   * individually for each method published so distinguishing between method
   * invocations from the bus has to happen through individual callback
   * handlers. Using these template, we create a new static callback function
   * for each HANDLER_CLASS::CALLBACK_METHOD to forward the call to.
   * */
  template <class HANDLER_CLASS,
            void (HANDLER_CLASS::*CALLBACK_METHOD)(const Json::Value&)>
  bool Call(const char* what,
            Json::Value parameters,
            HANDLER_CLASS* callback_receiver) {
    LSErrorSafe ls_error;
    bool err = false;
    if (parameters.isObject() &&
        (parameters["subscribe"].asBool() || parameters["watch"].asBool())) {
      err =
          LSCall(service_handle_, what, util::JsonToString(parameters).c_str(),
                 bus_callback_json<HANDLER_CLASS, CALLBACK_METHOD>,
                 callback_receiver, NULL, &ls_error);
    } else {
      err = LSCallOneReply(service_handle_, what,
                           util::JsonToString(parameters).c_str(),
                           bus_callback_json<HANDLER_CLASS, CALLBACK_METHOD>,
                           callback_receiver, NULL, &ls_error);
    }
    if (!err) {
      LOG_WARNING(MSGID_LUNA_API, 0, "Failed to call in %s Service: %s",
                  ServiceName(), ls_error.message);
      return false;
    }
    return true;
  };

  virtual LSMethod* Methods() const = 0;
  virtual const char* ServiceName() const = 0;
  virtual const char* Category() const { return "/"; };
  virtual GMainLoop* MainLoop() const;
  LSHandle* service_handle_;

 private:
  PalmServiceBase(const PalmServiceBase&) = delete;
  PalmServiceBase& operator=(const PalmServiceBase&) = delete;
  static bool ServiceConnectCallback(LSHandle* sh,
                                     LSMessage* message,
                                     void* ctx);

  bool Call(LSHandle* service,
            const char* what,
            Json::Value qParameters,
            const char* applicationId,
            LSCalloutContext* context);
  std::string service_name_;
};

#endif  // WEBOS_PALM_SERVICE_BASE_H_
