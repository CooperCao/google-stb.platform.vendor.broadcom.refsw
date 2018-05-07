/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_tmu_blit.h"
#include "glxx_server.h"
#include "glxx_hw.h"
#include "../common/khrn_render_state.h"
#include "libs/core/v3d/v3d_tlb.h"
#include "glxx_ds_to_color.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "glxx_tmu_blit_shaders.h"
#include "libs/util/log/log.h"
#include "libs/util/gfx_util/gfx_util_rect.h"

typedef struct
{
   float x;
   float y;
}fpoint;

typedef struct
{
   float inf[2];
   float sup[2];
}fbox;


// vtx layout
// 2------3
// |      |
// |      |
// 0------1
static void fbox_to_fpoints(const fbox *rect, fpoint vtx[4])
{
   vtx[0].x = rect->inf[0];
   vtx[0].y = rect->inf[1];

   vtx[3].x = rect->sup[0];
   vtx[3].y = rect->sup[1];

   vtx[1].x = vtx[3].x;
   vtx[1].y = vtx[0].y;

   vtx[2].x = vtx[0].x;
   vtx[2].y = vtx[3].y;
}

static void fswap(float *x, float *y)
{
   float temp = *x;
   *x = *y;
   *y = temp;
}

void normalize_fbox(fbox *rect, unsigned width, unsigned height)
{
   unsigned size[2] = {width, height};
   for (unsigned i = 0; i < 2; i++)
   {
      rect->inf[i]/= size[i];
      rect->sup[i]/= size[i];
   }
}

static void gfx_rect_to_fbox(fbox *box, const gfx_rect *rect)
{
   box->inf[0] = (float)rect->x;
   box->inf[1] = (float)rect->y;
   box->sup[0] = (float)(rect->x + rect->width);
   box->sup[1] = (float)(rect->y + rect->height);
}

static void clip_fbox(fbox *src_rect, const fbox *clip,
         const bool flip[2], fbox *dst_rect)
{
   for (unsigned i = 0; i < 2; i++)
   {
      float out_left, out_right, scale;

      out_left = out_right = 0.0f;
      scale = (dst_rect->sup[i] - dst_rect->inf[i]) /
         (src_rect->sup[i] - src_rect->inf[i]);

      if (src_rect->inf[i] < clip->inf[i])
      {
         out_left = (clip->inf[i] - src_rect->inf[i]);
         src_rect->inf[i] = clip->inf[i];
      }
      if (src_rect->sup[i] > clip->sup[i])
      {
         out_right = (src_rect->sup[i] - clip->sup[i]);
         src_rect->sup[i] = clip->sup[i];
      }

      //if flip, swap left with right
      if (flip[i])
         fswap(&out_left, &out_right);

      dst_rect->inf[i] += out_left * scale;
      dst_rect->sup[i] -= out_right * scale;
   }
}

// vtx_pos and tex_coord layout
// 2 -- 3
// |    |
// 0 -- 1
static void pos_and_tex_coords(const gfx_rect blit_rect[2], const bool flip[2],
      const gfx_rect clip[2], fpoint vtx_pos[4], fpoint tex_coord[4])
{
   fbox f_blit_rect[2];
   fbox f_clip[2];
   for (unsigned i = 0; i < 2 ; i++)
   {
      gfx_rect_to_fbox(&f_blit_rect[i], &blit_rect[i]);
      gfx_rect_to_fbox(&f_clip[i], &clip[i]);
   }

   clip_fbox(&f_blit_rect[0], &f_clip[0], flip, &f_blit_rect[1]);
   clip_fbox(&f_blit_rect[1], &f_clip[1], flip, &f_blit_rect[0]);

   normalize_fbox(&f_blit_rect[0], clip[0].width, clip[0].height);
   for (unsigned i = 0; i < 2; i++)
   {
      if (flip[i])
         fswap(&f_blit_rect[0].inf[i], &f_blit_rect[0].sup[i]);
   }

   fbox_to_fpoints(&f_blit_rect[0], tex_coord);
   for (unsigned i = 0; i < 4; i++)
   {
      assert(tex_coord[i].x >= 0.0f && tex_coord[i].x <= 1.0f);
      assert(tex_coord[i].y >= 0.0f && tex_coord[i].y <= 1.0f);
   }
   fbox_to_fpoints(&f_blit_rect[1], vtx_pos);
}

typedef struct
{
   bool is_used;

   bool is_16;
   bool is_int;
   uint8_t config_byte;
}rt_info_t;

//return true if any rt is configured for 32 bit access
static bool color_buffers_rt_info(rt_info_t rt_info[4],
      const GLXX_HW_FRAMEBUFFER_T *dst_hw_fb, uint32_t draw_bufs_mask,
      unsigned components_to_write)
{
   bool any_rt_is_32bit = false;

   for (unsigned i = 0; i < 4; i++)
      rt_info[i].is_used = false;

   for (unsigned i = 0; draw_bufs_mask; i++, draw_bufs_mask >>= 1)
   {
      if (draw_bufs_mask & 0x1)
      {
         rt_info[i].is_used = true;
         v3d_rt_type_t rt_type = dst_hw_fb->color_rt_format[i].type;
         rt_info[i].is_16 = v3d_tlb_rt_type_use_rw_cfg_16(rt_type);
         rt_info[i].is_int = v3d_tlb_rt_type_is_int(rt_type);

         unsigned vec_sz = rt_info[i].is_16 ? (components_to_write + 1)/2 : components_to_write;
         rt_info[i].config_byte = v3d_tlb_config_color(i, rt_info[i].is_16,
               rt_info[i].is_int, vec_sz, /*per_sample*/ true);

         if (!rt_info[i].is_16)
            any_rt_is_32bit = true;
      }
   }
   return any_rt_is_32bit;
}

// vtx_pos and tex_coord layout
// 2 -- 3
// |    |
// 0 -- 1
#define NUM_ATTRS 3
static v3d_addr_t nvshader(khrn_fmem *fmem,
      v3d_addr_t fshader_addr, v3d_addr_t funifs_addr,
      const fpoint vtx_pos[4], const fpoint tex_coord[4])
{
   size_t size = V3D_SHADREC_GL_MAIN_PACKED_SIZE + NUM_ATTRS * V3D_SHADREC_GL_ATTR_PACKED_SIZE;

   v3d_addr_t rec_addr;
   uint32_t *rec_packed = khrn_fmem_data(&rec_addr, fmem, size, V3D_SHADREC_ALIGN);
   if (!rec_packed)
      return 0;

   V3D_SHADREC_GL_MAIN_T rec = {0, };
   V3D_SHADREC_GL_ATTR_T attr[NUM_ATTRS];
   memset(attr, 0, sizeof(attr));

   rec.clipping = 0;
   rec.num_varys = 2;
   //xc, yc, zc, wc
   uint32_t cs_out_sectors = 1;
   // 6 (xs, ys, zs, 1/wc + 2 varyings)
   uint32_t vs_out_sectors = 1;

#if V3D_VER_AT_LEAST(4,1,34,0)
   rec.cs_input_size  = (V3D_IN_SEG_ARGS_T){ /*.sectors = */0, /*.min_req = */1 };
   rec.cs_output_size = (V3D_OUT_SEG_ARGS_T){ cs_out_sectors, 0 };
   rec.vs_input_size  = (V3D_IN_SEG_ARGS_T){ /*.sectors = */0, /*.min_req = */1 };
   rec.vs_output_size = (V3D_OUT_SEG_ARGS_T){ vs_out_sectors, 0 };
#if V3D_VER_AT_LEAST(4,1,34,0)
   rec.fs.single_seg  = false;
#endif
#else
   rec.cs_output_size = cs_out_sectors;
   rec.vs_output_size = vs_out_sectors;
#endif
   rec.fs.threading   = V3D_THREADING_4;
   rec.fs.addr        = fshader_addr;
   rec.fs.unifs_addr  = funifs_addr;

   v3d_addr_t data_addr;
   unsigned vtx_count = 4;
   unsigned num_entries = 6;
   uint32_t *data = khrn_fmem_data(&data_addr, fmem, vtx_count * num_entries * sizeof(float), V3D_ATTR_ALIGN);
   for (unsigned i = 0, pos = 0; i < vtx_count; i++)
   {
      //xs
      data[pos++] = gfx_float_to_uint32(vtx_pos[i].x * (1 << 8));
      //ys
      data[pos++] = gfx_float_to_uint32(vtx_pos[i].y * (1<< 8));
      //zs
      data[pos++] = gfx_float_to_bits(1.0f);
      //1/wcs
      data[pos++] = gfx_float_to_bits(1.0f);

#if V3D_VER_AT_LEAST(4,1,34,0)
      //vary 0 - t coord
      data[pos++] = gfx_float_to_bits(tex_coord[i].y);
      //vary 1 - s coord
      data[pos++] = gfx_float_to_bits(tex_coord[i].x);
#else
      //we need to write first s and then t
      //vary 0 - s coord
      data[pos++] = gfx_float_to_bits(tex_coord[i].x);
      //vary 1 - t coord
      data[pos++] = gfx_float_to_bits(tex_coord[i].y);
#endif
    }

   // xc, yc, zc, wc - clipping is disabled so we just use any 4 values(use the
   // same as xs, ys, zs, 1/wc)
   attr[0].addr = data_addr;
   attr[0].size = 4;
   attr[0].type = V3D_ATTR_TYPE_FLOAT;
   attr[0].cs_num_reads = 4;
   attr[0].vs_num_reads = 0;
   attr[0].stride = 0;
#if V3D_VER_AT_LEAST(4,1,34,0)
   attr[0].max_index = 0;
#endif

   //xs, ys, zs, 1/wc
   attr[1].addr = data_addr;
   attr[1].size = 4;
   attr[1].type = V3D_ATTR_TYPE_FLOAT;
   attr[1].cs_num_reads = 4;
   attr[1].vs_num_reads = 4;
   attr[1].stride = num_entries * sizeof(float);
#if V3D_VER_AT_LEAST(4,1,34,0)
   attr[1].max_index = 3;
#endif

   //vary 0, vary 1 (s, t texture coords)
   attr[2].addr = data_addr + 4 * sizeof(float);
   attr[2].size = 2;
   attr[2].type = V3D_ATTR_TYPE_FLOAT;
   attr[2].cs_num_reads = 0;
   attr[2].vs_num_reads = 2;
   attr[2].stride =  num_entries * sizeof(float);
#if V3D_VER_AT_LEAST(4,1,34,0)
   attr[2].max_index = 3;
#endif

   uint32_t *curr = rec_packed;
   v3d_pack_shadrec_gl_main(curr, &rec);
   curr += V3D_SHADREC_GL_MAIN_PACKED_SIZE / sizeof(*rec_packed);
   for (unsigned i = 0; i < 3; i++)
   {
      v3d_pack_shadrec_gl_attr(curr, &attr[i]);
      curr += V3D_SHADREC_GL_ATTR_PACKED_SIZE / sizeof(*rec_packed);

   }
   return rec_addr;
}

static bool clip_and_cfgbits_instr(glxx_dirty_set_t *dirty,
      uint32_t draw_bufs_mask, const GLXX_HW_FRAMEBUFFER_T *dst_hw_fb,
      const gfx_rect *clip_rect,
      GLXX_HW_RENDER_STATE_T *rs)
{
   unsigned size = V3D_CL_CLIP_SIZE + V3D_CL_CLIPZ_SIZE + V3D_CL_CFG_BITS_SIZE
      + V3D_CL_COLOR_WMASKS_SIZE + V3D_CL_VIEWPORT_OFFSET_SIZE
#if V3D_VER_AT_LEAST(4,1,34,0)
      + V3D_CL_BLEND_ENABLES_SIZE
#endif
      + V3D_CL_SAMPLE_STATE_SIZE;

   if (!rs->clist_start && !(rs->clist_start = khrn_fmem_begin_clist(&rs->fmem)))
      return false;

   // Set up control list
   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, size);
   if (!instr)
      return false;

   //  Emit scissor/clipper/viewport instructions
   v3d_cl_clip(&instr, clip_rect->x, clip_rect->y, clip_rect->width, clip_rect->height);
   v3d_cl_clipz(&instr, 0.0f, 1.0f);
   khrn_render_state_set_add(&dirty->viewport, rs);

   //cfg bits
   early_z_update_cfg(&rs->ez, V3D_COMPARE_FUNC_ALWAYS,
      /*depth_update*/ false, /*stencil_update*/ false,
      V3D_STENCIL_OP_ZERO, V3D_STENCIL_OP_ZERO,
      V3D_STENCIL_OP_ZERO, V3D_STENCIL_OP_ZERO);
   v3d_cl_cfg_bits(&instr, /*front_prims*/ true, /*back_prims*/ true,
      /*cwise_is_front*/ false, /*depth offset*/ false,
      /*aa_lines*/ false, /*rast_oversample*/ V3D_MS_1X,
      /*cov_pipe*/ false, /*cov_update*/ V3D_COV_UPDATE_NONZERO,
      /*wireframe_tris*/ false,
      /*depth_test - zfunc*/ V3D_COMPARE_FUNC_ALWAYS, /*depth_update - enzu*/ false,
      /*cfg_bits_ez*/ false, /* cfg_bits_ez_update */ false,
      /*stencil */ false, /* blend */ false,
      V3D_WIREFRAME_MODE_LINES, /*d3d_prov_vtx*/ false);
   khrn_render_state_set_add(&dirty->cfg, rs);

   /* Disable alpha writes for buffers which don't have alpha channels. We need
    * the alpha in the TLB to be 1 to get correct blending in the case where
    * the buffer doesn't have alpha. The TLB will set alpha to 1 when it loads
    * an alpha-less buffer, but we need to explicitly mask alpha writes after
    * that to prevent it changing. */
   uint32_t w_disable_mask = 0;
   for (unsigned i = 0; draw_bufs_mask; i++, draw_bufs_mask >>= 1)
   {
      if (draw_bufs_mask & 0x1)
      {
         khrn_image *image = dst_hw_fb->color[i].image;
         if (image && !gfx_lfmt_has_alpha(image->api_fmt))
            w_disable_mask |= 1u << ((i * 4) + 3);
      }
   }
   v3d_cl_color_wmasks(&instr, w_disable_mask);
   khrn_render_state_set_add(&dirty->color_write, rs);

#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_cl_viewport_offset(&instr, 0, 0, 0, 0);
#else
   v3d_cl_viewport_offset(&instr, 0, 0);
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
   //no blending
   v3d_cl_blend_enables(&instr, 0);
   khrn_render_state_set_add(&dirty->blend_enables, rs);
#endif

   float sample_coverage = 1.0f; /* Disable */
#if V3D_VER_AT_LEAST(4,1,34,0)
   uint8_t mask = 0xf;     /* Disable */
   v3d_cl_sample_state(&instr, mask, sample_coverage);
#else
   v3d_cl_sample_state(&instr, sample_coverage);
#endif
   khrn_render_state_set_add(&dirty->sample_state, rs);

   khrn_fmem_end_cle(&rs->fmem, instr);
   return true;
}

static bool draw_quad(v3d_addr_t fshader_addr, v3d_addr_t funifs_addr,
      const fpoint vtx_pos[4], const fpoint tex_coord[4],
      GLXX_HW_RENDER_STATE_T *rs)
{
   v3d_addr_t shadrec_addr = nvshader(&rs->fmem, fshader_addr,
         funifs_addr, vtx_pos, tex_coord);
   if (shadrec_addr == 0)
      return false;

   //provoking vertex = last --> indices (0, 1, 2) (1, 3, 2)
   uint8_t indices[] = {0, 1, 2, 1, 3, 2};
   unsigned indices_size = sizeof(indices);
   unsigned num_indices = indices_size/sizeof(indices[0]);

   v3d_addr_t indices_addr;
   uint32_t *indices_ptr = khrn_fmem_data(&indices_addr, &rs->fmem, indices_size,
         V3D_INDICES_REC_ALIGN);
   if (!indices_ptr)
      return false;
   memcpy(indices_ptr, indices, indices_size);

#if V3D_VER_AT_LEAST(4,1,34,0)
   glxx_prim_drawn_by_us_record(rs, 2);
   if (!glxx_tf_record_disable(rs))
      return false;
#endif

   // cache + shader_rec + draw_elems
   unsigned size = V3D_CL_VCM_CACHE_SIZE_SIZE + V3D_CL_GL_SHADER_SIZE +
#if V3D_VER_AT_LEAST(4,1,34,0)
      V3D_CL_INDEX_BUFFER_SETUP_SIZE +
#endif
      V3D_CL_INDEXED_PRIM_LIST_SIZE;

   uint8_t *instr = khrn_fmem_begin_cle(&rs->fmem, size);
   if (!instr)
      return false;
   v3d_cl_vcm_cache_size(&instr, 4, 4);
   v3d_cl_nv_shader(&instr, NUM_ATTRS, shadrec_addr);
#if V3D_VER_AT_LEAST(4,1,34,0)
   //setup index buffer
   v3d_cl_index_buffer_setup(&instr, indices_addr, indices_size);
#endif
   V3D_CL_INDEXED_PRIM_LIST_T indexed_prim_list =
   {
      .prim_mode = V3D_PRIM_MODE_TRIS, .index_type = V3D_INDEX_TYPE_8BIT,
      .num_indices = num_indices, .prim_restart = false,
#if V3D_VER_AT_LEAST(4,1,34,0)
      .indices_offset = 0,
#else
      .indices_addr = indices_addr,
      .max_index = 3,
#endif
   };
   v3d_cl_indexed_prim_list_indirect(&instr, &indexed_prim_list);
   khrn_fmem_end_cle_exact(&rs->fmem, instr);

   return true;
}

/* Create a new image/blob that uses the same resource as the passed in
 * depth/stencil image. The new image will describe only one plane in the
 * original image (if the original image has more than one plane).
 * We also change the fmt of the new image from DS to R8/16/32UI
 */
static khrn_image* to_color_img(const khrn_image *image, unsigned plane)
{
   GFX_LFMT_T img_plane_lfmt = khrn_image_get_lfmt(image, plane);

   assert(gfx_lfmt_has_depth(img_plane_lfmt) || gfx_lfmt_has_stencil(img_plane_lfmt));
   assert(khrn_image_get_num_planes(image) > plane);

   const khrn_blob *blob = image->blob;
   GFX_BUFFER_DESC_T desc[KHRN_MAX_MIP_LEVELS];

   GFX_LFMT_T color_lfmt = glxx_ds_lfmt_to_color(img_plane_lfmt);
   memset(desc, 0, sizeof(desc));
   for (unsigned i = 0; i < blob->num_mip_levels; i++)
   {
      desc[i].width = blob->desc[i].width;
      desc[i].height = blob->desc[i].height;
      desc[i].depth = blob->desc[i].depth;
      desc[i].num_planes = 1;
      desc[i].planes[0] = blob->desc[i].planes[plane];
      /* change lfmt to color; since we wrap the original resource, do it for
       * each mip level */
      assert(gfx_lfmt_fmt(desc[i].planes[0].lfmt) == gfx_lfmt_fmt(img_plane_lfmt));
      desc[i].planes[0].lfmt = gfx_lfmt_set_format(desc[i].planes[0].lfmt, color_lfmt);
   }

   khrn_blob *new_blob = khrn_blob_create_from_resource(blob->res,
         desc, blob->num_mip_levels, blob->num_array_elems,
         blob->array_pitch, blob->usage, blob->secure);

   if (!new_blob)
      return NULL;

   khrn_image *plane_img;
   assert(khrn_image_is_one_elem_slice(image));
   plane_img = khrn_image_create_one_elem_slice(new_blob,
             image->start_elem, image->start_slice, image->level, color_lfmt);

   KHRN_MEM_ASSIGN(new_blob, NULL); /* plane_img has a reference to the new_blob */

   return plane_img;
}

static bool to_color_img_plane(khrn_image_plane *color, const khrn_image_plane *ds)
{
   color->image = to_color_img(ds->image, ds->plane_idx);
   if (!color->image)
      return false;
   color->plane_idx = 0;
   return true;
}

#if V3D_VER_AT_LEAST(4,1,34,0)
static v3d_addr_t sampler_state_addr(khrn_fmem *fmem, GLenum filter)
{
   V3D_TMU_SAMPLER_T s;
   memset(&s, 0, sizeof(V3D_TMU_SAMPLER_T));
   s.wrap_s = s.wrap_t = V3D_TMU_WRAP_CLAMP;
#if V3D_VER_AT_LEAST(4,1,34,0)
   glxx_set_tmu_filters(&s, filter, filter, 1.0f);
#else
   s.filters = glxx_get_tmu_filters(filter, filter, 1.0f);
#endif

   uint8_t hw_sampler[V3D_TMU_SAMPLER_PACKED_SIZE];
   v3d_pack_tmu_sampler(hw_sampler, &s);
   return khrn_fmem_add_tmu_sampler(fmem, hw_sampler, /*sampler_extended*/ false, false);
}

static v3d_addr_t texture_state_addr(khrn_fmem *fmem, const glxx_hw_tex_params *tp)
{
   uint8_t hw_tex_state[V3D_TMU_TEX_STATE_PACKED_SIZE + V3D_TMU_TEX_EXTENSION_PACKED_SIZE];
   bool tex_state_extended = glxx_pack_tex_state(hw_tex_state, tp);
   return khrn_fmem_add_tmu_tex_state(fmem, hw_tex_state, tex_state_extended, false);
}
#else
static v3d_addr_t texture_state_addr(khrn_fmem *fmem, const glxx_hw_tex_params *tp,
#if V3D_VER_AT_LEAST(3,3,0,0)
      bool tmu_output_32bit,
#endif
      GLenum filter)
{
   V3D_TMU_INDIRECT_T ind;
   memset(&ind, 0, sizeof(V3D_TMU_INDIRECT_T));
   ind.filters = glxx_get_tmu_filters(filter, filter, 1.0f);
   ind.base = tp->l0_addr;
   ind.arr_str = tp->arr_str;
   ind.width   = tp->w;
   ind.height  = tp->h;
   ind.depth   = tp->d;
   ind.ttype   = tp->tmu_trans.type;
   ind.srgb    = tp->tmu_trans.srgb;
#if !V3D_VER_AT_LEAST(3,3,0,0)
   if (tp->tmu_trans.shader_swizzle)
   {
      ind.swizzles[0] = V3D_TMU_SWIZZLE_R;
      ind.swizzles[1] = V3D_TMU_SWIZZLE_G;
      ind.swizzles[2] = V3D_TMU_SWIZZLE_B;
      ind.swizzles[3] = V3D_TMU_SWIZZLE_A;
   }
   else
#endif
      memcpy(ind.swizzles, tp->composed_swizzles, sizeof(ind.swizzles));
   ind.flipy     = tp->yflip;
   ind.base_level = tp->base_level;
   assert(tp->base_level < V3D_MAX_MIP_COUNT);
   assert(tp->base_level == tp->max_level);
   ind.min_lod = tp->base_level << V3D_TMU_F_BITS;
   ind.max_lod = ind.min_lod;
#if V3D_VER_AT_LEAST(3,3,0,0)
   ind.output_32 = tmu_output_32bit;
#endif
   ind.ub_pad = tp->uif_cfg.ub_pads[0];
   ind.ub_xor = tp->uif_cfg.ub_xor;
   ind.uif_top = tp->uif_cfg.force;
   ind.xor_dis = tp->uif_cfg.xor_dis;

   uint32_t hw_indirect[V3D_TMU_INDIRECT_PACKED_SIZE / sizeof(uint32_t)];
   v3d_pack_tmu_indirect(hw_indirect, &ind);
   return khrn_fmem_add_tmu_indirect(fmem, hw_indirect);
}
#endif

/* converts img to an image we can texture from */
static bool texture_view(glxx_texture_view *tex_view,
      khrn_image *img, unsigned plane, bool ds_to_color,
      glxx_context_fences *fences)
{
   khrn_image *tex_img = glxx_get_image_for_texturing(img, fences);
   if (!tex_img)
      return false;

   assert(khrn_image_get_depth(tex_img) * khrn_image_get_num_elems(tex_img) == 1);

   tex_view->img_base = tex_img;
   tex_view->plane = plane;
   tex_view->num_levels = 1;

   tex_view->dims = 2;
   tex_view->cubemap_mode = false;

   tex_view->fmt = gfx_lfmt_fmt(khrn_image_get_lfmt(tex_img, plane));
   if (ds_to_color)
      tex_view->fmt = glxx_ds_lfmt_to_color(tex_view->fmt);

   tex_view->need_depth_type = false;
   //swizzle RGBA
   for (unsigned i = 0; i < 4; i++)
      tex_view->swizzle[i] = 2 + i;

   return true;
}

static uint32_t get_words_enable(const glxx_hw_tex_params *tp,
#if V3D_VER_AT_LEAST(3,3,0,0)
      bool tmu_output_32bit,
#endif
      unsigned required_components)
{
   unsigned words = 0;
#if V3D_VER_AT_LEAST(3,3,0,0)
   words = tmu_output_32bit ? required_components : (required_components + 1)/2;
#else
   if (!tp->tmu_trans.shader_swizzle)
   {
      bool out_32bit = v3d_tmu_output_32(tp->tmu_trans.type, /*shadow=*/false);
      words = out_32bit ? required_components : (required_components + 1)/2;
   }
   else
   {

#define SWIZZLES_EQ(R, G, B, A) (                 \
      tp->composed_swizzles[0] == V3D_TMU_SWIZZLE_##R && \
      tp->composed_swizzles[1] == V3D_TMU_SWIZZLE_##G && \
      tp->composed_swizzles[2] == V3D_TMU_SWIZZLE_##B && \
      tp->composed_swizzles[3] == V3D_TMU_SWIZZLE_##A)
      unsigned read_components = 0;
      switch(required_components)
      {
      case 4:
         if (SWIZZLES_EQ(R,0,0,1))
            read_components = 1;
         else if(SWIZZLES_EQ(R,G,0,1))
            read_components = 2;
         else
         {
            assert(SWIZZLES_EQ(R,G,B,A));
            read_components = 4;
         }
         break;
      case 1:
         assert(SWIZZLES_EQ(R,0,0,1));
         read_components = 1;
         break;
      default:
         unreachable();
      }
#undef  SWIZZLES_EQ

      switch (tp->tmu_trans.ret)
      {
      case GFX_LFMT_TMU_RET_8:
      case GFX_LFMT_TMU_RET_1010102:
         words = 1;
         break;
      case GFX_LFMT_TMU_RET_16:
         words = (read_components + 1)/2;
         break;
      case GFX_LFMT_TMU_RET_32:
         words = read_components;
         break;
      default:
         unreachable();
      }
   }
#endif

   uint32_t words_en = 0;
   for (unsigned i = 0; i < words; i++)
      words_en |= 1 << i;
   return words_en;
}

static bool get_tmu_config(uint32_t tmu_config[2],
      const glxx_hw_tex_params *tp,
      GLenum filter,
#if V3D_VER_AT_LEAST(3,3,0,0)
      bool tmu_output_32bit,
#endif
      unsigned required_components, GLXX_HW_RENDER_STATE_T *rs)
{
   uint32_t words_en = get_words_enable(tp,
#if V3D_VER_AT_LEAST(3,3,0,0)
         tmu_output_32bit,
#endif
         required_components);

#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_addr_t tex_state_addr = texture_state_addr(&rs->fmem, tp);
   if (!tex_state_addr)
      return false;
   tmu_config[0] = tex_state_addr | words_en;

   V3D_TMU_PARAM1_T p1;
   p1.sampler_addr = sampler_state_addr(&rs->fmem, filter);
   if (!p1.sampler_addr)
      return false;
   p1.pix_mask = true,
   p1.output_32 = tmu_output_32bit;
   p1.unnorm = false;
   tmu_config[1] = v3d_pack_tmu_param1(&p1);
#else
   V3D_TMU_PARAM0_CFG1_T p0;
   memset(&p0, 0, sizeof(p0));
   p0.ltype = V3D_TMU_LTYPE_2D;
   p0.pix_mask = 1;
   p0.wrap_s = p0.wrap_t = p0.wrap_r = V3D_TMU_WRAP_CLAMP;
   tmu_config[0] = v3d_pack_tmu_param0_cfg1(&p0);

   /* on 3.3 hw, OVRTMUOUT(in MISCCFG Register) is set to 1, so field
    * output_type from texture state is used to specify the type/precision of
    * the return data; in our case, output_type was set to requested
    * tmu_output_32bit in glxx_texture_key_and_uniforms */
   /* on 3.2 hw, we cannot ask for a certain tmu return precision,
    * (texture_state output_type is set to AUTO)
    */
   v3d_addr_t tex_state_addr = texture_state_addr(&rs->fmem, tp,
#if V3D_VER_AT_LEAST(3,3,0,0)
         tmu_output_32bit,
#endif
         filter);
   if (!tex_state_addr)
      return false;
   tmu_config[1] = tex_state_addr | words_en;
#endif
   return true;
}

/* the image must be usable as texture */
static bool blit_color(glxx_dirty_set_t *dirty,
      const glxx_texture_view *tex_view, uint32_t draw_bufs_mask,
      GLXX_HW_FRAMEBUFFER_T *dst_hw_fb,
      const uint32_t *mask_comp_word, GLenum filter,
      const fpoint vtx_pos[4], const fpoint tex_coord[4], const gfx_rect *clip_rect,
      GLXX_HW_RENDER_STATE_T *rs)
{
   unsigned required_components = mask_comp_word ? 1 : 4;

   rt_info_t rt_info[4];
   bool any_rt_is_32bit = color_buffers_rt_info(rt_info, dst_hw_fb,
         draw_bufs_mask, required_components);

   /* if not texture filterable, always use GL_NEAREST */
   if (!glxx_is_texture_filterable_api_fmt(tex_view->img_base->api_fmt))
      filter = GL_NEAREST;

   /* set src image as a 2D texture and get tmu config uniforms + record
    * reading from image in rs; */
   if (!khrn_fmem_sync_res(&rs->fmem, khrn_image_get_resource(tex_view->img_base),
            0, V3D_BARRIER_TMU_DATA_READ))
      return false;

   glxx_hw_tex_params tp;
   glxx_get_hw_tex_params(&tp, tex_view);

   /* request tmu output type to be the highest precisions needed for all rt types;
    * on hw 3.2 tmu we cannot ask for a certain tmu output precision,
    * return type varies with different texture types
    */
   uint32_t tmu_config[2];
   if (!get_tmu_config(tmu_config, &tp, filter,
#if V3D_VER_AT_LEAST(3,3,0,0)
         any_rt_is_32bit,
#endif
         required_components, rs))
      return false;

   if (!clip_and_cfgbits_instr(dirty, draw_bufs_mask, dst_hw_fb, clip_rect, rs))
      return false;

   for (unsigned i = 0; i < 4; i++)
   {
      if (rt_info[i].is_used)
      {
         v3d_addr_t fshader_addr, funifs_addr;
         uint32_t *fshader_ptr = khrn_fmem_data(&fshader_addr, &rs->fmem,
               V3D_TMU_BLIT_SHADER_MAX_SIZE, V3D_QPU_INSTR_ALIGN);
         uint32_t *funifs_ptr = khrn_fmem_data(&funifs_addr, &rs->fmem,
               V3D_TMU_BLIT_SHADER_MAX_UNIFS_SIZE, V3D_QPU_UNIFS_ALIGN);
         if (!fshader_ptr || !funifs_ptr)
            return false;

         if (mask_comp_word)
         {
            //assert we have only only red channel, uint
            assert(gfx_lfmt_get_type(&tp.tex_fmt)== GFX_LFMT_TYPE_UINT);
            assert(gfx_lfmt_present_channels(tp.tex_fmt) == GFX_LFMT_CHAN_R_BIT);
            glxx_blit_fshader_and_unifs_red_uint_with_mask(fshader_ptr, funifs_ptr,
                  *mask_comp_word, tmu_config, rt_info[i].config_byte);
         }
         else
         {
             bool tmu_output_32bit = any_rt_is_32bit;
#if !V3D_VER_AT_LEAST(3,3,0,0)
            tmu_output_32bit = v3d_tmu_output_32(tp.tmu_trans.type, /*shadow=*/false);

            if (tp.tmu_trans.shader_swizzle)
            {
               assert(!rt_info[i].is_16);
               glxx_blit_int_fshader_and_unifs(fshader_ptr, funifs_ptr,
                     gfx_lfmt_get_type(&tp.tex_fmt), tp.tmu_trans.ret, tp.composed_swizzles,
                     tmu_config, rt_info[i].config_byte);
            }
            else
#endif
            {
               glxx_blit_fshader_and_unifs(fshader_ptr, funifs_ptr,
                     gfx_lfmt_get_type(&tp.tex_fmt), tmu_output_32bit, !rt_info[i].is_16,
                     tmu_config, rt_info[i].config_byte);
            }
         }

         if (!draw_quad(fshader_addr, funifs_addr, vtx_pos, tex_coord, rs))
            return false;

         rs->has_rasterization = true;
         glxx_bufstate_rw(&rs->color_buffer_state[i]);
      }
   }
   return true;
}

static bool make_color_hw_fb_from_ds(GLXX_HW_FRAMEBUFFER_T *hw_fb,
      uint32_t mask_word[2], unsigned *num_targets,
      bool use_depth, const khrn_image_plane *depth,
      bool use_stencil, const khrn_image_plane *stencil)
{
   assert(use_depth || use_stencil);

   memset(hw_fb, 0, sizeof(GLXX_HW_FRAMEBUFFER_T));

   *num_targets = 0;
   if (use_depth)
   {
      if (!to_color_img_plane(&hw_fb->color[0], depth))
         return false;
      GFX_LFMT_T lfmt_d = khrn_image_get_lfmt(depth->image, depth->plane_idx);
      mask_word[0] = glxx_ds_color_lfmt_get_depth_mask(lfmt_d);
      hw_fb->rt_count = 1;
      (*num_targets)++;
      if (use_stencil && depth->plane_idx == stencil->plane_idx)
      {
         // packed depth stencil
         assert(depth->image->api_fmt == stencil->image->api_fmt);
         GFX_LFMT_T lfmt_s = khrn_image_get_lfmt(stencil->image, stencil->plane_idx);
         assert(lfmt_s == lfmt_d);
         mask_word[0] |= glxx_ds_color_lfmt_get_stencil_mask(lfmt_s);
         use_stencil = false;
      }
   }

   if (use_stencil)
   {
      if (!to_color_img_plane(&hw_fb->color[1], stencil))
      {
         KHRN_MEM_ASSIGN(hw_fb->color[0].image, NULL);
         return false;
      }
      GFX_LFMT_T lfmt = khrn_image_get_lfmt(stencil->image, stencil->plane_idx);
      mask_word[1] = glxx_ds_color_lfmt_get_stencil_mask(lfmt);
      hw_fb->rt_count = 2;
      (*num_targets)++;
   }
   assert(hw_fb->rt_count >= 1);

   // complete info about hw_fb
   hw_fb->width = hw_fb->height = hw_fb->layers = UINT32_MAX;
   for (unsigned i = 0; i < hw_fb->rt_count; i++)
   {
      const khrn_image_plane *image_plane = &hw_fb->color[i];
      if (image_plane->image)
      {
         unsigned w = khrn_image_get_width(image_plane->image);
         unsigned h = khrn_image_get_height(image_plane->image);
         unsigned layers = khrn_image_get_depth(image_plane->image) *
            khrn_image_get_num_elems(image_plane->image);
         assert(layers == 1);
         hw_fb->width = gfx_umin(hw_fb->width, w);
         hw_fb->height = gfx_umin(hw_fb->height, h);
         hw_fb->layers = gfx_umin(hw_fb->layers, layers);
         gfx_lfmt_translate_rt_format(&hw_fb->color_rt_format[i],
               khrn_image_plane_lfmt(image_plane));
      }
      else
      {
         hw_fb->color_rt_format[i].bpp = V3D_RT_BPP_32;
         hw_fb->color_rt_format[i].type = V3D_RT_TYPE_8;
#if V3D_VER_AT_LEAST(4,1,34,0)
         hw_fb->color_rt_format[i].clamp = V3D_RT_CLAMP_NONE;
#endif
      }
   }
   return true;
}

/*
 * if we have packed depth stencil, and mask is DEPTH | STENCIL, we have one
 * tex_view at index 1; otherwise, depth tex_view is at index 1 and stencil
 * tex_view is at index 2 */
static bool make_texture_views(glxx_texture_view tex_view[3], bool *ms,
      const GLXX_FRAMEBUFFER_T *fb, GLbitfield mask, glxx_context_fences *fences)
{
   bool res;

   for (unsigned i = 0; i < 3; i++)
      tex_view[i].img_base = NULL;

   if (mask & GL_COLOR_BUFFER_BIT)
   {
      khrn_image *img;
      if(!glxx_fb_acquire_read_image(fb, GLXX_PREFER_MULTISAMPLED, &img, ms))
         return false;
      res = texture_view(&tex_view[0], img, 0, false, fences);
      KHRN_MEM_ASSIGN(img, NULL);
      if (!res)
         return false;
   }

   if (mask & GL_DEPTH_BUFFER_BIT)
   {
      khrn_image *img;
      if (!glxx_attachment_acquire_image(&fb->attachment[GLXX_DEPTH_ATT],
               GLXX_PREFER_MULTISAMPLED, /*use_0_if_layered */ true, fences,
               &img, ms))
         goto fail;
      res = texture_view(&tex_view[1], img, 0, true, fences);
      KHRN_MEM_ASSIGN(img, NULL);
      if (!res)
         goto fail;
   }

   if (mask & GL_STENCIL_BUFFER_BIT)
   {
      khrn_image *img;
      if (!glxx_attachment_acquire_image(&fb->attachment[GLXX_STENCIL_ATT],
               GLXX_PREFER_MULTISAMPLED, /*use_0_if_layered */ true, fences,
               &img, ms))
         goto fail;

      // stencil is always in the last plane
      unsigned stencil_plane_idx = khrn_image_get_num_planes(img) - 1;
      /* if packed depth stencil, we  use the tex_view[1], so nothing to do here */
      if (tex_view[1].img_base && stencil_plane_idx == 0)
         res = true;
      else
         res = texture_view(&tex_view[2], img, stencil_plane_idx, true, fences);
      KHRN_MEM_ASSIGN(img, NULL);
      if (!res)
         goto fail;
   }
   return true;

fail:
   for (unsigned i = 0; i < 3; i++)
      KHRN_MEM_ASSIGN(tex_view[i].img_base, NULL);
   return false;
}

static void calc_dimensions(unsigned dim[2],
      const glxx_texture_view tex_view[3], bool ms)
{
   dim[0]= UINT_MAX;
   dim[1]= UINT_MAX;
   for (unsigned i = 0; i < 3; i++)
   {
      if (tex_view[i].img_base)
      {
         unsigned w = khrn_image_get_width(tex_view[i].img_base);
         unsigned h = khrn_image_get_height(tex_view[i].img_base);
         if (ms)
         {
            w = w/2;
            h = h/2;
         }
         dim[0] = gfx_umin(dim[0], w);
         dim[1] = gfx_umin(dim[1], h);
      }
   }
}

/* Format conversion:
   - integer formats can only be converted to integer formats with the same signedness
   - fixed point or floating point formats can be converted to fixed point or floating point
   - for color buffers, pixel groups are converted to match the destination format.
     However, colors are clamped only if all draw color buffers have fixed-point components.
   - source and destination depth and stencil formats must match if mask includes DEPTH or STENCIL
*/
static bool tmu_blit(GLXX_SERVER_STATE_T *state,
      const gfx_rect blit_rect[2], const bool flip[2],
      GLbitfield mask, GLenum filter)
{
   GLXX_HW_FRAMEBUFFER_T dst_hw_fb;
   const GLXX_FRAMEBUFFER_T *dst_fb = state->bound_draw_framebuffer;
   if (!glxx_init_hw_framebuffer_just_layer0(dst_fb, &dst_hw_fb,
            &state->fences))
      return false;

   bool ms_src;
   glxx_texture_view tex_view[3];
   if (!make_texture_views(tex_view, &ms_src, state->bound_read_framebuffer,
            mask, &state->fences))
   {
      glxx_destroy_hw_framebuffer(&dst_hw_fb);
      return false;
   }

   unsigned dim_src[2];
   calc_dimensions(dim_src, tex_view, ms_src);

   bool res = false;
   gfx_rect clip_rect[2];
   clip_rect[0] = (gfx_rect){0, 0, dim_src[0], dim_src[1]};
   clip_rect[1] = (gfx_rect){0, 0, dst_hw_fb.width, dst_hw_fb.height};

   if (state->caps.scissor_test)
   {
      gfx_rect_intersect(&clip_rect[1], &state->scissor);
      if(!clip_rect[1].width || !clip_rect[1].height)
      {
         //noting to do
         res = true;
         goto out;
      }
   }

   fpoint tex_coord[4], vtx_pos[4];
   pos_and_tex_coords(blit_rect, flip, clip_rect, vtx_pos, tex_coord);

   //blit color to draw buffers
   if (tex_view[0].img_base)
   {
      if (ms_src)
         filter = GL_LINEAR;

      bool existing;
      GLXX_HW_RENDER_STATE_T *rs = glxx_install_rs(state, &dst_hw_fb, &existing, false);
      if (!rs)
         goto out;

      khrn_render_state_disallow_flush((khrn_render_state*)rs);

      uint32_t draw_bufs_mask = glxx_fb_get_valid_draw_buf_mask(dst_fb);
      res = blit_color(&state->dirty, &tex_view[0], draw_bufs_mask, &dst_hw_fb,
               NULL, filter, vtx_pos, tex_coord, &clip_rect[1], rs);
      if (!res)
      {
         glxx_hw_discard_frame(rs);
         goto out;
      }

      khrn_render_state_allow_flush((khrn_render_state*)rs);
   }

   //blit depth and/or stencil
   if (tex_view[1].img_base || tex_view[2].img_base)
   {
      /* we've already checked that src and dst formats match */
      //create a dst fb using depth/stencil buffers pretending to be color buffers
      GLXX_HW_FRAMEBUFFER_T ds_dst_hw_fb;
      uint32_t mask_rw[2];
      unsigned num_rts;
      bool use_depth = mask & GL_DEPTH_BUFFER_BIT;
      bool use_stencil = mask & GL_STENCIL_BUFFER_BIT;
      res = make_color_hw_fb_from_ds(&ds_dst_hw_fb, mask_rw, &num_rts,
            use_depth, &dst_hw_fb.depth, use_stencil, &dst_hw_fb.stencil);
      if (!res)
         goto out;

      bool existing;
      GLXX_HW_RENDER_STATE_T *rs = glxx_install_rs(state, &ds_dst_hw_fb, &existing, false);
      if (!rs)
      {
         res = false;
         goto inner;
      }
      khrn_render_state_disallow_flush((khrn_render_state*)rs);

      assert(ds_dst_hw_fb.rt_count >= 1);
      for (unsigned rt = 0; rt < 2; rt++)
      {
         if (ds_dst_hw_fb.color[rt].image == NULL)
            continue;

         assert(ds_dst_hw_fb.color[rt].plane_idx == 0);

         //used src and dst fmts are equal
         assert(gfx_lfmt_fmt(khrn_image_get_lfmt(ds_dst_hw_fb.color[rt].image, 0)) ==
                  tex_view[1 + rt].fmt);

         uint32_t draw_bufs_mask = 1 << rt;
         res = blit_color(&state->dirty, &tex_view[1 + rt], draw_bufs_mask, &ds_dst_hw_fb,
               &mask_rw[rt], GL_NEAREST,
               vtx_pos, tex_coord, &clip_rect[1], rs);
         if (!res)
         {
            glxx_hw_discard_frame(rs);
            goto inner;
         }
      }
      khrn_render_state_allow_flush((khrn_render_state*)rs);
inner:
      glxx_destroy_hw_framebuffer(&ds_dst_hw_fb);
      if (!res)
         goto out;
   }

   res = true;
out:
   for (unsigned i = 0; i < 3; i++)
      KHRN_MEM_ASSIGN(tex_view[i].img_base, NULL);
   glxx_destroy_hw_framebuffer(&dst_hw_fb);
   return res;
}

static void points_to_rect(int x0, int x1, int y0, int y1, gfx_rect *rect)
{
   rect->x = gfx_smin(x0, x1);
   rect->y = gfx_smin(y0, y1);
   rect->width = gfx_smax(x0, x1) - rect->x;
   rect->height = gfx_smax(y0, y1) - rect->y;
}

void glxx_tmu_blit_framebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
      GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
      GLbitfield mask, GLenum filter)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state)
      return;

   gfx_rect blit_rect[2];
   points_to_rect(srcX0, srcX1, srcY0, srcY1, &blit_rect[0]);
   points_to_rect(dstX0, dstX1, dstY0, dstY1, &blit_rect[1]);

   bool flip[2];
   flip[0] = srcX0 > srcX1 ? true : false;
   if (dstX0 > dstX1)
      flip[0] = !flip[0];
   flip[1] = srcY0 > srcY1 ? true : false;
   if (dstY0 > dstY1)
      flip[1] = !flip[1];

   if (!tmu_blit(state, blit_rect, flip, mask, filter))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

   glxx_unlock_server_state();
}
