#ifndef __SEAR_XML_GENERATOR_H_
#define __SEAR_XML_GENERATOR_H_

#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <string>
#include <string_view>

#include "logger.hpp"
#include "security_request.hpp"

namespace SEAR {
// XMLGenerator Generates an XML String from a JSON string
class XMLGenerator {
 private:
  static std::string convertOperation(std::string_view operation);
  static std::string convertOperator(std::string_view trait_operator);
  static std::string convertAdminType(std::string_view admin_type);
  std::string JSONValueToString(const nlohmann::json& trait);
  void buildPugixmlHeaderAttributes(pugi::xml_node& node,
                                    const SEAR::SecurityRequest& request,
                                    std::string_view true_admin_type);
  void buildPugixmlRequestData(pugi::xml_node& node,
                               std::string_view true_admin_type,
                               std::string_view admin_type,
                               nlohmann::json request_data);
  static void buildPugixmlSingleTrait(pugi::xml_node& node,
                                      std::string_view tag,
                                      std::string_view operation,
                                      std::string_view value);

 public:
  void buildXMLString(SEAR::SecurityRequest& request);
};
}  // namespace SEAR

#endif
