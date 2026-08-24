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

std::string RACRouteAuth::accessName(int access_code) {
  if (access_code == RACROUTE_AUTH_ACCESS_READ) {
    return "READ";
  } else if (access_code == RACROUTE_AUTH_ACCESS_UPDATE) {
    return "UPDATE";
  } else if (access_code == RACROUTE_AUTH_ACCESS_CONTROL) {
    return "CONTROL";
  } else if (access_code == RACROUTE_AUTH_ACCESS_ALTER) {
    return "ALTER";
  }

  return "UNKNOWN";
}

int RACRouteAuth::statusCode(const nlohmann::json &options) {
  if (options == nullptr || !options.contains("status")) {
    return RACROUTE_AUTH_STATUS_NONE;
  }

  std::string status = options["status"].get<std::string>();
  std::transform(status.begin(), status.end(), status.begin(),
                 [](unsigned char character) { return std::toupper(character); });

  if (status == "NONE") {
    return RACROUTE_AUTH_STATUS_NONE;
  } else if (status == "ACCESS") {
    return RACROUTE_AUTH_STATUS_ACCESS;
  }

  throw SEARError("sear: unsupported RACROUTE AUTH status option '" + status +
                  "'");
}

void RACRouteAuth::check(SecurityRequest &request) {
  const std::string class_name_ebcdic = fromUTF8(request.getClassName());
  const std::string entity_ebcdic     = fromUTF8(request.getProfileName());
  const std::string_view class_name_view(class_name_ebcdic);
  const std::string_view entity_view(entity_ebcdic);
  const int access_code               = accessCode(request.getAccess());
  const int status_code               = statusCode(request.getRACRouteOptions());

  auto raw_request = std::make_unique<racroute_auth_request_t>();
  std::memset(raw_request.get(), 0, sizeof(racroute_auth_request_t));
  raw_request->class_name_length = class_name_view.length();
  std::memcpy(raw_request->class_name, class_name_view.data(),
              class_name_view.length());
  raw_request->entity_length = entity_view.length();
  std::memcpy(raw_request->entity, entity_view.data(), entity_view.length());
  raw_request->access_code = access_code;
  raw_request->status_code = status_code;

  Logger::getInstance().debug("RACROUTE AUTH request buffer:");
  Logger::getInstance().hexDump(reinterpret_cast<char *>(raw_request.get()),
                                sizeof(racroute_auth_request_t));

  int racf_return_code = 0;
  int racf_reason_code = 0;
  const int saf_return_code = sear_racroute_auth_asm(
      class_name_view.data(), class_name_view.length(), entity_view.data(),
      entity_view.length(), access_code, status_code, &racf_return_code,
      &racf_reason_code);

  request.setRawRequestPointer(reinterpret_cast<char *>(raw_request.get()));
  raw_request.release();
  request.setRawRequestLength(sizeof(racroute_auth_request_t));
  request.setSAFReturnCode(saf_return_code);
  request.setRACFReturnCode(racf_return_code);
  request.setRACFReasonCode(racf_reason_code);
  request.setSEARReturnCode(0);
  nlohmann::json result_json = {{"authorized", saf_return_code == 0}};
  if (status_code == RACROUTE_AUTH_STATUS_ACCESS && saf_return_code == 0) {
    result_json["access"] = accessName(racf_reason_code);
  }
  request.setIntermediateResultJSON(result_json);
}

}  // namespace SEAR