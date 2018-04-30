/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <directfb_version.h>
#include <directfb.h>

#include "EGL/egl.h"

typedef struct
{
   IDirectFB                    *dfb;
   BEGL_MemoryInterface         *memInterface;
   BEGL_HWInterface             *hwInterface;
} DBPL_Display;
