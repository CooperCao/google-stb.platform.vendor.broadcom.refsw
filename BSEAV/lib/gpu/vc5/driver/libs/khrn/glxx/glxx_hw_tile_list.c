/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_hw_tile_list.h"
#include "glxx_hw.h"

#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"

static bool prep_tlb_ldst(struct v3d_tlb_ldst_params *ls, uint32_t *layer_stride,
      GLXX_HW_RENDER_STATE_T *rs, bool tlb_ms,
      const khrn_image_plane *img_plane, bool img_ms, bool color, bool load, bool store)
{
   const khrn_image *img = img_plane->image;
   khrn_blob *blob = img->blob;
   assert(img->level < blob->num_mip_levels);
   const GFX_BUFFER_DESC_T *desc = &blob->desc[img->level];
   assert(img_plane->plane_idx < desc->num_planes);
   const GFX_BUFFER_DESC_PLANE_T *plane = &desc->planes[img_plane->plane_idx];

   assert(tlb_ms || !img_ms);
   assert(load || store);

   if (!khrn_blob_alloc_storage(blob))
      return false;

   assert(plane->region < blob->res->num_handles);
   uint32_t render_rw_flags =
      (load ? V3D_BARRIER_TLB_IMAGE_READ : 0) |
      (store ? V3D_BARRIER_TLB_IMAGE_WRITE : 0);
   v3d_addr_t base_addr = khrn_fmem_sync_and_get_addr(&rs->fmem, blob->res->handles[plane->region],
         /*bin_rw_flags=*/0, render_rw_flags);

   v3d_dither_t dither = V3D_DITHER_OFF;
   if (color && rs->dither && !khrn_options.force_dither_off)
   {
      if (gfx_lfmt_alpha_bits(plane->lfmt) > 2)
         dither = V3D_DITHER_RGBA;
      else
         dither = V3D_DITHER_RGB;
   }

   assert(img->start_elem < blob->num_array_elems);
   gfx_buffer_translate_tlb_ldst(ls,
         base_addr + (img->start_elem * blob->array_pitch),
         desc, img_plane->plane_idx, img->start_slice, color,
         tlb_ms, img_ms, dither);

#if V3D_VER_AT_LEAST(4,1,34,0)
   if (khrn_options.use_rgba5551_am && (ls->pixel_format == V3D_PIXEL_FORMAT_A1_BGR5))
      ls->pixel_format = V3D_PIXEL_FORMAT_A1_BGR5_AM;
#else
   if (khrn_options.use_rgba5551_am && color && (ls->output_format.pixel == V3D_PIXEL_FORMAT_A1_BGR5))
      ls->output_format.pixel = V3D_PIXEL_FORMAT_A1_BGR5_AM;
#endif

   if (layer_stride)
      *layer_stride = khrn_image_plane_layer_stride(img_plane);
   return true;
}

/* Any loads/stores from/to fb_ops->depth_ls? */
static bool have_depth_ldst(const struct glxx_hw_tile_list_fb_ops *fb_ops)
{
   return fb_ops->depth_load || fb_ops->depth_store ||
      (fb_ops->stencil_in_depth && (fb_ops->stencil_load || fb_ops->stencil_store));
}

/* Any loads/stores from/to fb_ops->stencil_ls? */
static bool have_stencil_ldst(const struct glxx_hw_tile_list_fb_ops *fb_ops)
{
   return !fb_ops->stencil_in_depth && (fb_ops->stencil_load || fb_ops->stencil_store);
}

bool glxx_hw_prep_tile_list_fb_ops(struct glxx_hw_tile_list_fb_ops *fb_ops,
      GLXX_HW_RENDER_STATE_T *rs, bool tlb_ms)
{
   const GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;

   fb_ops->rt_clear_mask = 0;
   fb_ops->rt_nonms_load_mask = 0;
   fb_ops->rt_ms_load_mask = 0;
   fb_ops->rt_nonms_store_mask = 0;
   fb_ops->rt_ms_store_mask = 0;
   for (unsigned b = 0; b != fb->rt_count; ++b)
   {
      uint32_t bit = 1u << b;

      if (glxx_bufstate_need_clear(rs->color_buffer_state[b]))
         fb_ops->rt_clear_mask |= bit;

      if (glxx_bufstate_need_load(rs->color_buffer_state[b]))
      {
         if (rs->color_load_from_ms[b])
            fb_ops->rt_ms_load_mask |= bit;
         else
            fb_ops->rt_nonms_load_mask |= bit;
      }

      if (glxx_bufstate_need_store(rs->color_buffer_state[b]))
      {
         if (fb->color[b].image)
            fb_ops->rt_nonms_store_mask |= bit;
         if (fb->color_ms[b].image && !rs->color_discard_ms)
            fb_ops->rt_ms_store_mask |= bit;
      }

      fb_ops->rt_nonms_layer_stride[b] = 0;
      fb_ops->rt_ms_layer_stride[b] = 0;
      if (((fb_ops->rt_nonms_load_mask | fb_ops->rt_nonms_store_mask) & bit))
      {
         if (!prep_tlb_ldst(&fb_ops->rt_nonms_ls[b],
                  &fb_ops->rt_nonms_layer_stride[b],
                  rs, tlb_ms,
                  &fb->color[b], /*img_ms=*/false, /*color=*/true,
                  !!(fb_ops->rt_nonms_load_mask & bit),
                  !!(fb_ops->rt_nonms_store_mask & bit)))
            return false;
      }

      if (((fb_ops->rt_ms_load_mask | fb_ops->rt_ms_store_mask) & bit))
      {
         if (!prep_tlb_ldst(&fb_ops->rt_ms_ls[b],
                  &fb_ops->rt_ms_layer_stride[b],
                  rs, tlb_ms,
                  &fb->color_ms[b], /*img_ms=*/true, /*color=*/true,
                  !!(fb_ops->rt_ms_load_mask & bit),
                  !!(fb_ops->rt_ms_store_mask & bit)))
            return false;
      }
   }

   fb_ops->stencil_in_depth = khrn_image_plane_equal(&fb->depth, &fb->stencil);
   fb_ops->depth_clear = glxx_bufstate_need_clear(rs->depth_buffer_state);
   fb_ops->stencil_clear = glxx_bufstate_need_clear(rs->stencil_buffer_state);
   fb_ops->depth_load = glxx_bufstate_need_load(rs->depth_buffer_state);
   fb_ops->stencil_load = glxx_bufstate_need_load(rs->stencil_buffer_state);
   fb_ops->depth_store = glxx_bufstate_need_store(rs->depth_buffer_state);
   fb_ops->stencil_store = glxx_bufstate_need_store(rs->stencil_buffer_state);

   if (have_depth_ldst(fb_ops))
   {
      if (!prep_tlb_ldst(&fb_ops->depth_ls,
               &fb_ops->depth_layer_stride,
               rs, tlb_ms,
               &fb->depth, fb->ms, /*color=*/false,
               fb_ops->depth_load || (fb_ops->stencil_in_depth && fb_ops->stencil_load),
               fb_ops->depth_store || (fb_ops->stencil_in_depth && fb_ops->stencil_store)))
         return false;
   }

   if (have_stencil_ldst(fb_ops))
   {
      if (!prep_tlb_ldst(&fb_ops->stencil_ls,
               &fb_ops->stencil_layer_stride,
               rs, tlb_ms,
               &fb->stencil, fb->ms, /*color=*/false,
               fb_ops->stencil_load,
               fb_ops->stencil_store))
         return false;
   }

   return true;
}

#if V3D_VER_AT_LEAST(4,1,34,0)

static void write_load(uint8_t **instr,
      v3d_ldst_buf_t buf, const struct v3d_tlb_ldst_params *ls, uint32_t layer_offset)
{
   v3d_cl_load(instr, buf, ls->memory_format, ls->flipy, ls->decimate, ls->pixel_format,
#if V3D_VER_AT_LEAST(4,1,34,0)
         ls->load_alpha_to_one, ls->chan_reverse, ls->rb_swap,
#endif
         ls->stride, ls->flipy_height_px, ls->addr + layer_offset);
}

static bool write_begin_and_loads(GLXX_HW_RENDER_STATE_T *rs, const struct glxx_hw_tile_list_fb_ops *fb_ops,
      unsigned layer)
{
   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem,
         V3D_CL_IMPLICIT_TILE_COORDS_SIZE +
         ((GLXX_MAX_RENDER_TARGETS + 2) * V3D_CL_LOAD_SIZE) +
         V3D_CL_END_LOADS_SIZE);
   if (!instr)
      return false;

   v3d_cl_implicit_tile_coords(&instr);

   for (uint32_t b = 0, nonms_mask = fb_ops->rt_nonms_load_mask, ms_mask = fb_ops->rt_ms_load_mask;
         nonms_mask || ms_mask; ++b, nonms_mask >>= 1, ms_mask >>= 1)
   {
      if (nonms_mask & 1)
      {
         assert(!(ms_mask & 1));
         write_load(&instr, v3d_ldst_buf_color(b), &fb_ops->rt_nonms_ls[b],
               layer * fb_ops->rt_nonms_layer_stride[b]);
      }
      if (ms_mask & 1)
         write_load(&instr, v3d_ldst_buf_color(b), &fb_ops->rt_ms_ls[b], layer
               * fb_ops->rt_ms_layer_stride[b]);
   }

   bool stencil_in_depth_load = fb_ops->stencil_in_depth && fb_ops->stencil_load;
   if (fb_ops->depth_load || stencil_in_depth_load)
      write_load(&instr, v3d_ldst_buf_ds(fb_ops->depth_load, stencil_in_depth_load), &fb_ops->depth_ls,
            layer * fb_ops->depth_layer_stride);
   if (!fb_ops->stencil_in_depth && fb_ops->stencil_load)
      write_load(&instr, V3D_LDST_BUF_STENCIL, &fb_ops->stencil_ls,
            layer * fb_ops->stencil_layer_stride);

   v3d_cl_end_loads(&instr);

   khrn_fmem_end_cle(&rs->fmem, instr);

   return true;
}

static void count_ops(
      unsigned num_rt_stores[GLXX_MAX_RENDER_TARGETS],
      uint32_t *rt_store_mask, bool *ds_load, bool *ds_store,
      const GLXX_HW_RENDER_STATE_T *rs, const struct glxx_hw_tile_list_fb_ops *fb_ops,
      unsigned layer)
{
   memset(num_rt_stores, 0, GLXX_MAX_RENDER_TARGETS * sizeof(*num_rt_stores));
   *rt_store_mask = 0;
   *ds_load = false;
   *ds_store = false;

   if (layer == 0)
   {
      for (unsigned i = 0; i != rs->num_blits; ++i)
      {
         const GLXX_BLIT_T *blit = &rs->tlb_blits[i];
         if (blit->color)
         {
            num_rt_stores[blit->color_read_buffer] += gfx_num_set_bits(blit->color_draw_to_buf);
            if (blit->color_draw_to_buf)
               *rt_store_mask |= 1u << blit->color_read_buffer;
         }
         if (blit->depth)
            *ds_store = true;
      }
   }

   for (uint32_t b = 0, nonms_mask = fb_ops->rt_nonms_store_mask, ms_mask = fb_ops->rt_ms_store_mask;
         nonms_mask || ms_mask; ++b, nonms_mask >>= 1, ms_mask >>= 1)
   {
      if (nonms_mask & 1)
         ++num_rt_stores[b];
      if (ms_mask & 1)
         ++num_rt_stores[b];
   }
   *rt_store_mask |= fb_ops->rt_nonms_store_mask | fb_ops->rt_ms_store_mask;
   if (fb_ops->depth_load || fb_ops->stencil_load)
      *ds_load = true;
   if (fb_ops->depth_store || fb_ops->stencil_store)
      *ds_store = true;
}

static void write_store(uint8_t **instr,
      v3d_ldst_buf_t buf, const struct v3d_tlb_ldst_params *ls, uint32_t layer_offset, bool clear)
{
   v3d_cl_store(instr, buf, ls->memory_format, ls->flipy, ls->dither,
         ls->decimate, ls->pixel_format, clear,
#if V3D_VER_AT_LEAST(4,1,34,0)
         ls->chan_reverse, ls->rb_swap,
#endif
         ls->stride, ls->flipy_height_px, ls->addr + layer_offset);
}

static void write_rt_store(uint8_t **instr, unsigned remaining_rt_stores[GLXX_MAX_RENDER_TARGETS],
      unsigned b, const struct v3d_tlb_ldst_params *ls, uint32_t layer_offset)
{
   write_store(instr, v3d_ldst_buf_color(b), ls, layer_offset, --remaining_rt_stores[b] == 0);
}

//blits use only layer 0 when src or dst layer are layered
static bool write_blit_stores(uint8_t **instr, unsigned remaining_rt_stores[GLXX_MAX_RENDER_TARGETS],
      GLXX_HW_RENDER_STATE_T *rs, bool tlb_ms, const GLXX_BLIT_T *blit)
{
   if (blit->color)
   {
      for (uint32_t b = 0, mask = blit->color_draw_to_buf; mask; ++b, mask >>= 1)
      {
         if (mask & 1)
         {
            struct v3d_tlb_ldst_params ls;
            if (!prep_tlb_ldst(&ls, NULL, rs, tlb_ms,
                     &blit->dst_fb.color[b], /*img_ms=*/false, /*color=*/true,
                     /*load=*/false, /*store=*/true))
               return false;
            write_rt_store(instr, remaining_rt_stores, blit->color_read_buffer, &ls, 0);
         }
      }
   }

   if (blit->depth)
   {
      struct v3d_tlb_ldst_params ls;
      if (!prep_tlb_ldst(&ls, NULL, rs, tlb_ms,
               &blit->dst_fb.depth, /*img_ms=*/blit->dst_fb.ms, /*color=*/false,
               /*load=*/false, /*store=*/true))
         return false;
      write_store(instr, V3D_LDST_BUF_DEPTH, &ls, 0, /*clear=*/false);
   }

   return true;
}

static void write_fb_stores(uint8_t **instr, unsigned remaining_rt_stores[GLXX_MAX_RENDER_TARGETS],
      const struct glxx_hw_tile_list_fb_ops *fb_ops, uint32_t layer)
{
   for (uint32_t b = 0, nonms_mask = fb_ops->rt_nonms_store_mask, ms_mask = fb_ops->rt_ms_store_mask;
         nonms_mask || ms_mask; ++b, nonms_mask >>= 1, ms_mask >>= 1)
   {
      if (ms_mask & 1)
         write_rt_store(instr, remaining_rt_stores, b, &fb_ops->rt_ms_ls[b],
               layer * fb_ops->rt_ms_layer_stride[b]);
      if (nonms_mask & 1)
         write_rt_store(instr, remaining_rt_stores, b, &fb_ops->rt_nonms_ls[b],
               layer * fb_ops->rt_nonms_layer_stride[b]);
   }

   bool stencil_in_depth_store = fb_ops->stencil_in_depth && fb_ops->stencil_store;
   if (fb_ops->depth_store || stencil_in_depth_store)
      write_store(instr, v3d_ldst_buf_ds(fb_ops->depth_store, stencil_in_depth_store), &fb_ops->depth_ls,
            layer * fb_ops->depth_layer_stride, /*clear=*/false);
   if (!fb_ops->stencil_in_depth && fb_ops->stencil_store)
      write_store(instr, V3D_LDST_BUF_STENCIL, &fb_ops->stencil_ls,
            layer * fb_ops->stencil_layer_stride, /*clear=*/false);
}

static bool write_stores_clears_end_and_rcfg(
      struct glxx_hw_tile_list_rcfg *rcfg,
      GLXX_HW_RENDER_STATE_T *rs, bool tlb_ms, bool double_buffer,
      const struct glxx_hw_tile_list_fb_ops *fb_ops, uint32_t layer)
{
   unsigned remaining_rt_stores[GLXX_MAX_RENDER_TARGETS];
   uint32_t rt_store_mask;
   bool ds_load, ds_store;
   count_ops(remaining_rt_stores, &rt_store_mask, &ds_load, &ds_store, rs, fb_ops, layer);

   size_t size =
      (layer == 0 ? (rs->num_blits * ((GLXX_MAX_RENDER_TARGETS + 1) * V3D_CL_STORE_SIZE)) : 0) + /* Blit stores */
      (((2 * GLXX_MAX_RENDER_TARGETS) + 2) * V3D_CL_STORE_SIZE) + /* Framebuffer stores */
      V3D_CL_CLEAR_SIZE +
      V3D_CL_END_TILE_SIZE +
      V3D_CL_RETURN_SIZE;

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, size);
   if (!instr)
      return false;

   if (layer == 0)
   {
      //blits use only layer 0
      for (unsigned i = 0; i != rs->num_blits; ++i)
         if (!write_blit_stores(&instr, remaining_rt_stores, rs, tlb_ms, &rs->tlb_blits[i]))
            return false;
   }

   write_fb_stores(&instr, remaining_rt_stores, fb_ops, layer);

   for (unsigned i = 0; i != countof(remaining_rt_stores); ++i)
      assert(remaining_rt_stores[i] == 0);

   /* Need at least one store. Put a dummy one in if necessary... */
   if (!rt_store_mask && !ds_store)
      v3d_cl_store(&instr,
            V3D_LDST_BUF_NONE,
            V3D_MEMORY_FORMAT_RASTER,
            /*flipy=*/false,
            V3D_DITHER_OFF,
            V3D_DECIMATE_SAMPLE0,
            V3D_PIXEL_FORMAT_SRGB8_ALPHA8,
            /*clear=*/false,
#if V3D_VER_AT_LEAST(4,1,34,0)
            /*chan_reverse=*/false,
            /*rb_swap=*/false,
#endif
            /*stride=*/0,
            /*height=*/0,
            /*addr=*/0);

   /* We always set the clear flag on the last store of each RT. If an RT is
    * not stored but needs to be cleared we do a full clear of all RTs at the
    * end. */
   bool clear_all_rts_at_end = (fb_ops->rt_clear_mask & ~rt_store_mask) != 0;

#if V3D_VER_AT_LEAST(4,1,34,0)
   /* Can enable early depth/stencil clear optimisation in HW if we are not
    * loading or storing depth/stencil. Never enable in double-buffer mode --
    * doesn't make sense. */
   rcfg->early_ds_clear = !double_buffer && !ds_load && !ds_store;
#else
   rcfg->early_ds_clear = false;
#endif

   if (clear_all_rts_at_end || !rcfg->early_ds_clear)
      v3d_cl_clear(&instr, clear_all_rts_at_end, !rcfg->early_ds_clear);

   v3d_cl_end_tile(&instr);

   v3d_cl_return(&instr);

   khrn_fmem_end_cle(&rs->fmem, instr);

   return true;
}

static v3d_empty_tile_mode calc_et_mode(const GLXX_HW_RENDER_STATE_T *rs,
      const struct glxx_hw_tile_list_fb_ops *fb_ops)
{
   /* Not currently supported with new TLB config stuff. See GFXH-1392 & GFXH-1393. */
   return V3D_EMPTY_TILE_MODE_NONE;
}

#else

/* Framebuffer loads/stores done with LOAD & STORE_SUBSAMPLE instructions,
 * except for multisample color buffer loads/stores which are done with raw
 * mode LOAD_GENERAL/STORE_GENERAL instructions.
 *
 * TLB blits are done with STORE_GENERAL instructions. It is assumed that all
 * blits in rs->tlb_blits[] are compatible with STORE_GENERAL! eg The target
 * buffers are all UIF. */

static bool write_begin_and_loads(GLXX_HW_RENDER_STATE_T *rs, const struct glxx_hw_tile_list_fb_ops *fb_ops,
      unsigned layer)
{
   assert(layer == 0);

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem,
         V3D_CL_LOAD_SIZE +
         (GLXX_MAX_RENDER_TARGETS * (V3D_CL_IMPLICIT_TILE_COORDS_SIZE + V3D_CL_STORE_GENERAL_SIZE + V3D_CL_LOAD_GENERAL_SIZE)) + /* Multisample color loads */
         V3D_CL_IMPLICIT_TILE_COORDS_SIZE);
   if (!instr)
      return false;

   bool loaded = fb_ops->rt_nonms_load_mask || fb_ops->depth_load || fb_ops->stencil_load;
   if (loaded)
      v3d_cl_load(&instr, fb_ops->stencil_load, fb_ops->depth_load,
            ~fb_ops->rt_nonms_load_mask & gfx_mask(V3D_MAX_RENDER_TARGETS));

   for (uint32_t b = 0, mask = fb_ops->rt_ms_load_mask; mask; ++b, mask >>= 1)
   {
      if (!(mask & 1))
         continue;

      if (loaded)
      {
         v3d_cl_implicit_tile_coords(&instr);

         // Dummy store general.
         v3d_cl_store_general(&instr,
               V3D_LDST_BUF_NONE,
               false,               // raw mode
               true, true, true,    // disable clears
               false,               // eof
               true,                // disable double buffer swap
               V3D_LDST_MEMORY_FORMAT_UIF_NO_XOR,
               0, 0);
      }
      loaded = true;

      const struct v3d_tlb_ldst_params *ls = &fb_ops->rt_ms_ls[b];
      assert(!ls->flipy);
      assert(ls->decimate == V3D_DECIMATE_ALL_SAMPLES);
      assert(ls->output_format.pixel == v3d_raw_mode_pixel_format(
         rs->installed_fb.color_rt_format[b].type,
         rs->installed_fb.color_rt_format[b].bpp));
      v3d_cl_load_general(&instr,
            v3d_ldst_buf_color(b), /*raw_mode=*/true,
            v3d_memory_format_to_ldst(ls->memory_format),
            ls->stride,
            ls->addr);
   }

   v3d_cl_implicit_tile_coords(&instr);

   khrn_fmem_end_cle(&rs->fmem, instr);

   return true;
}

static void rcfg_init_unused_params(struct v3d_tlb_ldst_params *ls, bool color)
{
   memset(ls, 0, sizeof(*ls));
   ls->dither = V3D_DITHER_INVALID;
   ls->decimate = V3D_DECIMATE_INVALID;
   if (color)
      ls->output_format.pixel = V3D_PIXEL_FORMAT_INVALID;
   else
      ls->output_format.depth = V3D_DEPTH_FORMAT_INVALID;
}

static void rcfg_init(struct glxx_hw_tile_list_rcfg *rcfg, const struct glxx_hw_tile_list_fb_ops *fb_ops)
{
   for (unsigned b = 0; b != GLXX_MAX_RENDER_TARGETS; ++b)
      if ((fb_ops->rt_nonms_load_mask | fb_ops->rt_nonms_store_mask) & (1u << b))
         rcfg->rt_ls[b] = fb_ops->rt_nonms_ls[b];
      else
         rcfg_init_unused_params(&rcfg->rt_ls[b], /*color=*/true);
   rcfg->rt_store_mask = fb_ops->rt_nonms_store_mask;

   bool stencil_ldst = have_stencil_ldst(fb_ops);
   rcfg->sep_stencil = stencil_ldst && (fb_ops->stencil_ls.output_format.depth == V3D_DEPTH_FORMAT_INVALID); /* S8 stencil */
   if (have_depth_ldst(fb_ops))
   {
      assert(!stencil_ldst || rcfg->sep_stencil);
      rcfg->depth_ls = fb_ops->depth_ls;
   }
   else if (stencil_ldst && !rcfg->sep_stencil)
      rcfg->depth_ls = fb_ops->stencil_ls;
   else
      rcfg_init_unused_params(&rcfg->depth_ls, /*color=*/false);
   if (rcfg->sep_stencil)
      rcfg->sep_stencil_ls = fb_ops->stencil_ls;
   else
      rcfg_init_unused_params(&rcfg->sep_stencil_ls, /*color=*/false);
   rcfg->depth_store = fb_ops->depth_store;
   rcfg->stencil_store = fb_ops->stencil_store;
}

static void rcfg_finalise_params(struct v3d_tlb_ldst_params *ls, bool color)
{
   /* Ensure all fields are set to valid values... */

   if (ls->dither == V3D_DITHER_INVALID)
      ls->dither = V3D_DITHER_OFF;

   if (ls->decimate == V3D_DECIMATE_INVALID)
      ls->decimate = V3D_DECIMATE_SAMPLE0;

   if (color)
   {
      if (ls->output_format.pixel == V3D_PIXEL_FORMAT_INVALID)
         ls->output_format.pixel = V3D_PIXEL_FORMAT_SRGB8_ALPHA8;
   }
   else
   {
      if (ls->output_format.depth == V3D_DEPTH_FORMAT_INVALID)
         ls->output_format.depth = V3D_DEPTH_FORMAT_32F;
   }
}

static void rcfg_finalise(struct glxx_hw_tile_list_rcfg *rcfg)
{
   if (rcfg->sep_stencil)
   {
      /* Any depth format without stencil will do... */
      if (rcfg->depth_ls.output_format.depth == V3D_DEPTH_FORMAT_INVALID)
         rcfg->depth_ls.output_format.depth = V3D_DEPTH_FORMAT_32F;
      else
         assert(!v3d_depth_format_has_stencil(rcfg->depth_ls.output_format.depth));
   }

   for (unsigned b = 0; b != GLXX_MAX_RENDER_TARGETS; ++b)
      rcfg_finalise_params(&rcfg->rt_ls[b], /*color=*/true);
   rcfg_finalise_params(&rcfg->depth_ls, /*color=*/false);
   rcfg_finalise_params(&rcfg->sep_stencil_ls, /*color=*/false);
}

static void write_store_general(uint8_t **instr,
      struct glxx_hw_tile_list_rcfg *rcfg, /* Needed only when raw_mode=false */
      const GLXX_HW_FRAMEBUFFER_T *hw_fb, /* Needed only when raw_mode=true */
      v3d_ldst_buf_t buf, const struct v3d_tlb_ldst_params *ls, bool raw_mode)
{
   v3d_ldst_buf_class_t buf_class = v3d_classify_ldst_buf(buf);

   assert(!ls->flipy);
   if (raw_mode)
   {
      assert(ls->decimate == V3D_DECIMATE_ALL_SAMPLES);
      assert(buf_class == V3D_LDST_BUF_CLASS_COLOR);
      unsigned b = v3d_ldst_buf_rt(buf);
      assert(ls->output_format.pixel == v3d_raw_mode_pixel_format(
         hw_fb->color_rt_format[b].type, hw_fb->color_rt_format[b].bpp));
   }
   else
   {
      /* Non-raw-mode STORE_GENERAL uses some fields from the main rendering
       * mode config! Ensure these fields are set appropriately... */
      assert(rcfg);

      struct v3d_tlb_ldst_params *rcfg_ls;
      if (buf_class == V3D_LDST_BUF_CLASS_COLOR)
      {
         rcfg_ls = &rcfg->rt_ls[v3d_ldst_buf_rt(buf)];

         if (rcfg_ls->output_format.pixel == V3D_PIXEL_FORMAT_INVALID)
            rcfg_ls->output_format.pixel = ls->output_format.pixel;
         else
            assert(v3d_pixel_format_equiv_for_store(rcfg_ls->output_format.pixel, ls->output_format.pixel));
      }
      else
      {
         assert(buf == V3D_LDST_BUF_DEPTH);
         rcfg_ls = &rcfg->depth_ls;

         if (rcfg_ls->output_format.depth == V3D_DEPTH_FORMAT_INVALID)
            rcfg_ls->output_format.depth = ls->output_format.depth;
         else
            assert(rcfg_ls->output_format.depth == ls->output_format.depth);
      }

      if (rcfg_ls->dither == V3D_DITHER_INVALID)
         rcfg_ls->dither = ls->dither;

      if (rcfg_ls->decimate == V3D_DECIMATE_INVALID)
         rcfg_ls->decimate = ls->decimate;
      else
         assert(rcfg_ls->decimate == ls->decimate);
   }

   v3d_cl_store_general(instr, buf, raw_mode,
         true, true, true, // disable clears
         false,            // eof (never set using flag)
         true,             // disable double buffer swap
         v3d_memory_format_to_ldst(ls->memory_format),
         ls->stride,
         ls->addr);
}

static bool write_blit_stores(uint8_t **instr, struct glxx_hw_tile_list_rcfg *rcfg,
      GLXX_HW_RENDER_STATE_T *rs, bool tlb_ms, const GLXX_BLIT_T *blit)
{
   if (blit->color)
   {
      for (uint32_t b = 0, mask = blit->color_draw_to_buf; mask; ++b, mask >>= 1)
      {
         if (mask & 1)
         {
            struct v3d_tlb_ldst_params ls;
            if (!prep_tlb_ldst(&ls, NULL, rs, tlb_ms,
                     &blit->dst_fb.color[b], /*img_ms=*/false, /*color=*/true,
                     /*load=*/false, /*store=*/true))
               return false;
            write_store_general(instr, rcfg, NULL,
                  v3d_ldst_buf_color(blit->color_read_buffer), &ls, /*raw_mode=*/false);
            v3d_cl_implicit_tile_coords(instr);
         }
      }
   }

   if (blit->depth)
   {
      struct v3d_tlb_ldst_params ls;
      if (!prep_tlb_ldst(&ls, NULL, rs, tlb_ms,
               &blit->dst_fb.depth, /*img_ms=*/blit->dst_fb.ms, /*color=*/false,
               /*load=*/false, /*store=*/true))
         return false;
      write_store_general(instr, rcfg, NULL,
            V3D_LDST_BUF_DEPTH, &ls, /*raw_mode=*/false);
      v3d_cl_implicit_tile_coords(instr);
   }

   return true;
}

static bool write_stores_clears_end_and_rcfg(
      struct glxx_hw_tile_list_rcfg *rcfg,
      GLXX_HW_RENDER_STATE_T *rs, bool tlb_ms, bool double_buffer,
      const struct glxx_hw_tile_list_fb_ops *fb_ops, uint32_t layer)
{
   assert(layer == 0);

   rcfg_init(rcfg, fb_ops);

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem,
         (rs->num_blits * ((GLXX_MAX_RENDER_TARGETS + 1) * (V3D_CL_STORE_GENERAL_SIZE + V3D_CL_IMPLICIT_TILE_COORDS_SIZE))) + /* Blit stores */
         (GLXX_MAX_RENDER_TARGETS * (V3D_CL_STORE_GENERAL_SIZE + V3D_CL_IMPLICIT_TILE_COORDS_SIZE)) + /* Multisample color stores */
         V3D_CL_STORE_SUBSAMPLE_SIZE +
         V3D_CL_RETURN_SIZE);
   if (!instr)
      return false;

   for (unsigned i = 0; i != rs->num_blits; ++i)
      if (!write_blit_stores(&instr, rcfg, rs, tlb_ms, &rs->tlb_blits[i]))
         return false;

   for (uint32_t b = 0, mask = fb_ops->rt_ms_store_mask; mask; ++b, mask >>= 1)
   {
      if (mask & 1)
      {
         write_store_general(&instr, NULL, &rs->installed_fb,
               v3d_ldst_buf_color(b), &fb_ops->rt_ms_ls[b], /*raw_mode=*/true);
         v3d_cl_implicit_tile_coords(&instr);
      }
   }

   // Short form store. Might not actually store anything. Clears all buffers.
   v3d_cl_store_subsample(&instr);

   v3d_cl_return(&instr);

   khrn_fmem_end_cle(&rs->fmem, instr);

   rcfg_finalise(rcfg);

   return true;
}

/* For restricitons on empty tile processing, see CT1CFG Register */

static v3d_empty_tile_mode calc_et_mode(const GLXX_HW_RENDER_STATE_T *rs,
      const struct glxx_hw_tile_list_fb_ops *fb_ops)
{
   /* Empty tile filtering/blasting not supported before 3.3.
    * Cannot skip empty tiles if doing any blitting. */
   if (V3D_VER_AT_LEAST(3,3,0,0) && (rs->num_blits == 0))
   {
      unsigned num_load_instructions =
         ((fb_ops->rt_nonms_load_mask || fb_ops->depth_load || fb_ops->stencil_load) ? 1 : 0) +
         gfx_num_set_bits(fb_ops->rt_ms_load_mask);
      /* We stick dummy STORE_GENERAL instructions between load instructions... */
      bool any_store_generals = fb_ops->rt_ms_store_mask || (num_load_instructions > 1);

      /* If storing either undefined data or what's already there, we can skip
       * empty tiles */
      if (!khrn_options.no_empty_tile_skip &&
            /* STORE_GENERAL instructions not skipped properly; see GFXH-1515.
             * Note all MS stores are done with STORE_GENERALs so there is no
             * separate check for MS stores here. */
            !any_store_generals &&
            ((fb_ops->rt_nonms_store_mask & fb_ops->rt_clear_mask) == 0) &&
            (!fb_ops->depth_store || !fb_ops->depth_clear) &&
            (!fb_ops->stencil_store || !fb_ops->stencil_clear))
         return V3D_EMPTY_TILE_MODE_SKIP;
   }

   return V3D_EMPTY_TILE_MODE_NONE;
}

#endif

static bool write_tile_list_branches(GLXX_HW_RENDER_STATE_T *rs)
{
   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem,
#if V3D_VER_AT_LEAST(4,1,34,0)
         V3D_CL_SET_INSTANCE_ID_SIZE +
#endif
#if !V3D_VER_AT_LEAST(3,3,0,0)
         (rs->workaround_gfxh_1313 ? glxx_workaround_gfxh_1313_size() : 0) +
#endif
         V3D_CL_ENABLE_Z_ONLY_SIZE +
         (rs->num_z_prepass_bins * V3D_CL_BRANCH_IMPLICIT_TILE_SIZE) +
         V3D_CL_DISABLE_Z_ONLY_SIZE +
#if !V3D_VER_AT_LEAST(4,1,34,0)
         (rs->fmem.br_info.bin_subjobs.num_subjobs * V3D_CL_PRIM_LIST_FORMAT_SIZE) +
#endif
         (rs->fmem.br_info.bin_subjobs.num_subjobs * V3D_CL_BRANCH_IMPLICIT_TILE_SIZE));
   if (!instr)
      return false;

#if V3D_VER_AT_LEAST(4,1,34,0)
   /* Must set base instance to 0 at start of tile; PTB assumes this! */
   v3d_cl_set_instance_id(&instr, 0);
#endif

#if !V3D_VER_AT_LEAST(3,3,0,0)
   if (rs->workaround_gfxh_1313 && !glxx_workaround_gfxh_1313(&instr, &rs->fmem, rs->installed_fb.width, rs->installed_fb.height))
      return false;
#endif

   if (rs->num_z_prepass_bins)
   {
      v3d_cl_enable_z_only(&instr);
      for (unsigned i = 0; i != rs->num_z_prepass_bins; ++i)
         v3d_cl_branch_implicit_tile(&instr, i);
      v3d_cl_disable_z_only(&instr);
   }

   for (unsigned i = 0; i != rs->fmem.br_info.bin_subjobs.num_subjobs; ++i)
   {
#if !V3D_VER_AT_LEAST(4,1,34,0)
      v3d_cl_prim_list_format(&instr, 3, false, false);
#endif
      v3d_cl_branch_implicit_tile(&instr, i);
   }

   khrn_fmem_end_cle(&rs->fmem, instr);

   return true;
}

bool glxx_hw_create_generic_tile_list(v3d_addr_range *range,
      struct glxx_hw_tile_list_rcfg *rcfg,
      GLXX_HW_RENDER_STATE_T *rs, bool tlb_ms, bool double_buffer,
      const struct glxx_hw_tile_list_fb_ops *fb_ops,
      unsigned layer)
{
   rs->fmem.br_info.details.empty_tile_mode = calc_et_mode(rs, fb_ops);

   range->begin = khrn_fmem_begin_clist(&rs->fmem);
   if (!range->begin ||
         !write_begin_and_loads(rs, fb_ops, layer) ||
         !write_tile_list_branches(rs) ||
         !write_stores_clears_end_and_rcfg(rcfg, rs, tlb_ms, double_buffer, fb_ops, layer))
      return false;
   range->end = khrn_fmem_end_clist(&rs->fmem);
   return true;
}
