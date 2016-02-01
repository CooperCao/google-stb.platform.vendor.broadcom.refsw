/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Tile list construction logic.
=============================================================================*/

#ifndef GLXX_HW_MASTER_CL_H
#define GLXX_HW_MASTER_CL_H

#include "helpers/v3d/v3d_cl.h"
#include "middleware/khronos/common/khrn_image_plane.h"
#include "interface/khronos/glxx/glxx_int_config.h"
#include "helpers/gfx/gfx_bufstate.h"

#define TILE_OP_MAX_GENERAL 16

// Load/store operation that is needed in the tile list
typedef struct
{
   bool                 raw;
   v3d_ldst_buf_t       buf;
   KHRN_IMAGE_PLANE_T   image_plane;
} TILE_OP_T;

// Short form load/store operations
typedef struct
{
   bool                 active;
   unsigned             color_disable_mask;
   bool                 depth;
   bool                 stencil;
} SHORT_TILE_OPS_T;

// Keeps track of operations that are needed for a buffer in the TLB
typedef struct
{
   KHRN_IMAGE_PLANE_T   short_form_img_plane;
   bool                 short_form_ms;
   bool                 load;             /* Will this image be loaded/stored */
   bool                 store;            /* TODO: We store this here only because the code's a mess */
} BUFFER_OPS_T;

// Keeps track of operations that are needed for each buffer in the TLB,
// and what configuration they require in the rendering mode config
typedef struct
{
   BUFFER_OPS_T         color[GLXX_MAX_RENDER_TARGETS];
   BUFFER_OPS_T         depth;
   BUFFER_OPS_T         stencil;

   SHORT_TILE_OPS_T     load_short;
   SHORT_TILE_OPS_T     store_short;
   TILE_OP_T            load_general[TILE_OP_MAX_GENERAL];
   TILE_OP_T            store_general[TILE_OP_MAX_GENERAL];
   unsigned             num_load_general;
   unsigned             num_store_general;

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
