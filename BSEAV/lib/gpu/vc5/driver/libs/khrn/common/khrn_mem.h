/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <limits.h>
#include "vcos.h"
#include "khrn_int_common.h"

typedef void (*khrn_mem_term_t)(void *);

#define KHRN_MEM_HEADER_MAGIC 0xBA5EBA11

/* Actual data is put at malloc() + sizeof(khrn_mem_header). We need to
 * ensure that the + sizeof(khrn_mem_header) doesn't destroy alignment, so
 * force alignof(khrn_mem_header) to maximum. */
typedef ALIGNED(MAX_ALIGN) struct khrn_mem_header
{
   uint32_t magic;
   volatile int ref_count;
   khrn_mem_term_t term;
   size_t size;
} khrn_mem_header;

static inline khrn_mem_header *khrn_mem_get_header(const void *p)
{
   assert(p);
   khrn_mem_header *hdr = (khrn_mem_header *)p - 1;
   assert(hdr->magic == KHRN_MEM_HEADER_MAGIC);
   return hdr;
}

static inline void khrn_mem_set_term(void *p, khrn_mem_term_t term)
{
   khrn_mem_header *header = khrn_mem_get_header(p);
   header->term = term;
}

static inline void khrn_mem_acquire(const void *p)
{
   khrn_mem_header *header = khrn_mem_get_header(p);
   int before_inc = vcos_atomic_inc(&header->ref_count);
   assert(before_inc < INT_MAX);
}

extern void khrn_mem_release(const void *p);

static inline size_t khrn_mem_get_size(const void *p)
{
   khrn_mem_header *header = khrn_mem_get_header(p);
   return header->size;
}

static inline khrn_mem_term_t khrn_mem_get_term(const void *p)
{
   khrn_mem_header *header = khrn_mem_get_header(p);
   return header->term;
}

extern const char *khrn_mem_empty_string;

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

static inline void *khrn_mem_assign_acquire_release(const void *x, const void *y)
{
   if (y != NULL)
      khrn_mem_acquire(y);
   if (x != NULL)
      khrn_mem_release(x);
   return (void *)y;
}

#define KHRN_MEM_ALLOC_STRUCT(STRUCT) khrn_mem_alloc(sizeof(STRUCT), #STRUCT)

extern void *khrn_mem_alloc(size_t size, const char *desc);

extern char *khrn_mem_strdup(const char *str);

extern bool khrn_mem_init(void);
extern void khrn_mem_term(void);
