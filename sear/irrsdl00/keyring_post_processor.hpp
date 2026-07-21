#ifndef __SEAR_KEYRING_POST_PROCESSOR_H_
#define __SEAR_KEYRING_POST_PROCESSOR_H_

#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <sstream>

#include "extractor.hpp"
#include "irrsdl00.hpp"
#include "sear_error.hpp"
#include "security_request.hpp"

// Use ntohl() to convert 32-bit values from big endian to little endian.
// use ntohs() to convert 16-bit values from big endian to little endian.
// On z/OS these macros do nothing since "network order" and z/Architecture are
// both big endian. This is only necessary for unit testing off platform.
#include <arpa/inet.h>

struct OpenSSLFree {
  void operator()(void *ptr) const {
    EVP_MD_CTX_free(reinterpret_cast<EVP_MD_CTX *>(ptr));
  }
};

template <typename T>
using OpenSSLPointer = std::unique_ptr<T, OpenSSLFree>;

namespace SEAR {
class KeyringPostProcessor {
 public:
  static void postProcessExtractKeyring(SecurityRequest &request);
  static void postProcessAddOrDeleteKeyring(SecurityRequest &request);

 private:
  static void convertASN1TIME(const ASN1_TIME *t, char *p_buf, size_t buf_len);
  static bool addSignature(nlohmann::json &add_to_json, X509 *x509_cert);
  static bool addHashs(nlohmann::json &add_to_json, void *p_cert,
                       size_t len_cert);
  static bool addUsages(nlohmann::json &add_to_json,
                        const X509_EXTENSION *p_ext);
  static bool addExtUsages(nlohmann::json &add_to_json,
                           const X509_EXTENSION *p_ext);
  static bool addSubjectAltName(nlohmann::json &add_to_json,
                                const X509_EXTENSION *p_ext);
  static bool addBasicConstraints(nlohmann::json &add_to_json,
                                  const X509_EXTENSION *p_ext);
  static bool addGenericExtension(nlohmann::json &add_to_json,
                                  const X509_EXTENSION *p_ext);
  static std::string strToHex(const std::uint8_t *data, const std::size_t len);
};
}  // namespace SEAR

#endif  // __SEAR_KEYRING_POST_PROCESSOR_H
