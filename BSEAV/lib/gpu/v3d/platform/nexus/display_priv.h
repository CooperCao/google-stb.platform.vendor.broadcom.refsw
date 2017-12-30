/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "EGL/egl.h"
#include "default_nexus.h"
#include "display_nexus.h"

typedef void(*BufferGetRequirementsFunc)(const BEGL_PixmapInfoEXT *bufferRequirements, BEGL_BufferSettings *bufferConstrainedRequirements);

typedef struct
{
   BufferGetRequirementsFunc  bufferGetRequirementsFunc;
   NEXUS_DISPLAYHANDLE        display;
   BEGL_MemoryInterface       *memInterface;
   BEGL_HWInterface           *hwInterface;
   bool                       useMMA;
} NXPL_Display;
