#pragma once

#include <sys/_types.h>
#include <sys/types.h>
#include <liballoc.h>
#include <inttypes.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

 int
     expand_number(const char *buf, uint64_t *num);

int
dehumanize_number(const char *buf, int64_t *num);


#ifdef __cplusplus
}
#endif
