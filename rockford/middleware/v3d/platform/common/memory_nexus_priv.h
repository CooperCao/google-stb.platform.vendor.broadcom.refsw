/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
This is a default implementation of a Nexus platform layer used by V3D.
This illustrates one way in which the abstract memory interface might be
implemented. You can replace this with your own custom version if preferred.
=============================================================================*/

#ifndef MEMORY_NEXUS_PRIV_H
#define MEMORY_NEXUS_PRIV_H

#define STRIP_HEIGHT 16

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
   NEXUS_MemoryBlockHandle yuvScratch;
   NEXUS_Addr              yuvScratchPhys;
#endif
} NXPL_MemoryData;

#endif /* MEMORY_NEXUS_PRIV_H */
