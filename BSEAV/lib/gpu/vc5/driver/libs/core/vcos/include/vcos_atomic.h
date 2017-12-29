/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__GNUC__)
#include "../gcc/vcos_atomic_gcc.h"
#else
#include "../cpp/vcos_atomic_cpp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void vcos_atomic_thread_fence(vcos_memory_order_t memorder);

#define VCOS_ATOMIC_DECLARATIONS_BOOL_PTR(name,T)\
T vcos_atomic_load_##name(T volatile const * ptr, vcos_memory_order_t memorder);\
void vcos_atomic_store_##name(T volatile* ptr, T val, vcos_memory_order_t memorder);\
\
bool vcos_atomic_compare_exchange_weak_##name(\
   T volatile* ptr,\
   T* expected,\
   T desired,\
   vcos_memory_order_t succ,\
   vcos_memory_order_t fail);

#define VCOS_ATOMIC_DECLARATIONS(name,T)\
VCOS_ATOMIC_DECLARATIONS_BOOL_PTR(name,T)\
T vcos_atomic_fetch_add_##name(T volatile* ptr, T val, vcos_memory_order_t memorder);\
T vcos_atomic_fetch_sub_##name(T volatile* ptr, T val, vcos_memory_order_t memorder);

VCOS_ATOMIC_DECLARATIONS_BOOL_PTR(bool, bool);
VCOS_ATOMIC_DECLARATIONS(int8, int8_t);
VCOS_ATOMIC_DECLARATIONS(int16, int16_t);
VCOS_ATOMIC_DECLARATIONS(int32, int32_t);
VCOS_ATOMIC_DECLARATIONS(int64, int64_t);
VCOS_ATOMIC_DECLARATIONS(uint8, uint8_t);
VCOS_ATOMIC_DECLARATIONS(uint16, uint16_t);
VCOS_ATOMIC_DECLARATIONS(uint32, uint32_t);
VCOS_ATOMIC_DECLARATIONS(uint64, uint64_t);
VCOS_ATOMIC_DECLARATIONS(uintptr, uintptr_t);
VCOS_ATOMIC_DECLARATIONS(size, size_t);
VCOS_ATOMIC_DECLARATIONS_BOOL_PTR(ptr, void*);

#ifdef __cplusplus
}
#endif
