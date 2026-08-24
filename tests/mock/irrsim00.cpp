#include "sear/irrsim00/irrsim00.hpp"

#include <arpa/inet.h>

#include <cstring>

#ifndef __TOS_390__
char *irrsim00_userid_mock             = NULL;
char *irrsim00_application_userid_mock = NULL;
int irrsim00_saf_rc_mock               = 0;
int irrsim00_racf_rc_mock              = 0;
int irrsim00_racf_reason_mock          = 0;
uint16_t irrsim00_function_code_actual = 0;
uint32_t irrsim00_option_word_actual   = 0;
#endif

extern "C" uint32_t callIrrsim00(char *__ptr32 arg_pointers) {
  auto *p_arg_pointers =
      reinterpret_cast<irrsim00_arg_pointers_t *>(arg_pointers);
  irrsim00_function_code_actual = *p_arg_pointers->p_function_code;
  irrsim00_option_word_actual   = *p_arg_pointers->p_option_word;

  if (irrsim00_userid_mock != NULL) {
    auto *p_racf_userid = p_arg_pointers->p_racf_userid;
    p_racf_userid->length = std::strlen(irrsim00_userid_mock);
    std::memcpy(p_racf_userid->value, irrsim00_userid_mock,
                p_racf_userid->length);
  }

  if (irrsim00_application_userid_mock != NULL) {
    auto *p_application_userid = p_arg_pointers->p_application_userid;
    p_application_userid->length =
        htons(std::strlen(irrsim00_application_userid_mock));
    std::memcpy(p_application_userid->value, irrsim00_application_userid_mock,
                std::strlen(irrsim00_application_userid_mock));
  }

  *p_arg_pointers->p_saf_return_code  = irrsim00_saf_rc_mock;
  *p_arg_pointers->p_racf_return_code = irrsim00_racf_rc_mock;
  *p_arg_pointers->p_racf_reason_code = irrsim00_racf_reason_mock;
  return 0;
}