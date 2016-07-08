/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Tile list construction logic.
=============================================================================*/

#ifndef GLXX_HW_MASTER_CL_H
#define GLXX_HW_MASTER_CL_H

#include "libs/core/v3d/v3d_cl.h"
#include "../common/khrn_image_plane.h"
#include "glxx_int_config.h"

#define TILE_OP_MAX_GENERAL 16

// Load/store operation that is needed in the tile list
typedef struct
{
   bool                 raw;
   v3d_ldst_buf_t       buf;
   KHRN_IMAGE_PLANE_T   image_plane;
} TILE_OP_T;

// Keeps track of operations that are needed for a buffer in the TLB
typedef struct
{
   KHRN_IMAGE_PLANE_T   short_form_img_plane;
   bool                 short_form_ms;
   bool                 load;             /* Will this image be loaded/stored */
   bool                 store;            /* TODO: We store this here only because the code's a mess */
} BUFFER_OPS_T;

typedef struct
{
   unsigned short_op_mask;
   unsigned num_general_ops;
   TILE_OP_T general_ops[TILE_OP_MAX_GENERAL];
} TILE_OPS_T;

typedef enum rcfg_ldst
{
   RCFG_LOAD = 0,
   RCFG_STORE = 1
} rcfg_ldst;

typedef enum rcfg_buffer
{
   RCFG_COLOR0    = 0,
   RCFG_DEPTH     = GLXX_MAX_RENDER_TARGETS + 0,
   RCFG_STENCIL   = GLXX_MAX_RENDER_TARGETS + 1
} rcfg_buffer;

typedef enum rcfg_buffer_mask
{
   RCFG_COLOR0_MASK  = 1 << RCFG_COLOR0,
   RCFG_DEPTH_MASK   = 1 << RCFG_DEPTH,
   RCFG_STENCIL_MASK = 1 << RCFG_STENCIL
} rcfg_buffer_mask;

#define RCFG_COLOR(x) (x)
#define RCFG_COLOR_MASK(x) (1 << RCFG_COLOR(x))

// Keeps track of operations that are needed for each buffer in the TLB,
// and what configuration they require in the rendering mode config
typedef struct
{
   BUFFER_OPS_T         buffers[GLXX_MAX_RENDER_TARGETS + 2];
   TILE_OPS_T           tile_ops[2];

   /* TODO: Deprecated things. They should be removed and recalculated */
   GFX_BUFFER_RCFG_COLOR_TRANSLATION_T color_translation[GLXX_MAX_RENDER_TARGETS];
   v3d_depth_type_t     depth_type;
   v3d_depth_format_t   depth_output_format;
   v3d_memory_format_t  depth_memory_format;
   v3d_memory_format_t  stencil_memory_format;
} RCFG_INFO_T;

typedef struct
{
   RCFG_INFO_T          rcfg;

   unsigned             num_tiles_x;
   unsigned             num_tiles_y;
   bool                 tlb_ms;
   bool                 tlb_double_buffer;
} MASTER_CL_INFO_T;

// Chooses required load/store operations and configures rendering mode
// configuration and stores this information in the info structure
extern void glxx_hw_build_master_cl_info(GLXX_HW_RENDER_STATE_T *rs, MASTER_CL_INFO_T *info);

// Fills generic tile list (to be used with supertiles) with the info
extern bool glxx_hw_generic_tile_list(GLXX_HW_RENDER_STATE_T *rs, const MASTER_CL_INFO_T *info);

// Generates a list of supertiles into render list
extern void glxx_hw_populate_master_cl_supertiles( GLXX_HW_RENDER_STATE_T *rs,
      uint32_t num_cores, uint32_t core, uint8_t **instr, unsigned int
      num_supertiles_x, unsigned int num_supertiles_y);

// Deallocates info
extern void glxx_hw_destroy_master_cl_info(MASTER_CL_INFO_T *info);

#endif // GLXX_HW_MASTER_CL_H
