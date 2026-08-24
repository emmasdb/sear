#include "irrsim00.hpp"

#include <arpa/inet.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>

#include "logger.hpp"
#include "sear_error.hpp"

namespace SEAR {
namespace {
struct IRRSIM00ArgAreaDeleter {
  void operator()(void *ptr) const {
    Logger::getInstance().debugFree(ptr);
    std::free(ptr);
    Logger::getInstance().debug("Done");
  }
};

std::unique_ptr<irrsim00_arg_area_t, IRRSIM00ArgAreaDeleter>
makeIRRSIM00ArgArea() {
  auto *p_arg_area = static_cast<irrsim00_arg_area_t *>(
      __malloc31(sizeof(irrsim00_arg_area_t)));
  if (p_arg_area == nullptr) {
    throw std::bad_alloc();
  }
  Logger::getInstance().debugAllocate(p_arg_area, 31,
                                      sizeof(irrsim00_arg_area_t));
  return std::unique_ptr<irrsim00_arg_area_t, IRRSIM00ArgAreaDeleter>(
      p_arg_area);
}
}  // namespace

void IRRSIM00::map(SecurityRequest &request) {
  auto arg_area_unique_ptr = makeIRRSIM00ArgArea();
  irrsim00_arg_area_t *p_arg_area = arg_area_unique_ptr.get();
  std::memset(p_arg_area, 0, sizeof(irrsim00_arg_area_t));

  IRRSIM00::buildRequest(p_arg_area, request);
  request.setRawRequestLength((int)sizeof(irrsim00_arg_area_t));
  request.setRawRequestPointer(IRRSIM00::cloneBuffer(
      reinterpret_cast<char *>(p_arg_area), request.getRawRequestLength()));

  Logger::getInstance().debug("IRRSIM00 request buffer:");
  Logger::getInstance().hexDump(reinterpret_cast<char *>(p_arg_area),
                                request.getRawRequestLength());

  Logger::getInstance().debug("Calling IRRSIM00 ...");
  callIrrsim00(reinterpret_cast<char *__ptr32>(&p_arg_area->arg_pointers));
  Logger::getInstance().debug("Done");

  request.setSAFReturnCode(p_arg_area->saf_return_code);
  request.setRACFReturnCode(p_arg_area->racf_return_code);
  request.setRACFReasonCode(p_arg_area->racf_reason_code);
  request.setRawResultLength((int)sizeof(irrsim00_arg_area_t));
  request.setRawResultPointer(IRRSIM00::cloneBuffer(
      reinterpret_cast<char *>(p_arg_area), request.getRawResultLength()));

  if (request.getSAFReturnCode() != 0 || request.getRACFReturnCode() != 0 ||
      request.getRACFReasonCode() != 0) {
    request.setSEARReturnCode(4);
    throw SEARError("unable to map application user");
  }

  request.setIntermediateResultJSON(
      IRRSIM00::buildResultJSON(*p_arg_area, request.getFunctionCode()));
  request.setSEARReturnCode(0);
}

void IRRSIM00::buildRequest(irrsim00_arg_area_t *p_arg_area,
                            const SecurityRequest &request) {
  p_arg_area->alet_saf_return_code  = 0;
  p_arg_area->alet_racf_return_code = 0;
  p_arg_area->alet_racf_reason_code = 0;
  p_arg_area->alet_remainder        = 0;
  p_arg_area->function_code         = request.getFunctionCode();
  p_arg_area->option_word           = 0;
  p_arg_area->arg_pointers.p_work_area =
      reinterpret_cast<char *__ptr32>(p_arg_area->work_area);
  p_arg_area->arg_pointers.p_alet_saf_return_code =
      &p_arg_area->alet_saf_return_code;
  p_arg_area->arg_pointers.p_saf_return_code = &p_arg_area->saf_return_code;
  p_arg_area->arg_pointers.p_alet_racf_return_code =
      &p_arg_area->alet_racf_return_code;
  p_arg_area->arg_pointers.p_racf_return_code =
      &p_arg_area->racf_return_code;
  p_arg_area->arg_pointers.p_alet_racf_reason_code =
      &p_arg_area->alet_racf_reason_code;
  p_arg_area->arg_pointers.p_racf_reason_code =
      &p_arg_area->racf_reason_code;
  p_arg_area->arg_pointers.p_alet_remainder = &p_arg_area->alet_remainder;
  p_arg_area->arg_pointers.p_function_code = &p_arg_area->function_code;
  p_arg_area->arg_pointers.p_option_word   = &p_arg_area->option_word;
  p_arg_area->arg_pointers.p_racf_userid   = &p_arg_area->racf_userid;
  p_arg_area->arg_pointers.p_certificate   = &p_arg_area->certificate;
  p_arg_area->arg_pointers.p_application_userid =
      &p_arg_area->application_userid;
  p_arg_area->arg_pointers.p_distinguished_name =
      &p_arg_area->distinguished_name;
  p_arg_area->arg_pointers.p_registry_name = &p_arg_area->registry_name;
#ifdef __TOS_390__
  *(reinterpret_cast<uint32_t *__ptr32>(
      &p_arg_area->arg_pointers.p_registry_name)) |= 0x80000000;
#endif

  if (!request.getProfileName().empty()) {
    IRRSIM00::copyRACFUserID(&p_arg_area->racf_userid,
                             request.getProfileName());
  }

  if (!request.getCertificateFile().empty()) {
    IRRSIM00::readCertificate(request.getCertificateFile(),
                              &p_arg_area->certificate);
  }

  IRRSIM00::copyText(&p_arg_area->application_userid.length,
                     p_arg_area->application_userid.value,
                     sizeof(p_arg_area->application_userid.value),
                     request.getApplicationUserID());
  IRRSIM00::copyText(&p_arg_area->distinguished_name.length,
                     p_arg_area->distinguished_name.value,
                     sizeof(p_arg_area->distinguished_name.value),
                     request.getDistinguishedName());
  IRRSIM00::copyText(&p_arg_area->registry_name.length,
                     p_arg_area->registry_name.value,
                     sizeof(p_arg_area->registry_name.value),
                     request.getRegistryName());
}

char *IRRSIM00::cloneBuffer(const char *p_buffer, int buffer_length) {
  auto buffer_unique_ptr = std::make_unique<char[]>(buffer_length);
  Logger::getInstance().debugAllocate(buffer_unique_ptr.get(), 64,
                                      buffer_length);
  std::memcpy(buffer_unique_ptr.get(), p_buffer, buffer_length);
  char *p_clone = buffer_unique_ptr.get();
  buffer_unique_ptr.release();
  return p_clone;
}

void IRRSIM00::copyRACFUserID(irrsim00_racf_userid_t *p_target,
                              std::string userid) {
  std::transform(userid.begin(), userid.end(), userid.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  p_target->length = userid.length();
  std::memcpy(p_target->value, userid.c_str(), userid.length());
}

void IRRSIM00::copyText(uint16_t *p_length, char *p_target,
                        std::size_t target_size,
                        const std::string &value) {
  if (value.empty()) {
    return;
  }
  if (value.length() > target_size) {
    throw SEARError("IRRSIM00 text parameter is too long");
  }
  *p_length = htons((uint16_t)value.length());
  std::memcpy(p_target, value.c_str(), value.length());
}

void IRRSIM00::readCertificate(const std::string &filename,
                               irrsim00_certificate_t *p_certificate) {
  FILE *fp = std::fopen(filename.c_str(), "rb");
  if (fp == nullptr) {
    throw SEARError("unable to open certificate file '" + filename + "'");
  }
  std::fseek(fp, 0L, SEEK_END);
  long file_size = std::ftell(fp);
  std::rewind(fp);
  if (file_size < 0 || file_size > IRRSIM00_CERTIFICATE_SIZE) {
    std::fclose(fp);
    throw SEARError("certificate file is too large for IRRSIM00");
  }
  size_t bytes_read = std::fread(p_certificate->value, 1, file_size, fp);
  std::fclose(fp);
  if (bytes_read != (size_t)file_size) {
    throw SEARError("unable to read certificate file '" + filename + "'");
  }
  p_certificate->length = htonl((uint32_t)file_size);
}

nlohmann::json IRRSIM00::buildResultJSON(const irrsim00_arg_area_t &arg_area,
                                         uint16_t function_code) {
  nlohmann::json result_json;

  if (function_code == UMAP_R_TO_L || function_code == UMAP_R_TO_N ||
      function_code == UMAP_R_TO_K || function_code == UMAP_R_TO_E) {
    uint16_t application_userid_length =
        ntohs(arg_area.application_userid.length);
    std::string application_userid(arg_area.application_userid.value,
                                   application_userid_length);
    result_json["application_userid"] = application_userid;
  } else {
    std::string userid(arg_area.racf_userid.value, arg_area.racf_userid.length);
    result_json["userid"] = userid;
  }

  return result_json;
}
}  // namespace SEAR