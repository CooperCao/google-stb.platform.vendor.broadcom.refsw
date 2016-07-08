/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008-2014 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Memory management

FILE DESCRIPTION

Khronos memory management API. Reference-counted objects on the normal malloc
heap.
=============================================================================*/
#ifndef KHRN_MEM_H
#define KHRN_MEM_H

#include <limits.h>
#include "vcos.h"
#include "khrn_int_common.h"

#define khrn_memcpy memcpy
#define khrn_memset memset

typedef void (*KHRN_MEM_TERM_T)(void *, size_t);

typedef void *KHRN_MEM_HANDLE_T;
#define KHRN_MEM_HANDLE_INVALID NULL
#define KHRN_MEM_HANDLE_UNUSED_VALUE ((KHRN_MEM_HANDLE_T)~0)

#define KHRN_MEM_HEADER_MAGIC 0xBA5EBA11

typedef enum
{
   KHRN_MEM_FLAG_NONE = 0,
   KHRN_MEM_FLAG_USER = 1
} KHRN_MEM_FLAG_T;

/* Actual data is put at malloc() + sizeof(KHRN_MEM_HEADER_T). We need to
 * ensure that the + sizeof(KHRN_MEM_HEADER_T) doesn't destroy alignment, so
 * force alignof(KHRN_MEM_HEADER_T) to maximum. */
typedef VCOS_ATTR_ALIGNED(VCOS_MAX_ALIGN) struct
{
   uint32_t magic;
   volatile int ref_count;
   KHRN_MEM_TERM_T term;
   size_t size;
   KHRN_MEM_FLAG_T flags;
} KHRN_MEM_HEADER_T;

static inline KHRN_MEM_HEADER_T *khrn_mem_header(KHRN_MEM_HANDLE_T handle)
{
   assert(handle != KHRN_MEM_HANDLE_INVALID);
   KHRN_MEM_HEADER_T *hdr = (KHRN_MEM_HEADER_T *)((uint8_t *)handle - sizeof(KHRN_MEM_HEADER_T));
   assert(hdr->magic == KHRN_MEM_HEADER_MAGIC);
   return hdr;
}

extern KHRN_MEM_HANDLE_T khrn_mem_alloc_ex(size_t size, const char *desc, bool init, bool resizeable, bool discardable);
static inline void khrn_mem_set_term(KHRN_MEM_HANDLE_T handle, KHRN_MEM_TERM_T term)
{
   KHRN_MEM_HEADER_T *header = khrn_mem_header(handle);
   header->term = term;
}
static inline void khrn_mem_acquire(KHRN_MEM_HANDLE_T handle)
{
   KHRN_MEM_HEADER_T *header = khrn_mem_header(handle);
   int before_inc = vcos_atomic_inc(&header->ref_count);
   vcos_unused_in_release(before_inc);
   assert(before_inc < INT_MAX);
}
extern void khrn_mem_release(KHRN_MEM_HANDLE_T handle);
static inline void *khrn_mem_lock(KHRN_MEM_HANDLE_T handle)
{
   return handle;
}
static inline void khrn_mem_unlock(KHRN_MEM_HANDLE_T handle)
{
   /* Nothing to do */
   UNUSED(handle);
}
static inline size_t khrn_mem_get_size(KHRN_MEM_HANDLE_T handle)
{
   KHRN_MEM_HEADER_T *header = khrn_mem_header(handle);
   return header->size;
}
static inline void khrn_mem_set_desc(KHRN_MEM_HANDLE_T handle, const char *desc) { /*ignored*/ UNUSED(handle); UNUSED(desc); }
static inline KHRN_MEM_TERM_T khrn_mem_get_term(KHRN_MEM_HANDLE_T handle)
{
   KHRN_MEM_HEADER_T *header = khrn_mem_header(handle);
   return header->term;
}
extern bool khrn_mem_resize(KHRN_MEM_HANDLE_T *handle, size_t size);

static inline KHRN_MEM_FLAG_T khrn_mem_get_flags(KHRN_MEM_HANDLE_T handle)
{
   KHRN_MEM_HEADER_T *header = khrn_mem_header(handle);
   return header->flags;
}
static inline void khrn_mem_set_user_flag(KHRN_MEM_HANDLE_T handle, KHRN_MEM_FLAG_T flag)
{
   KHRN_MEM_HEADER_T *header = khrn_mem_header(handle);
   header->flags = flag;
}
static inline unsigned khrn_mem_get_ref_count(KHRN_MEM_HANDLE_T handle)
{
   KHRN_MEM_HEADER_T *header = khrn_mem_header(handle);
   return header->ref_count;
}
extern bool khrn_mem_try_release(KHRN_MEM_HANDLE_T handle);
static inline bool khrn_mem_retain(KHRN_MEM_HANDLE_T handle) { return khrn_mem_get_size(handle) != 0; }
static inline void khrn_mem_unretain(KHRN_MEM_HANDLE_T handle) { /* ignore */ UNUSED(handle); }

#define KHRN_MEM_HANDLE_ZERO_SIZE khrn_mem_handle_zero_size
#define KHRN_MEM_HANDLE_EMPTY_STRING khrn_mem_handle_empty_string
extern KHRN_MEM_HANDLE_T khrn_mem_handle_zero_size;
extern KHRN_MEM_HANDLE_T khrn_mem_handle_empty_string;

/* The lhs of KHRN_MEM_ASSIGN(lhs, rhs) may be evaluated multiple times. The
 * rhs will only be evaluated once though. */
#ifdef __GNUC__
/* __typeof__(y) cast to prevent undesired conversions */
#define KHRN_MEM_ASSIGN(x, y) \
   ((x) = (__typeof__(y))khrn_mem_assign_acquire_release((x), (y)))
#else
/* No checking of x/y type compatibility! */
#define KHRN_MEM_ASSIGN(x, y) \
   ((x) = khrn_mem_assign_acquire_release((x), (y)))
#endif

static inline void *khrn_mem_assign_acquire_release(void *x, void *y)
{
   if (y != NULL)
      khrn_mem_acquire(y);
   if (x != NULL)
      khrn_mem_release(x);
   return y;
}

#define KHRN_MEM_ALLOC_STRUCT(STRUCT) khrn_mem_alloc(sizeof(STRUCT), #STRUCT)

static inline KHRN_MEM_HANDLE_T khrn_mem_alloc(size_t size, const char *desc)
{
   return khrn_mem_alloc_ex(size, desc, true, false, false);
}

static inline void *khrn_mem_maybe_lock(KHRN_MEM_HANDLE_T handle)
{
   if (handle == KHRN_MEM_HANDLE_INVALID)
      return NULL;
   else
      return khrn_mem_lock(handle);
}
static inline void khrn_mem_maybe_unlock(KHRN_MEM_HANDLE_T handle)
{
   if (handle != KHRN_MEM_HANDLE_INVALID)
      khrn_mem_unlock(handle);
}

extern KHRN_MEM_HANDLE_T khrn_mem_strdup(const char *str);

extern bool khrn_mem_init(void);
extern void khrn_mem_term(void);

#endif
