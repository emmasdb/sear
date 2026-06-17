#include <node_api.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "sear.h"

static pthread_mutex_t sear_mutex = PTHREAD_MUTEX_INITIALIZER;

static napi_value call_sear(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2];
    napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

    // Get request string (arg 0)
    size_t request_length;
    napi_get_value_string_utf8(env, argv[0], NULL, 0, &request_length);
    char* request = malloc(request_length + 1);
    napi_get_value_string_utf8(env, argv[0], request, request_length + 1, &request_length);

    // Get debug bool (arg 1, optional)
    bool debug = false;
    if (argc >= 2) {
        napi_get_value_bool(env, argv[1], &debug);
    }

    pthread_mutex_lock(&sear_mutex);
    sear_result_t* result = sear(request, (int)request_length, debug);
    free(request);

    // Build result object { raw_request, raw_result, result_json }
    napi_value result_obj;
    napi_create_object(env, &result_obj);

    napi_value raw_request, raw_result, result_json;
    napi_create_buffer_copy(env, result->raw_request_length,
                            result->raw_request, NULL, &raw_request);
    napi_create_buffer_copy(env, result->raw_result_length,
                            result->raw_result, NULL, &raw_result);
    napi_create_string_utf8(env, result->result_json,
                            result->result_json_length, &result_json);

    napi_set_named_property(env, result_obj, "raw_request", raw_request);
    napi_set_named_property(env, result_obj, "raw_result", raw_result);
    napi_set_named_property(env, result_obj, "result_json", result_json);

    pthread_mutex_unlock(&sear_mutex);
    return result_obj;
}

NAPI_MODULE_INIT() {
    napi_value fn;
    napi_create_function(env, "call_sear", NAPI_AUTO_LENGTH,
                         call_sear, NULL, &fn);
    napi_set_named_property(env, exports, "call_sear", fn);
    return exports;
}