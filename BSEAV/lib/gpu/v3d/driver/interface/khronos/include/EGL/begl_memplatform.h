/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "begl_dispplatform.h"

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum BEGL_MemInfoType
{
   BEGL_MemHeapStartPhys,
   BEGL_MemHeapEndPhys,
   BEGL_MemCacheLineSize,
   BEGL_MemLargestBlock,
   BEGL_MemFree
} BEGL_MemInfoType;

typedef struct BEGL_MemCopy2d
{
   uintptr_t         dstOffset;     /* physical address (bus offset) */
   uintptr_t         srcOffset;     /* physical address (bus offset) */
   void              *dstCached;    /* useful if your CPU is quicker for small blocks */
   void              *srcCached;    /* useful if your CPU is quicker for small blocks */
   uint16_t          width;
   uint16_t          height;
   int32_t           stride;
   int32_t           dstStride;
   BEGL_BufferFormat format;
   bool              secure;
} BEGL_MemCopy2d;

/* The platform MUST provide an implementation of this interface in order that the EGL driver
 * can interact with platform memory.
 */
typedef struct BEGL_MemoryInterface
{
   /* Context pointer - opaque to the 3d driver code, but passed out in all function pointer calls.
      Prevents the client code needing to perform context lookups. */
   void *context;

   /* Allocate aligned device memory, and return the cached address, or NULL on failure.
    * Alignment is in units of bytes - not 2^alignment bytes */
   void *(*Alloc)(void *context, size_t numBytes, uint32_t alignment, bool secure);

   /* Free a previously allocated block of device memory. Pass a cached address.*/
   void (*Free)(void *context, void *pCached);

   /* Flush the cache for the given address range.*/
   void (*FlushCache)(void *context, void *pCached, size_t numBytes);

   /* Retrieve some information about the memory system */
   uint32_t (*GetInfo)(void *context, BEGL_MemInfoType type);

   /* Copy data as fast as possible. Might use DMA where available (if both pointers are in contiguous device memory) */
   void(*MemCopy2d)(void *context, BEGL_MemCopy2d *params);

   /* Lock device memory. Once it is locked it needs to be CPU readable/writable and not move until unlocked */
   void *(*MemLock)(void *context, void *handle);

   /* Lock device memory offset. */
   uintptr_t (*MemLockOffset)(void *context, void *handle);

   /* Unlock device memory. Once it is unlocked it *MAY* not be CPU accessible and can be moved */
   void (*MemUnlock)(void *context, void *handle);

   /*** Special interface for KHRN_AUTOCLIF (Internal use only) ****/
   void *(*DebugAutoclifAddrToPtr)(void *context, uint32_t addr);
   uint32_t (*DebugAutoclifPtrToAddr)(void *context, void *p);
   const char *(*DebugAutoclifGetClifFilename)(void *context);
   void(*DebugAutoclifReset)(void *context);

} BEGL_MemoryInterface;

#ifdef __cplusplus
}
#endif
