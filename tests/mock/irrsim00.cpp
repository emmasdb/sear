#include "irrsim00.hpp"

#include <arpa/inet.h>

#include <cstring>

#include "zoslib.h"

#ifndef __TOS_390__
char *irrsim00_userid_mock             = NULL;
char *irrsim00_application_userid_mock = NULL;
int irrsim00_saf_rc_mock               = 0;
int irrsim00_racf_rc_mock              = 0;
int irrsim00_racf_reason_mock          = 0;
uint16_t irrsim00_function_code_actual = 0;
uint32_t irrsim00_option_word_actual   = 0;
#endif

extern void IRRSIM00(char *work_area, uint32_t alet_saf_rc, int *saf_rc,
                     uint32_t alet_racf_rc, int *racf_rc,
                     uint32_t alet_racf_rsn, int *racf_rsn,
                     uint32_t alet_remainder, uint16_t *function_code,
                     uint32_t *option_word, char *racf_userid,
                     char *certificate, char *application_userid,
                     char *distinguished_name, char *registry_name) {
  irrsim00_function_code_actual = *function_code;
  irrsim00_option_word_actual   = *option_word;

  if (irrsim00_userid_mock != NULL) {
    auto *p_racf_userid =
        reinterpret_cast<irrsim00_racf_userid_t *>(racf_userid);
    p_racf_userid->length = std::strlen(irrsim00_userid_mock);
    std::memcpy(p_racf_userid->value, irrsim00_userid_mock,
                p_racf_userid->length);
    __a2e_l(p_racf_userid->value, p_racf_userid->length);
  }

  if (irrsim00_application_userid_mock != NULL) {
    auto *p_application_userid =
        reinterpret_cast<irrsim00_application_userid_t *>(application_userid);
    p_application_userid->length =
        htons(std::strlen(irrsim00_application_userid_mock));
    std::memcpy(p_application_userid->value, irrsim00_application_userid_mock,
                std::strlen(irrsim00_application_userid_mock));
    __a2e_l(p_application_userid->value,
            std::strlen(irrsim00_application_userid_mock));
  }

  *saf_rc   = irrsim00_saf_rc_mock;
  *racf_rc  = irrsim00_racf_rc_mock;
  *racf_rsn = irrsim00_racf_reason_mock;
}