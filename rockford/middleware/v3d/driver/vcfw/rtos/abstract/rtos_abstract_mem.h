/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Memory management
File     :  $RCSfile$
Revision :  $Revision$

FILE DESCRIPTION
Khronos memory management API.
=============================================================================*/

#ifndef COMMON_MEM_H
#define COMMON_MEM_H

#include "vcinclude/common.h"

#include <assert.h>

#include "interface/vcos/vcos.h"
#include "interface/khronos/common/khrn_int_image.h"

#define MAGIC 0xB00B1E55

#ifdef _MSC_VER
   #define RCM_ALIGNOF(T) __alignof(T)
#else
   #define RCM_ALIGNOF(T) (sizeof(struct { T t; char ch; }) - sizeof(T))
#endif

#ifdef _MSC_VER
   #define RCM_INLINE __inline
#else
   #ifdef __LCC__
      #define RCM_INLINE
   #elif defined __GNUC__
      #define RCM_INLINE inline __attribute__((always_inline)) __attribute__((no_instrument_function))
   #elif __STDC_VERSION__
      #define RCM_INLINE inline
   #else
      #define RCM_INLINE __inline__       /* C89 version */
   #endif
#endif

/*
   A MEM_HANDLE_T may optionally have a terminator. This is a function that will
   be called just before the MEM_HANDLE_T becomes invalid: see mem_release.
*/

typedef void (*MEM_TERM_T)(void *, uint32_t);

typedef struct {

   uint32_t magic;      /* must be first */

   volatile unsigned int ref_count;
#ifdef BCG_VC4_DEFRAG
   volatile unsigned int lock_count;
#endif
   uint32_t flags;

   uint32_t align;
   uint32_t size;
   uint32_t allocedSize;
   const char *desc;
   MEM_TERM_T term;

   void * ptr;          /* actual allocation (virtual address) or in the case of MMA the block handle */

   /* these are filled in on MMA systems when the block is first locked */
   uintptr_t blockOffset;      /* cached offset for this block */
   void * blockPtr;           /* cached virtual for this block */

} MEM_HEADER_T;

typedef struct {

   void * p;
   uintptr_t offset;

   /* for my debug to verify its within range */
   uint32_t size;
   const char * desc;

} MEM_LOCK_T;

/******************************************************************************
Pool management API
******************************************************************************/

/*
   Options for mem_compact.
*/

typedef enum
{
   /* these values are duplicated in rtos_common_mem.inc */

   MEM_COMPACT_NONE       = 0,   /* No compaction allowed */
   MEM_COMPACT_NORMAL     = 1,   /* Move unlocked blocks */
   MEM_COMPACT_DISCARD    = 2,   /* _NORMAL + discard blocks where possible */
   MEM_COMPACT_AGGRESSIVE = 4,   /* _NORMAL + move locked blocks where possible */
   MEM_COMPACT_ALL        = MEM_COMPACT_NORMAL | MEM_COMPACT_DISCARD | MEM_COMPACT_AGGRESSIVE,

   MEM_COMPACT_SHUFFLE    = 0x80 /* Move the lowest unlocked block up to the top
                                    (space permitting) - for testing */
} mem_compact_mode_t;

/*
   Get default values for memory pool defined in the platform makefile
*/

extern void mem_get_default_partition(void **mempool_base, uint32_t *mempool_size, void **mempool_handles_base, uint32_t *mempool_handles_size);

/*
   Initialize the memory subsystem, allocating a pool of a given size and
   with space for the given number of handles.
*/

extern int mem_init(void * memInterface);

/*
   Terminate the memory subsystem, releasing the pool.
*/

extern void mem_term(void);

/*
   The heap is compacted to the maximum possible extent. If (mode & MEM_COMPACT_DISCARD)
   is non-zero, all discardable, unlocked, and unretained MEM_HANDLE_Ts are resized to size 0.
   If (mode & MEM_COMPACT_AGGRESSIVE) is non-zero, all long-term block owners (which are
   obliged to have registered a callback) are asked to unlock their blocks for the duration
   of the compaction.
*/

extern void mem_compact(mem_compact_mode_t mode);

uint32_t mem_map_cached_to_physical(void *virt);
void *mem_map_physical_to_cached(uint32_t phys);
void * mem_map_cached_to_uncached(void *virt);
void * mem_map_uncached_to_cached(void *virt);

/******************************************************************************
Movable memory core API
******************************************************************************/

/*
   A MEM_HANDLE_T refers to a movable block of memory.

   The only way to get a MEM_HANDLE_T is to call mem_alloc.

   The MEM_HANDLE_T you get is immutable and remains valid until its reference
   count reaches 0.

   A MEM_HANDLE_T has an initial reference count of 1. This can be incremented
   by calling mem_acquire and decremented by calling mem_release.
*/

/*
   MEM_ZERO_SIZE_HANDLE is a preallocated handle to a zero-size block of memory
   MEM_EMPTY_STRING_HANDLE is a preallocated handle to a block of memory containing the empty string

   MEM_HANDLE_INVALID is the equivalent of NULL for MEM_HANDLE_Ts -- no valid
   MEM_HANDLE_T will ever equal MEM_HANDLE_INVALID.
*/

typedef uintptr_t MEM_HANDLE_T;
#define MEM_HANDLE_INVALID 0
#define MEM_INVALID_HANDLE 0

/*
   Flags are set once in mem_alloc -- they do not change over the lifetime of
   the MEM_HANDLE_T.
*/

typedef enum {
   MEM_FLAG_NONE = 0,

   /*
      If a MEM_HANDLE_T is discardable, the memory manager may resize it to size
      0 at any time when it is not locked or retained.
   */

   MEM_FLAG_DISCARDABLE = 1 << 0,

   /*
      MEM_FLAG_RETAINED should only ever be used when passing flags to
      mem_alloc. If it is set, the initial retain count is 1, otherwise it is 0.
   */

   MEM_FLAG_RETAINED = 1 << 9, /* shared with MEM_FLAG_ABANDONED. only used when passing flags to mem_alloc */

   /*
      If a MEM_HANDLE_T is executable, it may be locked with mem_lock_exec as
      well as mem_lock.
   */

   MEM_FLAG_EXECUTABLE = 1 << 1,

   /*
      Define a mask to extract the memory alias used by the block of memory.
   */

   MEM_FLAG_ALIAS_MASK = 3 << 2,

   /*
      If a MEM_HANDLE_T is allocating (or normal), its block of memory will be
      accessed in an allocating fashion through the cache.
   */

   MEM_FLAG_NORMAL = 0 << 2,
   MEM_FLAG_ALLOCATING = MEM_FLAG_NORMAL,

   /*
      If a MEM_HANDLE_T is direct, its block of memory will be accessed
      directly, bypassing the cache.
   */

   MEM_FLAG_DIRECT = 1 << 2,

   /*
      If a MEM_HANDLE_T is coherent, its block of memory will be accessed in a
      non-allocating fashion through the cache.
   */

   MEM_FLAG_COHERENT = 2 << 2,

   /*
      If a MEM_HANDLE_T is L1-nonallocating, its block of memory will be accessed by
      the VPU in a fashion which is allocating in L2, but only coherent in L1.
   */

   MEM_FLAG_L1_NONALLOCATING = (MEM_FLAG_DIRECT | MEM_FLAG_COHERENT),

   /*
      If a MEM_HANDLE_T is zero'd, its contents are set to 0 rather than
      MEM_HANDLE_INVALID on allocation and resize up.
   */

   MEM_FLAG_ZERO = 1 << 4,

   /*
      If a MEM_HANDLE_T is uninitialised, it will not be reset to a defined value
      (either zero, or all 1's) on allocation.
    */

   MEM_FLAG_NO_INIT = 1 << 5,

   /*
      The INIT flag is a placeholder, designed to make it explicit that
      initialisation is required, and to make it possible to change the sense
      of this bit at a later date.
    */

   MEM_FLAG_INIT    = 0 << 5,

   /*
      Hints.
   */

   MEM_FLAG_HINT_PERMALOCK = 1 << 6, /* Likely to be locked for long periods of time. */
   MEM_FLAG_HINT_GROW      = 1 << 7, /* Likely to grow in size over time. If this flag is specified, MEM_FLAG_RESIZEABLE must also be. */

   MEM_FLAG_HINT_ALL = 0xc0,

   /*
      User flag. Intended for private use by subsystems; 3d uses this for resource
      use counting. Currently collides with MEM_FLAG_HINT_GROW, which is ignored.
   */

   MEM_FLAG_USER = 1 << 7,

   /*
      If a MEM_HANDLE_T is to be resized with mem_resize, this flag must be
      present. This flag prevents things from being allocated out of the small
      allocation pool.
   */
   MEM_FLAG_RESIZEABLE = 1 << 8,

   /*
      Automatically set for blocks allocated with MEM_FLAG_NO_INIT.
   */

   MEM_FLAG_ABANDONED = 1 << 9, /* shared with MEM_FLAG_RETAINED. never used when passing flags to mem_alloc */

   /* Internal use only - do not specify when allocating */
   MEM_FLAG_WRAPPED = 1 << 10,

   /*
      If the 256bit flag is set then allocate a complete 256bits off the end of the buffer
   */
   MEM_FLAG_256BIT_PAD = 1 << 11,

   /* There is currently room in MEM_HEADER_X_T for 12 flags */
   MEM_FLAG_ALL = 0xfff
} MEM_FLAG_T;

/*
   A common way of storing a MEM_HANDLE_T together with an offset into it.
*/

typedef struct
{
   MEM_HANDLE_T mh_handle;
   uint32_t offset;
} MEM_HANDLE_OFFSET_T;

/* gets the size of a cacheline in the host CPU */
uint32_t mem_cacheline_size(void);

/*
   Attempts to allocate a movable block of memory of the specified size and
   alignment. size may be 0. A MEM_HANDLE_T with size 0 is special: see
   mem_lock/mem_unlock. mode specifies the types of compaction permitted,
   including MEM_COMPACT_NONE.

   Preconditions:

   - align is a power of 2.
   - flags only has set bits within the range specified by MEM_FLAG_ALL.
   - desc is NULL or a pointer to a null-terminated string.
   - the caller of this function is contracted to later call mem_release (or pass such responsibility on) if we don't return MEM_HANDLE_INVALID

   Postconditions:

   If the attempt succeeds:
   - A fresh MEM_HANDLE_T referring to the allocated block of memory is
     returned.
   - The MEM_HANDLE_T is unlocked, without a terminator, and has a reference
     count of 1.
   - If MEM_FLAG_RETAINED was specified, the MEM_HANDLE_T has a retain count of
     1, otherwise it is unretained.
   - If the MEM_FLAG_ZERO flag was specified, the block of memory is set to 0.
     Otherwise, each aligned word is set to MEM_HANDLE_INVALID.

   If the attempt fails:
   - MEM_HANDLE_INVALID is returned.
*/

extern MEM_HANDLE_T mem_alloc_ex(
   uint32_t size,
   uint32_t align,
   MEM_FLAG_T flags,
   const char *desc,
   mem_compact_mode_t mode);

#define mem_alloc(s,a,f,d) mem_alloc_ex(s,a,f,d,MEM_COMPACT_ALL)

/* Wrap an existing block of memory (that was allocated from the correct heap) in a MEM_HANDLE_T */
extern MEM_HANDLE_T mem_wrap(void *p, uint32_t offset, uint32_t size, uint32_t align, MEM_FLAG_T flags, const char *desc);

/*
   Preconditions:

   - handle is a valid MEM_HANDLE_T.

   Postconditions:

   - The reference count of the MEM_HANDLE_T is incremented.
*/

static RCM_INLINE void mem_acquire(MEM_HANDLE_T handle)
{
   MEM_HEADER_T * h = (MEM_HEADER_T *)handle;

   assert(h->magic == MAGIC);
   assert(h->ref_count != 0);

   vcos_atomic_increment( &h->ref_count );

   assert(h->ref_count);        /*  check we've not wrapped */
}

/* in this version of the memory manager, retain means nothing! -> just acquire */
#define mem_acquire_retain mem_acquire

/*
   If the reference count of the MEM_HANDLE_T is 1 and it has a terminator, the
   terminator is called with a pointer to and the size of the MEM_HANDLE_T's
   block of memory (or NULL/0 if the size of the MEM_HANDLE_T is 0). The
   MEM_HANDLE_T may not be used during the call.

   Preconditions:

   - handle is a valid MEM_HANDLE_T.
   - If its reference count is 1, it must not be locked or retained.

   Postconditions:

   If the reference count of the MEM_HANDLE_T was 1:
   - The MEM_HANDLE_T is now invalid. The associated block of memory has been
     freed.

   Otherwise:
   - The reference count of the MEM_HANDLE_T is decremented.
*/

extern void mem_release_inner(MEM_HEADER_T *header);

static RCM_INLINE void mem_release(MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   unsigned int ref_count;

   assert(h->magic == MAGIC);
   assert(h->ref_count != 0);

   ref_count = vcos_atomic_decrement( &h->ref_count );

   if (ref_count == 0)
      mem_release_inner(h);
}

/*
   Preconditions:

   - handle is a valid MEM_HANDLE_T.

   Postconditions:

   If the reference count of the MEM_HANDLE_T was 1:
   - false is returned.
   - The reference count of the MEM_HANDLE_T is still 1.

   Otherwise:
   - true is returned.
   - The reference count of the MEM_HANDLE_T is decremented.
*/

extern int mem_try_release(
   MEM_HANDLE_T handle);

/*
   uint32_t mem_get_size(MEM_HANDLE_T handle);

   The size of the MEM_HANDLE_T's block of memory is returned.
   This is consistent with the size requested in a mem_alloc call.

   Implementation notes:

   -

   Preconditions:

   handle is not MEM_HANDLE_INVALID

   Postconditions:

   result <= INT_MAX

   Invariants preserved:

   -
*/

static RCM_INLINE uint32_t mem_get_size(MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   vcos_assert(h->magic == MAGIC);
   return h->size;
}

/*
   Preconditions:

   - handle is a valid MEM_HANDLE_T.

   Postconditions:

   - The minimum required alignment of the MEM_HANDLE_T's block of memory (as
     passed to mem_alloc) is returned.
*/

static RCM_INLINE uint32_t mem_get_align(MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   vcos_assert(h->magic == MAGIC);
   vcos_assert(h->ref_count != 0);
   return h->align;
}

/*
   Preconditions:

   - handle is a valid MEM_HANDLE_T.

   Postconditions:

   - The MEM_HANDLE_T's flags (as passed to mem_alloc) are returned.
*/

static RCM_INLINE MEM_FLAG_T mem_get_flags(MEM_HANDLE_T handle)
{
   MEM_HEADER_T *h = (MEM_HEADER_T *)handle;
   vcos_assert(h->magic == MAGIC);
   vcos_assert(h->ref_count != 0);
   return (MEM_FLAG_T)h->flags;
}

/*
   Preconditions:

   - handle is a valid MEM_HANDLE_T.

   Postconditions:

   - The MEM_HANDLE_T's reference count is returned.
*/

extern uint32_t mem_get_ref_count(
   MEM_HANDLE_T handle);

/*
   Preconditions:

   - handle is a valid MEM_HANDLE_T.

   Postconditions:

   - The MEM_HANDLE_T's lock count is returned.
*/

extern uint32_t mem_get_lock_count(
   MEM_HANDLE_T handle);

/*
   Preconditions:

   - handle is a valid MEM_HANDLE_T.

   Postconditions:

   - The MEM_HANDLE_T's description string is returned.
*/

extern const char *mem_get_desc(
   MEM_HANDLE_T handle);

/*
   Preconditions:

   - handle is a valid MEM_HANDLE_T.
   - desc is NULL or a pointer to a null-terminated string.

   Postconditions:

   - The MEM_HANDLE_T's description is set to desc.
*/

extern void mem_set_desc(
   MEM_HANDLE_T handle,
   const char *desc);

/*
   void mem_set_term(MEM_HANDLE_T handle, MEM_TERM_T term)

   The MEM_HANDLE_T's terminator is set to term (if term was NULL, the
   MEM_HANDLE_T no longer has a terminator).
   The MEM_HANDLE_T's terminator is called just before the MEM_HANDLE_T becomes
   invalid: see mem_release.

   Preconditions:

   handle is a valid handle to a (possibly uninitialised or partially initialised*)
   object of type X

   This implies mem_get_size(handle) == sizeof(type X)

   memory management invariants for handle are satisfied

   term must be NULL or a pointer to a function compatible with the MEM_TERM_T
   type:

      void *term(void *v, uint32_t size)

   if term is not NULL, its precondition must be no stronger than the following:
       is only called from memory manager internals during destruction of an object of type X
       v is a valid pointer to a (possibly uninitialised or partially initialised*) object of type X
       memory management invariants for v are satisfied
       size == sizeof(type X)

   if term is not NULL, its postcondition must be at least as strong as the following:
       Frees any references to resources that are referred to by the object of type X


   Postconditions:

   -
*/

extern void mem_set_term(
   MEM_HANDLE_T handle,
   MEM_TERM_T term);

/*
   Preconditions:

   - handle is a valid MEM_HANDLE_T.

   Postconditions:

   - The MEM_HANDLE_T's terminator is returned, or NULL if there is none.
*/

extern MEM_TERM_T mem_get_term(
   MEM_HANDLE_T handle);

/*
   void mem_set_user_flag(MEM_HANDLE_T handle, int flag)

   Preconditions:

   - handle is a valid MEM_HANDLE_T.

   Postconditions:

   - The MEM_HANDLE_T's user flag is set to 0 if flag is 0, or to 1 otherwise.
*/

extern void mem_set_user_flag(
   MEM_HANDLE_T handle, int flag);

/*
   Attempts to resize the MEM_HANDLE_T's block of memory. The attempt is
   guaranteed to succeed if the new size is less than or equal to the old size.
   size may be 0. A MEM_HANDLE_T with size 0 is special: see
   mem_lock/mem_unlock. mode specifies the types of compaction permitted,
   including MEM_COMPACT_NONE.

   Preconditions:

   - handle is a valid MEM_HANDLE_T.
   - It must not be locked.

   Postconditions:

   If the attempt succeeds:
   - true is returned.
   - The MEM_HANDLE_T's block of memory has been resized.
   - The contents in the region [0, min(old size, new size)) are unchanged. If
     the MEM_HANDLE_T is zero'd, the region [min(old size, new size), new size)
     is set to 0. Otherwise, each aligned word in the region
     [min(old size, new size), new size) is set to MEM_HANDLE_INVALID.

   If the attempt fails:
   - false is returned.
   - The MEM_HANDLE_T is still valid.
   - Its block of memory is unchanged.
*/

extern int mem_resize_ex(
   MEM_HANDLE_T handle,
   uint32_t size,
   mem_compact_mode_t mode);

#define mem_resize(h,s) mem_resize_ex(h,s,MEM_COMPACT_ALL)

/*
   A MEM_HANDLE_T with a lock count greater than 0 is considered to be locked
   and may not be moved by the memory manager.

   Preconditions:

   - handle is a valid MEM_HANDLE_T.
   - If it is locked, the previous lock must have been by mem_lock.

   Postconditions:

   If the MEM_HANDLE_T's size is 0:
   - NULL is returned.
   - The MEM_HANDLE_T is completely unchanged.

   Otherwise:
   - A pointer to the MEM_HANDLE_T's block of memory is returned. The pointer is
     valid until the MEM_HANDLE_T's lock count reaches 0.
   - The MEM_HANDLE_T's lock count is incremented.
   - Clears MEM_FLAG_ABANDONED.
*/

extern void * mem_lock(MEM_HANDLE_T handle, MEM_LOCK_T *lbh);


/*
   Lock a number of memory handles and store the results in an array of pointers.
   May be faster than calling mem_lock repeatedly as we only need to acquire the
   memory mutex once.
   For convenience you can also pass invalid handles in, and get out either null pointers or
   valid pointers (depending on the associated offset field)

   Preconditions:

   pointers is a valid pointer to n elements
   handles is a valid pointer to n elements
   For all 0 <= i < n
   - handles[i].mh_handle is MEM_HANDLE_INVALID or a valid MEM_HANDLE_T.
   - If handles[i].mh_handle is locked, the previous lock must have been by mem_lock.
   - If handles[i] != MEM_HANDLE_INVALID then handles[i].offset <= handles[i].size

   Postconditions:

   For all 0 <= i < n
      If handles[i] == MEM_HANDLE_INVALID:
      - pointers[i] is set to offsets[i]

      Else if handles[i].mh_handle.size == 0:
      - pointers[i] is set to 0.
      - handles[i].mh_handle is completely unchanged.

      Otherwise:
      - pointers[i] is set to a pointer which is valid until handles[i].lockcount reaches 0
      - pointers[i] points to a block of size (handles[i].size - handles[i].offset)
      - handles[i].mh_handle.lockcount is incremented.
      - MEM_FLAG_ABANDONED is cleared in handles[i].mh_handle.x.flags
*/

extern void mem_lock_multiple(
   void **pointers,
   MEM_LOCK_T *lbh,
   MEM_HANDLE_OFFSET_T *handles,
   uint32_t n);


/*
   Preconditions:

   - handle is a valid MEM_HANDLE_T.
   - If its size is not 0, it must be locked.

   Postconditions:

   If the MEM_HANDLE_T's size is 0:
   - The MEM_HANDLE_T is completely unchanged.

   Otherwise:
   - The MEM_HANDLE_T's lock count is decremented.
*/

extern void mem_unlock(MEM_HANDLE_T handle);

/*
   Like mem_unlock_multiple, but will unretain handles if they are discardable.
   Also releases handles.
*/

extern void mem_unlock_unretain_release_multiple(
   MEM_HANDLE_OFFSET_T *handles,
   uint32_t n);

/*
   A discardable MEM_HANDLE_T with a retain count greater than 0 is
   considered retained and may not be discarded by the memory manager.

   Preconditions:

   - handle is a valid MEM_HANDLE_T.
   - It must be discardable.

   Postconditions:

   - 0 is returned if the size of the MEM_HANDLE_T's block of memory is 0,
     otherwise 1 is returned.
   - The retain count of the MEM_HANDLE_T is incremented.
*/

extern int mem_retain(
   MEM_HANDLE_T handle);

/*
   Preconditions:

   - handle is a valid MEM_HANDLE_T.
   - It must be retained.

   Postconditions:

   - The retain count of the MEM_HANDLE_T is decremented.
*/

extern void mem_unretain(
   MEM_HANDLE_T handle);


/******************************************************************************
Movable memory helpers
******************************************************************************/

extern MEM_HANDLE_T mem_strdup_ex(
   const char *str,
   mem_compact_mode_t mode);

#define mem_strdup(str) mem_strdup_ex((str),MEM_COMPACT_ALL)

/*
   Attempts to allocate a movable block of memory of the same size and alignment
   as the specified structure type.

   Implementation Notes:

   The returned object obeys the invariants of the memory subsystem only.  Invariants
   of the desired structure type may not yet be obeyed.
   The memory will be filled such that any handles in the structure would be
   interpreted as MEM_HANDLE_INVALID

   Preconditions:

   STRUCT is a structure type

   the caller of this macro is contracted to later call mem_release (or pass such responsibility on) if we don't return MEM_HANDLE_INVALID

   Postconditions:

   If the attempt succeeds:
   - A fresh MEM_HANDLE_T referring to the allocated block of memory is
     returned.
   - The MEM_HANDLE_T is unlocked, unretained, without a terminator, and has a
     reference count of 1.

   If the attempt fails:
   - MEM_HANDLE_INVALID is returned.
*/

#define MEM_ALLOC_STRUCT_EX(STRUCT, mode) mem_alloc_ex(sizeof(STRUCT), RCM_ALIGNOF(STRUCT), MEM_FLAG_NONE, #STRUCT, mode)
#define MEM_ALLOC_STRUCT(STRUCT) mem_alloc(sizeof(STRUCT), RCM_ALIGNOF(STRUCT), MEM_FLAG_NONE, #STRUCT)

#ifndef VCMODS_LCC
/* LCC doesn't support inline so cannot define these functions in a header file */

static RCM_INLINE void mem_assign(MEM_HANDLE_T *x, MEM_HANDLE_T y)
{
   if (y != MEM_HANDLE_INVALID)
      mem_acquire(y);
   if (*x != MEM_HANDLE_INVALID)
      mem_release(*x);

   *x = y;
}

/*
   MEM_ASSIGN(x, y)

   Overwrite a handle with another handle, managing reference counts appropriately

   Implementation notes:

   Always use the macro version rather than the inline function above

   Preconditions:

   each of x and y is MEM_HANDLE_INVALID or a handle to a block with non-zero ref_count
   if x is not MEM_HANDLE_INVALID and x != y and x.ref_count is 1, x.lock_count is zero
   is y is not MEM_HANDLE_INVALID there must at some point be a MEM_ASSIGN(x, MEM_HANDLE_INVALID)

   Postconditions:

   Invariants preserved:
*/

#define MEM_ASSIGN(x, y)            mem_assign(&(x), (y))

/*@null@*/ static RCM_INLINE void * mem_maybe_lock(MEM_HANDLE_T handle, MEM_LOCK_T *lbh)
{
   if (handle == MEM_HANDLE_INVALID)
      return 0;
   else
      return mem_lock(handle, lbh);
}

static RCM_INLINE void mem_maybe_unlock(MEM_HANDLE_T handle)
{
   if (handle != MEM_HANDLE_INVALID)
      mem_unlock(handle);
}

#endif

extern void mem_flush_cache(void);
extern void mem_flush_cache_range(void * p, size_t numBytes);
extern void mem_copy2d(KHRN_IMAGE_FORMAT_T format, MEM_HANDLE_T hDst, MEM_HANDLE_T hSrc,
                       uint16_t width, uint16_t height, int32_t stride);

/******************************************************************************
header/link
******************************************************************************/

extern MEM_HANDLE_T MEM_ZERO_SIZE_HANDLE;
extern MEM_HANDLE_T MEM_EMPTY_STRING_HANDLE;

#endif
