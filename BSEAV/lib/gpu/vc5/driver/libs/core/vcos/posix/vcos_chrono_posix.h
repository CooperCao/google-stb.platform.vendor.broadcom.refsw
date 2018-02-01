/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include <stdint.h>
#include "libs/util/assert_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline uint64_t vcos_linux_clock_now_ms(clockid_t id)
{
   struct timespec ts;
   verif(clock_gettime(id, &ts) == 0);
   return (uint64_t)ts.tv_sec*1000u + ts.tv_nsec/1000000u;
}

static inline uint64_t vcos_linux_clock_now_us(clockid_t id)
{
   struct timespec ts;
   verif(clock_gettime(id, &ts) == 0);
   return (uint64_t)ts.tv_sec*1000000u + ts.tv_nsec/1000u;
}

static inline uint64_t vcos_linux_clock_now_ns(clockid_t id)
{
   struct timespec ts;
   verif(clock_gettime(id, &ts) == 0);
   return (uint64_t)ts.tv_sec*1000000000u + ts.tv_nsec;
}

static inline uint64_t vcos_system_clock_now_ms(void) { return vcos_linux_clock_now_ms(CLOCK_REALTIME); }
static inline uint64_t vcos_system_clock_now_us(void) { return vcos_linux_clock_now_us(CLOCK_REALTIME); }
static inline uint64_t vcos_system_clock_now_ns(void) { return vcos_linux_clock_now_ns(CLOCK_REALTIME); }

static inline uint64_t vcos_steady_clock_now_ms(void) { return vcos_linux_clock_now_ms(CLOCK_MONOTONIC); }
static inline uint64_t vcos_steady_clock_now_us(void) { return vcos_linux_clock_now_us(CLOCK_MONOTONIC); }
static inline uint64_t vcos_steady_clock_now_ns(void) { return vcos_linux_clock_now_ns(CLOCK_MONOTONIC); }

static inline uint64_t vcos_high_res_clock_now_ms(void) { return vcos_linux_clock_now_ms(CLOCK_MONOTONIC); }
static inline uint64_t vcos_high_res_clock_now_us(void) { return vcos_linux_clock_now_us(CLOCK_MONOTONIC); }
static inline uint64_t vcos_high_res_clock_now_ns(void) { return vcos_linux_clock_now_ns(CLOCK_MONOTONIC); }

#ifdef __cplusplus
}
#endif
