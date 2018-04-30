/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_common.h"
#include "vcfw/rtos/abstract/rtos_abstract_mem.h"
#include "interface/khronos/glxx/gl11_int_config.h"
#include "middleware/khronos/gl11/gl11_matrix.h"
#include "middleware/khronos/gl11/gl11_texunit.h"
#include "middleware/khronos/gl11/gl11_server.h"
#include "middleware/khronos/glxx/2708/glxx_inner_4.h"
#include <GLES/gl.h>
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
   void **cunifmap_out,
   void **vunifmap_out,
   void **funifmap_out,
   uint32_t *color_varyings_out,
   GLXX_SERVER_STATE_T *state,
   GLXX_ATTRIB_T *attrib,
   uint32_t *mergeable_attribs,
   uint32_t * cattribs_order_out,
   uint32_t * vattribs_order_out);
