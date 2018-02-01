/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
   VCOS_MEMORY_ORDER_RELAXED = __ATOMIC_RELAXED,
   VCOS_MEMORY_ORDER_CONSUME = __ATOMIC_CONSUME,
   VCOS_MEMORY_ORDER_ACQUIRE = __ATOMIC_ACQUIRE,
   VCOS_MEMORY_ORDER_RELEASE = __ATOMIC_RELEASE,
   VCOS_MEMORY_ORDER_ACQ_REL = __ATOMIC_ACQ_REL,
   VCOS_MEMORY_ORDER_SEQ_CST = __ATOMIC_SEQ_CST
} vcos_memory_order_t;

#ifdef __cplusplus
extern "C" {
#endif

static inline void vcos_atomic_thread_fence(vcos_memory_order_t memorder)
{
   __atomic_thread_fence(memorder);
}

#define VCOS_ATOMIC_DEFINITIONS_BOOL_PTR(name,T)\
static inline T vcos_atomic_load_##name(T volatile const* ptr, vcos_memory_order_t memorder)\
{\
   return __atomic_load_n(ptr, memorder);\
}\
static inline void vcos_atomic_store_##name(T volatile* ptr, T val, vcos_memory_order_t memorder)\
{\
   __atomic_store_n(ptr, val, memorder);\
}\
static inline bool vcos_atomic_compare_exchange_weak_##name(\
   T volatile* ptr,\
   T* expected,\
   T desired,\
   vcos_memory_order_t succ,\
   vcos_memory_order_t fail)\
{\
   return __atomic_compare_exchange_n(ptr, expected, desired, true, succ, fail);\
}\

#define VCOS_ATOMIC_DEFINITIONS(name,T)\
VCOS_ATOMIC_DEFINITIONS_BOOL_PTR(name,T)\
static inline T vcos_atomic_fetch_add_##name(T volatile* ptr, T val, vcos_memory_order_t memorder)\
{\
   return __atomic_fetch_add(ptr, val, memorder);\
}\
\
static inline T vcos_atomic_fetch_sub_##name(T volatile* ptr, T val, vcos_memory_order_t memorder)\
{\
   return __atomic_fetch_sub(ptr, val, memorder);\
}\
\
static inline T vcos_atomic_fetch_or_##name(T volatile* ptr, T val, vcos_memory_order_t memorder)\
{\
   return __atomic_fetch_or(ptr, val, memorder);\
}\
\
static inline T vcos_atomic_fetch_and_##name(T volatile* ptr, T val, vcos_memory_order_t memorder)\
{\
   return __atomic_fetch_and(ptr, val, memorder);\
}

VCOS_ATOMIC_DEFINITIONS_BOOL_PTR(bool, bool);
VCOS_ATOMIC_DEFINITIONS(int8, int8_t);
VCOS_ATOMIC_DEFINITIONS(int16, int16_t);
VCOS_ATOMIC_DEFINITIONS(int32, int32_t);
VCOS_ATOMIC_DEFINITIONS(int64, int64_t);
VCOS_ATOMIC_DEFINITIONS(uint8, uint8_t);
VCOS_ATOMIC_DEFINITIONS(uint16, uint16_t);
VCOS_ATOMIC_DEFINITIONS(uint32, uint32_t);
VCOS_ATOMIC_DEFINITIONS(uint64, uint64_t);
VCOS_ATOMIC_DEFINITIONS(uintptr, uintptr_t);
VCOS_ATOMIC_DEFINITIONS(size, size_t);
VCOS_ATOMIC_DEFINITIONS_BOOL_PTR(ptr, void*);

#ifdef __cplusplus
}
#endif
