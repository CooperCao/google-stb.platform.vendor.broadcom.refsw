/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <EGL/begl_platform.h>
#include "default_nexus.h"
#include "display_nexus.h"

typedef struct
{
   NEXUS_DISPLAYHANDLE        display;
   BEGL_MemoryInterface       *memInterface;
   BEGL_HWInterface           *hwInterface;
   bool                       useMMA;
   EventContext               *eventContext;
} NXPL_Display;
