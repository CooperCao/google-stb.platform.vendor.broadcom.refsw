/*=============================================================================
  Broadcom Proprietary and Confidential. (c)2013 Broadcom.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
=============================================================================*/
#ifndef GLXX_TEXTURE_DEFINES_H
#define GLXX_TEXTURE_DEFINES_H

#include "vcos.h"
#include "libs/core/v3d/v3d_limits.h"
#include "libs/util/gfx_util/gfx_util.h"

#define LOG2_MAX_TEXTURE_SIZE 12 /* hard code this till we get the V3D defines right */
#define MAX_TEXTURE_SIZE (1 << LOG2_MAX_TEXTURE_SIZE)
#define LOG2_MAX_3D_TEXTURE_SIZE 8
#define MAX_3D_TEXTURE_SIZE (1 << LOG2_MAX_3D_TEXTURE_SIZE)
#define MAX_ARRAY_TEXTURE_LAYERS 256
#define MAX_FACES 6
#define KHRN_MAX_MIP_LEVELS (GFX_MAX(LOG2_MAX_TEXTURE_SIZE, LOG2_MAX_3D_TEXTURE_SIZE)+1)


#endif
