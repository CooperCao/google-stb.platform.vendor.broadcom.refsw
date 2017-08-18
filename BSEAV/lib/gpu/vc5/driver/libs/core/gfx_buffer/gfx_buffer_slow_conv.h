/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/lfmt/lfmt_block.h"
#include "gfx_buffer.h"
#include "gfx_buffer_desc_gen.h"
#include "gfx_buffer_slow_conv_compr.h"
#include "gfx_buffer_slow_conv_xform.h"
#include "vcos.h"

EXTERN_C_BEGIN

/* TODO? add get_block and/or put_block as steps for greater flexibility, more
 * code sharing with glGenerateMipmap */

typedef struct
{
   GFX_BUFFER_DESC_T desc;

   void *p; /* Pass to read/write as last argument */
   void (*read)(void *dst, size_t src_offset, size_t size, void *p); /* NULL means read directly from image, base p */
   void (*write)(size_t dst_offset, const void *src, size_t size, void *p); /* NULL means write directly to image, base p */

   uint32_t x, y, z; /* Blit offset (in elements) */
} GFX_BUFFER_BLIT_TGT_FUNC_T;

static inline void gfx_buffer_blit_tgt_func_set_pos(
   GFX_BUFFER_BLIT_TGT_FUNC_T *t, uint32_t x, uint32_t y, uint32_t z)
{
   t->x = x;
   t->y = y;
   t->z = z;
}

static inline void gfx_buffer_blit_tgt_func_from_regular(
   GFX_BUFFER_BLIT_TGT_FUNC_T *btf, const GFX_BUFFER_BLIT_TGT_T *bt)
{
   btf->desc = bt->desc;

   btf->p = bt->p;
   btf->read = NULL;
   btf->write = NULL;

   btf->x = bt->x;
   btf->y = bt->y;
   btf->z = bt->z;
}

extern void gfx_buffer_blit_func(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *dst,
   const GFX_BUFFER_BLIT_TGT_FUNC_T *src,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   uint32_t width, uint32_t height, uint32_t depth);
extern void gfx_buffer_blit_func_full(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *dst,
   const GFX_BUFFER_BLIT_TGT_FUNC_T *src,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq);

static inline void gfx_buffer_blit_func_no_xform(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *dst,
   const GFX_BUFFER_BLIT_TGT_FUNC_T *src,
   uint32_t width, uint32_t height, uint32_t depth)
{
   GFX_BUFFER_XFORM_SEQ_T seq;
   gfx_buffer_xform_seq_init(&seq, &src->desc);
   gfx_buffer_blit_func(dst, src, &seq, width, height, depth);
}

static inline void gfx_buffer_blit_func_full_no_xform(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *dst,
   const GFX_BUFFER_BLIT_TGT_FUNC_T *src)
{
   GFX_BUFFER_XFORM_SEQ_T seq;
   gfx_buffer_xform_seq_init(&seq, &src->desc);
   gfx_buffer_blit_func_full(dst, src, &seq);
}

static inline void gfx_buffer_blit(
   const GFX_BUFFER_BLIT_TGT_T *dst,
   const GFX_BUFFER_BLIT_TGT_T *src,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   uint32_t width, uint32_t height, uint32_t depth)
{
   GFX_BUFFER_BLIT_TGT_FUNC_T dst_func, src_func;
   gfx_buffer_blit_tgt_func_from_regular(&dst_func, dst);
   gfx_buffer_blit_tgt_func_from_regular(&src_func, src);
   gfx_buffer_blit_func(&dst_func, &src_func, xform_seq, width, height, depth);
}

static inline void gfx_buffer_blit_full(
   const GFX_BUFFER_BLIT_TGT_T *dst,
   const GFX_BUFFER_BLIT_TGT_T *src,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq)
{
   GFX_BUFFER_BLIT_TGT_FUNC_T dst_func, src_func;
   gfx_buffer_blit_tgt_func_from_regular(&dst_func, dst);
   gfx_buffer_blit_tgt_func_from_regular(&src_func, src);
   gfx_buffer_blit_func_full(&dst_func, &src_func, xform_seq);
}

static inline void gfx_buffer_blit_no_xform(
   const GFX_BUFFER_BLIT_TGT_T *dst,
   const GFX_BUFFER_BLIT_TGT_T *src,
   uint32_t width, uint32_t height, uint32_t depth)
{
   GFX_BUFFER_XFORM_SEQ_T seq;
   gfx_buffer_xform_seq_init(&seq, &src->desc);
   gfx_buffer_blit(dst, src, &seq, width, height, depth);
}

static inline void gfx_buffer_blit_full_no_xform(
   const GFX_BUFFER_BLIT_TGT_T *dst,
   const GFX_BUFFER_BLIT_TGT_T *src)
{
   GFX_BUFFER_XFORM_SEQ_T seq;
   gfx_buffer_xform_seq_init(&seq, &src->desc);
   gfx_buffer_blit_full(dst, src, &seq);
}

static inline void gfx_buffer_blit_debug_view(
   const GFX_BUFFER_BLIT_TGT_T *dst,
   const GFX_BUFFER_BLIT_TGT_T *src,
   uint32_t width, uint32_t height, uint32_t depth)
{
   GFX_BUFFER_XFORM_SEQ_T seq;
   gfx_buffer_xform_seq_construct_desc(&seq, &dst->desc, &src->desc,
      GFX_BUFFER_XFORM_CONVS_DEBUG_VIEW, NULL);
   gfx_buffer_blit(dst, src, &seq, width, height, depth);
}

static inline void gfx_buffer_blit_full_debug_view(
   const GFX_BUFFER_BLIT_TGT_T *dst,
   const GFX_BUFFER_BLIT_TGT_T *src)
{
   GFX_BUFFER_XFORM_SEQ_T seq;
   gfx_buffer_xform_seq_construct_desc(&seq, &dst->desc, &src->desc,
      GFX_BUFFER_XFORM_CONVS_DEBUG_VIEW, NULL);
   gfx_buffer_blit_full(dst, src, &seq);
}

/* Format of dst & src must match */
extern void gfx_buffer_subsample_func(
   const GFX_BUFFER_BLIT_TGT_FUNC_T *dst,
   const GFX_BUFFER_BLIT_TGT_FUNC_T *src,
   /* transmute_options may be NULL (meaning use defaults) */
   const GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T *transmute_options);

static inline void gfx_buffer_subsample(
   const GFX_BUFFER_BLIT_TGT_T *dst,
   const GFX_BUFFER_BLIT_TGT_T *src,
   /* transmute_options may be NULL (meaning use defaults) */
   const GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T *transmute_options)
{
   GFX_BUFFER_BLIT_TGT_FUNC_T dst_func, src_func;
   gfx_buffer_blit_tgt_func_from_regular(&dst_func, dst);
   gfx_buffer_blit_tgt_func_from_regular(&src_func, src);
   gfx_buffer_subsample_func(&dst_func, &src_func, transmute_options);
}

/** Block offset calculation */

extern size_t gfx_buffer_block_offset(
   const GFX_BUFFER_DESC_PLANE_T *plane, const GFX_LFMT_BASE_DETAIL_T *bd,
   uint32_t x_in_blocks, uint32_t y_in_blocks, uint32_t z_in_blocks,
   /* Needed for y-flipping */
   uint32_t height);

static inline void *gfx_buffer_block_p(
   const GFX_BUFFER_DESC_PLANE_T *plane, const GFX_LFMT_BASE_DETAIL_T *bd,
   void *p,
   uint32_t x_in_blocks, uint32_t y_in_blocks, uint32_t z_in_blocks,
   /* Needed for y-flipping */
   uint32_t height)
{
   return (uint8_t *)p + gfx_buffer_block_offset(plane, bd,
      x_in_blocks, y_in_blocks, z_in_blocks, height);
}

extern void *gfx_buffer_plane_get_block(
   const GFX_BUFFER_DESC_PLANE_T *src_plane,
   void *src_ptr,
   uint32_t x, uint32_t y, uint32_t z,
   uint32_t height);

/** Get pixel from block */

/* If block is a single pixel already, will just copy it to px */
extern void gfx_buffer_get_pixel_from_block(
   GFX_LFMT_BLOCK_T *px, const GFX_LFMT_BLOCK_T *block,
   /* Block-relative coords */
   uint32_t x, uint32_t y, uint32_t z);

/** Describe offset & contents of an element in a buffer */

/* Like vcos_safe_sprintf.
 * Output may span multiple lines. Will always end with a newline. */
extern size_t gfx_buffer_sprint_elem(char *buf, size_t buf_size, size_t offset,
   const GFX_BUFFER_DESC_T *desc, const void *p, uint32_t x, uint32_t y, uint32_t z,
   bool print_pointer);

#define GFX_BUFFER_SPRINT_ELEM(BUF_NAME, DESC, P, X, Y, Z, PRINT_POINTER) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 2048, gfx_buffer_sprint_elem, DESC, P, X, Y, Z, PRINT_POINTER)

EXTERN_C_END
