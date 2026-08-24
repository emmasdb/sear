#ifndef __IRRSIM00_H_
#define __IRRSIM00_H_

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

#ifdef __TOS_390__
#include <unistd.h>
#else
#include "zoslib.h"
#endif

#include "security_request.hpp"

#define IRRSIM00_WORK_AREA_SIZE 1024
#define IRRSIM00_RACF_USERID_SIZE 9
#define IRRSIM00_CERTIFICATE_SIZE 4096
#define IRRSIM00_APPLICATION_USERID_SIZE 248
#define IRRSIM00_DISTINGUISHED_NAME_SIZE 248
#define IRRSIM00_REGISTRY_NAME_SIZE 257

const uint16_t UMAP_R_TO_L   = 1;
const uint16_t UMAP_L_TO_R   = 2;
const uint16_t UMAP_R_TO_N   = 3;
const uint16_t UMAP_N_TO_R   = 4;
const uint16_t UMAP_R_TO_K   = 5;
const uint16_t UMAP_K_TO_R   = 6;
const uint16_t UMAP_DID_TO_R = 8;
const uint16_t UMAP_R_TO_E   = 9;
const uint16_t UMAP_E_TO_R   = 10;

typedef struct {
  uint8_t length;
  char value[8];
} irrsim00_racf_userid_t;

typedef struct {
  uint32_t length;
  char value[IRRSIM00_CERTIFICATE_SIZE];
} irrsim00_certificate_t;

typedef struct {
  uint16_t length;
  char value[246];
} irrsim00_application_userid_t;

typedef struct {
  uint16_t length;
  char value[246];
} irrsim00_distinguished_name_t;

typedef struct {
  uint16_t length;
  char value[255];
} irrsim00_registry_name_t;

typedef struct {
  char *__ptr32 p_work_area;
  uint32_t *__ptr32 p_alet_saf_return_code;
  int *__ptr32 p_saf_return_code;
  uint32_t *__ptr32 p_alet_racf_return_code;
  int *__ptr32 p_racf_return_code;
  uint32_t *__ptr32 p_alet_racf_reason_code;
  int *__ptr32 p_racf_reason_code;
  uint32_t *__ptr32 p_alet_remainder;
  uint16_t *__ptr32 p_function_code;
  uint32_t *__ptr32 p_option_word;
  irrsim00_racf_userid_t *__ptr32 p_racf_userid;
  irrsim00_certificate_t *__ptr32 p_certificate;
  irrsim00_application_userid_t *__ptr32 p_application_userid;
  irrsim00_distinguished_name_t *__ptr32 p_distinguished_name;
  irrsim00_registry_name_t *__ptr32 p_registry_name;
} irrsim00_arg_pointers_t;

typedef struct {
  alignas(8) char work_area[IRRSIM00_WORK_AREA_SIZE];
  uint32_t alet_saf_return_code;
  int saf_return_code;
  uint32_t alet_racf_return_code;
  int racf_return_code;
  uint32_t alet_racf_reason_code;
  int racf_reason_code;
  uint32_t alet_remainder;
  uint16_t function_code;
  uint32_t option_word;
  irrsim00_racf_userid_t racf_userid;
  irrsim00_certificate_t certificate;
  irrsim00_application_userid_t application_userid;
  irrsim00_distinguished_name_t distinguished_name;
  irrsim00_registry_name_t registry_name;
  irrsim00_arg_pointers_t arg_pointers;
} irrsim00_arg_area_t;

extern "C" uint32_t callIrrsim00(char *__ptr32);

namespace SEAR {
class IRRSIM00 {
 private:
  static void buildRequest(irrsim00_arg_area_t *p_arg_area,
                           const SecurityRequest &request);
  static char *cloneBuffer(const char *p_buffer, int buffer_length);
  static void copyRACFUserID(irrsim00_racf_userid_t *p_target,
                             std::string_view userid);
  static void copyText(uint16_t *p_length, char *p_target,
                       std::size_t target_size, std::string_view value);
  static void readCertificate(const std::string &filename,
                              irrsim00_certificate_t *p_certificate);
  static nlohmann::json buildResultJSON(const irrsim00_arg_area_t &arg_area,
                                        uint16_t function_code);

 public:
  static void map(SecurityRequest &request);
};
}  // namespace SEAR

#endif