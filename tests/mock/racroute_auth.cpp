#include "racroute_auth.hpp"

#include "tests/mock/racroute_auth.hpp"

int racroute_auth_rc_mock             = 0;
int racroute_auth_racf_rc_mock        = 0;
int racroute_auth_racf_reason_mock    = 0;
int racroute_auth_access_code_actual  = 0;

extern "C" int sear_racroute_auth_asm(const char *class_name,
                                       int class_name_length,
                                       const char *entity, int entity_length,
                                       int access_code,
                                       int *racf_return_code,
                                       int *racf_reason_code) {
  (void)class_name;
  (void)class_name_length;
  (void)entity;
  (void)entity_length;
  racroute_auth_access_code_actual = access_code;
  *racf_return_code                = racroute_auth_racf_rc_mock;
  *racf_reason_code                = racroute_auth_racf_reason_mock;
  return racroute_auth_rc_mock;
}