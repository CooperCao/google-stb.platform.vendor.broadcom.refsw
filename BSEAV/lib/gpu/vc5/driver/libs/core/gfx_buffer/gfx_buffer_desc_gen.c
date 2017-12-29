/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gfx_buffer_desc_gen.h"

#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/v3d/v3d_limits.h"
#include "gfx_buffer_uif_config.h"

static void check_usage(gfx_buffer_usage_t usage)
{
   assert(!(usage & ~GFX_BUFFER_USAGE_ALL));

   if (usage & GFX_BUFFER_USAGE_V3D_CUBEMAP)
      assert(usage & GFX_BUFFER_USAGE_V3D_TEXTURE);

#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (usage & GFX_BUFFER_USAGE_V3D_TLB_RAW)
      assert(usage & GFX_BUFFER_USAGE_V3D_TLB);
#endif
}

size_t gfx_buffer_sprint_usage(char *buf, size_t buf_size, size_t offset,
   gfx_buffer_usage_t usage)
{
   check_usage(usage);

   if (!usage)
      return vcos_safe_sprintf(buf, buf_size, offset, "0");

   #define HANDLE(X)                                                 \
      if (usage & GFX_BUFFER_USAGE_##X)                              \
      {                                                              \
         usage &= ~GFX_BUFFER_USAGE_##X;                             \
         offset = vcos_safe_sprintf(buf, buf_size, offset, "%s%s",   \
            #X, usage ? " | " : "");                                 \
      }
   HANDLE(V3D_TEXTURE)
   HANDLE(V3D_CUBEMAP)
   HANDLE(V3D_RENDER_TARGET)
   HANDLE(V3D_DEPTH_STENCIL)
#if !V3D_VER_AT_LEAST(4,0,2,0)
   HANDLE(V3D_TLB_RAW)
#endif
   HANDLE(YFLIP_IF_POSSIBLE)
   #undef HANDLE

   assert(!usage);

   assert(offset > 0); /* We return early (above) when no usage bits are set */
   return offset;
}

const char *gfx_buffer_desc_usage(gfx_buffer_usage_t usage)
{
   static GFX_BUFFER_SPRINT_USAGE(desc, usage);
   return desc;
}

/* This will *not* choose between UIF and UIF_XOR -- that needs to be handled
 * by the caller */
static GFX_LFMT_SWIZZLING_T choose_swizzling_for_tex(
   const GFX_LFMT_BASE_DETAIL_T *bd,
   uint32_t w_in_blocks, uint32_t h_in_blocks)
{
   assert(gfx_lfmt_have_ut_dims(bd));
   if ((w_in_blocks <= bd->ut_w_in_blocks_2d) || (h_in_blocks <= bd->ut_h_in_blocks_2d))
      return GFX_LFMT_SWIZZLING_LT;
   if (w_in_blocks <= (2 * gfx_lfmt_ub_w_in_blocks_2d(bd, GFX_LFMT_SWIZZLING_UBLINEAR)))
      return GFX_LFMT_SWIZZLING_UBLINEAR;
   return GFX_LFMT_SWIZZLING_UIF;
}

#define DDR_PAGE_IN_UB_ROWS (GFX_UIF_PAGE_SIZE / (GFX_UIF_COL_W_IN_UB * GFX_UIF_UB_SIZE))
#define DDR_PAGE_15_IN_UB_ROWS ((DDR_PAGE_IN_UB_ROWS * 3) / 2)
/* How many UIF-block rows the "page cache" covers */
#define PC_IN_UB_ROWS (GFX_UIF_NUM_BANKS * DDR_PAGE_IN_UB_ROWS)
#define PCM15_IN_UB_ROWS (PC_IN_UB_ROWS - DDR_PAGE_15_IN_UB_ROWS)

static uint32_t choose_ub_pad(uint32_t height_in_ub)
{
   /* Either (a) pad such that bank conflicts are at least half a page apart in
    * image space, eg:
    *
    * +-----+-----+
    * |     |  2  |
    * |  0  +-----+
    * |     |     |
    * +-----+  3  | -+
    * |     |     |  +- Conflicting 0s are half a page apart
    * |  1  +-----+ -+
    * |     |     |
    * +-----+  0  |
    * |  2  |     |
    * +-----+-----+
    *
    * or (b) pad to a multiple of the page cache size, which will trigger the
    * XOR rule to avoid bank conflicts.
    *
    * There isn't really any point in padding if there's only one column, but
    * it doesn't seem worth complicating this logic with such a test:
    * - It only matters for the level 1 mipmap -- the level 0 mipmap padding
    *   doesn't need to be allocated in software in the one-column case, and
    *   levels 2 and above are padded up to a power-of-2 anyway.
    * - Only very tall and narrow images will be unnecessarily padded. Such
    *   images are rare. */

   const uint32_t pcm = height_in_ub % PC_IN_UB_ROWS;
   if (pcm == 0)
      /* (b) Already an exact multiple of the page cache size. XOR rule will be
       * triggered. */
      return 0;
   else if (pcm < DDR_PAGE_15_IN_UB_ROWS)
   {
      /* Just over (n * page cache size) */
      if (height_in_ub < PC_IN_UB_ROWS)
         /* (a) n = 0 special case: assuming at least 4 banks, page cache
          * covers more than two columns, so even without padding there will be
          * no close bank conflicts */
         return 0;
      else
         /* (a) Pad just enough to keep a half-page distance between
          * conflicting pages */
         return DDR_PAGE_15_IN_UB_ROWS - pcm;
   }
   else if (pcm > PCM15_IN_UB_ROWS)
      /* (b) Just under (n * page cache size). Pad up to an exact multiple.
       * This will trigger the XOR rule. */
      return PC_IN_UB_ROWS - pcm;
   else
      /* (a) Somewhere in the middle. Should be no close bank conflicts, so
       * leave as is. */
      return 0;
}

static void adjust_lfmt_and_padding(
   GFX_LFMT_T *lfmt, const GFX_LFMT_BASE_DETAIL_T *bd,
   uint32_t *padded_w_in_blocks, uint32_t *padded_h_in_blocks, uint32_t *padded_d_in_blocks,
   uint32_t height, gfx_buffer_usage_t usage, const struct gfx_buffer_ml_cfg *ml_cfg, uint32_t plane) /* ml_cfg must be non-NULL */
{
   GFX_LFMT_SWIZZLING_T provided_swizzling = gfx_lfmt_get_swizzling(lfmt);
   bool swizzling_provided = provided_swizzling != GFX_LFMT_SWIZZLING_NONE;

   /* Figure out what swizzling to use */
   GFX_LFMT_SWIZZLING_T swizzling;
   if (ml_cfg->uif.force)
   {
      swizzling = GFX_LFMT_SWIZZLING_UIF;
      if (ml_cfg->uif.ub_xor)
         swizzling = gfx_lfmt_to_uif_xor_family(swizzling);
      if (ml_cfg->uif.ub_noutile)
         swizzling = gfx_lfmt_to_uif_noutile_family(swizzling);
   }
#if !V3D_VER_AT_LEAST(4,0,2,0)
   else if (usage & GFX_BUFFER_USAGE_V3D_TLB_RAW)
      swizzling = GFX_LFMT_SWIZZLING_UIF;
#endif
   else if (usage & GFX_BUFFER_USAGE_V3D_TEXTURE)
   {
      switch (gfx_lfmt_get_dims(lfmt))
      {
      case GFX_LFMT_DIMS_1D:  swizzling = GFX_LFMT_SWIZZLING_RSO; break;
      case GFX_LFMT_DIMS_2D:
      case GFX_LFMT_DIMS_3D:  swizzling = choose_swizzling_for_tex(bd, *padded_w_in_blocks, *padded_h_in_blocks); break;
      default: unreachable();
      }
   }
   else if (usage & GFX_BUFFER_USAGE_V3D_TLB)
   {
      if (swizzling_provided)
      {
         swizzling = provided_swizzling;
         switch (swizzling)
         {
         case GFX_LFMT_SWIZZLING_RSO:
         case GFX_LFMT_SWIZZLING_LT:
         case GFX_LFMT_SWIZZLING_UIF:
         case GFX_LFMT_SWIZZLING_UIF_XOR:
         case GFX_LFMT_SWIZZLING_UBLINEAR:
            break;
         default:
            assert(0);
         }
      }
      else
      {
         switch (gfx_lfmt_get_dims(lfmt))
         {
         case GFX_LFMT_DIMS_1D:  swizzling = GFX_LFMT_SWIZZLING_RSO; break;
         case GFX_LFMT_DIMS_2D:
         case GFX_LFMT_DIMS_3D:  swizzling = choose_swizzling_for_tex(bd, *padded_w_in_blocks, *padded_h_in_blocks); break; /* Use texture rules */
         default: unreachable();
         }
      }
   }
   else if (swizzling_provided)
      swizzling = provided_swizzling;
   else
      swizzling = GFX_LFMT_SWIZZLING_RSO;

   /* Figure out padded dims & adjust UIF XOR-ness */
   if (gfx_lfmt_is_uif_family((GFX_LFMT_T)swizzling))
   {
      *padded_w_in_blocks = gfx_uround_up_p2(*padded_w_in_blocks, gfx_lfmt_ucol_w_in_blocks_2d(bd, swizzling));
      bool single_col = *padded_w_in_blocks <= gfx_lfmt_ucol_w_in_blocks_2d(bd, swizzling);

      uint32_t padded_height_in_ub = gfx_udiv_round_up(*padded_h_in_blocks, gfx_lfmt_ub_h_in_blocks_2d(bd, swizzling));
      if (ml_cfg->uif.force)
         padded_height_in_ub += ml_cfg->uif.ub_pads[plane];
      else if (!gfx_lfmt_is_uif_xor_family((GFX_LFMT_T)swizzling))
         padded_height_in_ub += choose_ub_pad(padded_height_in_ub);
      else if (!single_col)
         /* Pad to a multiple of the page cache size */
         padded_height_in_ub = gfx_uround_up_p2(padded_height_in_ub, PC_IN_UB_ROWS);
      *padded_h_in_blocks = padded_height_in_ub * gfx_lfmt_ub_h_in_blocks_2d(bd, swizzling);

      bool xor_ok = !ml_cfg->uif.xor_dis &&
         /* Always ok if there is only one column -- XOR mode only affects odd
          * columns */
         (single_col ||
         /* XOR mode should not be enabled unless padded height is a multiple
          * of the page cache size, as block addresses might otherwise end up
          * outside of the buffer */
         ((padded_height_in_ub % PC_IN_UB_ROWS) == 0));
      if (gfx_lfmt_is_uif_xor_family((GFX_LFMT_T)swizzling))
         assert(xor_ok);
      else if (!ml_cfg->uif.force && xor_ok && !single_col)
         /* Activate XOR mode if possible. But only if we're wider than a
          * single UIF column -- XOR mode only affects odd columns, so it's
          * pointless to enable unless there's more than one column. */
         swizzling = gfx_lfmt_to_uif_xor_family(swizzling);
   }
   else if (gfx_lfmt_is_sand_family((GFX_LFMT_T)swizzling))
      *padded_w_in_blocks = gfx_uround_up_p2(*padded_w_in_blocks, gfx_lfmt_sandcol_w_in_blocks_2d(bd, swizzling));
   else switch (swizzling)
   {
   case GFX_LFMT_SWIZZLING_RSO:
      if (usage & GFX_BUFFER_USAGE_V3D_TEXTURE)
         *padded_w_in_blocks = gfx_uround_up_p2(*padded_w_in_blocks, bd->ut_w_in_blocks_1d);

      if(usage & GFX_BUFFER_USAGE_V3D_TLB)
      {
         switch(gfx_lfmt_get_base(lfmt))
         {
            case GFX_LFMT_BASE_BSTC:
            case GFX_LFMT_BASE_BSTCYFLIP:
               /* TLB reads BSTC buffers in blocks of 8x8 pixels (2x2 blocks) */
               *padded_w_in_blocks = gfx_uround_up(*padded_w_in_blocks, 2);
               *padded_h_in_blocks = gfx_uround_up(*padded_h_in_blocks, 2);
               break;

            default:
               break;
         }
      }
      break;
   case GFX_LFMT_SWIZZLING_LT:
      *padded_w_in_blocks = gfx_uround_up_p2(*padded_w_in_blocks, bd->ut_w_in_blocks_2d);
      *padded_h_in_blocks = gfx_uround_up_p2(*padded_h_in_blocks, bd->ut_h_in_blocks_2d);
      break;
   case GFX_LFMT_SWIZZLING_UBLINEAR:
      *padded_w_in_blocks = gfx_uround_up_p2(*padded_w_in_blocks, gfx_lfmt_ub_w_in_blocks_2d(bd, swizzling));
      *padded_h_in_blocks = gfx_uround_up_p2(*padded_h_in_blocks, gfx_lfmt_ub_h_in_blocks_2d(bd, swizzling));
      break;
   default:
      unreachable();
   }

   /* If swizzling is specified, it must be compatible with usage and will be
    * passed through unmodified */
   if (swizzling_provided && (provided_swizzling != swizzling))
   {
      /* Exception: UIF swizzlings may be adjusted to their XOR equivalents */
      assert(gfx_lfmt_is_uif_family((GFX_LFMT_T)provided_swizzling));
      assert(gfx_lfmt_to_uif_xor_family(provided_swizzling) == swizzling);
   }
   gfx_lfmt_set_swizzling(lfmt, swizzling);


   bool do_yflip = (usage & GFX_BUFFER_USAGE_YFLIP_IF_POSSIBLE) && gfx_lfmt_is_2d(*lfmt);

   if (usage & (GFX_BUFFER_USAGE_V3D_TEXTURE | GFX_BUFFER_USAGE_V3D_TLB))
   {
      assert(gfx_lfmt_yflip_consistent(*lfmt));
      if ((usage & (
         GFX_BUFFER_USAGE_V3D_CUBEMAP /* TMU does not support y-flip for cubemaps */
         | GFX_BUFFER_USAGE_V3D_DEPTH_STENCIL /* TLB only supports y-flip for color buffers */
#if !V3D_VER_AT_LEAST(4,0,2,0)
         | GFX_BUFFER_USAGE_V3D_TLB_RAW /* Store/load general cannot be y-flipped */
#endif
         )) ||
         ((height % bd->block_h) != 0) || !gfx_lfmt_can_yflip_base(gfx_lfmt_get_base(lfmt)))
      {
         assert(!gfx_lfmt_get_yflip(lfmt));
         do_yflip = false;
      }
   }

   if (do_yflip)
   {
      gfx_lfmt_set_yflip(lfmt, gfx_lfmt_invert_yflip(gfx_lfmt_get_yflip(lfmt)));
      if (gfx_lfmt_can_yflip_base(gfx_lfmt_get_base(lfmt)))
         gfx_lfmt_set_base(lfmt, gfx_lfmt_yflip_base(gfx_lfmt_get_base(lfmt)));
   }

   assert(gfx_lfmt_dims_and_layout_compatible(*lfmt));
}

static uint32_t get_slice_pitch(
   const struct gfx_buffer_ml_cfg *ml_cfg, /* Must be non-NULL */
   uint32_t min_slice_pitch)
{
   if (!ml_cfg->force_slice_pitch)
      return min_slice_pitch;

   assert(ml_cfg->slice_pitch >= min_slice_pitch);
   return ml_cfg->slice_pitch;
}

static void calc_pitches_and_size(
   uint32_t *pitch, uint32_t *slice_pitch, size_t *size,
   GFX_LFMT_T lfmt, const GFX_LFMT_BASE_DETAIL_T *bd,
   uint32_t padded_w_in_blocks, uint32_t padded_h_in_blocks, uint32_t padded_d_in_blocks,
   const struct gfx_buffer_ml_cfg *ml_cfg) /* Must be non-NULL */
{
   uint32_t pitch_par_dim_in_blocks, pitch_perp_dim_in_blocks;
   if (gfx_lfmt_pitch_is_vertical(lfmt))
   {
      assert(gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&lfmt)) >= 2);
      pitch_par_dim_in_blocks = padded_h_in_blocks;
      pitch_perp_dim_in_blocks = padded_w_in_blocks;
   }
   else
   {
      pitch_par_dim_in_blocks = padded_w_in_blocks;
      pitch_perp_dim_in_blocks = padded_h_in_blocks;
   }

   *size = pitch_par_dim_in_blocks * bd->bytes_per_block;

   if (gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&lfmt)) >= 2)
   {
      *pitch = *size;
      *size *= pitch_perp_dim_in_blocks;
   }
   else
      *pitch = 0;

   if (gfx_lfmt_get_dims(&lfmt) == GFX_LFMT_DIMS_3D)
   {
      *slice_pitch = get_slice_pitch(ml_cfg, *size);
      *size = padded_d_in_blocks * *slice_pitch;
   }
   else
      *slice_pitch = 0;
}

void gfx_buffer_desc_gen(
   GFX_BUFFER_DESC_T *mls, /* one for each mip level. [0] is the top mip level */
   size_t *size, size_t *align, /* total size/align */
   gfx_buffer_usage_t usage,
   uint32_t width, uint32_t height, uint32_t depth,
   uint32_t num_mip_levels,
   uint32_t num_planes, const GFX_LFMT_T *plane_lfmts)
{
   gfx_buffer_desc_gen_with_ml0_cfg(
      mls, size, align,
      usage, width, height, depth,
      num_mip_levels, num_planes, plane_lfmts,
      NULL);
}

#if !V3D_VER_AT_LEAST(4,2,13,0)
static uint32_t dodgy_astc_l1_dim(uint32_t l0_dim, uint32_t block_dim)
{
   uint32_t const l0_blocks = gfx_udiv_round_up(l0_dim, block_dim);
   return ((l0_blocks + 1) / 2) * block_dim;
}
#endif

void gfx_buffer_desc_gen_with_ml0_cfg(
   GFX_BUFFER_DESC_T *mls, /* one for each mip level. [0] is the top mip level */
   size_t *size, size_t *align, /* total size/align */
   gfx_buffer_usage_t usage,
   uint32_t width, uint32_t height, uint32_t depth,
   uint32_t num_mip_levels,
   uint32_t num_planes, const GFX_LFMT_T *plane_lfmts,
   const struct gfx_buffer_ml_cfg *ml0_cfg)
{
   check_usage(usage);

   uint32_t dims = gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&plane_lfmts[0]));
   assert((dims >= 2) || (height == 1));
   assert((dims >= 3) || (depth == 1));
   if (usage & GFX_BUFFER_USAGE_V3D_CUBEMAP)
      assert(dims == 2);
#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (usage & GFX_BUFFER_USAGE_V3D_TLB_RAW)
      assert(dims == 2);
#endif
   assert(num_mip_levels > 0);
   assert(num_mip_levels <= GFX_BUFFER_MAX_MIP_LEVELS);
#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (usage & GFX_BUFFER_USAGE_V3D_TLB_RAW)
      assert(num_mip_levels == 1);
#endif

   assert(num_planes > 0);
   assert(num_planes <= GFX_BUFFER_MAX_PLANES);

   /* Assert UIF configuration is sane */
   assert(GFX_UIF_PAGE_SIZE > 0);
   assert(GFX_UIF_NUM_BANKS > 0);
   assert(GFX_UIF_XOR_ADDR > 0);

   struct gfx_buffer_ml_cfg ml1p_cfg;
   gfx_buffer_default_ml_cfg(&ml1p_cfg);
   if (ml0_cfg)
      ml1p_cfg.uif.xor_dis = ml0_cfg->uif.xor_dis;
   else
      ml0_cfg = &ml1p_cfg;

   uint32_t l1_width, l1_height, l1_depth;
   gfx_buffer_mip_dims(
      &l1_width, &l1_height, &l1_depth,
      width, height, depth,
      1);

   struct plane
   {
      GFX_LFMT_BASE_DETAIL_T bd;

      uint32_t l1_width, l1_height;

      /* Power-of-2 padding depends on block dimensions, so it may be different
       * for each plane */
      uint32_t p2_width, p2_height, p2_depth;

      /* Size of each mipmap level, including all padding */
      size_t ml_sizes[GFX_BUFFER_MAX_MIP_LEVELS];

      /* Total size of plane, so size of all mipmap levels plus padding between
       * them (each plane is separate and contiguous) */
      size_t size;

      /* Required alignment of level 0 */
      size_t ml0_align;
   } planes[GFX_BUFFER_MAX_PLANES];

   for (uint32_t i = 0; i != num_planes; ++i)
   {
      struct plane *p = &planes[i];

      /* All planes must have the same number of dimensions */
      assert(gfx_lfmt_dims_from_enum(gfx_lfmt_get_dims(&plane_lfmts[i])) == dims);

      gfx_lfmt_base_detail(&p->bd, plane_lfmts[i]);

#if !V3D_VER_AT_LEAST(4,2,13,0)
      if(gfx_lfmt_is_astc_family(plane_lfmts[i]))
      {
         p->l1_width = dodgy_astc_l1_dim(width, p->bd.block_w);
         p->l1_height = dodgy_astc_l1_dim(height, p->bd.block_h);
      }
      else
#endif
      {
         p->l1_width = l1_width;
         p->l1_height = l1_height;
      }

      /* Padding to power-of-2 is always done with level 1 size, and we
       * actually pad the size in utiles/blocks, not the size in texels (this
       * only matters for odd block sizes, like 5x5). 2* is to get back up to
       * level 0 size. */
      p->p2_width = 2 * gfx_next_power_of_2(gfx_udiv_round_up(p->l1_width, p->bd.block_w)) * p->bd.block_w;
      if (dims < 2)
         p->p2_height = 1; /* Avoid rounding up to block size... */
      else
         p->p2_height = 2 * gfx_next_power_of_2(gfx_udiv_round_up(p->l1_height, p->bd.block_h)) * p->bd.block_h;
      if (dims < 3)
         p->p2_depth = 1; /* Avoid rounding up to block size... */
      else
         p->p2_depth = 2 * gfx_next_power_of_2(gfx_udiv_round_up(l1_depth, p->bd.block_d)) * p->bd.block_d;

      /* Size accumulated in main loop below */
      p->size = 0;

      /* ml0_align filled in below */
   }

   for (uint32_t mip_level = 0; mip_level != num_mip_levels; ++mip_level)
   {
      GFX_BUFFER_DESC_T *ml = &mls[mip_level];
      const struct gfx_buffer_ml_cfg *ml_cfg = (mip_level == 0) ? ml0_cfg : &ml1p_cfg;

      gfx_buffer_mip_dims(
         &ml->width, &ml->height, &ml->depth,
         width, height, depth,
         mip_level);
      assert(
         !((ml->width == 1) && (ml->height == 1) && (ml->depth == 1)) ||
         (mip_level == (num_mip_levels - 1)));

      /* Set a default color conversion */
      ml->colorimetry = GFX_BUFFER_COLORIMETRY_DEFAULT;

      ml->num_planes = num_planes;

      for (uint32_t i = 0; i != num_planes; ++i)
      {
         GFX_BUFFER_DESC_PLANE_T *dplane = &ml->planes[i];
         struct plane *p = &planes[i];

         uint32_t ml_p2pad_width, ml_p2pad_height, ml_p2pad_depth;
         if (usage & GFX_BUFFER_USAGE_V3D_TEXTURE)
         {
            gfx_buffer_mip_dims(
               &ml_p2pad_width, &ml_p2pad_height, &ml_p2pad_depth,
               /* Width and height only padded to power-of-2 for levels 2 and
                * smaller */
               (mip_level >= 2) ? p->p2_width : width,
               (mip_level >= 2) ? p->p2_height : height,
               /* Depth padded to power-of-2 for all levels except level 0 */
               (mip_level >= 1) ? p->p2_depth : depth,
               mip_level);

            if(mip_level == 1)
            {
               ml_p2pad_width = p->l1_width;
               ml_p2pad_height = p->l1_height;
            }
         }
         else
         {
            ml_p2pad_width = ml->width;
            ml_p2pad_height = ml->height;
            ml_p2pad_depth = ml->depth;
         }

         dplane->lfmt = plane_lfmts[i];
         uint32_t ml_pad_w_in_blocks = gfx_udiv_round_up(ml_p2pad_width, p->bd.block_w);
         uint32_t ml_pad_h_in_blocks = gfx_udiv_round_up(ml_p2pad_height, p->bd.block_h);
         uint32_t ml_pad_d_in_blocks = gfx_udiv_round_up(ml_p2pad_depth, p->bd.block_d);
         adjust_lfmt_and_padding(&dplane->lfmt, &p->bd,
            &ml_pad_w_in_blocks, &ml_pad_h_in_blocks, &ml_pad_d_in_blocks,
            ml->height, usage, ml_cfg, i);
         GFX_LFMT_SWIZZLING_T swizzling = gfx_lfmt_get_swizzling(&dplane->lfmt);

         dplane->region = 0;

         calc_pitches_and_size(&dplane->pitch, &dplane->slice_pitch, &p->ml_sizes[mip_level],
            dplane->lfmt, &p->bd,
            ml_pad_w_in_blocks, ml_pad_h_in_blocks, ml_pad_d_in_blocks,
            ml_cfg);
         /* Add padding to guarantee page alignment of levels 1 and smaller
          * when necessary, which is essentially whenever any of them could be
          * UIF XOR. The power-of-2 padding of levels 2 and smaller preserves
          * page alignment for as long as is necessary, so we only actually
          * need to pad level 1 here. */
         if ((usage & GFX_BUFFER_USAGE_V3D_TEXTURE) && (mip_level == 1) &&
            (ml_pad_w_in_blocks > gfx_lfmt_ucol_w_in_blocks_2d(&p->bd, swizzling)) &&
            (ml_pad_h_in_blocks > (PCM15_IN_UB_ROWS * gfx_lfmt_ub_h_in_blocks_2d(&p->bd, swizzling))))
         {
            p->ml_sizes[mip_level] = gfx_zround_up(p->ml_sizes[mip_level], GFX_UIF_PAGE_SIZE);
         }

         size_t align = gfx_buffer_get_align(dplane->lfmt, GFX_BUFFER_ALIGN_RECOMMENDED);
         if (usage & GFX_BUFFER_USAGE_V3D_TEXTURE)
            align = gfx_zmax(align,
#if !V3D_VER_AT_LEAST(4,0,2,0)
               (mip_level == 0) ? V3D_TMU_CFG0_BASE_PTR_ALIGN :
#endif
               V3D_TMU_ML_ALIGN);
         assert(gfx_size_is_power_of_2(align));

         if (mip_level == 0)
            p->ml0_align = align;
         else
         {
            /* We only enforce mip level 0 alignment below. So make sure that
             * correct alignment of the other mip levels is implied by correct
             * alignment of level 0.. */

            /* Mip level 0 should have greatest alignment requirement */
            p->ml0_align = gfx_zmax(p->ml0_align, align);

            /* Offset to this mip level from mip level 0 should not destroy
             * alignment */
            size_t misalign = (p->ml_sizes[mip_level] + p->size - p->ml_sizes[0]) % align;
            if (misalign != 0)
            {
               /* The TMU rules should be enough. We shouldn't need to add any
                * extra padding here. */
               assert(!(usage & GFX_BUFFER_USAGE_V3D_TEXTURE));

               p->ml_sizes[mip_level] += align - misalign;
               assert(((p->ml_sizes[mip_level] + p->size - p->ml_sizes[0]) % align) == 0);
            }
         }

         p->size += p->ml_sizes[mip_level];
      }
   }

   /* The order of the planes in memory doesn't matter. We put plane 0 first
    * here. */
   size_t offset = 0;
   size_t max_align = 0;
   for (uint32_t i = 0; i != num_planes; ++i)
   {
      const struct plane *p = &planes[i];
      size_t ml0_offset_in_plane = p->size - p->ml_sizes[0];

      /* Bump up starting offset of this plane to ensure correct alignment of
       * level 0 mipmap */
      offset = gfx_zround_up(offset + ml0_offset_in_plane, p->ml0_align) - ml0_offset_in_plane;

      /* TMU needs mipmaps to be in reverse order. We always do that to keep
       * things simple. */
      for (int32_t mip_level = num_mip_levels - 1; mip_level != -1; --mip_level)
      {
         mls[mip_level].planes[i].offset = offset;
         offset += p->ml_sizes[mip_level];
      }

      /* Check we got mip level 0 aligned correctly */
      assert((mls[0].planes[i].offset % p->ml0_align) == 0);

      /* Current offset should match offset of first (smallest) mipmap, plus
       * total size of plane */
      assert(offset == (mls[num_mip_levels - 1].planes[i].offset + p->size));

      max_align = gfx_zmax(max_align, p->ml0_align);
   }

   *size = offset;
   *align = max_align;
}

void gfx_buffer_get_tmu_uif_cfg(
   struct gfx_buffer_uif_cfg *uif_cfg,
   const GFX_BUFFER_DESC_T *ml0, uint32_t plane_i)
{
   const GFX_BUFFER_DESC_PLANE_T *p = &ml0->planes[plane_i];

   memset(uif_cfg, 0, sizeof(*uif_cfg));

   if (gfx_lfmt_is_uif_family(p->lfmt))
   {
      GFX_LFMT_SWIZZLING_T swizzling = gfx_lfmt_get_swizzling(&p->lfmt);
      GFX_LFMT_BASE_DETAIL_T bd;
      gfx_lfmt_base_detail(&bd, p->lfmt);
      uint32_t w_in_blocks = gfx_udiv_round_up(ml0->width, bd.block_w);
      uint32_t h_in_blocks = gfx_udiv_round_up(ml0->height, bd.block_h);
      uint32_t height_in_ub = gfx_udiv_round_up(h_in_blocks, gfx_lfmt_ub_h_in_blocks_2d(&bd, swizzling));
      uint32_t padded_height_in_ub = gfx_buffer_uif_height_in_ub(ml0, plane_i);

      if ((choose_swizzling_for_tex(&bd, w_in_blocks, h_in_blocks) != GFX_LFMT_SWIZZLING_UIF) ||
         ((w_in_blocks > gfx_lfmt_ucol_w_in_blocks_2d(&bd, swizzling)) &&
            (((padded_height_in_ub % PC_IN_UB_ROWS) == 0) != gfx_lfmt_is_uif_xor_family(p->lfmt))) ||
         ((height_in_ub + choose_ub_pad(height_in_ub)) != padded_height_in_ub))
      {
         uif_cfg->force = true;
         uif_cfg->ub_xor = gfx_lfmt_is_uif_xor_family(p->lfmt);
         assert(!gfx_lfmt_is_noutile_family(p->lfmt));
         uif_cfg->ub_pads[0] = padded_height_in_ub - height_in_ub;
      }
   }
}
