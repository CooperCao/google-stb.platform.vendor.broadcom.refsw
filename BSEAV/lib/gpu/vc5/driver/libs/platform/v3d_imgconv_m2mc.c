/******************************************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "v3d_platform.h"
#include "v3d_imgconv_internal.h"
#include "v3d_driver_api.h"

#if V3D_PLATFORM_ABSTRACT

LOG_DEFAULT_CAT("v3d_imgconv_m2mc")

#include "gmem_abstract.h"

static BEGL_BufferFormat lfmt_to_begl_format(GFX_LFMT_T lfmt)
{
   if (gfx_lfmt_is_uif_family(lfmt))
   {
      // M2MC only handles RGBA8888_UIF
      if (lfmt == GFX_LFMT_R8_G8_B8_A8_UNORM_2D_UIF)
         return BEGL_BufferFormat_eTILED;
      return BEGL_BufferFormat_INVALID;
   }

   switch (gfx_lfmt_fmt(lfmt))
   {
   case GFX_LFMT_R8_G8_B8_A8_UNORM: return BEGL_BufferFormat_eA8B8G8R8;
   case GFX_LFMT_R8_G8_B8_X8_UNORM: return BEGL_BufferFormat_eX8B8G8R8;
   case GFX_LFMT_B5G6R5_UNORM:      return BEGL_BufferFormat_eR5G6B5;
   case GFX_LFMT_A4B4G4R4_UNORM:    return BEGL_BufferFormat_eA4B4G4R4;
   case GFX_LFMT_X4B4G4R4_UNORM:    return BEGL_BufferFormat_eX4B4G4R4;
   case GFX_LFMT_A1B5G5R5_UNORM:    return BEGL_BufferFormat_eA1B5G5R5;
   case GFX_LFMT_X1B5G5R5_UNORM:    return BEGL_BufferFormat_eX1B5G5R5;
#if V3D_VER_AT_LEAST(3,3,0,0)
   case GFX_LFMT_BSTC_RGBA_UNORM:   return BEGL_BufferFormat_eBSTC;
#else
   case GFX_LFMT_BSTC_RGBA_UNORM:   return BEGL_BufferFormat_INVALID;
#endif
   default:                         return BEGL_BufferFormat_INVALID;
   }
}

static BEGL_Colorimetry convert_colorimetry(gfx_buffer_colorimetry_t cp)
{
   switch (cp)
   {
   case GFX_BUFFER_COLORIMETRY_DEFAULT               : return BEGL_Colorimetry_RGB;
   case GFX_BUFFER_COLORIMETRY_BT_709                : return BEGL_Colorimetry_BT_709;
   case GFX_BUFFER_COLORIMETRY_UNKNOWN               : return BEGL_Colorimetry_Unknown;
   case GFX_BUFFER_COLORIMETRY_DVI_FULL_RANGE_RGB    : return BEGL_Colorimetry_Dvi_Full_Range_RGB;
   case GFX_BUFFER_COLORIMETRY_FCC                   : return BEGL_Colorimetry_FCC;
   case GFX_BUFFER_COLORIMETRY_BT_470_2_BG           : return BEGL_Colorimetry_BT_470_2_BG;
   case GFX_BUFFER_COLORIMETRY_SMPTE_170M            : return BEGL_Colorimetry_Smpte_170M;
   case GFX_BUFFER_COLORIMETRY_SMPTE_240M            : return BEGL_Colorimetry_Smpte_240M;
   case GFX_BUFFER_COLORIMETRY_XVYCC_709             : return BEGL_Colorimetry_XvYCC_709;
   case GFX_BUFFER_COLORIMETRY_XVYCC_601             : return BEGL_Colorimetry_XvYCC_601;
   case GFX_BUFFER_COLORIMETRY_BT_2020_NCL           : return BEGL_Colorimetry_BT_2020_NCL;
   case GFX_BUFFER_COLORIMETRY_BT_2020_CL            : return BEGL_Colorimetry_BT_2020_CL;
   case GFX_BUFFER_COLORIMETRY_HDMI_FULL_RANGE_YCBCR : return BEGL_Colorimetry_Hdmi_Full_Range_YCbCr;
   default                                           : unreachable();
   }
}

bool v3d_imgconv_m2mc_convert_surface(const v3d_imgconv_gmem_tgt *src, uint32_t src_off,
                                      const v3d_imgconv_gmem_tgt *dst, uint32_t dst_off,
                                      bool validateOnly)
{
   BEGL_SurfaceConversionInfo info = {};
   info.width  = src->base.desc.width;
   info.height = src->base.desc.height;
   info.secure = dst->handles[0] && (gmem_get_usage(dst->handles[0]) & GMEM_USAGE_SECURE) != 0;
   info.srcNativeSurface = src->handles[0] ? gmem_get_external_context(src->handles[0]) : NULL;
   info.srcColorimetry   = convert_colorimetry(src->base.desc.colorimetry);

   // We don't ever convert to sand, only from
   assert(!gfx_lfmt_is_sand_family(dst->base.desc.planes[0].lfmt));
   info.dstFormat = lfmt_to_begl_format(dst->base.desc.planes[0].lfmt);
   if (info.dstFormat == BEGL_BufferFormat_INVALID)
      return false;

   info.dstAlignment    = /*log2(4096)*/12;
   info.dstPitch        = dst->base.desc.planes[0].pitch;
   info.dstMemoryBlock  = dst->handles[0] ? gmem_get_platform_handle(dst->handles[0]) : NULL;
   info.dstMemoryOffset = dst->offset + dst->base.desc.planes[0].offset + dst_off;

   return gmem_convert_surface(&info, validateOnly);
}

static bool claim_m2mc_conversion(
   const struct v3d_imgconv_base_tgt *dst,
   const struct v3d_imgconv_base_tgt *src,
   unsigned int width, unsigned int height,
   unsigned int depth, const conversion_info_t *info)
{
   if (!v3d_imgconv_valid_hw_conv_info(info))
      return false;

   if (info->secure_context && info->secure_src)
      return false;

   /* Both src & dst must be in contiguous memory to use the M2MC */
   if (!info->contiguous_src || !info->contiguous_dst)
      return false;

   GFX_LFMT_T dst_lfmt = dst->desc.planes[0].lfmt;
   if (!gfx_lfmt_is_uif(dst_lfmt) && !gfx_lfmt_is_rso(dst_lfmt))
      return false;

   /* We can only use the 7260A0 M2MC with UIF images that are an even
    * number of UBs in height.
    */
   if (gfx_lfmt_is_uif(dst_lfmt) &&
       (v3d_scheduler_get_soc_quirks() & V3D_SOC_QUIRK_HWBCM7260_81) &&
       (gfx_buffer_uif_height_in_ub(&dst->desc, 0) & 0x1))
   {
      return false;
   }

   /* only from/to origin */
   if (dst->x != 0 || dst->y != 0 || dst->z != 0 || src->x != 0 || src->y != 0 || src->z != 0)
      return false;

   /* we must copy to the whole dst; we might be able to add a lesser
   * restriction , multiple of utiles, but we should be ok with this for the
   * moment */
   if (width != dst->desc.width || height != dst->desc.height || depth != dst->desc.depth)
      return false;

   assert(width <= src->desc.width && height <= src->desc.height && depth <= src->desc.depth);

   if (gfx_lfmt_has_depth(dst->desc.planes[0].lfmt) || gfx_lfmt_has_stencil(dst->desc.planes[0].lfmt))
      return false;

   if (src->desc.num_planes <= 2 && dst->desc.num_planes == 1)
   {
      v3d_imgconv_gmem_tgt src_gmem_tgt;
      memset(&src_gmem_tgt, 0, sizeof(src_gmem_tgt));
      src_gmem_tgt.base = *src;

      v3d_imgconv_gmem_tgt dst_gmem_tgt;
      memset(&dst_gmem_tgt, 0, sizeof(dst_gmem_tgt));
      dst_gmem_tgt.base = *dst;

      if (!v3d_imgconv_m2mc_convert_surface(&src_gmem_tgt, 0,
                                            &dst_gmem_tgt, 0,
                                            /*validateOnly=*/true))
         return false;

      return true;
   }

   return false;
}

typedef struct
{
   v3d_imgconv_gmem_tgt src;
   unsigned int         src_off;
   v3d_imgconv_gmem_tgt dst;
   unsigned int         dst_off;
} user_mode_data;

static void user_mode_convert(void *data)
{
   user_mode_data *umd = (user_mode_data*)data;

   bool ok = v3d_imgconv_m2mc_convert_surface(&umd->src, umd->src_off,
                                              &umd->dst, umd->dst_off,
                                              /*validateOnly=*/false);
   assert(ok);

   free(umd);
}

// Configure async intermediate conversion to RGBA using the M2MC then final pass using TFU
static bool convert_m2mc_async(
   const struct v3d_imgconv_gmem_tgt *dst, unsigned int dst_off,
   const struct v3d_imgconv_gmem_tgt *src, unsigned int src_off,
   const v3d_scheduler_deps *deps, uint64_t *job_id,
   unsigned int width, unsigned int height,
   unsigned int depth)
{
   // Build data for a usermode job
   user_mode_data *umd = (user_mode_data*)calloc(1, sizeof(user_mode_data));
   umd->src     = *src;
   umd->src_off = src_off;
   umd->dst     = *dst;
   umd->dst_off = 0;

   assert(gmem_get_usage(src->handles[0]) & GMEM_USAGE_CONTIGUOUS);
   assert(gmem_get_usage(dst->handles[0]) & GMEM_USAGE_CONTIGUOUS);

   // Create a user-mode job that will do the conversion using the M2MC when the src
   // deps are satisfied
   *job_id = v3d_scheduler_submit_usermode_job(deps, user_mode_convert, umd);

   log_trace("Queued M2MC conv jobid %"PRIu64" : %dx%d, from(%s)",
              *job_id, width, height, gfx_lfmt_desc(src->base.desc.planes[0].lfmt));
   // Can only use gfx_lfmt_desc() once per log
   log_trace("       ->to(%s)", gfx_lfmt_desc(dst->base.desc.planes[0].lfmt));

   return true;
}

/* gmem->M2MC->gmem */
static v3d_imgconv_methods m2mc_path =
{
   .claim         = claim_m2mc_conversion,
   .convert_async = convert_m2mc_async,
   .convert_sync  = NULL,
   .convert_prep  = NULL
};
#else

bool v3d_imgconv_m2mc_convert_surface(const v3d_imgconv_gmem_tgt *src, uint32_t src_off,
                                      const v3d_imgconv_gmem_tgt *dst, uint32_t dst_off,
                                      bool validateOnly)
{
   return false;
}

#endif // V3D_PLATFORM_ABSTRACT

const v3d_imgconv_methods *get_m2mc_path(void)
{
#if V3D_PLATFORM_ABSTRACT
   return &m2mc_path;
#else
   return NULL;
#endif
}
