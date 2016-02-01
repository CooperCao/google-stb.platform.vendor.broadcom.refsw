/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Creates GLES1.1 shaders as dataflow graphs and passes them to the compiler
backend.
=============================================================================*/
#ifndef GL11_SHADER_4_H
#define GL11_SHADER_4_H

#include "interface/khronos/glxx/glxx_int_attrib.h"
#include "middleware/khronos/gl11/2708/gl11_shadercache_4.h"
#include "middleware/khronos/glxx/2708/glxx_inner_4.h"

#define UNIF_VIEWPORT 0xc0000030                         /* [scale_x, scale_y, scale_z, offset_z] */
#define UNIF_ATTR_COLOR 0xc0000200
#define UNIF_ATTR_NORMAL 0xc0000204
#define UNIF_ATTR_TEXTURE_COORD(t) (0xc0000208+4*(t))
#define UNIF_ATTR_POINT_SIZE 0xc0000218

extern bool gl11_hw_emit_shaders(GL11_CACHE_KEY_T *v, GLXX_LINK_RESULT_DATA_T *data, uint32_t *color_varyings);


#endif
