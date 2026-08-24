#include "racroute_auth.hpp"

#include <cstring>
#include <string>

#include "sear/conversion.hpp"
#include "tests/mock/racroute_auth.hpp"

int racroute_auth_rc_mock             = 0;
int racroute_auth_racf_rc_mock        = 0;
int racroute_auth_racf_reason_mock    = 0;
int racroute_auth_access_code_actual  = 0;
int racroute_auth_status_code_actual  = 0;
int racroute_auth_identity_type_actual = 0;
char racroute_auth_authid_actual[9]    = {0};

extern "C" int sear_racroute_auth_asm(const char *class_name,
                                       int class_name_length,
                                       const char *entity, int entity_length,
                                       int access_code,
                                       int status_code,
                                       const char *authid,
                                       int authid_length,
                                       int identity_type,
                                       int *racf_return_code,
                                       int *racf_reason_code) {
  (void)class_name;
  (void)class_name_length;
  (void)entity;
  (void)entity_length;
  racroute_auth_access_code_actual = access_code;
  racroute_auth_status_code_actual = status_code;
  racroute_auth_identity_type_actual = identity_type;
  std::memset(racroute_auth_authid_actual, 0, sizeof(racroute_auth_authid_actual));
  const std::string authid_ascii = SEAR::toUTF8(std::string(authid, authid_length));
  std::memcpy(racroute_auth_authid_actual, authid_ascii.data(), authid_ascii.length());
  *racf_return_code                = racroute_auth_racf_rc_mock;
  *racf_reason_code                = racroute_auth_racf_reason_mock;
  return racroute_auth_rc_mock;
}