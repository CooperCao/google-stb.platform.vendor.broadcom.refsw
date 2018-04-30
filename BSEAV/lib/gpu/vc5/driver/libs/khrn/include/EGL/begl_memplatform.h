/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BEGL_MEMPLATFORM_H__
#define __BEGL_MEMPLATFORM_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "begl_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
   BEGL_MemCacheLineSize,
   BEGL_MemLargestBlock,
   BEGL_MemFree,
   BEGL_MemPrintHeapData,
   BEGL_MemPagetablePhysAddr,
   BEGL_MemMmuMaxVirtAddr,
   BEGL_MemMmuUnsecureBinTranslation,
   BEGL_MemMmuSecureBinTranslation,
   BEGL_MemPlatformToken,
   BEGL_MemTotal,
   BEGL_MemAllocsAreFlushed
} BEGL_MemInfoType;

/* These must match gmem usage flags */
typedef enum
{
   BEGL_USAGE_V3D_READ     = 1 << 0,
   BEGL_USAGE_V3D_WRITE    = 1 << 1,
   BEGL_USAGE_COHERENT     = 1 << 5,
   BEGL_USAGE_HINT_DYNAMIC = 1 << 6,
   BEGL_USAGE_SECURE       = 1 << 7,
   BEGL_USAGE_CONTIGUOUS   = 1 << 8,
} BEGL_MemAllocFlags;

typedef void *BEGL_MemHandle;


enum
{
   BEGL_MaxPlanes = 3
};

typedef struct
{
   uint32_t            width;                 // Surface visible width
   uint32_t            height;                // Surface visible height
   bool                secure;                // Do a secure conversion

   BEGL_Colorimetry    srcColorimetry;
   uint32_t            stripeWidth;          // Stripe width (SAND format)

   struct
   {
      BEGL_BufferFormat   format;             // Format of the destination surface
      BEGL_MemHandle      block;              // Memory block, or NULL
      uint64_t            offset;             // Physical offset or offset from the start of memory block
      uint32_t            pitch;              // Pitch of the surface
      uint32_t            stripeHeight;       // Stripe height (SAND format)
   } src[BEGL_MaxPlanes], dst;
} BEGL_SurfaceConversionInfo;

typedef struct BEGL_MemoryInterface
{
   BEGL_MemHandle (*Alloc)(void *context, size_t numBytes, size_t alignment, uint32_t flags, const char *desc);
   void           (*Free)(void *context, BEGL_MemHandle h);
   void          *(*Map)(void *context, BEGL_MemHandle h, size_t offset, size_t length, uint32_t usage_flags);
   void           (*Unmap)(void *context, BEGL_MemHandle h, void *cpu_ptr, size_t length);
   uint32_t       (*Lock)(void *context, BEGL_MemHandle h);
   void           (*Unlock)(void *context, BEGL_MemHandle h);
   void           (*FlushCache)(void *context, BEGL_MemHandle h, void *pCached, size_t numBytes);
   uint64_t       (*GetInfo)(void *context, BEGL_MemInfoType type);

   /*
    * Optional init for interfaces that need resources which cannot
    * be acquired at the point the interface itself is registered.
    */
   int            (*Init)(void *context);
   void           (*Term)(void *context);

   /*
    * Optional entrypoint to create a wrapper handle to manage a MMU pagetable
    * mapping for externally allocated physically contiguous memory ranges
    */
   BEGL_MemHandle (*WrapExternal)(void *context, uint64_t physaddr, size_t length, const char *desc);

   /* Ask the platform to perform a surface conversion as described in info.
    * If validateOnly is set, no conversion will be performed, but the return status indicates
    * whether the platform is capable of performing the conversion.
    */
   BEGL_Error     (*ConvertSurface)(void *context, const BEGL_SurfaceConversionInfo *info,
                                    bool validateOnly);

   void           *context;
} BEGL_MemoryInterface;


extern void BEGL_RegisterMemoryInterface(BEGL_MemoryInterface *iface);

typedef void (*PFN_BEGL_RegisterMemoryInterface)(BEGL_MemoryInterface *);

#ifdef __cplusplus
}
#endif

#endif
