#include "racroute_auth.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

#include "conversion.hpp"
#include "logger.hpp"
#include "sear_error.hpp"

namespace SEAR {

int RACRouteAuth::accessCode(std::string_view access) {
  std::string upper_access(access);
  std::transform(upper_access.begin(), upper_access.end(), upper_access.begin(),
                 [](unsigned char character) { return std::toupper(character); });

  if (upper_access == "READ") {
    return RACROUTE_AUTH_ACCESS_READ;
  } else if (upper_access == "UPDATE") {
    return RACROUTE_AUTH_ACCESS_UPDATE;
  } else if (upper_access == "CONTROL") {
    return RACROUTE_AUTH_ACCESS_CONTROL;
  } else if (upper_access == "ALTER") {
    return RACROUTE_AUTH_ACCESS_ALTER;
  }

  throw SEARError("sear: unsupported RACROUTE access level '" +
                  std::string(access) + "'");
}

void RACRouteAuth::check(SecurityRequest &request) {
  const std::string class_name_ebcdic = fromUTF8(request.getClassName());
  const std::string entity_ebcdic     = fromUTF8(request.getProfileName());
  const std::string_view class_name_view(class_name_ebcdic);
  const std::string_view entity_view(entity_ebcdic);
  const int access_code               = accessCode(request.getAccess());

  auto raw_request = std::make_unique<racroute_auth_request_t>();
  std::memset(raw_request.get(), 0, sizeof(racroute_auth_request_t));
  raw_request->class_name_length = class_name_view.length();
  std::memcpy(raw_request->class_name, class_name_view.data(),
              class_name_view.length());
  raw_request->entity_length = entity_view.length();
  std::memcpy(raw_request->entity, entity_view.data(), entity_view.length());
  raw_request->access_code = access_code;

  Logger::getInstance().debug("RACROUTE AUTH request buffer:");
  Logger::getInstance().hexDump(reinterpret_cast<char *>(raw_request.get()),
                                sizeof(racroute_auth_request_t));

  const int saf_return_code = sear_racroute_auth_asm(
    class_name_view.data(), class_name_view.length(), entity_view.data(),
    entity_view.length(), access_code);

  request.setRawRequestPointer(reinterpret_cast<char *>(raw_request.get()));
  raw_request.release();
  request.setRawRequestLength(sizeof(racroute_auth_request_t));
  request.setSAFReturnCode(saf_return_code);
  request.setRACFReturnCode(0);
  request.setRACFReasonCode(0);
  request.setSEARReturnCode(0);
  request.setIntermediateResultJSON({{"authorized", saf_return_code == 0}});
}

}  // namespace SEAR