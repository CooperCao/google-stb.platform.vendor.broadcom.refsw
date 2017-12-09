/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#define STRIP_HEIGHT 16

#include "nexus_memory.h"
#include "nexus_platform.h"
#include <EGL/eglplatform.h>

enum
{
   eNO_PROBLEMS            = 0,
   eNOT_HD_GRAPHICS_HEAP   = 1 << 0,
   eCROSSES_256MB          = 1 << 1,
   eGUARD_BANDED           = 1 << 2,
   eWRONG_TYPE             = 1 << 3,
   eCROSSES_1GB            = 1 << 4,
   eTOO_SMALL              = 1 << 5
};

typedef struct
{
   NEXUS_HeapHandle        heap;
   void                    *heapStartCached;
   uint32_t                heapStartPhys;
   uint32_t                heapSize;

} NXPL_HeapMapping;

typedef struct
{
   NXPL_HeapMapping        heapMap;
   NXPL_HeapMapping        heapMapSecure;
   int                     l2CacheSize;
   bool                    useMMA;
   size_t                  heapGrow;

#if NEXUS_HAS_GRAPHICS2D
   BKNI_EventHandle        checkpointEvent;
   BKNI_EventHandle        packetSpaceAvailableEvent;
   NEXUS_Graphics2DHandle  gfx;
   NEXUS_Graphics2DHandle  gfxSecure;
   NEXUS_MemoryBlockHandle yuvScratch;
   NEXUS_Addr              yuvScratchPhys;
   NEXUS_MemoryBlockHandle yuvScratchSecure;
   NEXUS_Addr              yuvScratchPhysSecure;

#endif

   void                    *translationTable;
   uint32_t                filecnt;
   BEGL_MemoryInterface    *memInterface;
} NXPL_MemoryData;
