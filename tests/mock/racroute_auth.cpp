#include "racroute_auth.hpp"

#include "tests/mock/racroute_auth.hpp"

int racroute_auth_rc_mock             = 0;
int racroute_auth_access_code_actual  = 0;

extern "C" int sear_racroute_auth_asm(const char *class_name,
                                       int class_name_length,
                                       const char *entity, int entity_length,
                                       int access_code) {
  (void)class_name;
  (void)class_name_length;
  (void)entity;
  (void)entity_length;
  racroute_auth_access_code_actual = access_code;
  return racroute_auth_rc_mock;
}