/*==============================================================================
 Copyright (c) 2014 Broadcom Europe Limited.
 All rights reserved.

 FILE DESCRIPTION

 Image Format Library - Public APIs

 A GFX_BUFFER_DESC_T is a description of a buffer (often an image), including
 pitches (aka "stride" but see precise definition below) and offsets relative
 to an out-of-band pointer/memory-handle/etc to the buffer data.

 These functions operate on GFX_BUFFER_DESC_Ts, but not on the buffer data
 itself. See gfx_buffer_slow_conv.h for functions that operate on buffer data.
==============================================================================*/
#pragma once

#include "helpers/gfx/gfx_lfmt.h"
#include "helpers/gfx/gfx_util.h"

#include "vcos.h"

VCOS_EXTERN_C_BEGIN

/** variable naming */

/* width/height/depth are always in elements
 * w_in_blocks/h_in_blocks/etc are used for dimensions in blocks */

/** pitch */

/* pitch is only relevant for 2d and 3d. its value is undefined for 1d
 *
 * for raster scan order, pitch is the address difference between the start of
 * two adjacent rows of *blocks*
 *
 * for lineartile, pitch means almost the same thing, but it only really makes
 * sense to talk about rows of utiles: ((utile height in blocks) * pitch) is
 * the address difference between two adjacent rows of *utiles*
 *
 * uif - only makes sense to talk about uifcolumns. uifcolumns are always 4
 * uifblocks wide. Do not confuse uifblocks with blocks. ((uifcolumn width in
 * blocks) * pitch) is address difference between two adjacent uifcolumns.
 * (note that layout only repeats every two uifcolumns if XOR bit is set)
 *
 * ublinear - ((uifblock height in blocks) * pitch) is address difference
 * between two adjacent rows of uifblocks.
 *
 * sand - ((sand-column width in blocks) * pitch) is address difference between
 * two adjacent sand-columns.
 *
 * slice_pitch is only relevant for 3d. its value is undefined for 1d and 2d
 *
 * all current 3d layouts are just 2d layouts trivially extended to 3d by
 * having a separate 2d buffer per slice, with the slices seperated in memory
 * by slice_pitch. note that there are some base formats (eg ASTC3x3x3) which
 * have blocks that are more than one element tall. so even though each slice
 * is stored separately and is always 1 block deep, slices may be more than 1
 * element deep */

/** multiple planes */

/* TODO describe what they're for and how they're handled */

/** mipmaps */

/* TODO describe how they're handled */

extern void gfx_buffer_mip_dims(
   uint32_t *mip_width, uint32_t *mip_height, uint32_t *mip_depth,
   uint32_t width, uint32_t height, uint32_t depth,
   uint32_t mip_level);

/** buffer description */

#define GFX_BUFFER_MAX_PLANES 3

typedef struct {
   GFX_LFMT_T lfmt;

   /* offset from base pointer to start of memory for plane, in bytes */
   uint32_t offset;

   uint32_t pitch, slice_pitch;
} GFX_BUFFER_DESC_PLANE_T;

typedef struct {
   uint32_t width, height, depth, num_planes;
   GFX_BUFFER_DESC_PLANE_T planes[GFX_BUFFER_MAX_PLANES];
} GFX_BUFFER_DESC_T;

#define GFX_BUFFER_MAX_MIP_LEVELS 16

static inline void gfx_buffer_desc_base_details(
   GFX_LFMT_BASE_DETAIL_T *bds,
   const GFX_BUFFER_DESC_T *desc)
{
   uint32_t p;
   for (p = 0; p != desc->num_planes; ++p)
   {
      gfx_lfmt_base_detail(bds+p, desc->planes[p].lfmt);
   }
}

/* Adjust base_addr & desc so that the smallest offset in desc is 0 */
extern void gfx_buffer_desc_rebase(
   uintptr_t *base_addr, GFX_BUFFER_DESC_T *desc);

extern bool gfx_buffer_equal(const GFX_BUFFER_DESC_T *lhs,
   const GFX_BUFFER_DESC_T *rhs);

extern bool gfx_buffer_equal_permit_diff_fmt(const GFX_BUFFER_DESC_T *lhs,
   const GFX_BUFFER_DESC_T *rhs);

extern bool gfx_buffer_equal_with_bases(
   uintptr_t base_addr_lhs, const GFX_BUFFER_DESC_T *lhs,
   uintptr_t base_addr_rhs, const GFX_BUFFER_DESC_T *rhs);

extern bool gfx_buffer_equal_with_bases_permit_diff_fmt(
   uintptr_t base_addr_lhs, const GFX_BUFFER_DESC_T *lhs,
   uintptr_t base_addr_rhs, const GFX_BUFFER_DESC_T *rhs);

/* desc_2d must be a 2D buffer. desc_3d must be a 3D buffer. Returns true iff
 * desc_2d is equal to a slice within desc_3d (formats are only required to
 * have the same block dims & size; they may be different otherwise). */
extern bool gfx_buffer_equal_slice_with_bases_permit_diff_fmt(
   uintptr_t base_addr_2d, const GFX_BUFFER_DESC_T *desc_2d,
   uintptr_t base_addr_3d, const GFX_BUFFER_DESC_T *desc_3d);

/* Returns number of dimensions (2 for 2D, 3 for 3D, etc) */
extern uint32_t gfx_buffer_dims(const GFX_BUFFER_DESC_T *desc);

/* set all lfmts to NONE */
static inline void gfx_buffer_lfmts_none(GFX_LFMT_T lfmts[GFX_BUFFER_MAX_PLANES])
{
   uint32_t p;
   for (p = 0; p != GFX_BUFFER_MAX_PLANES; ++p)
      lfmts[p] = GFX_LFMT_NONE;
}

/* return num planes, where NONE means no plane */
static inline uint32_t gfx_buffer_lfmts_num_planes(const GFX_LFMT_T lfmts[GFX_BUFFER_MAX_PLANES])
{
   uint32_t p;
   for (p = 1; p != GFX_BUFFER_MAX_PLANES; ++p)
      if (lfmts[p] == GFX_LFMT_NONE)
         return p;
   return GFX_BUFFER_MAX_PLANES;
}

static inline void gfx_fmts_from_desc(GFX_LFMT_T *fmts, const GFX_BUFFER_DESC_T *desc)
{
   for (uint32_t i = 0; i != desc->num_planes; ++i)
      fmts[i] = gfx_lfmt_fmt(desc->planes[i].lfmt);
}

/** blit tgt */

typedef struct {
   GFX_BUFFER_DESC_T desc;
   void *p;
   uint32_t x, y, z; /* Blit offset (in elements) */
} GFX_BUFFER_BLIT_TGT_T;

static inline void gfx_buffer_blit_tgt(
   GFX_BUFFER_BLIT_TGT_T *t,
   GFX_LFMT_T lfmt,
   uint32_t width, uint32_t height, uint32_t depth,
   void *p,
   uint32_t pitch, uint32_t slice_pitch)
{
   t->desc.width = width;
   t->desc.height = height;
   t->desc.depth = depth;
   t->desc.num_planes = 1;
   t->desc.planes[0].lfmt = lfmt;
   t->desc.planes[0].offset = 0;
   t->desc.planes[0].pitch = pitch;
   t->desc.planes[0].slice_pitch = slice_pitch;
   t->p = p;
   t->x = 0;
   t->y = 0;
   t->z = 0;
}

// x,y,z in pixels.
static inline void gfx_buffer_convert_to_block_coords(
      uint32_t *x_in_blocks, uint32_t *y_in_blocks, uint32_t *z_in_blocks,
      const GFX_LFMT_BASE_DETAIL_T* bd,
      uint32_t x, uint32_t y, uint32_t z)
{
   *x_in_blocks = x / bd->block_w;
   *y_in_blocks = y / bd->block_h;
   *z_in_blocks = z / bd->block_d;
}

// x,y,z in pixels.
static inline void gfx_buffer_convert_to_block_coords_exact(
      uint32_t *x_in_blocks, uint32_t *y_in_blocks, uint32_t *z_in_blocks,
      const GFX_LFMT_BASE_DETAIL_T* bd,
      uint32_t x, uint32_t y, uint32_t z)
{
   assert((x % bd->block_w) == 0);
   assert((y % bd->block_h) == 0);
   assert((z % bd->block_d) == 0);
   gfx_buffer_convert_to_block_coords(x_in_blocks, y_in_blocks, z_in_blocks,
      bd, x, y, z);
}

// x,y,z in pixels.
static inline void gfx_buffer_blit_tgt_set_pos(
   GFX_BUFFER_BLIT_TGT_T *t, uint32_t x, uint32_t y, uint32_t z)
{
   t->x = x;
   t->y = y;
   t->z = z;
}

extern uint32_t gfx_buffer_size_plane(const GFX_BUFFER_DESC_T *desc, uint32_t plane_i);

static inline uint32_t gfx_buffer_size(const GFX_BUFFER_DESC_T *desc)
{
   assert(desc->num_planes == 1);
   return gfx_buffer_size_plane(desc, 0);
}

extern uint32_t gfx_buffer_lt_width_in_ut(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i);

extern uint32_t gfx_buffer_ublinear_width_in_ub(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i);

extern uint32_t gfx_buffer_uif_height_in_ub(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i);

/* Returns 0 if not UIF */
static inline uint32_t gfx_buffer_maybe_uif_height_in_ub(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i)
{
   if (!gfx_lfmt_is_uif_family(desc->planes[plane_i].lfmt))
      return 0;
   return gfx_buffer_uif_height_in_ub(desc, plane_i);
}

/* returns padded height (in UIF-blocks) minus the minimum possible height to
 * contain a UIF image of height = desc->height. */
extern uint32_t gfx_buffer_uif_height_pad_in_ub(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i);

extern uint32_t gfx_buffer_rso_padded_width(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i);

/* Returns 0 if not RSO */
static inline uint32_t gfx_buffer_maybe_rso_padded_width(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i)
{
   if (!gfx_lfmt_is_rso(desc->planes[plane_i].lfmt))
      return 0;
   return gfx_buffer_rso_padded_width(desc, plane_i);
}

extern uint32_t gfx_buffer_sand_padded_height(
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i);

extern void gfx_buffer_padded_width_height(
   uint32_t *width, uint32_t *height,
   const GFX_BUFFER_DESC_T *desc, uint32_t plane_i);

static inline void gfx_buffer_pad(GFX_BUFFER_DESC_T *desc)
{
   /* This doesn't really make sense if there are multiple planes -- they may
    * have conflicting padded widths/heights */
   assert(desc->num_planes == 1);

   /* TODO depth? */
   gfx_buffer_padded_width_height(&desc->width, &desc->height, desc, 0);
}

/* Like vcos_safe_sprintf */
extern size_t gfx_buffer_sprint_desc(char *buf, size_t buf_size, size_t offset,
   const GFX_BUFFER_DESC_T *desc);

#define GFX_BUFFER_SPRINT_DESC(BUF_NAME, DESC) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 512, gfx_buffer_sprint_desc, DESC)

/* Returned string only valid until next call */
extern const char *gfx_buffer_desc(const GFX_BUFFER_DESC_T *desc);

typedef enum
{
   GFX_BUFFER_ALIGN_MIN,
   GFX_BUFFER_ALIGN_RECOMMENDED
} gfx_buffer_align_t;

extern size_t gfx_buffer_get_align(GFX_LFMT_T lfmt, gfx_buffer_align_t a);

VCOS_EXTERN_C_END
