/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include "libs/core/lfmt/lfmt.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/v3d/v3d_ver.h"

#define V3D_TMU_BLIT_SHADER_MAX_UNIFS_SIZE (5 * sizeof(uint32_t))
#define V3D_TMU_BLIT_SHADER_MAX_SIZE (20 * sizeof(uint64_t))

/* Shaders do a 2d texture lookup, read rgba and write rgba */
void glxx_blit_fshader_and_unifs(uint32_t *shader_ptr, uint32_t *unif_ptr,
      GFX_LFMT_TYPE_T tex_fmt_type, bool tmu_ret_is_32bit, bool tlb_access_is_32bit,
      const uint32_t tmu_config[2], uint8_t tlb_config_byte);

/* tmu and tlb have only red uint channel,
 * we read r component from texture and write to the tlb's red component (r & ~mask)
 */
void glxx_blit_fshader_and_unifs_red_uint_with_mask(uint32_t *shader_ptr, uint32_t *unif_ptr,
      uint32_t mask_component, const uint32_t tmu_config[2], uint8_t tlb_config_byte);

#if !V3D_VER_AT_LEAST(3,3,0,0)
/*
 * 3.2 hw doesn't do swizzle for int/uint textures, so we need to add swizzle
 * in the shader
 */
/* Shaders do a 2d texture lookup, we decide how many words to read based on
 * tmu_ret and swizzle;
 * write rgba to a 32 bit int/uint tlb, using (0,0,1) for missing GBA channels
 */
void glxx_blit_int_fshader_and_unifs(uint32_t *shader_ptr, uint32_t *unif_ptr,
      GFX_LFMT_TYPE_T tex_fmt_type, gfx_lfmt_tmu_ret_t tmu_ret,
      const v3d_tmu_swizzle_t swizzle[4],
      const uint32_t tmu_config[2], uint8_t tlb_config_byte);

#endif
