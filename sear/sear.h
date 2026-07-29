#ifndef __SEAR_H_
#define __SEAR_H_

#include "sear_result.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

sear_result_t *sear(const char *request_json, int length, bool debug);

#ifdef __cplusplus
}
#endif

#pragma export(sear)

#endif
