#include <node_api.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include <cstdlib>
#include <exception>
#include <string>
#include <vector>

#include "sear_error.hpp"
#include "sear.h"

static pthread_mutex_t sear_mutex = PTHREAD_MUTEX_INITIALIZER;

static void report_exception_configuration() {
  static bool reported = false;
  if (reported) {
    return;
  }
  reported = true;

#ifdef __cpp_exceptions
  const char* cpp_exceptions = "defined";
#else
  const char* cpp_exceptions = "not defined";
#endif

#ifdef __EXCEPTIONS
  const char* exceptions = "defined";
#else
  const char* exceptions = "not defined";
#endif

#ifdef JSON_NOEXCEPTION
  const char* json_noexception = "defined";
#else
  const char* json_noexception = "not defined";
#endif

  fprintf(stderr,
          "[_sear.cpp] exception config: __cpp_exceptions=%s "
          "__EXCEPTIONS=%s JSON_NOEXCEPTION=%s\n",
          cpp_exceptions, exceptions, json_noexception);
  fflush(stderr);
}

static void sear_terminate_handler() noexcept {
  fprintf(stderr, "[_sear.cpp] std::terminate while calling sear()\n");
  std::exception_ptr exception = std::current_exception();
  if (exception) {
    try {
      std::rethrow_exception(exception);
    } catch (const SEAR::SEARError& error) {
      const std::vector<std::string>& errors = error.getErrors();
      if (!errors.empty()) {
        fprintf(stderr, "[_sear.cpp] active SEARError: %s\n",
                errors.front().c_str());
      }
    } catch (const std::exception& error) {
      fprintf(stderr, "[_sear.cpp] active std::exception: %s\n", error.what());
    } catch (...) {
      fprintf(stderr, "[_sear.cpp] active non-standard exception\n");
    }
  }
  fflush(stderr);
  std::abort();
}

static bool throw_if_napi_failed(napi_env env, napi_status status,
                                 const char* message) {
  if (status == napi_ok) {
    return false;
  }
  napi_throw_error(env, nullptr, message);
  return true;
}

static napi_status create_buffer_or_empty(napi_env env, char* data, int length,
                                          napi_value* value) {
  if (length <= 0) {
    return napi_create_buffer_copy(env, 0, "", nullptr, value);
  }
  if (data == nullptr) {
    return napi_invalid_arg;
  }
  return napi_create_buffer_copy(env, static_cast<size_t>(length), data,
                                 nullptr, value);
}

static napi_status create_string_or_empty(napi_env env, const char* data,
                                          int length, napi_value* value) {
  if (length <= 0 || data == nullptr) {
    return napi_create_string_utf8(env, "", 0, value);
  }
  return napi_create_string_utf8(env, data, static_cast<size_t>(length), value);
}

static void append_json_string(std::string* json, const std::string& value) {
  json->push_back('"');
  for (char ch : value) {
    switch (ch) {
      case '"':
        json->append("\\\"");
        break;
      case '\\':
        json->append("\\\\");
        break;
      case '\b':
        json->append("\\b");
        break;
      case '\f':
        json->append("\\f");
        break;
      case '\n':
        json->append("\\n");
        break;
      case '\r':
        json->append("\\r");
        break;
      case '\t':
        json->append("\\t");
        break;
      default:
        json->push_back(ch);
        break;
    }
  }
  json->push_back('"');
}

static std::string build_error_result_json(
    const std::vector<std::string>& errors) {
  std::string json = "{\"errors\":[";
  for (size_t index = 0; index < errors.size(); index++) {
    if (index > 0) {
      json.push_back(',');
    }
    append_json_string(&json, errors[index]);
  }
  json.append("],\"return_codes\":{\"saf_return_code\":null,");
  json.append("\"racf_return_code\":null,\"racf_reason_code\":null,");
  json.append("\"sear_return_code\":4}}");
  return json;
}

static napi_value build_result_object(napi_env env, sear_result_t* result) {
  napi_value result_obj;
  if (throw_if_napi_failed(env, napi_create_object(env, &result_obj),
                           "Failed to create SEAR result object")) {
    return nullptr;
  }

  napi_value raw_request;
  napi_value raw_result;
  napi_value result_json;
  if (throw_if_napi_failed(env,
                           create_buffer_or_empty(env, result->raw_request,
                                                  result->raw_request_length,
                                                  &raw_request),
                           "Failed to create raw_request buffer")) {
    return nullptr;
  }
  if (throw_if_napi_failed(env,
                           create_buffer_or_empty(env, result->raw_result,
                                                  result->raw_result_length,
                                                  &raw_result),
                           "Failed to create raw_result buffer")) {
    return nullptr;
  }
  if (throw_if_napi_failed(env,
                           create_string_or_empty(env, result->result_json,
                                                  result->result_json_length,
                                                  &result_json),
                           "Failed to create result_json string")) {
    return nullptr;
  }

  if (throw_if_napi_failed(env,
                           napi_set_named_property(env, result_obj,
                                                   "raw_request", raw_request),
                           "Failed to set raw_request result property")) {
    return nullptr;
  }
  if (throw_if_napi_failed(env,
                           napi_set_named_property(env, result_obj,
                                                   "raw_result", raw_result),
                           "Failed to set raw_result result property")) {
    return nullptr;
  }
  if (throw_if_napi_failed(env,
                           napi_set_named_property(env, result_obj,
                                                   "result_json", result_json),
                           "Failed to set result_json result property")) {
    return nullptr;
  }

  return result_obj;
}

static napi_value call_sear(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  if (throw_if_napi_failed(env,
                           napi_get_cb_info(env, info, &argc, argv, nullptr,
                                            nullptr),
                           "Failed to read call_sear arguments")) {
    return nullptr;
  }

  if (argc < 1) {
    napi_throw_type_error(env, nullptr,
                          "call_sear requires a request JSON string");
    return nullptr;
  }

  size_t request_length;
  if (throw_if_napi_failed(env,
                           napi_get_value_string_utf8(env, argv[0], nullptr, 0,
                                                      &request_length),
                           "call_sear request must be a string")) {
    return nullptr;
  }

  std::vector<char> request(request_length + 1, 0);
  if (throw_if_napi_failed(env,
                           napi_get_value_string_utf8(env, argv[0],
                                                      request.data(),
                                                      request.size(),
                                                      &request_length),
                           "Failed to read request JSON string")) {
    return nullptr;
  }

  bool debug = false;
  if (argc >= 2) {
    if (throw_if_napi_failed(env, napi_get_value_bool(env, argv[1], &debug),
                             "call_sear debug flag must be a boolean")) {
      return nullptr;
    }
  }

  fprintf(stderr, "[_sear.cpp] call_sear: length=%zu debug=%d\n",
          request_length, debug);
  fprintf(stderr, "[_sear.cpp] request: %s\n", request.data());
    report_exception_configuration();
  fflush(stderr);

  pthread_mutex_lock(&sear_mutex);
  fprintf(stderr, "[_sear.cpp] calling sear()\n");
  fflush(stderr);

  sear_result_t* result = nullptr;
  std::terminate_handler previous_terminate =
      std::set_terminate(sear_terminate_handler);
  try {
    result = sear(request.data(), static_cast<int>(request_length), debug);
  } catch (const SEAR::SEARError& error) {
    std::set_terminate(previous_terminate);
    fprintf(stderr, "[_sear.cpp] caught SEARError from sear()\n");
    fflush(stderr);
    std::string error_json = build_error_result_json(error.getErrors());
    sear_result_t error_result = {nullptr,
                                  0,
                                  nullptr,
                                  0,
                                  error_json.data(),
                                  static_cast<int>(error_json.length())};
    napi_value result_obj = build_result_object(env, &error_result);
    pthread_mutex_unlock(&sear_mutex);
    return result_obj;
  } catch (const std::exception& error) {
    std::set_terminate(previous_terminate);
    fprintf(stderr, "[_sear.cpp] caught std::exception from sear(): %s\n",
            error.what());
    fflush(stderr);
    pthread_mutex_unlock(&sear_mutex);
    napi_throw_error(env, nullptr, error.what());
    return nullptr;
  } catch (...) {
    std::set_terminate(previous_terminate);
    fprintf(stderr, "[_sear.cpp] caught unknown exception from sear()\n");
    fflush(stderr);
    pthread_mutex_unlock(&sear_mutex);
    napi_throw_error(env, nullptr, "Unknown SEAR native exception");
    return nullptr;
  }
  std::set_terminate(previous_terminate);

  fprintf(stderr, "[_sear.cpp] sear() returned\n");
  fflush(stderr);

  if (result == nullptr) {
    pthread_mutex_unlock(&sear_mutex);
    napi_throw_error(env, nullptr, "SEAR returned no result");
    return nullptr;
  }

  napi_value result_obj = build_result_object(env, result);
  pthread_mutex_unlock(&sear_mutex);
  return result_obj;
}

NAPI_MODULE_INIT() {
  napi_value fn;
  if (throw_if_napi_failed(env,
                           napi_create_function(env, "call_sear",
                                                NAPI_AUTO_LENGTH, call_sear,
                                                nullptr, &fn),
                           "Failed to create call_sear function")) {
    return nullptr;
  }
  if (throw_if_napi_failed(
          env, napi_set_named_property(env, exports, "call_sear", fn),
          "Failed to export call_sear function")) {
    return nullptr;
  }
  return exports;
}
