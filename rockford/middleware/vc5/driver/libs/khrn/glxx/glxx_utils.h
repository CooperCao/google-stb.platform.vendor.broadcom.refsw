/*=============================================================================
  Broadcom Proprietary and Confidential. (c)2015 Broadcom.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Miscellaneous glxx functions
=============================================================================*/
#pragma once
#include "gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "glxx_int_config.h"
#include "glxx_enum_types.h"
#include "libs/core/lfmt/lfmt.h"

static inline uint32_t glxx_get_index_type_size(GLXX_INDEX_T type)
{
   switch (type)
   {
   case E_GL_UNSIGNED_BYTE:   return 1u;
   case E_GL_UNSIGNED_SHORT:  return 2u;
   case E_GL_UNSIGNED_INT:    return 4u;
   default: unreachable();
   }
}

extern bool glxx_is_color_renderable_internalformat(GLenum internalformat);
extern bool glxx_is_depth_renderable_internalformat(GLenum internallformat);
extern bool glxx_is_stencil_renderable_internalformat(GLenum internalformat);
extern bool glxx_has_integer_internalformat(GLenum internalformat);

extern bool glxx_is_color_renderable_from_api_fmt(GFX_LFMT_T fmt);
extern bool glxx_is_depth_renderable_from_api_fmt(GFX_LFMT_T fmt);
extern bool glxx_is_stencil_renderable_from_api_fmt(GFX_LFMT_T fmt);

typedef enum
{
   GLXX_NO_MS = 0,
   GLXX_4X_MS = 4    /* The values in this enum must really be the number of
                        samples we are using for that mode */
}glxx_ms_mode;
/* if we move to supporting 16x MS, we need to change the above enum and fix
 * the ms functions bellow */
static_assrt(GLXX_4X_MS == GLXX_CONFIG_MAX_SAMPLES);

extern glxx_ms_mode glxx_max_ms_mode_for_internalformat(GLenum internalformat);
extern glxx_ms_mode glxx_samples_to_ms_mode(unsigned samples);
extern unsigned glxx_ms_mode_get_scale(glxx_ms_mode ms_mode);
