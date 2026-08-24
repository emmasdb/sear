#include "tests/irrsim00/test_irrsim00.hpp"

#include <cstring>

#include "sear/conversion.hpp"
#include "sear/irrsim00/irrsim00.hpp"
#include "sear/sear.h"
#include "tests/mock/irrsim00.hpp"
#include "tests/unity/unity.h"

static void reset_irrsim00_mocks() {
  irrsim00_userid_mock             = NULL;
  irrsim00_application_userid_mock = NULL;
  irrsim00_saf_rc_mock             = 0;
  irrsim00_racf_rc_mock            = 0;
  irrsim00_racf_reason_mock        = 0;
  irrsim00_function_code_actual    = 0;
  irrsim00_option_word_actual      = 0;
}

void test_generate_map_userid_to_application_user_request() {
  reset_irrsim00_mocks();
  irrsim00_application_userid_mock = (char *)"user@example.test";

  const char *request_json =
      R"({"operation":"map","admin_type":"application-user","function_code":9,"userid":"mapusr"})";

  sear_result_t *result = sear(request_json, std::strlen(request_json), false);

  auto *p_arg_area = reinterpret_cast<irrsim00_arg_area_t *>(result->raw_request);
  std::string userid = SEAR::toUTF8(
      std::string(p_arg_area->racf_userid.value,
                  p_arg_area->racf_userid.length));

  TEST_ASSERT_EQUAL_INT32(sizeof(irrsim00_arg_area_t),
                          result->raw_request_length);
  TEST_ASSERT_EQUAL_UINT16(9, p_arg_area->function_code);
  TEST_ASSERT_EQUAL_UINT32(0, p_arg_area->option_word);
  TEST_ASSERT_EQUAL_UINT8(6, p_arg_area->racf_userid.length);
  TEST_ASSERT_EQUAL_STRING("MAPUSR", userid.c_str());
  TEST_ASSERT_EQUAL_UINT16(9, irrsim00_function_code_actual);
  TEST_ASSERT_EQUAL_UINT32(0, irrsim00_option_word_actual);
}

void test_parse_map_userid_to_application_user_result() {
  reset_irrsim00_mocks();
  irrsim00_application_userid_mock = (char *)"user@example.test";

  const char *request_json =
      R"({"operation":"map","admin_type":"application-user","function_code":9,"userid":"MAPUSR"})";
  const char *result_json_expected =
      R"({"application_userid":"user@example.test","return_codes":{"racf_reason_code":0,"racf_return_code":0,"saf_return_code":0,"sear_return_code":0}})";

  sear_result_t *result = sear(request_json, std::strlen(request_json), false);

  TEST_ASSERT_EQUAL_STRING(result_json_expected, result->result_json);
  TEST_ASSERT_EQUAL_INT32(std::strlen(result_json_expected),
                          result->result_json_length);
  TEST_ASSERT_EQUAL_CHAR(0, result->result_json[result->result_json_length]);
}

void test_parse_map_application_user_to_userid_failure() {
  reset_irrsim00_mocks();
  irrsim00_saf_rc_mock      = 8;
  irrsim00_racf_rc_mock     = 8;
  irrsim00_racf_reason_mock = 16;

  const char *request_json =
      R"({"operation":"map","admin_type":"application-user","function_code":10,"application_userid":"user@example.test"})";
  const char *result_json_expected =
      R"({"errors":["sear: unable to map application user"],"return_codes":{"racf_reason_code":16,"racf_return_code":8,"saf_return_code":8,"sear_return_code":4}})";

  sear_result_t *result = sear(request_json, std::strlen(request_json), false);

  TEST_ASSERT_EQUAL_STRING(result_json_expected, result->result_json);
}