/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once
#include <stdint.h>

#if defined(__unix__)
#include "../posix/vcos_chrono_posix.h"
#else
#include "../cpp/vcos_chrono_cpp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

uint64_t vcos_system_clock_now_ms(void);
uint64_t vcos_steady_clock_now_ms(void);
uint64_t vcos_high_res_clock_now_ms(void);

uint64_t vcos_system_clock_now_us(void);
uint64_t vcos_steady_clock_now_us(void);
uint64_t vcos_high_res_clock_now_us(void);

uint64_t vcos_system_clock_now_ns(void);
uint64_t vcos_steady_clock_now_ns(void);
uint64_t vcos_high_res_clock_now_ns(void);

#ifdef __cplusplus
}
#endif
