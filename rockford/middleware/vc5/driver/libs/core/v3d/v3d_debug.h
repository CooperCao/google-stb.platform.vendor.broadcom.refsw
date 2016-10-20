/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef V3D_DEBUG_H
#define V3D_DEBUG_H

#include "v3d_gen.h"
#include "v3d_bcfg.h"
#include "v3d_rcfg.h"
#include "libs/core/gfx_buffer/gfx_buffer.h"
#include "libs/core/gfx_buffer/gfx_buffer_slow_conv.h"
#include <stdint.h>

struct v3d_debug_bin_start
{
   v3d_addr_t last_bcfg_instr_end_addr;
   const struct v3d_bcfg *bcfg;
};

struct v3d_debug_vertex
{
   int32_t x, y;
};

struct v3d_debug_prim
{
   uint32_t num_verts;
   struct v3d_debug_vertex v[3];
   union
   {
      struct
      {
         uint32_t hsize;
      } point;
      struct
      {
         bool perp_endcap;
         uint32_t xoff;
         uint32_t yoff;
      } line;
   } u;
};

typedef enum
{
   V3D_DEBUG_PRIM_NOT_DISCARDED,
   V3D_DEBUG_PRIM_DISCARD_REASON_ZERO_AREA,
   V3D_DEBUG_PRIM_DISCARD_REASON_ZERO_SAMPLES, /* Not necessarily zero area, but would hit no samples */
   V3D_DEBUG_PRIM_DISCARD_REASON_BACKFACE
} v3d_debug_prim_discard_reason_t;

struct v3d_debug_ptb_prim
{
   v3d_addr_t shader_record_addr;
   struct v3d_debug_prim prim;
   /* V3D_DEBUG_PRIM_NOT_DISCARDED if not discarded */
   v3d_debug_prim_discard_reason_t discard_reason;
};

struct v3d_debug_render_start
{
   v3d_addr_t last_rcfg_instr_end_addr;
   const struct v3d_rcfg *rcfg;
};

struct v3d_debug_tlb_store
{
   v3d_addr_t last_rcfg_instr_end_addr;
   const struct v3d_rcfg *rcfg;

   v3d_ldst_buf_t buf;
   const struct v3d_tlb_ldst_params *ldst_params;

   uint32_t tile_x;
   uint32_t tile_y;

   struct
   {
      uint32_t x;
      uint32_t y;
      uint32_t width;
      uint32_t height;
   } clipped_ext_tile_dec;

   const GFX_BUFFER_BLIT_TGT_T *interm_t;
   const GFX_BUFFER_BLIT_TGT_FUNC_T *ext_t;
};

typedef enum
{
   V3D_DEBUG_EVENT_BIN_START,
   V3D_DEBUG_EVENT_PTB_PRIM,

   V3D_DEBUG_EVENT_RENDER_START,
   V3D_DEBUG_EVENT_TLB_STORE
} v3d_debug_event_type_t;

struct v3d_debug_event
{
   v3d_debug_event_type_t type;
   union
   {
      struct v3d_debug_bin_start bin_start;
      struct v3d_debug_ptb_prim ptb_prim;

      struct v3d_debug_render_start render_start;
      struct v3d_debug_tlb_store tlb_store;
   } u;
};

typedef void (*v3d_debug_callback_t)(const struct v3d_debug_event *e, void *p);

#endif
