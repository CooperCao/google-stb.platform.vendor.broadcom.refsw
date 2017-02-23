/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Handles vertex and fragment shader caches.
=============================================================================*/
#ifndef GL11_SHADERCACHE_4_H
#define GL11_SHADERCACHE_4_H

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/glxx/gl11_int_config.h"
#include "middleware/khronos/gl11/gl11_matrix.h"
#include "middleware/khronos/gl11/gl11_texunit.h"
#include "middleware/khronos/gl11/gl11_server.h"
#include "middleware/khronos/glxx/2708/glxx_inner_4.h"
#include "interface/khronos/include/GLES/gl.h"
#include "middleware/khronos/glxx/glxx_hw.h"

extern void gl11_hw_shader_cache_reset(void);

typedef struct {
   bool used;
   GL11_CACHE_KEY_T key;
   GLXX_LINK_RESULT_DATA_T data;
   uint32_t color_varyings;
} GL11_CACHE_ENTRY_T;

extern bool gl11_hw_get_shaders(
    GLXX_HW_SHADER_RECORD_T *shader_out,
    MEM_HANDLE_T *cunifmap_out,
    MEM_HANDLE_T *vunifmap_out,
    MEM_HANDLE_T *funifmap_out,
    uint32_t *color_varyings_out,
    GLXX_SERVER_STATE_T *state,
    GLXX_ATTRIB_T *attrib,
    uint32_t *mergeable_attribs,
    uint32_t * cattribs_order_out,
    uint32_t * vattribs_order_out);

#endif
