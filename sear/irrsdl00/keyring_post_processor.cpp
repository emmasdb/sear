#include "keyring_post_processor.hpp"

#include <cstring>

namespace SEAR {
void KeyringPostProcessor::postProcessExtractKeyring(SecurityRequest &request) {
  nlohmann::json keyring;
  keyring["keyring"] = nlohmann::json::object();

  union {
    char RACF_user_id[9];
    char label[256];
    char datestr[128];
  } union_work;

  std::vector<nlohmann::json> repeat_group_certs;
  std::vector<nlohmann::json> repeat_group_extensions;

  keyring_extract_parms_results_t *p_result_buffer =
      reinterpret_cast<keyring_extract_parms_results_t *>(
          request.getRawResultPointer());
  request.setRawResultLength(p_result_buffer->result_buffer_length);

  uint32_t keyrings_count =
      ntohl(p_result_buffer->union_ring_result.ring_result.ring_count);
  char *work = reinterpret_cast<char *>(
      &p_result_buffer->union_ring_result.ring_result.ring_info);

  if (keyrings_count > 0) {
    int help_len;
    int cert_index = 0;

    // skip user_id
    help_len = *work;
    work++;
    work += help_len;

    // skip ring name
    help_len = *work;
    work++;
    work += help_len;

    int repeat_group_certs_count = ntohl(*(reinterpret_cast<uint32_t *>(work)));
    work += 4;

    for (int j = 0; j < repeat_group_certs_count; j++) {
      repeat_group_certs.push_back(nlohmann::json::object());

      std::memset(&union_work.RACF_user_id[0], 0, 9);
      help_len = *work;
      work++;
      std::strncpy(&union_work.RACF_user_id[0], work, help_len);
      __e2a_l(&union_work.RACF_user_id[0], help_len);
      repeat_group_certs[j]["owner"] = union_work.RACF_user_id;
      work += help_len;

      std::memset(&union_work.label[0], 0, 256);
      help_len = *work;
      work++;
      std::strncpy(&union_work.label[0], work, help_len);
      __e2a_l(&union_work.label[0], help_len);
      repeat_group_certs[j]["label"] = union_work.label;
      work += help_len;

      cddlx_get_cert_t *p_parm_get_cert =
          &p_result_buffer->p_get_cert_buffer->result_buffer_get_cert;
      p_parm_get_cert = reinterpret_cast<cddlx_get_cert_t *>(
          ((reinterpret_cast<uint64_t>(p_parm_get_cert)) +
           (cert_index * sizeof(get_cert_buffer_t))));

      // DN
      const unsigned char *lpSDN = p_parm_get_cert->cddlx_sdn_ptr;
      X509_NAME *x509_name_out =
          d2i_X509_NAME(NULL, &lpSDN, p_parm_get_cert->cddlx_sdn_len);
      if (x509_name_out) {
        char *lpDN = X509_NAME_oneline(x509_name_out, 0, 0);
        if (lpDN) {
          repeat_group_certs[j]["DN"] = lpDN;
          OPENSSL_free(lpDN);
          lpDN = nullptr;
        } else {
          throw SEARError(std::string("X509_NAME_oneline failed"));
        }
        X509_NAME_free(x509_name_out);
      } else {
        throw SEARError(std::string("d2i_X509_NAME failed"));
      }

      // usage
      if (ntohl(*(reinterpret_cast<uint32_t *>(
              &p_parm_get_cert->cddlx_cert_usage[0]))) == 8) {
        repeat_group_certs[j]["usage"] = "personal";
      } else if (ntohl(*(reinterpret_cast<uint32_t *>(
                     &p_parm_get_cert->cddlx_cert_usage[0]))) == 2) {
        repeat_group_certs[j]["usage"] = "certauth";
      } else if (ntohl(*(reinterpret_cast<uint32_t *>(
                     &p_parm_get_cert->cddlx_cert_usage[0]))) == 0) {
        repeat_group_certs[j]["usage"] = "site";
      }

      // default
      if (p_parm_get_cert->cddlx_cert_default) {
        repeat_group_certs[j]["default"] = "yes";
      }

      // status
      if (ntohl(*(reinterpret_cast<uint32_t *>(
              &p_parm_get_cert->cddlx_status[0]))) == 0x80000000) {
        repeat_group_certs[j]["status"] = "TRUST";
      } else if (ntohl(*(reinterpret_cast<uint32_t *>(
                     &p_parm_get_cert->cddlx_status[0]))) == 0x40000000) {
        repeat_group_certs[j]["status"] = "HIGHTRUST";
      } else if (ntohl(*(reinterpret_cast<uint32_t *>(
                     &p_parm_get_cert->cddlx_status[0]))) == 0x20000000) {
        repeat_group_certs[j]["status"] = "NOTRUST";
      }

      // Private key
      if (p_parm_get_cert->cddlx_pk_len > 0) {
        repeat_group_certs[j]["privateKey"] = "yes";
        repeat_group_certs[j]["keySize"] =
            ntohl(p_parm_get_cert->cddlx_pk_bitsize);
      }

      // Get cert details
      const unsigned char *p_cert = reinterpret_cast<const unsigned char *>(
          p_parm_get_cert->cddlx_cert_ptr);
      X509 *x509_cert =
          d2i_X509(NULL, &p_cert, p_parm_get_cert->cddlx_cert_len);

      if (x509_cert) {
        // Version
        int version                      = X509_get_version(x509_cert) + 1;
        repeat_group_certs[j]["version"] = version;

        // Serial number
        const ASN1_INTEGER *serial = X509_get0_serialNumber(x509_cert);
        repeat_group_certs[j]["serialNumber"] =
            strToHex(ASN1_STRING_get0_data(serial), ASN1_STRING_length(serial));

        // Issuer
        const X509_NAME *issuer_name = X509_get_issuer_name(x509_cert);
        if (issuer_name) {
          char *lpDN = X509_NAME_oneline(issuer_name, 0, 0);
          if (lpDN) {
            repeat_group_certs[j]["issuer"] = lpDN;
            OPENSSL_free(lpDN);
            lpDN = nullptr;
          } else {
            throw SEARError(std::string("X509_NAME_oneline failed"));
          }
        } else {
          throw SEARError(std::string("X509_get_issuer_name failed"));
        }

        // Validity
        const ASN1_TIME *not_before = X509_get0_notBefore(x509_cert);
        if (not_before) {
          convertASN1TIME(not_before, &union_work.datestr[0],
                          sizeof(union_work.datestr));
          repeat_group_certs[j]["notBefore"] = union_work.datestr;
        }
        const ASN1_TIME *not_after = X509_get0_notAfter(x509_cert);
        if (not_after) {
          convertASN1TIME(not_after, &union_work.datestr[0],
                          sizeof(union_work.datestr));
          repeat_group_certs[j]["notAfter"] = union_work.datestr;
        }

        // Hash's
        addHashs(repeat_group_certs[j], p_parm_get_cert->cddlx_cert_ptr,
                 ntohl(p_parm_get_cert->cddlx_cert_len));

        // Signature
        addSignature(repeat_group_certs[j], x509_cert);

        // Extensions
        const STACK_OF(X509_EXTENSION) *p_ext_stack =
            X509_get0_extensions(x509_cert);

        if (p_ext_stack) {
          for (int k = 0; k < sk_X509_EXTENSION_num(p_ext_stack); k++) {
            X509_EXTENSION *p_ext = sk_X509_EXTENSION_value(p_ext_stack, k);
            const ASN1_OBJECT *p_obj    = X509_EXTENSION_get_object(p_ext);

            unsigned int nid      = OBJ_obj2nid(p_obj);

            repeat_group_extensions.push_back(nlohmann::json::object());

            if (!X509_EXTENSION_get_critical(p_ext))
              repeat_group_extensions[k]["critical"] = "yes";
            else
              repeat_group_extensions[k]["critical"] = "no";

            if (nid == NID_undef) {
              char extname[256];
              OBJ_obj2txt(extname, sizeof(extname), p_obj, 1);
              repeat_group_extensions[k]["name"] = extname;
            } else {
              const char *p_extname = OBJ_nid2sn(nid);

              switch (nid) {
                case NID_key_usage:
                  addUsages(repeat_group_extensions[k], p_ext);
                  break;

                case NID_ext_key_usage:
                  addExtUsages(repeat_group_extensions[k], p_ext);
                  break;

                case NID_subject_alt_name:
                  addSubjectAltName(repeat_group_extensions[k], p_ext);
                  break;

                case NID_basic_constraints:
                  addBasicConstraints(repeat_group_extensions[k], p_ext);
                  break;

                default:
                  addGenericExtension(repeat_group_extensions[k], p_ext);
              }

              repeat_group_extensions[k]["name"] = p_extname;
            }
          }

          repeat_group_certs[j]["extensions"] = repeat_group_extensions;
          repeat_group_extensions.clear();
        }

        X509_free(x509_cert);
      }

      cert_index++;
    }

    keyring["keyring"]["certificates"] = repeat_group_certs;
    repeat_group_certs.clear();
  }

  request.setIntermediateResultJSON(keyring);
}

void KeyringPostProcessor::convertASN1TIME(const ASN1_TIME *t, char *p_buf,
                                           size_t buf_len) {
  int rc;

  tm sometime;
  rc = ASN1_TIME_to_tm(t, &sometime);
  if (rc <= 0) {
    throw SEARError(std::string("ASN1_TIME_print failed or wrote no data."));
  }

  std::snprintf(p_buf, buf_len, "%04d-%02d-%02d %02d:%02d:%02d",
                sometime.tm_year + 1900, sometime.tm_mon + 1, sometime.tm_mday,
                sometime.tm_hour, sometime.tm_min, sometime.tm_sec);
}

bool KeyringPostProcessor::addSignature(nlohmann::json &add_to_json,
                                        X509 *x509_cert) {
  bool ret                        = true;

  nlohmann::json signature        = nlohmann::json::object();

  const ASN1_BIT_STRING *asn1_sig = nullptr;
  const X509_ALGOR *sig_type      = nullptr;

  X509_get0_signature(&asn1_sig, &sig_type, x509_cert);

  char algo[128];
  OBJ_obj2txt(algo, sizeof(algo), sig_type->algorithm, 0);

  signature["algorithm"]   = algo;
  signature["value"]       = strToHex(ASN1_STRING_get0_data(asn1_sig),
                                      ASN1_STRING_length(asn1_sig));

  add_to_json["signature"] = signature;

  return ret;
}

bool KeyringPostProcessor::addHashs(nlohmann::json &add_to_json, void *p_cert,
                                    size_t len_cert) {
  bool ret = true;

  std::vector<nlohmann::json> repeat_group_fingerprints;

  OpenSSLPointer<EVP_MD_CTX> context(EVP_MD_CTX_new());

  if (context) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    if (EVP_DigestInit_ex(context.get(), EVP_sha256(), NULL)) {
      if (EVP_DigestUpdate(context.get(), p_cert, len_cert)) {
        if (EVP_DigestFinal_ex(context.get(), hash, &hash_len)) {
          nlohmann::json sha256obj = nlohmann::json::object();
          sha256obj["sha256"]      = strToHex(&hash[0], hash_len);
          repeat_group_fingerprints.push_back(sha256obj);
        } else
          throw SEARError(std::string("EVP_DigestFinal_ex failed"));
      } else
        throw SEARError(std::string("EVP_DigestUpdate failed"));
    } else
      throw SEARError(std::string("EVP_DigestInit_ex failed"));

    if (EVP_DigestInit_ex(context.get(), EVP_sha1(), NULL)) {
      if (EVP_DigestUpdate(context.get(), p_cert, len_cert)) {
        if (EVP_DigestFinal_ex(context.get(), hash, &hash_len)) {
          nlohmann::json sha1obj = nlohmann::json::object();
          sha1obj["sha1"]        = strToHex(&hash[0], hash_len);
          repeat_group_fingerprints.push_back(sha1obj);
        } else
          throw SEARError(std::string("EVP_DigestFinal_ex failed"));
      } else
        throw SEARError(std::string("EVP_DigestUpdate failed"));
    } else
      throw SEARError(std::string("EVP_DigestInit_ex failed"));

    if (EVP_DigestInit_ex(context.get(), EVP_md5(), NULL)) {
      if (EVP_DigestUpdate(context.get(), p_cert, len_cert)) {
        if (EVP_DigestFinal_ex(context.get(), hash, &hash_len)) {
          nlohmann::json md5obj = nlohmann::json::object();
          md5obj["md5"]         = strToHex(&hash[0], hash_len);
          repeat_group_fingerprints.push_back(md5obj);
        } else
          throw SEARError(std::string("EVP_DigestFinal_ex failed"));
      } else
        throw SEARError(std::string("EVP_DigestUpdate failed"));
    } else
      throw SEARError(std::string("EVP_DigestInit_ex failed"));

    add_to_json["fingerprints"] = repeat_group_fingerprints;
  }

  return ret;
}

bool KeyringPostProcessor::addUsages(nlohmann::json &add_to_json,
                                     X509_EXTENSION *p_ext) {
  bool ret = true;

  std::vector<std::string> repeat_group_usages;

  ASN1_BIT_STRING *usage =
      reinterpret_cast<ASN1_BIT_STRING *>(X509V3_EXT_d2i(p_ext));

  const unsigned char *usage_data = ASN1_STRING_get0_data(usage);
  int usage_length                = ASN1_STRING_length(usage);

  int flags = usage_data[0];
  if (usage_length > 1) flags |= usage_data[1] << 8;

  if (flags & X509v3_KU_DIGITAL_SIGNATURE)
    repeat_group_usages.push_back("digitalSignature");
  if (flags & X509v3_KU_NON_REPUDIATION)
    repeat_group_usages.push_back("nonRepudiation");
  if (flags & X509v3_KU_KEY_ENCIPHERMENT)
    repeat_group_usages.push_back("keyEncipherment");
  if (flags & X509v3_KU_DATA_ENCIPHERMENT)
    repeat_group_usages.push_back("dataEncipherment");
  if (flags & X509v3_KU_KEY_AGREEMENT)
    repeat_group_usages.push_back("keyAgreement");
  if (flags & X509v3_KU_KEY_CERT_SIGN)
    repeat_group_usages.push_back("keyCertSign");
  if (flags & X509v3_KU_CRL_SIGN) repeat_group_usages.push_back("cRLSign");
  if (flags & X509v3_KU_ENCIPHER_ONLY)
    repeat_group_usages.push_back("encipherOnly");
  if (flags & X509v3_KU_DECIPHER_ONLY)
    repeat_group_usages.push_back("decipherOnly");

  add_to_json["usages"] = repeat_group_usages;
  repeat_group_usages.clear();

  return ret;
}

bool KeyringPostProcessor::addExtUsages(nlohmann::json &add_to_json,
                                        X509_EXTENSION *p_ext) {
  bool ret = true;

  std::vector<std::string> repeat_group_usages;

  EXTENDED_KEY_USAGE *usage =
      reinterpret_cast<EXTENDED_KEY_USAGE *>(X509V3_EXT_d2i(p_ext));

  for (int l = 0; l < sk_ASN1_OBJECT_num(usage); l++) {
    const char *p_usage =
        OBJ_nid2sn(OBJ_obj2nid(sk_ASN1_OBJECT_value(usage, l)));
    repeat_group_usages.push_back(p_usage);
  }
  add_to_json["usages"] = repeat_group_usages;
  repeat_group_usages.clear();

  EXTENDED_KEY_USAGE_free(usage);

  return ret;
}

bool KeyringPostProcessor::addSubjectAltName(nlohmann::json &add_to_json,
                                             X509_EXTENSION *p_ext) {
  bool ret = true;

  std::vector<nlohmann::json> repeat_group_altnames;

  char name_buffer[1024];

  GENERAL_NAMES *subject_alt_names =
      reinterpret_cast<GENERAL_NAMES *>(X509V3_EXT_d2i(p_ext));

  for (int l = 0; l < sk_GENERAL_NAME_num(subject_alt_names); l++) {
    GENERAL_NAME *name = sk_GENERAL_NAME_value(subject_alt_names, l);

    if (name->type == GEN_DNS) {
      repeat_group_altnames.push_back(nlohmann::json::object());

      const unsigned char *dns_name = ASN1_STRING_get0_data(name->d.dNSName);
      int dns_name_length           = ASN1_STRING_length(name->d.dNSName);
      repeat_group_altnames[l]["type"] = "DNS";
      repeat_group_altnames[l]["name"] = std::string(
          reinterpret_cast<const char *>(dns_name), dns_name_length);
    } else if (name->type == GEN_IPADD) {
      repeat_group_altnames.push_back(nlohmann::json::object());

      const unsigned char *ip_address = ASN1_STRING_get0_data(name->d.iPAddress);
      int ip_address_length           = ASN1_STRING_length(name->d.iPAddress);
      if (ip_address_length == 4) {
        std::snprintf(&name_buffer[0], sizeof(name_buffer), "%d.%d.%d.%d",
                      ip_address[0], ip_address[1], ip_address[2],
                      ip_address[3]);
        repeat_group_altnames[l]["type"] = "IPv4";
        repeat_group_altnames[l]["name"] = name_buffer;
      } else if (ip_address_length == 16) {
        std::snprintf(&name_buffer[0], sizeof(name_buffer),
                      "%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X:%4.4X",
                      ip_address[0], ip_address[1], ip_address[2],
                      ip_address[3], ip_address[4], ip_address[5],
                      ip_address[6], ip_address[7]);
        repeat_group_altnames[l]["type"] = "IPv6";
        repeat_group_altnames[l]["name"] = name_buffer;
      }
    } else {
      X509V3_EXT_METHOD *method =
          const_cast<X509V3_EXT_METHOD *>(X509V3_EXT_get(p_ext));
      STACK_OF(CONF_VALUE) *nval = i2v_GENERAL_NAME(
          reinterpret_cast<X509V3_EXT_METHOD *>(method), name, NULL);
      if (nval) {
        BIO *some_bio = BIO_new(BIO_s_mem());
        X509V3_EXT_val_prn(some_bio, nval, 0, 0);
        BIO_gets(some_bio, &name_buffer[0], sizeof(name_buffer));
        repeat_group_altnames[l]["type"] = "other";
        repeat_group_altnames[l]["name"] = name_buffer;

        BIO_free_all(some_bio);
        sk_CONF_VALUE_pop_free(nval, X509V3_conf_free);
      }
    }
  }
  add_to_json["altnames"] = repeat_group_altnames;
  repeat_group_altnames.clear();

  sk_GENERAL_NAME_pop_free(subject_alt_names, GENERAL_NAME_free);

  return ret;
}

bool KeyringPostProcessor::addBasicConstraints(nlohmann::json &add_to_json,
                                               X509_EXTENSION *p_ext) {
  bool ret = true;

  nlohmann::json constraints;

  BASIC_CONSTRAINTS *bs =
      reinterpret_cast<BASIC_CONSTRAINTS *>(X509V3_EXT_d2i(p_ext));

  if (bs->ca) constraints["isCA"] = "yes";
  if (bs->pathlen)
    constraints["pathLenConstraint"] = ASN1_INTEGER_get(bs->pathlen);

  add_to_json["constraints"] = constraints;
  constraints.clear();

  BASIC_CONSTRAINTS_free(bs);

  return ret;
}

bool KeyringPostProcessor::addGenericExtension(nlohmann::json &add_to_json,
                                               X509_EXTENSION *p_ext) {
  bool ret     = true;

  BIO *ext_bio = BIO_new(BIO_s_mem());

  X509V3_EXT_print(ext_bio, p_ext, 0, 0);

  char *bio_data  = nullptr;
  long bio_length = BIO_get_mem_data(ext_bio, &bio_data);

  std::string value(bio_data, bio_length);
  while (!value.empty() && (value.back() == '\n' || value.back() == '\r')) {
    value.pop_back();
  }

  add_to_json["value"] = value;

  BIO_free_all(ext_bio);

  return ret;
}

void KeyringPostProcessor::postProcessAddOrDeleteKeyring(
    SecurityRequest &request) {}

std::string KeyringPostProcessor::strToHex(const std::uint8_t *data,
                                           const std::size_t len) {
  std::stringstream ss;
  ss << std::hex;

  for (std::size_t i = 0; i < len; ++i) {
    if (i > 0) ss << ":";
    ss << std::uppercase << std::setw(2) << std::setfill('0')
       << static_cast<int>(data[i]);
  }

  return ss.str();
}

}  // namespace SEAR
