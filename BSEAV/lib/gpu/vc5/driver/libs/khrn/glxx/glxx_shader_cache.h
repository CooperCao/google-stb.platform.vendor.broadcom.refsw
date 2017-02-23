/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Stuff for 1.1 and 2.0 shader caches
=============================================================================*/
#ifndef GLXX_SHADER_CACHE_H
#define GLXX_SHADER_CACHE_H

#include "../common/khrn_res_interlock.h"
#include "glxx_int_config.h"
#include "gl_public_api.h"
#include "../glsl/glsl_gadgettype.h"
#include "../glsl/glsl_ir_program.h"
#include "../glsl/glsl_binary_program.h"
#include "libs/core/v3d/v3d_gen.h"
#include "libs/core/v3d/v3d_vpm.h"

/* Specifies uniform stream.  We have one type-value pair for every uniform. */
typedef struct
{
   uint32_t count;
   uint32_t entry[1];  /* extendable array */
} GLXX_UNIFORM_MAP_T;

typedef struct
{
   v3d_size_t code_offset; // Offset of code for this shader in GLXX_LINK_RESULT_DATA_T.res_i
   GLXX_UNIFORM_MAP_T *uniform_map;
   v3d_threading_t threading;
} GLXX_SHADER_DATA_T;

#define GLXX_SHADER_FLAGS_POINT_SIZE_SHADED_VERTEX_DATA  (1<<0)
#define GLXX_SHADER_FLAGS_VS_READS_VERTEX_ID             (1<<2)   // 2 bits (bin/render)
#define GLXX_SHADER_FLAGS_VS_READS_INSTANCE_ID           (1<<4)   // 2 bits (bin/render)
#define GLXX_SHADER_FLAGS_VS_READS_BASE_INSTANCE         (1<<6)   // 2 bits (bin/render)
#define GLXX_SHADER_FLAGS_VS_SEPARATE_I_O_VPM_BLOCKS     (1<<8)   // 2 bits (bin/render)
#define GLXX_SHADER_FLAGS_FS_WRITES_Z                    (1<<10)
#define GLXX_SHADER_FLAGS_FS_EARLY_Z_DISABLE             (1<<11)
#define GLXX_SHADER_FLAGS_TLB_WAIT_FIRST_THRSW           (1<<12)
#define GLXX_SHADER_FLAGS_PER_SAMPLE                     (1<<13)
#define GLXX_SHADER_FLAGS_TCS_BARRIERS                   (1<<14)
#define GLXX_SHADER_FLAGS_PRIM_ID_USED                   (1<<15)
#define GLXX_SHADER_FLAGS_PRIM_ID_TO_FS                  (1<<16)

struct attr_rec {
   int idx;
   uint8_t c_scalars_used;
   uint8_t v_scalars_used;
};

typedef enum glxx_vertex_pipe_stage
{
   GLXX_SHADER_VPS_VS,
#if GLXX_HAS_TNG
   GLXX_SHADER_VPS_GS,     // order of T+G stages matches shader record
   GLXX_SHADER_VPS_TCS,
   GLXX_SHADER_VPS_TES,
#endif
   GLXX_SHADER_VPS_COUNT
} glxx_vertex_pipe_stage;

typedef struct glxx_link_result_data
{
   KHRN_RES_INTERLOCK_T *res_i;
   GLXX_SHADER_DATA_T vps[GLXX_SHADER_VPS_COUNT][MODE_COUNT];
   GLXX_SHADER_DATA_T fs;
   uint32_t num_varys;
   uint8_t vary_map[GLXX_CONFIG_MAX_VARYING_SCALARS];

#if !V3D_VER_AT_LEAST(3,3,0,0)
   bool bin_uses_control_flow;
   bool render_uses_control_flow;
#endif

   uint32_t num_bin_qpu_instructions;

   uint32_t        attr_count;
   struct attr_rec attr[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];

   uint32_t flags;
   uint8_t vs_input_words[MODE_COUNT];
   uint8_t vs_output_words[MODE_COUNT];
#if GLXX_HAS_TNG
   uint8_t tcs_output_vertices_per_patch;             // set to 0 if TCS doesn't write vertex data, glxx_hw will create 1 invocation for the patch.
   uint8_t tcs_output_words_per_patch[MODE_COUNT];
   uint8_t tcs_output_words[MODE_COUNT];
   uint8_t tes_output_words[MODE_COUNT];
   uint16_t gs_output_words[MODE_COUNT];

   uint8_t geom_invocations;
   uint8_t geom_prim_type    : 2; // v3d_cl_geom_prim_type_t

   uint8_t tess_type         : 2; // v3d_cl_tess_type_t
   uint8_t tess_point_mode   : 1; // bool
   uint8_t tess_edge_spacing : 2; // v3d_cl_tess_edge_spacing_t
   uint8_t tess_clockwise    : 1; // bool

   union
   {
      struct
      {
         uint8_t has_tess : 1;
         uint8_t has_geom : 1;
      };
      uint8_t has_tng;
   };
#endif

#define GLXX_NUM_SHADING_FLAG_WORDS ((GLXX_CONFIG_MAX_VARYING_SCALARS+23)/24)
   uint32_t varying_centroid[GLXX_NUM_SHADING_FLAG_WORDS];
   uint32_t varying_flat[GLXX_NUM_SHADING_FLAG_WORDS];

   struct
   {
      struct
      {
         union
         {
#if GLXX_HAS_TNG
            struct { uint8_t num_patch_vertices; } tg;
#endif
            struct { bool z_pre_pass; } v;
         };
         bool valid;
      } key;

      v3d_vpm_cfg_v vpm_cfg_v;
#if GLXX_HAS_TNG
      uint32_t shadrec_tg_packed[V3D_SHADREC_GL_TESS_OR_GEOM_PACKED_SIZE/4];
#endif
   } cached_vpm_cfg;

} GLXX_LINK_RESULT_DATA_T;

/*
33222222222211111111110000000000
10987654321098765432109876543210
--------aaaadzwffwffwffwffpp-ccc   backend

a = advanced blend type
p = prim point
f = framebuffer type
w = fb alpha workaround
z = z only write
d = fez safe with discard
*/

/* backend */
#define GLXX_SAMPLE_MS       (1<<0)
#define GLXX_SAMPLE_ALPHA    (1<<1)
#define GLXX_SAMPLE_MASK     (1<<2)
#define GLXX_SAMPLE_OPS_M    (0x7<<0)
#define GLXX_PRIM_NOT_POINT_OR_LINE (0<<4)
#define GLXX_PRIM_POINT             (1<<4)
#define GLXX_PRIM_LINE              (2<<4)
#define GLXX_PRIM_M                 (3<<4)

#define GLXX_FB_GADGET_M (0x7)
#define GLXX_FB_GADGET_S 6
#define GLXX_FB_F16         3    /* These numbers correspond to the TLB write */
#define GLXX_FB_F32         0    /* types used by the HW in the config reg.   */
#define GLXX_FB_I32         1
#define GLXX_FB_NOT_PRESENT 2
/* V3Dv3.3 and earlier must write alpha if it's present for 16-bit RTs */
#define GLXX_FB_ALPHA_16_WORKAROUND (1<<2)

/* Leave space for 4 fb gadgets. 6, 9, 12, 15 */

#define GLXX_Z_ONLY_WRITE          (1<<18)
#define GLXX_FEZ_SAFE_WITH_DISCARD (1<<19)

/* Advanced blend */
#define GLXX_ADV_BLEND_S              20
#define GLXX_ADV_BLEND_M              (0xf << GLXX_ADV_BLEND_S)
#define GLXX_ADV_BLEND_MULTIPLY       1
#define GLXX_ADV_BLEND_SCREEN         2
#define GLXX_ADV_BLEND_OVERLAY        3
#define GLXX_ADV_BLEND_DARKEN         4
#define GLXX_ADV_BLEND_LIGHTEN        5
#define GLXX_ADV_BLEND_COLORDODGE     6
#define GLXX_ADV_BLEND_COLORBURN      7
#define GLXX_ADV_BLEND_HARDLIGHT      8
#define GLXX_ADV_BLEND_SOFTLIGHT      9
#define GLXX_ADV_BLEND_DIFFERENCE     10
#define GLXX_ADV_BLEND_EXCLUSION      11
#define GLXX_ADV_BLEND_HSL_HUE        12
#define GLXX_ADV_BLEND_HSL_SATURATION 13
#define GLXX_ADV_BLEND_HSL_COLOR      14
#define GLXX_ADV_BLEND_HSL_LUMINOSITY 15

typedef struct glxx_link_result_key
{
   uint32_t backend;

#if !V3D_VER_AT_LEAST(3,3,0,0)
   glsl_gadgettype_t gadgettype[GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
   glsl_gadgettype_t img_gadgettype[GLXX_CONFIG_MAX_IMAGE_UNITS];
#endif

} GLXX_LINK_RESULT_KEY_T;

typedef struct
{
   GLXX_LINK_RESULT_KEY_T key;
   GLXX_LINK_RESULT_DATA_T data;
   bool used;
} GLXX_BINARY_CACHE_ENTRY_T;

#define GLXX_BINARY_CACHE_SIZE 16

typedef struct
{
   GLXX_BINARY_CACHE_ENTRY_T entry[GLXX_BINARY_CACHE_SIZE];
   uint32_t used;
   uint32_t next;
} GLXX_BINARY_CACHE_T;

extern GLXX_LINK_RESULT_DATA_T *glxx_binary_cache_get_shaders(
   GLXX_BINARY_CACHE_T *cache,
   GLXX_LINK_RESULT_KEY_T *key);

extern GLXX_LINK_RESULT_DATA_T *glxx_get_shaders_and_cache(
   GLXX_BINARY_CACHE_T *cache,
   IR_PROGRAM_T *ir,
   GLXX_LINK_RESULT_KEY_T *key);

extern void glxx_binary_cache_invalidate(GLXX_BINARY_CACHE_T *cache);

#endif
