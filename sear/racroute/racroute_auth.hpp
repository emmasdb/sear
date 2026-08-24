#ifndef __SEAR_RACROUTE_AUTH_H_
#define __SEAR_RACROUTE_AUTH_H_

#include <cstdint>
#include <string>
#include <string_view>

#include "security_request.hpp"

namespace SEAR {

constexpr int RACROUTE_AUTH_ACCESS_READ    = 0x02;
constexpr int RACROUTE_AUTH_ACCESS_UPDATE  = 0x04;
constexpr int RACROUTE_AUTH_ACCESS_CONTROL = 0x08;
constexpr int RACROUTE_AUTH_ACCESS_ALTER   = 0x80;
constexpr int RACROUTE_AUTH_STATUS_NONE    = 0;
constexpr int RACROUTE_AUTH_STATUS_ACCESS  = 1;
constexpr int RACROUTE_AUTH_STATUS_ACCESS_RC = 0x14;
constexpr int RACROUTE_AUTH_STATUS_ACCESS_NONE = 0x00;
constexpr int RACROUTE_AUTH_STATUS_ACCESS_READ = 0x04;
constexpr int RACROUTE_AUTH_STATUS_ACCESS_UPDATE = 0x08;
constexpr int RACROUTE_AUTH_STATUS_ACCESS_CONTROL = 0x0c;
constexpr int RACROUTE_AUTH_STATUS_ACCESS_ALTER = 0x10;
constexpr int RACROUTE_AUTH_IDENTITY_NONE = 0;
constexpr int RACROUTE_AUTH_IDENTITY_USER = 1;
constexpr int RACROUTE_AUTH_IDENTITY_GROUP = 2;

#pragma pack(push, 1)
typedef struct {
  uint32_t class_name_length;
  char class_name[8];
  uint32_t entity_length;
  char entity[246];
  uint32_t access_code;
  uint32_t status_code;
  uint32_t identity_type;
  uint32_t authid_length;
  char authid[8];
} racroute_auth_request_t;
#pragma pack(pop)

class RACRouteAuth {
 public:
  void check(SecurityRequest &request);

 private:
  static int accessCode(std::string_view access);
  static std::string statusAccessName(int access_code);
  static int statusCode(const nlohmann::json &options);
};

}  // namespace SEAR

extern "C" int sear_racroute_auth_asm(const SEAR::racroute_auth_request_t *request,
                                       int *racf_return_code,
                                       int *racf_reason_code);

#endif