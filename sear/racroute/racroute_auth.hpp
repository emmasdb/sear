#ifndef __SEAR_RACROUTE_AUTH_H_
#define __SEAR_RACROUTE_AUTH_H_

#include <cstdint>
#include <string_view>

#include "security_request.hpp"

namespace SEAR {

constexpr int RACROUTE_AUTH_ACCESS_READ    = 1;
constexpr int RACROUTE_AUTH_ACCESS_UPDATE  = 2;
constexpr int RACROUTE_AUTH_ACCESS_CONTROL = 3;
constexpr int RACROUTE_AUTH_ACCESS_ALTER   = 4;

#pragma pack(push, 1)
typedef struct {
  uint32_t class_name_length;
  char class_name[8];
  uint32_t entity_length;
  char entity[246];
  uint32_t access_code;
} racroute_auth_request_t;
#pragma pack(pop)

class RACRouteAuth {
 public:
  void check(SecurityRequest &request);

 private:
  static int accessCode(std::string_view access);
};

}  // namespace SEAR

extern "C" int sear_racroute_auth_asm(const char *class_name,
                                       int class_name_length,
                                       const char *entity, int entity_length,
                                       int access_code);

#endif