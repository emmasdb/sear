#ifndef __IRRSIM00_MOCK_H_
#define __IRRSIM00_MOCK_H_

#include <cstddef>
#include <cstdint>

#ifndef __TOS_390__
extern char *irrsim00_userid_mock;
extern char *irrsim00_application_userid_mock;
extern int irrsim00_saf_rc_mock;
extern int irrsim00_racf_rc_mock;
extern int irrsim00_racf_reason_mock;
extern uint16_t irrsim00_function_code_actual;
extern uint32_t irrsim00_option_word_actual;
#else
char *irrsim00_userid_mock             = NULL;
char *irrsim00_application_userid_mock = NULL;
int irrsim00_saf_rc_mock               = 0;
int irrsim00_racf_rc_mock              = 0;
int irrsim00_racf_reason_mock          = 0;
uint16_t irrsim00_function_code_actual = 0;
uint32_t irrsim00_option_word_actual   = 0;
#endif

extern "C" {
uint32_t callIrrsim00(char *__ptr32);
}

#endif