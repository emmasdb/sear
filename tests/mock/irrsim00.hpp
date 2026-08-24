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
void IRRSIM00(char *,          // Workarea
              uint32_t, int *, // safrc
              uint32_t, int *, // racfrc
              uint32_t, int *, // racfrsn
              uint32_t,        // ALET for the remaining parameters
              uint16_t *,      // Function code
              uint32_t *,      // Option word
              char *,          // RACF userid
              char *,          // Certificate
              char *,          // Application userid
              char *,          // Distinguished name
              char *           // Registry name
);
}

#endif