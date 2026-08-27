#ifndef __SEAR_TRAIT_VALIDATION_H_
#define __SEAR_TRAIT_VALIDATION_H_

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "security_request.hpp"

void validate_traits(std::string_view admin_type,
                     SEAR::SecurityRequest& request);
void validate_json_value_to_string(const nlohmann::json& trait,
                                   char expected_type,
                                   std::vector<std::string>& errors);

#endif
