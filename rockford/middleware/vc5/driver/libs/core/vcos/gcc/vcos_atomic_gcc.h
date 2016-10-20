/*
 * Broadcom Proprietary and Confidential. (c)2015 Broadcom.  All rights reserved.
 */
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

#define VCOS_ATOMIC_DEFINITIONS_BOOL(name,T)\
static inline T vcos_atomic_load_##name(const volatile T* ptr, vcos_memory_order_t memorder)\
{\
   return __atomic_load_n(ptr, memorder);\
}\
static inline void vcos_atomic_store_##name(volatile T* ptr, T val, vcos_memory_order_t memorder)\
{\
   __atomic_store_n(ptr, val, memorder);\
}

#define VCOS_ATOMIC_DEFINITIONS(name,T)\
VCOS_ATOMIC_DEFINITIONS_BOOL(name,T)\
static inline bool vcos_atomic_compare_exchange_weak_##name(\
   volatile T* ptr,\
   T* expected,\
   T desired,\
   vcos_memory_order_t succ,\
   vcos_memory_order_t fail)\
{\
   return __atomic_compare_exchange_n(ptr, expected, desired, true, succ, fail);\
}\
static inline T vcos_atomic_fetch_add_##name(volatile T* ptr, T val, vcos_memory_order_t memorder)\
{\
   return __atomic_fetch_add(ptr, val, memorder);\
}\
\
static inline T vcos_atomic_fetch_sub_##name(volatile T* ptr, T val, vcos_memory_order_t memorder)\
{\
   return __atomic_fetch_sub(ptr, val, memorder);\
}

VCOS_ATOMIC_DEFINITIONS_BOOL(bool, bool);
VCOS_ATOMIC_DEFINITIONS(int8, int8_t);
VCOS_ATOMIC_DEFINITIONS(int16, int16_t);
VCOS_ATOMIC_DEFINITIONS(int32, int32_t);
VCOS_ATOMIC_DEFINITIONS(int64, int64_t);
VCOS_ATOMIC_DEFINITIONS(uint8, uint8_t);
VCOS_ATOMIC_DEFINITIONS(uint16, uint16_t);
VCOS_ATOMIC_DEFINITIONS(uint32, uint32_t);
VCOS_ATOMIC_DEFINITIONS(uint64, uint64_t);

#ifdef __cplusplus
}
#endif
