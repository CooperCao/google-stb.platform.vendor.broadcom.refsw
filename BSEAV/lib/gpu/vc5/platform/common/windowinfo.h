/******************************************************************************
 *  Copyright (C) 2019 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#if NEXUS_HAS_GRAPHICS2D
#include "nexus_graphics2d.h"
#endif

#include <stdbool.h>
#include <stdint.h>

typedef enum DisplayType
{
   _2D = 0,
   _3D_LEFT_RIGHT,
   _3D_OVER_UNDER
} DisplayType;

typedef struct WindowInfo
{
   uint32_t             width;
   uint32_t             height;
   uint32_t             x;
   uint32_t             y;
   bool                 stretch;
   uint32_t             clientID;
   uint32_t             zOrder;
#if NEXUS_HAS_GRAPHICS2D
   NEXUS_BlendEquation  colorBlend;
   NEXUS_BlendEquation  alphaBlend;
#endif
   DisplayType          type;
   uint32_t             videoWidth;
   uint32_t             videoHeight;
   uint32_t             videoX;
   uint32_t             videoY;
   uint32_t             magic;
} WindowInfo;