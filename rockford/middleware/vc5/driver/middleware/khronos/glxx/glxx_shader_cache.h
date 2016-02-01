/*=============================================================================
Copyright (c); 2011 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Stuff for 1.1 and 2.0 shader caches
=============================================================================*/
#ifndef GLXX_SHADER_CACHE_H
#define GLXX_SHADER_CACHE_H

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/glxx/glxx_int_config.h"
#include "interface/khronos/glxx/gl_public_api.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "middleware/khronos/gl11/gl11_texunit.h"
#include "middleware/khronos/glsl/glsl_gadgettype.h"
#include "middleware/khronos/common/khrn_res_interlock.h"
#include "middleware/khronos/glsl/glsl_ir_program.h"
#include "middleware/khronos/common/khrn_fmem.h"

/* Specifies uniform stream.  We have one type-value pair for every uniform. */
typedef struct
{
   uint32_t count;
   uint32_t entry[1];  /* extendable array */
} GLXX_UNIFORM_MAP_T;

typedef struct
{
   /* we need to keep info about the memory
      around for deferred freeing */
   KHRN_RES_INTERLOCK_T *res_i;
   GLXX_UNIFORM_MAP_T *uniform_map;
   v3d_threading_t threading;
} GLXX_SHADER_DATA_T;

#define GLXX_SHADER_FLAGS_POINT_SIZE_SHADED_VERTEX_DATA  (1<<0)
#define GLXX_SHADER_FLAGS_ENABLE_CLIPPING                (1<<1)
#define GLXX_SHADER_FLAGS_CS_READS_VERTEX_ID             (1<<2)
#define GLXX_SHADER_FLAGS_CS_READS_INSTANCE_ID           (1<<3)
#define GLXX_SHADER_FLAGS_VS_READS_VERTEX_ID             (1<<4)
#define GLXX_SHADER_FLAGS_VS_READS_INSTANCE_ID           (1<<5)
#define GLXX_SHADER_FLAGS_FS_WRITES_Z                    (1<<6)
#define GLXX_SHADER_FLAGS_FS_NOT_EARLY_Z_COMPATIBLE      (1<<7)
#define GLXX_SHADER_FLAGS_CS_SEPARATE_I_O_VPM_BLOCKS     (1<<8)
#define GLXX_SHADER_FLAGS_VS_SEPARATE_I_O_VPM_BLOCKS     (1<<9)

struct attr_rec {
   int idx;
   uint8_t c_scalars_used;
   uint8_t v_scalars_used;
};

typedef struct
{
   GLXX_SHADER_DATA_T v;
   GLXX_SHADER_DATA_T c;
   GLXX_SHADER_DATA_T f;
   uint32_t num_varys;

   bool bin_uses_control_flow;
   bool render_uses_control_flow;

   uint32_t num_bin_qpu_instructions;

   uint32_t        attr_count;
   struct attr_rec attr[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];

   /*
   flags
   0: Point Size included in shaded vertex data
   1: Enable Clipping (TRUE)
   2: Vertex_Id read by Coordinate shader
   3: Instance_Id read by Coordinate shader
   4: Vertex_Id read by Vertex shader
   5: Instance_Id read by Vertex shader
   6: Fragment shader does Z writes (e.g. Discard shaders)
   7: Turn off early-z test because fragment shader may modify the Z value in the wrong direction
   8: Coordinate Shader has separate input and output VPM blocks
   9: Vertex Shader has separate input and output VPM blocks
   */
   uint16_t flags;
   /*
   cs_size/vs_size
   If coordinate/vertex Shader has separate input and output VPM blocks:
      3:0  Input attribute size in 8x32-bit word multiples. 0=>16
      7:4  Output vertex size in 8x32-bit word multiples. 0=>16
   Else:
      7:0  Input attribute size = Output vertex size in 8x32-bit word multiples.
   */
   uint8_t cs_output_size;
   uint8_t vs_output_size;

#define GLXX_NUM_SHADING_FLAG_WORDS ((GL20_CONFIG_MAX_VARYING_SCALARS+23)/24)
   uint32_t varying_centroid[GLXX_NUM_SHADING_FLAG_WORDS];
   uint32_t varying_flat[GLXX_NUM_SHADING_FLAG_WORDS];

} GLXX_LINK_RESULT_DATA_T;

/*
----------------dz------ffpp-ccc   backend
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

#define GLXX_FB_GADGET_M (0x3)
#define GLXX_FB_GADGET_S 6
#define GLXX_FB_F16         3    /* These numbers correspond to the TLB write */
#define GLXX_FB_F32         0    /* types used by the HW in the config reg.   */
#define GLXX_FB_I32         1
#define GLXX_FB_NOT_PRESENT 2
/* Leave space for 4 fb gadgets. 6, 8, 10, 12 */
#define GLXX_Z_ONLY_WRITE (1<<14)
#define GLXX_SKIP_DISCARD (1<<15)


typedef struct
{
   uint32_t backend;

   glsl_gadgettype_t gadgettype[GLXX_CONFIG_MAX_TEXTURE_UNITS];
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
