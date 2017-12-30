/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

/* gfx_buffer_raw: helper class for creating buffers using malloc. */

#include "libs/util/demand.h"
#include "gfx_buffer_slow_conv.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
   GFX_BUFFER_DESC_T descs[GFX_BUFFER_MAX_MIP_LEVELS];
   uint32_t num_mip_levels;
   size_t size;
   /* TODO Remove this field. It doesn't really make sense -- malloc() does not
    * take an align argument. */
   size_t align;
   void *p;
} GFX_BUFFER_RAW_T;

static inline void gfx_buffer_blit_tgt_from_raw(GFX_BUFFER_BLIT_TGT_T *t, const GFX_BUFFER_RAW_T *b, uint32_t mip_level)
{
   t->desc = b->descs[mip_level];
   t->p = b->p;
   t->x = 0;
   t->y = 0;
   t->z = 0;
}

static inline void gfx_buffer_raw_blit_full(
   const GFX_BUFFER_RAW_T *dst, uint32_t dst_mip_level,
   const GFX_BUFFER_RAW_T *src, uint32_t src_mip_level,
   GFX_BUFFER_XFORM_SEQ_T *xform_seq)
{
   GFX_BUFFER_BLIT_TGT_T blit_tgt_dst;
   GFX_BUFFER_BLIT_TGT_T blit_tgt_src;

   gfx_buffer_blit_tgt_from_raw(&blit_tgt_dst, dst, dst_mip_level);
   gfx_buffer_blit_tgt_from_raw(&blit_tgt_src, src, src_mip_level);

   gfx_buffer_blit_full(&blit_tgt_dst, &blit_tgt_src, xform_seq);
}

static inline void gfx_buffer_raw_malloc(GFX_BUFFER_RAW_T *br)
{
   br->p = malloc(br->size);
   demand(br->p);
   memset(br->p, 0, br->size);
}

/* initialise fields of GFX_BUFFER_RAW_T accordingly, but don't allocate any
 * storage (no _destroy required) and set br->p to NULL */
static inline void gfx_buffer_raw_create_no_storage(GFX_BUFFER_RAW_T *br,
   uint32_t w, uint32_t h, uint32_t d, GFX_LFMT_T lfmt, bool mipmaps, gfx_buffer_usage_t usage)
{
   if (mipmaps) {
      br->num_mip_levels = gfx_msb(gfx_umax3(w,h,d)) + 1;
   } else {
      br->num_mip_levels = 1;
   }
   assert(br->num_mip_levels < GFX_BUFFER_MAX_MIP_LEVELS);

   gfx_buffer_desc_gen(br->descs, &br->size, &br->align,
      usage,
      w, h, d,
      br->num_mip_levels,
      1, &lfmt);

   br->p = NULL;
}

// Caller must _destroy
static inline void gfx_buffer_raw_create(GFX_BUFFER_RAW_T *br,
   uint32_t w, uint32_t h, uint32_t d, GFX_LFMT_T lfmt, bool mipmaps, gfx_buffer_usage_t usage)
{
   gfx_buffer_raw_create_no_storage(br, w, h, d, lfmt, mipmaps, usage);

   gfx_buffer_raw_malloc(br);
}

static inline void gfx_buffer_raw_create_dims_from_desc_no_storage(GFX_BUFFER_RAW_T *br,
   const GFX_BUFFER_DESC_T *desc, GFX_LFMT_T lfmt, bool mipmaps, gfx_buffer_usage_t usage)
{
   gfx_buffer_raw_create_no_storage(br, desc->width, desc->height, desc->depth,
      lfmt, mipmaps, usage);
}

// Caller must _destroy
static inline void gfx_buffer_raw_create_dims_from_desc(GFX_BUFFER_RAW_T *br,
   const GFX_BUFFER_DESC_T *desc, GFX_LFMT_T lfmt, bool mipmaps, gfx_buffer_usage_t usage)
{
   gfx_buffer_raw_create(br, desc->width, desc->height, desc->depth,
      lfmt, mipmaps, usage);
}

// Caller must _destroy
static inline void gfx_buffer_raw_create_from_desc(
   GFX_BUFFER_RAW_T *br, const GFX_BUFFER_DESC_T *desc)
{
   GFX_LFMT_BASE_DETAIL_T bd;

   assert(desc->num_planes == 1); /* TODO */

   gfx_lfmt_base_detail(&bd, desc->planes[0].lfmt);

   br->descs[0] = *desc;
   br->descs[0].planes[0].region = 0;
   br->descs[0].planes[0].offset = 0;
   br->num_mip_levels = 1;

   br->size = gfx_buffer_size(desc);
   br->align = bd.bytes_per_word;

   gfx_buffer_raw_malloc(br);
}

// Caller must call _destroy
static inline void gfx_buffer_raw_deep_copy(GFX_BUFFER_RAW_T *out, const GFX_BUFFER_RAW_T *in)
{
   memcpy(out, in, sizeof(*out));
   out->p = malloc(out->size);
   demand(out->p);
   memcpy(out->p, in->p, out->size);
}

static inline void gfx_buffer_raw_destroy(GFX_BUFFER_RAW_T *br)
{
   free(br->p);
   br->p = NULL;
}

static inline void gfx_buffer_raw_from_blit_tgt(GFX_BUFFER_RAW_T *br,
   const GFX_BUFFER_BLIT_TGT_T *bt)
{
   br->descs[0] = bt->desc;
   br->p = bt->p;
   br->size = ~0U; //TODO
   br->align = 1;
   br->num_mip_levels = 1;
}

static inline void gfx_buffer_raw_read_pixel(GFX_LFMT_BLOCK_T *px,
   const GFX_BUFFER_RAW_T *br,
   uint32_t x, uint32_t y, uint32_t z, uint32_t mip_level, uint32_t plane)
{
   assert(mip_level < br->num_mip_levels);
   const GFX_BUFFER_DESC_T *desc = &br->descs[mip_level];

   assert(x < desc->width);
   assert(y < desc->height);
   assert(z < desc->depth);

   assert(plane < desc->num_planes);
   const GFX_BUFFER_DESC_PLANE_T *p = &desc->planes[plane];

   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, p->lfmt);

   void *block_p = gfx_buffer_block_p(p, &bd, br->p,
      x / bd.block_w, y / bd.block_h, z / bd.block_d,
      desc->height);

   GFX_LFMT_BLOCK_T block;
   block.fmt = gfx_lfmt_fmt(p->lfmt);
   memcpy(&block.u, block_p, bd.bytes_per_block);

   gfx_buffer_get_pixel_from_block(px, &block,
      x % bd.block_w, y % bd.block_h, z % bd.block_d);
}

#ifdef __cplusplus
}
#endif
