/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Tile list construction logic.

See http://confluence/display/MobileMultimedia/GFX+VC5+Control+list+generation

=============================================================================*/

#include "glxx_hw_master_cl.h"
#include "glxx_hw_render_state.h"
#include "glxx_hw.h"
#include "../common/khrn_options.h"
#include "libs/util/gfx_util/gfx_util_morton.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/v3d/v3d_util.h"

#define MAX_BOPS 11     /* Should be large enough to hold all blits. TODO: it isn't */

typedef enum {
   OP_LOAD = RCFG_LOAD,
   OP_STORE = RCFG_STORE
} buf_op_t;

typedef struct {
   buf_op_t op;
   KHRN_IMAGE_PLANE_T image;
   bool ms;                   /* If true, load/store ms data, otherwise up/down sample */
} BOP_T;

typedef struct
{
   BOP_T op[MAX_BOPS];
   int n_ops;
} LDST_BUFFER_T;

typedef struct
{
   LDST_BUFFER_T buffers[GLXX_MAX_RENDER_TARGETS + 2];
} LDST_T;

static void buffer_ldst_term(LDST_BUFFER_T *ldst) {
   for (int i=0; i<MAX_BOPS; i++) {
      khrn_image_plane_assign(&ldst->op[i].image, NULL);
   }
}

static inline void ldst_init(LDST_T *ldst)
{
   memset(ldst, 0, sizeof(*ldst));
}

static inline void ldst_term(LDST_T *ldst)
{
   for (unsigned i = 0; i != countof(ldst->buffers); ++i)
      buffer_ldst_term(&ldst->buffers[i]);
}

static void add_buffer_op(LDST_BUFFER_T *ldst, buf_op_t op, bool ms, KHRN_IMAGE_PLANE_T *image)
{
   int idx = ldst->n_ops++;
   assert(idx < MAX_BOPS);
   khrn_image_plane_assign(&ldst->op[idx].image, image);
   ldst->op[idx].ms = ms;
   ldst->op[idx].op = op;
}

static void load_or_store_buffer(LDST_BUFFER_T *ldst, bool load, bool store,
                                 bool ms, KHRN_IMAGE_PLANE_T *image)
{
   if (load)  add_buffer_op(ldst, OP_LOAD, ms, image);
   if (store) add_buffer_op(ldst, OP_STORE, ms, image);
}

static void flush_with_blit(GLXX_BUFSTATE_FLUSH_T *f, glxx_bufstate_t bufstate, bool does_blit)
{
   /* If the image hasn't changed (state MEM) then we will neither load nor
    * store it. This is good for bandwidth use but bad if we're trying to blit
    * it elsewhere. Here we force the image load if we're blitting it
    */
   glxx_bufstate_flush(f, bufstate);
   if (does_blit && bufstate == GLXX_BUFSTATE_MEM) f->load = true;
}

static void ldst_from_renderstate(LDST_T *ldst, GLXX_HW_RENDER_STATE_T *rs)
{
   bool blits_color = false;
   bool blits_depth = false;
   bool blits_stencil = false;

   for (unsigned i = 0; i<rs->num_blits; i++) {
      if (rs->tlb_blits[i].color) blits_color = true;
      if (rs->tlb_blits[i].depth) blits_depth = true;
      if (rs->tlb_blits[i].stencil) blits_stencil = true;
   }

   /* TODO: Does this do the right things wrt color_draw_buffers? */
   for (int b = 0; b < GLXX_MAX_RENDER_TARGETS; b++)
   {
      GLXX_BUFSTATE_FLUSH_T f, f_ms;
      flush_with_blit(&f, rs->color_buffer_state[b], blits_color);
      flush_with_blit(&f_ms, rs->ms_color_buffer_state[b], blits_color);

      /* If we have both a downsampled color buffer and a multisample color
       * buffer, both load flags mean load from the multisample buffer. See the
       * comments in GLXX_HW_RENDER_STATE_T for more details. */
      if ((rs->color_buffer_state   [b] != GLXX_BUFSTATE_MISSING) &&
          (rs->ms_color_buffer_state[b] != GLXX_BUFSTATE_MISSING)   )
      {
         assert(f.load == f_ms.load);
         f.load = false;
      }

      load_or_store_buffer(&ldst->buffers[RCFG_COLOR(b)], f_ms.load, f_ms.store,
                                            true, &rs->installed_fb.color_ms[b]);
      load_or_store_buffer(&ldst->buffers[RCFG_COLOR(b)], f.load, f.store,
                                            false, &rs->installed_fb.color[b]);
   }

   GLXX_BUFSTATE_FLUSH_T f;
   flush_with_blit(&f, rs->depth_buffer_state, blits_depth);
   load_or_store_buffer(&ldst->buffers[RCFG_DEPTH], f.load, f.store, rs->installed_fb.ms, &rs->installed_fb.depth);

   flush_with_blit(&f, rs->stencil_buffer_state, blits_stencil);
   load_or_store_buffer(&ldst->buffers[RCFG_STENCIL], f.load, f.store, rs->installed_fb.ms, &rs->installed_fb.stencil);

   for (unsigned i=0; i<rs->num_blits; i++) {
      GLXX_BLIT_T *blit = &rs->tlb_blits[i];
      if (blit->color)
      {
         for (int b=0; b<GLXX_MAX_RENDER_TARGETS; b++) {
            if (blit->color_draw_to_buf[b]) {
               KHRN_IMAGE_PLANE_T *plane = &blit->dst_fb.color[b];
               bool is_ms_img = false;
               if (blit->dst_fb.color_ms[b].image) {
                  plane = &blit->dst_fb.color_ms[b];
                  is_ms_img = true;
               }

               load_or_store_buffer(&ldst->buffers[RCFG_COLOR(blit->color_read_buffer)], false, true,
                                    is_ms_img, plane);
            }
         }
      }
      if (blit->depth)
         load_or_store_buffer(&ldst->buffers[RCFG_DEPTH], false, true, blit->dst_fb.ms, &blit->dst_fb.depth);
      if (blit->stencil)
         load_or_store_buffer(&ldst->buffers[RCFG_STENCIL], false, true, blit->dst_fb.ms, &blit->dst_fb.stencil);
   }
}

static void deinit_tile_op(TILE_OP_T *op)
{
   khrn_image_plane_assign(&op->image_plane, NULL);
}

static void deinit_buffer_ops(BUFFER_OPS_T *ops)
{
   khrn_image_plane_assign(&ops->short_form_img_plane, NULL);
}

static void init_master_cl_info(MASTER_CL_INFO_T *info)
{
   memset(info, 0, sizeof(MASTER_CL_INFO_T));

   for (unsigned i = 0; i < GLXX_MAX_RENDER_TARGETS; ++i)
      info->rcfg.color_translation[i] = gfx_buffer_rcfg_color_translation_dummy();

   info->rcfg.depth_type = V3D_DEPTH_TYPE_32F;
   info->rcfg.depth_output_format = V3D_DEPTH_FORMAT_32F;
   info->rcfg.depth_memory_format = V3D_MEMORY_FORMAT_LINEARTILE;
   info->rcfg.stencil_memory_format = V3D_MEMORY_FORMAT_LINEARTILE;
}

static bool check_image_storages(const MASTER_CL_INFO_T *info)
{
   for (unsigned s = 0; s != countof(info->rcfg.tile_ops); ++s)
   {
      TILE_OPS_T const* ops = &info->rcfg.tile_ops[s];

      // Short form loads/stores.
      unsigned mask = ops->short_op_mask;
      for (unsigned i = 0; mask && i != countof(info->rcfg.buffers); ++i, mask >>= 1)
      {
         if ((mask & 1) && !khrn_blob_alloc_storage(info->rcfg.buffers[i].short_form_img_plane.image->blob))
            return false;
      }

      // General loads/stores.
      for (unsigned i = 0; i != ops->num_general_ops; ++i)
      {
         if (!khrn_blob_alloc_storage(ops->general_ops[i].image_plane.image->blob))
            return false;
      }
   }

   return true;
}

static void add_tile_op_general(bool cfg_non_ms,
                                RCFG_INFO_T *rcfg,
                                rcfg_buffer buf,
                                const BOP_T *bop)
{
   TILE_OPS_T* ops = &rcfg->tile_ops[bop->op];

   assert(ops->num_general_ops < TILE_OP_MAX_GENERAL);
   TILE_OP_T* op = &ops->general_ops[ops->num_general_ops++];
   op->raw = bop->ms && cfg_non_ms;
   switch (buf)
   {
   default:
      assert((buf - RCFG_COLOR0) < GLXX_MAX_RENDER_TARGETS);
      op->buf = V3D_LDST_BUF_COLOR0 + (buf - RCFG_COLOR0);
      break;
   case RCFG_DEPTH:
      op->buf = V3D_LDST_BUF_DEPTH;
      break;
   case RCFG_STENCIL:
      op->buf = V3D_LDST_BUF_STENCIL;
      break;
   }
   khrn_image_plane_assign(&op->image_plane, &bop->image);
}

static inline void add_tile_op_short(RCFG_INFO_T *rcfg, bool load, rcfg_buffer buf)
{
   rcfg->tile_ops[load ? RCFG_LOAD : RCFG_STORE].short_op_mask |= 1 << buf;
}

static bool blits_depth(const GLXX_HW_RENDER_STATE_T *rs)
{
   for (unsigned i = 0; i < rs->num_blits; ++i)
      if (rs->tlb_blits[i].depth)
         return true;

   return false;
}

static bool choose_packed_depth_stencil_strategy(const GLXX_HW_RENDER_STATE_T *rs)
{
   const GLXX_HW_FRAMEBUFFER_T *fb = &rs->installed_fb;
   // We can have packed DS even if one of the D or S are not attached.
   const KHRN_IMAGE_PLANE_T *packed_image = fb->depth.image ? &fb->depth : &fb->stencil;
   GFX_LFMT_T packed_lfmt;

   if(!packed_image->image)
      // If there is no depth or stencil attached, this doesnt really matter.
      return false;

   packed_lfmt = khrn_image_plane_lfmt(packed_image);

   if ((gfx_lfmt_depth_bits(packed_lfmt) != 24) || (gfx_lfmt_stencil_bits(packed_lfmt) != 8))
      return false;

   // In multisample mode, we need to use all_samples if we want to use
   // packed depth stencil. But we cannot use all_samples if there are
   // any non_ms ldst operations. Only source for these would be TLB-blits
   // with depth (stencil is currently never handled by TLB-blits).
   if (fb->ms)
      return !blits_depth(rs);

   return true;
}

#ifndef NDEBUG
static bool pixel_format_match(v3d_pixel_format_t lhs, v3d_pixel_format_t rhs)
{
   if (lhs == rhs)
      return true;

   if ((lhs == V3D_PIXEL_FORMAT_RGBA8 && rhs == V3D_PIXEL_FORMAT_RGBX8) ||
       (lhs == V3D_PIXEL_FORMAT_RGBX8 && rhs == V3D_PIXEL_FORMAT_RGBA8))
      return true;

   return false;
}
#endif

static bool lfmt_needs_short(GFX_LFMT_T lfmt) {
   return !(gfx_lfmt_is_uif(lfmt) || gfx_lfmt_is_uif_xor(lfmt)) || gfx_lfmt_get_yflip(&lfmt);
}

static BOP_T *choose_short_image(LDST_BUFFER_T *ldst, bool raster_ms) {
   for (int i=0; i<ldst->n_ops; i++)
      if (lfmt_needs_short(khrn_image_plane_lfmt(&ldst->op[i].image)))
         return &ldst->op[i];

   /* If rasterising ms then any non-ms stores must be short-form */
   if (raster_ms) {
      for (int i=0; i<ldst->n_ops; i++) if (!ldst->op[i].ms) return &ldst->op[i];
   }

   /* Nothing forces us to use short form. Just set up using store[0] */
   /* TODO: It's not obvious this is valid, but the old code did it */
   return &ldst->op[0];
}

static void setup_rcfg(BUFFER_OPS_T *buf_ops, LDST_BUFFER_T *ldst, bool raster_ms)
{
   BOP_T *short_op;
   /* Don't need an rcfg for unused buffers */
   if (ldst->n_ops == 0) return;

   short_op = choose_short_image(ldst, raster_ms);

   khrn_image_plane_assign(&buf_ops->short_form_img_plane, &short_op->image);
   buf_ops->short_form_ms = short_op->ms;
}

#ifndef NDEBUG
static bool color_compatible(const BUFFER_OPS_T *op, const BOP_T *bop)
{
   GFX_BUFFER_RCFG_COLOR_TRANSLATION_T t1;
   GFX_BUFFER_RCFG_COLOR_TRANSLATION_T t2;
   /* TODO: Don't hackily use 0 here. It makes no difference to this code, but we have to say something */
   khrn_image_plane_translate_rcfg_color(&op->short_form_img_plane, 0, 0, &t1);
   khrn_image_plane_translate_rcfg_color(&bop->image, 0, 0, &t2);

   if (t1.internal_type != t2.internal_type ||
       t1.internal_bpp  != t2.internal_bpp    )
   {
      return false;
   }

   v3d_pixel_format_t raw_format = v3d_pixel_format_raw_mode(t1.pixel_format);
   if (!pixel_format_match(t2.pixel_format, t1.pixel_format) &&
       !pixel_format_match(t2.pixel_format, raw_format)             )
      return false;

   return true;
}
#endif

#ifndef NDEBUG
static bool depth_compatible(const BUFFER_OPS_T *op, const BOP_T *bop)
{
   GFX_LFMT_T lfmt = khrn_image_plane_lfmt(&bop->image);
   assert(!gfx_lfmt_get_yflip(&lfmt));
   v3d_depth_type_t depth_type = gfx_lfmt_translate_depth_type(lfmt);
   v3d_depth_format_t output_format = gfx_lfmt_translate_depth_format(lfmt);

   GFX_LFMT_T short_lfmt = khrn_image_plane_lfmt(&op->short_form_img_plane);
   v3d_depth_type_t short_depth_type = gfx_lfmt_translate_depth_type(short_lfmt);
   v3d_depth_format_t short_output_format = gfx_lfmt_translate_depth_format(short_lfmt);

   if (depth_type != short_depth_type) return false;
   if (output_format != short_output_format) return false;

   return true;
}
#endif

#ifndef NDEBUG
static bool stencil_compatible(const BUFFER_OPS_T *buf_ops, const BOP_T *bop)
{
   /* TODO: This doesn't seem right, but it's all the checking that was here before */
   return true;
}
#endif

#ifndef NDEBUG
static bool general_compatible(const BUFFER_OPS_T *buf_op, const BOP_T *bop, rcfg_buffer buf)
{
   if (lfmt_needs_short(khrn_image_plane_lfmt(&bop->image)))
      return false;

   switch (buf)
   {
   default:             return color_compatible(buf_op, bop);
   case RCFG_DEPTH:     return depth_compatible(buf_op, bop);
   case RCFG_STENCIL:   return stencil_compatible(buf_op, bop);
   }
}
#endif

static bool use_short_ldst(BUFFER_OPS_T *buffer_ops, const BOP_T *bop)
{
   return khrn_image_plane_equal(&buffer_ops->short_form_img_plane, &bop->image);
}

static void add_tileop_for_buffer(RCFG_INFO_T *rcfg, rcfg_buffer buf,
                                  BOP_T *bop, bool packed_depth_stencil)
{
   rcfg_buffer ops_index = buf == RCFG_STENCIL && packed_depth_stencil ? RCFG_DEPTH : buf;
   BUFFER_OPS_T *buffer_ops = &rcfg->buffers[ops_index];

   if(use_short_ldst(buffer_ops, bop)) {
      bool load = (bop->op == OP_LOAD);
      if (load) buffer_ops->load = true;
      else      buffer_ops->store = true;

      assert(bop->ms == buffer_ops->short_form_ms);
      add_tile_op_short(rcfg, load, buf);
   } else {
      assert(general_compatible(buffer_ops, bop, buf));
      add_tile_op_general(!buffer_ops->short_form_ms, rcfg, buf, bop);
   }
}

static void get_rcfgs(LDST_T *ldst, RCFG_INFO_T *rcfg, bool packed_depth_stencil, int width, int height, bool raster_ms)
{
   /* Choose the short form images to be used */
   static_assrt(countof(rcfg->buffers) == countof(ldst->buffers));
   for (unsigned b = 0; b != countof(rcfg->buffers); ++b)
   {
      setup_rcfg(&rcfg->buffers[b], &ldst->buffers[b], raster_ms);
   }

   /* Depth needs to be configured in rendering mode config even when only
    * stencil part from packed depth and stencil image is used.            */
   /* TODO: Packed depth stencil handling could be much, much better */
   if (packed_depth_stencil && ldst->buffers[RCFG_DEPTH].n_ops == 0)
   {
      khrn_image_plane_assign(&rcfg->buffers[RCFG_DEPTH].short_form_img_plane, &rcfg->buffers[RCFG_STENCIL].short_form_img_plane);
      rcfg->buffers[RCFG_DEPTH].short_form_ms = rcfg->buffers[RCFG_STENCIL].short_form_ms;
   }

   /* Classify ops into load/store, short/general */
   for (unsigned b = 0; b != countof(ldst->buffers); ++b)
   {
      for (unsigned i = 0; i != ldst->buffers[b].n_ops; ++i)
      {
         /* TODO: Packed depth stencil will hackily switch to the depth short image here */
         add_tileop_for_buffer(rcfg, b, &ldst->buffers[b].op[i], packed_depth_stencil);
      }
   }

   /* TODO: Don't calculate this stuff here to cache it. Just calculate it when
    *       needed, later.
    */
   for (unsigned b=0; b<GLXX_MAX_RENDER_TARGETS; b++) {
      GFX_BUFFER_RCFG_COLOR_TRANSLATION_T *rcfg_t = &rcfg->color_translation[b];

      BUFFER_OPS_T* buffer_ops = &rcfg->buffers[RCFG_COLOR(b)];
      if (buffer_ops->short_form_img_plane.image == NULL) continue;

      if (buffer_ops->short_form_ms)
         khrn_image_plane_translate_rcfg_color(&buffer_ops->short_form_img_plane, width*2, height*2, rcfg_t);
      else
         khrn_image_plane_translate_rcfg_color(&buffer_ops->short_form_img_plane, width, height, rcfg_t);
   }

   BUFFER_OPS_T* depth_ops = &rcfg->buffers[RCFG_DEPTH];
   if (depth_ops->short_form_img_plane.image != NULL) {
      rcfg->depth_type = gfx_lfmt_translate_depth_type(khrn_image_plane_lfmt(&depth_ops->short_form_img_plane));
      rcfg->depth_output_format = gfx_lfmt_translate_depth_format(khrn_image_plane_lfmt(&depth_ops->short_form_img_plane));
      rcfg->depth_memory_format = khrn_image_plane_translate_memory_format(&depth_ops->short_form_img_plane);
   }

   BUFFER_OPS_T* stencil_ops = &rcfg->buffers[RCFG_STENCIL];
   if (stencil_ops->short_form_img_plane.image != NULL)
      rcfg->stencil_memory_format = khrn_image_plane_translate_memory_format(&stencil_ops->short_form_img_plane);

   if (packed_depth_stencil && ldst->buffers[RCFG_DEPTH].n_ops && ldst->buffers[RCFG_STENCIL].n_ops)
      assert(rcfg->depth_output_format == V3D_DEPTH_FORMAT_24_STENCIL8);
}

// Public API

void glxx_hw_build_master_cl_info(GLXX_HW_RENDER_STATE_T *rs, MASTER_CL_INFO_T *info_out)
{
   LDST_T ldst;
   ldst_init(&ldst);
   ldst_from_renderstate(&ldst, rs);

   init_master_cl_info(info_out);

   // Choose if we should use packed depth stencil or not
   bool packed_depth_stencil = choose_packed_depth_stencil_strategy(rs);

   // Choose image format for short form images and classify loads and stores
   get_rcfgs(&ldst, &info_out->rcfg, packed_depth_stencil,
             rs->installed_fb.width, rs->installed_fb.height, rs->installed_fb.ms);

   ldst_term(&ldst);

   // Make sure images that are loaded/stored have storage
   bool ok = check_image_storages(info_out);

   assert(ok); // TODO This can actually happen due to out of memory - signal this and handle!
   vcos_unused_in_release(ok);

   // figure out if we can double buffer in the tlb
   GLXX_HW_FRAMEBUFFER_T const *fb = &rs->installed_fb;
   bool tlb_ms = khrn_options.force_multisample || fb->ms;
   bool tlb_double_buffer = khrn_options.nonms_double_buffer && !tlb_ms;

   // compute number of tiles
   unsigned tilewidth;
   unsigned tileheight;
   v3d_tile_size_pixels(&tilewidth, &tileheight,
      fb->rt_count,
      tlb_double_buffer,
      v3d_translate_from_rt_bpp(fb->max_bpp),
      tlb_ms
      );
   assert(tilewidth >= 8);
   assert(tileheight >= 8);
   info_out->num_tiles_x = gfx_udiv_round_up(fb->width, tilewidth);
   info_out->num_tiles_y = gfx_udiv_round_up(fb->height, tileheight);
   info_out->tlb_ms = tlb_ms;
   info_out->tlb_double_buffer = tlb_double_buffer;
}

bool glxx_hw_generic_tile_list(GLXX_HW_RENDER_STATE_T *rs, const MASTER_CL_INFO_T *info)
{
   TILE_OPS_T const* loads = &info->rcfg.tile_ops[RCFG_LOAD];
   TILE_OPS_T const* stores = &info->rcfg.tile_ops[RCFG_STORE];

   // Quick upper bound for CL size.
   unsigned cl_size  = 0
      + (V3D_CL_LOAD_SIZE + V3D_CL_IMPLICIT_TILE_COORDS_SIZE)
      + (V3D_CL_LOAD_GENERAL_SIZE + V3D_CL_IMPLICIT_TILE_COORDS_SIZE + V3D_CL_STORE_GENERAL_SIZE) * loads->num_general_ops
      + (V3D_CL_ENABLE_Z_ONLY_SIZE + V3D_CL_DISABLE_Z_ONLY_SIZE)
      + (V3D_CL_BRANCH_IMPLICIT_TILE_SIZE * rs->num_z_prepass_bins)
      + (V3D_CL_BRANCH_IMPLICIT_TILE_SIZE * rs->fmem.br_info.num_bins)
      + (V3D_CL_IMPLICIT_TILE_COORDS_SIZE + V3D_CL_STORE_GENERAL_SIZE) * stores->num_general_ops
      + V3D_CL_STORE_SUBSAMPLE_SIZE
      + V3D_CL_RETURN_SIZE;

   if (rs->workaround_gfxh_1313)
      cl_size += glxx_workaround_gfxh_1313_size();

   uint8_t* instr = (uint8_t *)khrn_fmem_begin_cle(&rs->fmem, cl_size);
   if (!instr)
      return false;

   // Short form load. This will perform all non-general loads
   unsigned load_short = loads->short_op_mask;
   if (load_short != 0)
   {
      v3d_cl_load(&instr,
         (load_short & RCFG_STENCIL_MASK) != 0,
         (load_short & RCFG_DEPTH_MASK) != 0,
         ~(load_short >> RCFG_COLOR0) & gfx_mask(GLXX_MAX_RENDER_TARGETS)  );
   }

   // General loads.
   for (unsigned i = 0; i != loads->num_general_ops; ++i)
   {
      // If not the first load.
      if (load_short != 0 || i > 0)
      {
         v3d_cl_implicit_tile_coords(&instr);

         // Dummy store general.
         v3d_cl_store_general(&instr,
            V3D_LDST_BUF_NONE,   // buffer
            false,               // raw mode
            true, true, true,    // disable clears
            false,               // eof
            true,                // disable double buffer swap
            V3D_LDST_MEMORY_FORMAT_UIF_NO_XOR,
            0, 0);
      }

      TILE_OP_T const* op = &loads->general_ops[i];
      v3d_cl_load_general(&instr,
         op->buf,
         op->raw,
         v3d_memory_format_to_ldst(khrn_image_plane_translate_memory_format(&op->image_plane)),
         khrn_image_plane_uif_height_in_ub(&op->image_plane),
         khrn_image_plane_lock(&op->image_plane, &rs->fmem, 0, GMEM_SYNC_CORE_READ));
   }

   // Tile coords.
   v3d_cl_implicit_tile_coords(&instr);

   if (rs->workaround_gfxh_1313 && !glxx_workaround_gfxh_1313(&instr, &rs->fmem))
      return false;

   // Play tile-lists.
   if (rs->num_z_prepass_bins)
   {
      v3d_cl_enable_z_only(&instr);
      for (unsigned i = 0; i != rs->num_z_prepass_bins; ++i)
      {
         v3d_cl_branch_implicit_tile(&instr, i);
      }
      v3d_cl_disable_z_only(&instr);
   }
   for (unsigned i = 0; i != rs->fmem.br_info.num_bins; ++i)
   {
      v3d_cl_branch_implicit_tile(&instr, i);
   }

   // General stores
   for (unsigned i = 0; i != stores->num_general_ops; ++i)
   {
      TILE_OP_T const* op = &stores->general_ops[i];
      v3d_cl_store_general(&instr,
         op->buf,                   // buffer
         op->raw,                   // raw mode
         true,                      // disable depth clear
         true,                      // disable stencil clear
         true,                      // disable color clear
         false,                     // eof (never set using flag)
         true,                      // disable double buffer swap
         v3d_memory_format_to_ldst( // memory format
            khrn_image_plane_translate_memory_format(&op->image_plane)
         ),
         khrn_image_plane_uif_height_in_ub(&op->image_plane),
         khrn_image_plane_lock(&op->image_plane, &rs->fmem, 0, GMEM_SYNC_CORE_WRITE)
         );

      v3d_cl_implicit_tile_coords(&instr);
   }

   // Short form store (can be dummy).
   v3d_cl_store_subsample(&instr);
   v3d_cl_return(&instr);

   khrn_fmem_end_cle(&rs->fmem, instr);

   return true;
}

static void master_cl_supertile_range(
   uint32_t *morton_flags, uint32_t *begin_supertile, uint32_t *end_supertile,
   uint32_t num_cores, uint32_t core,
   unsigned int num_supertiles_x, unsigned int num_supertiles_y)
{
   uint32_t num_supertiles = num_supertiles_y * num_supertiles_x;

   *morton_flags = 0;

   if (khrn_options.partition_supertiles_in_sw)
   {
      /* Same supertile order for all cores, but give each core an exclusive
       * subset of the supertiles... */

      uint32_t supertiles_per_core = num_supertiles / num_cores;
      uint32_t extra_supertiles = num_supertiles - (num_cores * supertiles_per_core);

      *begin_supertile = core * supertiles_per_core;
      *end_supertile = (core + 1) * supertiles_per_core;

      /* Distribute extra supertiles */
      *begin_supertile += gfx_umin(core, extra_supertiles);
      *end_supertile += gfx_umin(core + 1, extra_supertiles);

      assert(*begin_supertile <= *end_supertile);
      assert(*end_supertile <= num_supertiles);
   }
   else
   {
      /* All cores are given all supertiles, but in a different order */

      /* TODO Experiment with this */
      switch (khrn_options.all_cores_same_st_order ? 0 :
         /* TODO Quick hack for 8 cores */
         (core & 3))
      {
      case 0:  break;
      case 1:  *morton_flags |= GFX_MORTON_REVERSE; break;
      case 2:  *morton_flags |= GFX_MORTON_FLIP_Y; break;
      case 3:  *morton_flags |= GFX_MORTON_FLIP_Y | GFX_MORTON_REVERSE; break;
      default: not_impl();
      }

      *begin_supertile = 0;
      *end_supertile = num_supertiles;
   }

   /* TODO Is this sensible? */
   if (num_supertiles_x < num_supertiles_y)
   {
      *morton_flags |= GFX_MORTON_TRANSPOSE;
   }
}

void glxx_hw_populate_master_cl_supertiles(
   GLXX_HW_RENDER_STATE_T *rs,
   unsigned num_cores,
   unsigned core,
   uint8_t **instr_ptr,
   unsigned int num_supertiles_x,
   unsigned int num_supertiles_y
   )
{
   uint32_t morton_flags, begin_supertile, end_supertile;
   master_cl_supertile_range(
      &morton_flags, &begin_supertile, &end_supertile,
      num_cores, core,
      num_supertiles_x, num_supertiles_y
      );

   GFX_MORTON_STATE_T morton;
   gfx_morton_init(&morton, num_supertiles_x, num_supertiles_y, morton_flags);

   uint8_t *instr = *instr_ptr; // use local ptr for iteration
   uint32_t x, y;
   for (unsigned i = 0; gfx_morton_next(&morton, &x, &y, NULL); ++i)
   {
      if (i >= begin_supertile && i < end_supertile)
      {
         v3d_cl_supertile_coords(&instr, x, y);
      }
   }
   *instr_ptr = instr;
}

void glxx_hw_destroy_master_cl_info(MASTER_CL_INFO_T *info)
{
   for (unsigned s = 0; s != countof(info->rcfg.tile_ops); ++s)
   {
      TILE_OPS_T* ops = &info->rcfg.tile_ops[s];
      for (unsigned i = 0; i != ops->num_general_ops; ++i)
      {
         deinit_tile_op(&ops->general_ops[i]);
      }
   }

   for (unsigned i = 0; i != countof(info->rcfg.buffers); ++i)
   {
      deinit_buffer_ops(&info->rcfg.buffers[i]);
   }
}
