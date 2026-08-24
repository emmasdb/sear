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

#pragma pack(push, 1)
typedef struct {
  uint32_t class_name_length;
  char class_name[8];
  uint32_t entity_length;
  char entity[246];
  uint32_t access_code;
  uint32_t status_code;
} racroute_auth_request_t;
#pragma pack(pop)

class RACRouteAuth {
 public:
  void check(SecurityRequest &request);

 private:
  static int accessCode(std::string_view access);
  static std::string accessName(int access_code);
  static int statusCode(const nlohmann::json &options);
};

}  // namespace SEAR

extern "C" int sear_racroute_auth_asm(const char *class_name,
                                       int class_name_length,
                                       const char *entity, int entity_length,
                                       int access_code,
                                       int status_code,
                                       int *racf_return_code,
                                       int *racf_reason_code);

#endif