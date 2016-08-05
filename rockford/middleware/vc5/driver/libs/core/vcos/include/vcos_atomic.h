/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#if defined(__GNUC__)
#include "../gcc/vcos_atomic_gcc.h"
#else
#include "../cpp/vcos_atomic_cpp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void vcos_atomic_thread_fence(vcos_memory_order_t memorder);

#define VCOS_ATOMIC_DECLARATIONS(name,T)\
T vcos_atomic_load_##name(const volatile T* ptr, vcos_memory_order_t memorder);\
void vcos_atomic_store_##name(volatile T* ptr, T val, vcos_memory_order_t memorder);

VCOS_ATOMIC_DECLARATIONS(bool, bool);
VCOS_ATOMIC_DECLARATIONS(int8, int8_t);
VCOS_ATOMIC_DECLARATIONS(int16, int16_t);
VCOS_ATOMIC_DECLARATIONS(int32, int32_t);
VCOS_ATOMIC_DECLARATIONS(int64, int64_t);
VCOS_ATOMIC_DECLARATIONS(uint8, uint8_t);
VCOS_ATOMIC_DECLARATIONS(uint16, uint16_t);
VCOS_ATOMIC_DECLARATIONS(uint32, uint32_t);
VCOS_ATOMIC_DECLARATIONS(uint64, uint64_t);

#ifdef __cplusplus
}
#endif
