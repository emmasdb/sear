#include <node_api.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include <exception>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "sear_error.hpp"
#include "security_admin.hpp"

static pthread_mutex_t sear_mutex = PTHREAD_MUTEX_INITIALIZER;

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

static bool get_string_field(const nlohmann::json& request, const char* name,
                             std::string* value) {
  if (!request.contains(name) || !request[name].is_string()) {
    return false;
  }
  *value = request[name].get<std::string>();
  return true;
}

static bool build_extract_request_json(const nlohmann::json& request,
                                       std::string* extract_request_json,
                                       std::string* duplicate_error) {
  std::string operation;
  std::string admin_type;
  if (!get_string_field(request, "operation", &operation) ||
      !get_string_field(request, "admin_type", &admin_type) ||
      operation != "add") {
    return false;
  }

  std::string profile_name;
  nlohmann::json extract_request = {{"operation", "extract"},
                                    {"admin_type", admin_type}};

  if (admin_type == "user") {
    if (!get_string_field(request, "userid", &profile_name)) {
      return false;
    }
    extract_request["userid"] = profile_name;
  } else if (admin_type == "group") {
    if (!get_string_field(request, "group", &profile_name)) {
      return false;
    }
    extract_request["group"] = profile_name;
  } else if (admin_type == "dataset") {
    if (!get_string_field(request, "dataset", &profile_name)) {
      return false;
    }
    extract_request["dataset"] = profile_name;
  } else if (admin_type == "resource") {
    std::string class_name;
    if (!get_string_field(request, "resource", &profile_name) ||
        !get_string_field(request, "class", &class_name)) {
      return false;
    }
    extract_request["resource"] = profile_name;
    extract_request["class"] = class_name;
    *duplicate_error = "sear: unable to add '" + profile_name +
                       "' in the '" + class_name +
                       "' class because a '" + admin_type +
                       "' profile already exists in the '" + class_name +
                       "' class with that name";
    *extract_request_json = extract_request.dump();
    return true;
  } else {
    return false;
  }

  *duplicate_error = "sear: unable to add '" + profile_name +
                     "' because a '" + admin_type +
                     "' profile already exists with that name";
  *extract_request_json = extract_request.dump();
  return true;
}

static bool result_has_sear_success(const sear_result_t& result) {
  if (result.result_json == nullptr || result.result_json_length <= 0) {
    return false;
  }

  nlohmann::json result_json = nlohmann::json::parse(
      std::string(result.result_json,
                  static_cast<size_t>(result.result_json_length)),
      nullptr, false);
  if (result_json.is_discarded() || result_json.contains("errors")) {
    return false;
  }

  const auto& return_codes = result_json["return_codes"];
  return return_codes.contains("sear_return_code") &&
         return_codes["sear_return_code"] == 0;
}

static void cleanup_result(sear_result_t* result) {
  delete[] result->raw_request;
  result->raw_request = nullptr;
  result->raw_request_length = 0;

  delete[] result->raw_result;
  result->raw_result = nullptr;
  result->raw_result_length = 0;

  delete[] result->result_json;
  result->result_json = nullptr;
  result->result_json_length = 0;
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
  fflush(stderr);

  pthread_mutex_lock(&sear_mutex);
  fprintf(stderr, "[_sear.cpp] calling sear()\n");
  fflush(stderr);

  nlohmann::json request_json = nlohmann::json::parse(
      std::string(request.data(), request_length), nullptr, false);
  if (!request_json.is_discarded()) {
    std::string extract_request_json;
    std::string duplicate_error;
    if (build_extract_request_json(request_json, &extract_request_json,
                                   &duplicate_error)) {
      sear_result_t extract_result = {nullptr, 0, nullptr, 0, nullptr, 0};
      SEAR::SecurityAdmin security_admin(&extract_result, false);
      security_admin.makeRequest(
          extract_request_json.data(),
          static_cast<int>(extract_request_json.length()));
      bool profile_exists = result_has_sear_success(extract_result);
      cleanup_result(&extract_result);

      if (profile_exists) {
        std::string error_json = build_error_result_json({duplicate_error});
        sear_result_t error_result = {nullptr,
                                      0,
                                      nullptr,
                                      0,
                                      error_json.data(),
                                      static_cast<int>(error_json.length())};
        napi_value result_obj = build_result_object(env, &error_result);
        pthread_mutex_unlock(&sear_mutex);
        return result_obj;
      }
    }
  }

  sear_result_t result = {nullptr, 0, nullptr, 0, nullptr, 0};
  try {
    SEAR::SecurityAdmin security_admin(&result, debug);
    security_admin.makeRequest(request.data(),
                               static_cast<int>(request_length));
  } catch (const SEAR::SEARError& error) {
    std::string error_json = build_error_result_json(error.getErrors());
    sear_result_t error_result = {nullptr,
                                  0,
                                  nullptr,
                                  0,
                                  error_json.data(),
                                  static_cast<int>(error_json.length())};
    napi_value result_obj = build_result_object(env, &error_result);
    cleanup_result(&result);
    pthread_mutex_unlock(&sear_mutex);
    return result_obj;
  } catch (const std::exception& error) {
    cleanup_result(&result);
    pthread_mutex_unlock(&sear_mutex);
    napi_throw_error(env, nullptr, error.what());
    return nullptr;
  } catch (...) {
    cleanup_result(&result);
    pthread_mutex_unlock(&sear_mutex);
    napi_throw_error(env, nullptr, "Unknown SEAR native exception");
    return nullptr;
  }

  fprintf(stderr, "[_sear.cpp] sear() returned\n");
  fflush(stderr);

  napi_value result_obj = build_result_object(env, &result);
  cleanup_result(&result);
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
