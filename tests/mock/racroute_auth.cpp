#include "sear/racroute/racroute_auth.hpp"

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

extern "C" int sear_racroute_auth_asm(const SEAR::racroute_auth_request_t *request,
                                       int *racf_return_code,
                                       int *racf_reason_code) {
  racroute_auth_access_code_actual = request->access_code;
  racroute_auth_status_code_actual = request->status_code;
  racroute_auth_identity_type_actual = request->identity_type;
  std::memset(racroute_auth_authid_actual, 0, sizeof(racroute_auth_authid_actual));
  const std::string authid_ascii = SEAR::toUTF8(
      std::string(request->authid, request->authid_length));
  std::memcpy(racroute_auth_authid_actual, authid_ascii.data(), authid_ascii.length());
  *racf_return_code                = racroute_auth_racf_rc_mock;
  *racf_reason_code                = racroute_auth_racf_reason_mock;
  return racroute_auth_rc_mock;
}