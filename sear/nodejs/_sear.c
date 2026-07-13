#include <node_api.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sear.h"

static pthread_mutex_t sear_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool throw_if_napi_failed(napi_env env, napi_status status, const char* message) {
    if (status == napi_ok) {
        return false;
    }
    napi_throw_error(env, NULL, message);
    return true;
}

static napi_status create_buffer_or_empty(napi_env env, char* data, int length, napi_value* value) {
    if (length <= 0) {
        return napi_create_buffer_copy(env, 0, "", NULL, value);
    }
    if (data == NULL) {
        return napi_invalid_arg;
    }
    return napi_create_buffer_copy(env, (size_t)length, data, NULL, value);
}

static napi_status create_string_or_empty(napi_env env, char* data, int length, napi_value* value) {
    if (length <= 0 || data == NULL) {
        return napi_create_string_utf8(env, "", 0, value);
    }
    return napi_create_string_utf8(env, data, (size_t)length, value);
}

static napi_value call_sear(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2];
    if (throw_if_napi_failed(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL),
                             "Failed to read call_sear arguments")) {
        return NULL;
    }

    if (argc < 1) {
        napi_throw_type_error(env, NULL, "call_sear requires a request JSON string");
        return NULL;
    }

    // Get request string (arg 0)
    size_t request_length;
    if (throw_if_napi_failed(env, napi_get_value_string_utf8(env, argv[0], NULL, 0, &request_length),
                             "call_sear request must be a string")) {
        return NULL;
    }
    char* request = malloc(request_length + 1);
    if (request == NULL) {
        napi_throw_error(env, NULL, "Failed to allocate request buffer");
        return NULL;
    }
    if (throw_if_napi_failed(env, napi_get_value_string_utf8(env, argv[0], request, request_length + 1, &request_length),
                             "Failed to read request JSON string")) {
        free(request);
        return NULL;
    }

    // Get debug bool (arg 1, optional)
    bool debug = false;
    if (argc >= 2) {
        if (throw_if_napi_failed(env, napi_get_value_bool(env, argv[1], &debug),
                                 "call_sear debug flag must be a boolean")) {
            free(request);
            return NULL;
        }
    }

    fprintf(stderr, "[_sear.c] call_sear: length=%zu debug=%d\n", request_length, debug);
    fprintf(stderr, "[_sear.c] request: %s\n", request);
    fflush(stderr);

    pthread_mutex_lock(&sear_mutex);
    fprintf(stderr, "[_sear.c] calling sear()\n");
    fflush(stderr);
    sear_result_t* result = sear(request, (int)request_length, debug);
    fprintf(stderr, "[_sear.c] sear() returned\n");
    fflush(stderr);
    free(request);

    if (result == NULL) {
        pthread_mutex_unlock(&sear_mutex);
        napi_throw_error(env, NULL, "SEAR returned no result");
        return NULL;
    }

    // Build result object { raw_request, raw_result, result_json }
    napi_value result_obj;
    if (throw_if_napi_failed(env, napi_create_object(env, &result_obj),
                             "Failed to create SEAR result object")) {
        pthread_mutex_unlock(&sear_mutex);
        return NULL;
    }

    napi_value raw_request, raw_result, result_json;
    if (throw_if_napi_failed(env, create_buffer_or_empty(env, result->raw_request,
                                                         result->raw_request_length,
                                                         &raw_request),
                             "Failed to create raw_request buffer")) {
        pthread_mutex_unlock(&sear_mutex);
        return NULL;
    }
    if (throw_if_napi_failed(env, create_buffer_or_empty(env, result->raw_result,
                                                         result->raw_result_length,
                                                         &raw_result),
                             "Failed to create raw_result buffer")) {
        pthread_mutex_unlock(&sear_mutex);
        return NULL;
    }
    if (throw_if_napi_failed(env, create_string_or_empty(env, result->result_json,
                                                         result->result_json_length,
                                                         &result_json),
                             "Failed to create result_json string")) {
        pthread_mutex_unlock(&sear_mutex);
        return NULL;
    }

    if (throw_if_napi_failed(env, napi_set_named_property(env, result_obj, "raw_request", raw_request),
                             "Failed to set raw_request result property")) {
        pthread_mutex_unlock(&sear_mutex);
        return NULL;
    }
    if (throw_if_napi_failed(env, napi_set_named_property(env, result_obj, "raw_result", raw_result),
                             "Failed to set raw_result result property")) {
        pthread_mutex_unlock(&sear_mutex);
        return NULL;
    }
    if (throw_if_napi_failed(env, napi_set_named_property(env, result_obj, "result_json", result_json),
                             "Failed to set result_json result property")) {
        pthread_mutex_unlock(&sear_mutex);
        return NULL;
    }

    pthread_mutex_unlock(&sear_mutex);
    return result_obj;
}

NAPI_MODULE_INIT() {
    napi_value fn;
    if (throw_if_napi_failed(env, napi_create_function(env, "call_sear", NAPI_AUTO_LENGTH,
                                                       call_sear, NULL, &fn),
                             "Failed to create call_sear function")) {
        return NULL;
    }
    if (throw_if_napi_failed(env, napi_set_named_property(env, exports, "call_sear", fn),
                             "Failed to export call_sear function")) {
        return NULL;
    }
    return exports;
}