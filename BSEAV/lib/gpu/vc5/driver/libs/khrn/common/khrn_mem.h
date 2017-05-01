/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_MEM_H
#define KHRN_MEM_H

#include <limits.h>
#include "vcos.h"
#include "khrn_int_common.h"

#define khrn_memcpy memcpy
#define khrn_memset memset

typedef void (*khrn_mem_term_t)(void *, size_t);

typedef void *khrn_mem_handle_t;
#define KHRN_MEM_HANDLE_INVALID NULL
#define KHRN_MEM_HANDLE_UNUSED_VALUE ((khrn_mem_handle_t)~0)

#define KHRN_MEM_HEADER_MAGIC 0xBA5EBA11

typedef enum khrn_mem_flag_t
{
   KHRN_MEM_FLAG_NONE = 0,
   KHRN_MEM_FLAG_USER = 1
} khrn_mem_flag_t;

/* Actual data is put at malloc() + sizeof(khrn_mem_header). We need to
 * ensure that the + sizeof(khrn_mem_header) doesn't destroy alignment, so
 * force alignof(khrn_mem_header) to maximum. */
typedef VCOS_ATTR_ALIGNED(VCOS_MAX_ALIGN) struct khrn_mem_header
{
   uint32_t magic;
   volatile int ref_count;
   khrn_mem_term_t term;
   size_t size;
   khrn_mem_flag_t flags;
} khrn_mem_header;

static inline khrn_mem_header *khrn_mem_get_header(khrn_mem_handle_t handle)
{
   assert(handle != KHRN_MEM_HANDLE_INVALID);
   khrn_mem_header *hdr = (khrn_mem_header *)((uint8_t *)handle - sizeof(khrn_mem_header));
   assert(hdr->magic == KHRN_MEM_HEADER_MAGIC);
   return hdr;
}

extern khrn_mem_handle_t khrn_mem_alloc_ex(size_t size, const char *desc, bool init, bool resizeable, bool discardable);
static inline void khrn_mem_set_term(khrn_mem_handle_t handle, khrn_mem_term_t term)
{
   khrn_mem_header *header = khrn_mem_get_header(handle);
   header->term = term;
}
static inline void khrn_mem_acquire(khrn_mem_handle_t handle)
{
   khrn_mem_header *header = khrn_mem_get_header(handle);
   int before_inc = vcos_atomic_inc(&header->ref_count);
   vcos_unused_in_release(before_inc);
   assert(before_inc < INT_MAX);
}
extern void khrn_mem_release(khrn_mem_handle_t handle);
static inline void *khrn_mem_lock(khrn_mem_handle_t handle)
{
   return handle;
}
static inline void khrn_mem_unlock(khrn_mem_handle_t handle)
{
   /* Nothing to do */
   vcos_unused(handle);
}
static inline size_t khrn_mem_get_size(khrn_mem_handle_t handle)
{
   khrn_mem_header *header = khrn_mem_get_header(handle);
   return header->size;
}
static inline void khrn_mem_set_desc(khrn_mem_handle_t handle, const char *desc) { /*ignored*/ vcos_unused(handle); vcos_unused(desc); }
static inline khrn_mem_term_t khrn_mem_get_term(khrn_mem_handle_t handle)
{
   khrn_mem_header *header = khrn_mem_get_header(handle);
   return header->term;
}
extern bool khrn_mem_resize(khrn_mem_handle_t *handle, size_t size);

static inline khrn_mem_flag_t khrn_mem_get_flags(khrn_mem_handle_t handle)
{
   khrn_mem_header *header = khrn_mem_get_header(handle);
   return header->flags;
}
static inline void khrn_mem_set_user_flag(khrn_mem_handle_t handle, khrn_mem_flag_t flag)
{
   khrn_mem_header *header = khrn_mem_get_header(handle);
   header->flags = flag;
}
static inline unsigned khrn_mem_get_ref_count(khrn_mem_handle_t handle)
{
   khrn_mem_header *header = khrn_mem_get_header(handle);
   return header->ref_count;
}
extern bool khrn_mem_try_release(khrn_mem_handle_t handle);
static inline bool khrn_mem_retain(khrn_mem_handle_t handle) { return khrn_mem_get_size(handle) != 0; }
static inline void khrn_mem_unretain(khrn_mem_handle_t handle) { /* ignore */ vcos_unused(handle); }

#define KHRN_MEM_HANDLE_EMPTY_STRING khrn_mem_handle_empty_string
extern khrn_mem_handle_t khrn_mem_handle_empty_string;

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

static inline khrn_mem_handle_t khrn_mem_alloc(size_t size, const char *desc)
{
   return khrn_mem_alloc_ex(size, desc, true, false, false);
}

static inline void *khrn_mem_maybe_lock(khrn_mem_handle_t handle)
{
   if (handle == KHRN_MEM_HANDLE_INVALID)
      return NULL;
   else
      return khrn_mem_lock(handle);
}
static inline void khrn_mem_maybe_unlock(khrn_mem_handle_t handle)
{
   if (handle != KHRN_MEM_HANDLE_INVALID)
      khrn_mem_unlock(handle);
}

extern khrn_mem_handle_t khrn_mem_strdup(const char *str);

extern bool khrn_mem_init(void);
extern void khrn_mem_term(void);

#endif
