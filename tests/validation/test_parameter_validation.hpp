#ifndef __SEAR_TEST_PARAMETER_VALIDATION_H_
#define __SEAR_TEST_PARAMETER_VALIDATION_H_

#define VALIDATION_REQUEST_SAMPLES "./tests/validation/request_samples/"
#define VALIDATION_RESULT_SAMPLES "./tests/validation/result_samples/"

// Request Samples
#define TEST_SYNTAX_ERROR_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_syntax_error_request.json"
#define TEST_SYNTAX_ERROR_NOT_JSON_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_syntax_error_not_json_request.json"
#define TEST_SYNTAX_ERROR_BINARY_DATA_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_syntax_error_binary_data_request.json"
#define TEST_NO_PARAMETERS_PROVIDED_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_no_parameters_provided_request.json"
#define TEST_JUNK_JSON_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_junk_json_request.json"
#define TEST_PARAMETERS_JUNK_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_parameters_junk_request.json"
#define TEST_PARAMETERS_NONSTRING_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_parameters_nonstring_request.json"
#define TEST_PARAMETERS_MISSING_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_parameters_missing_request.json"
#define TEST_EXTRANEOUS_AND_MISSING_PARAMETERS_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES                                \
  "test_extraneous_and_missing_parameters_request.json"
#define TEST_AUTH_RESOURCE_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_auth_resource_request.json"
#define TEST_AUTH_RESOURCE_ACCESS_STATUS_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_auth_resource_access_status_request.json"
#define TEST_AUTH_DATASET_DENIED_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_auth_dataset_denied_request.json"
#define TEST_AUTH_UNSUPPORTED_IDENTITY_REQUEST_JSON \
  VALIDATION_REQUEST_SAMPLES "test_auth_unsupported_identity_request.json"

// Result Samples
#define TEST_SYNTAX_ERROR_RESULT_JSON \
  VALIDATION_RESULT_SAMPLES "test_syntax_error_result.json"
#define TEST_SYNTAX_ERROR_NOT_JSON_RESULT_JSON \
  VALIDATION_RESULT_SAMPLES "test_syntax_error_not_json_result.json"
#define TEST_SYNTAX_ERROR_BINARY_DATA_RESULT_JSON \
  VALIDATION_RESULT_SAMPLES "test_syntax_error_binary_data_result.json"
#define TEST_AUTH_UNSUPPORTED_IDENTITY_RESULT_JSON \
  VALIDATION_RESULT_SAMPLES "test_auth_unsupported_identity_result.json"

// Prototypes
void test_handle_syntax_error();
void test_handle_syntax_error_not_json();
void test_handle_syntax_error_binary_data();
void test_parse_no_parameters_provided_error();
void test_parse_junk_json_error();
void test_parse_parameters_junk_error();
void test_parse_parameters_missing_error();
void test_parse_extraneous_and_missing_parameters_error();
void test_parse_parameters_nonstring_error();
void test_auth_resource_allowed();
void test_auth_resource_access_status();
void test_auth_dataset_denied();
void test_auth_unsupported_identity_error();

#endif
