#ifndef __SEAR_SECURITY_ADMIN_H_
#define __SEAR_SECURITY_ADMIN_H_

#include <cstdint>

#include "extractor.hpp"
#include "keyring_modifier.hpp"
#include "logger.hpp"
#include "sear_result.h"
#include "security_request.hpp"

namespace SEAR {
class SecurityAdmin {
 private:
  SecurityRequest request_;
  void doExtract(Extractor &extractor);
  void doAddAlterDelete();
  void doAddAlterDeleteKeyring(KeyringModifier &modifier);
  void doAddCertificate(KeyringModifier &modifier);
  void doDeleteCertificate(KeyringModifier &modifier);
  void doRemoveCertificate(KeyringModifier &modifier);

 public:
  SecurityAdmin(sear_result_t *p_result, bool debug);
  void makeRequest(const char *p_request_json_string, int length);
};
}  // namespace SEAR

#endif
