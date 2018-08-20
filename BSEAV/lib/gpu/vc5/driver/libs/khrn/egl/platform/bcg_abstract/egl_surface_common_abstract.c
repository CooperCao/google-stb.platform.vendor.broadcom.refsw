/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "../../egl_display.h"
#include "../../egl_surface.h"
#include "../../egl_surface_base.h"
#include "../../egl_thread.h"
#include "../../egl_context_gl.h"

#include "libs/platform/bcm_sched_api.h"
#include "../../../common/khrn_process.h"
#include "../../../common/khrn_fmem.h"

#include "libs/platform/gmem.h"
#include "libs/platform/v3d_scheduler.h"

#include "egl_platform_abstract.h"
#include "egl_surface_common_abstract.h"

static GFX_LFMT_T sand_lfmt(BEGL_BufferFormat format)
{
   switch (format)
   {
   case BEGL_BufferFormat_eY8:         return GFX_LFMT_Y8_UNORM_2D;
   case BEGL_BufferFormat_eCr8Cb8:     return GFX_LFMT_U8_V8_2X2_UNORM_2D;
   case BEGL_BufferFormat_eCb8Cr8:     return GFX_LFMT_V8_U8_2X2_UNORM_2D;
   case BEGL_BufferFormat_eY10:        return GFX_LFMT_Y10_UNORM_2D;
   case BEGL_BufferFormat_eCr10Cb10:   return GFX_LFMT_U10V10_2X2_UNORM_2D;
   case BEGL_BufferFormat_eCb10Cr10:   return GFX_LFMT_V10U10_2X2_UNORM_2D;
   default:
      assert(0);
      return GFX_LFMT_NONE;
   }
}

static GFX_LFMT_SWIZZLING_T sand_swizzling(uint32_t stripeWidth)
{
   uint32_t ddrMap = v3d_scheduler_get_ddr_map_ver();

#if V3D_VER_AT_LEAST(3,3,0,0)
   if (stripeWidth == 256)
   {
      switch (ddrMap)
      {
      case 2:  return GFX_LFMT_SWIZZLING_SAND_256_MAP2_BIGEND;
      case 5:  return GFX_LFMT_SWIZZLING_SAND_256_MAP5_BIGEND;
      case 8:  return GFX_LFMT_SWIZZLING_SAND_256_MAP8_BIGEND;
      default: unreachable();
      }
   }
   else
   {
      assert(stripeWidth == 128);
      switch (ddrMap)
      {
      case 2:  return GFX_LFMT_SWIZZLING_SAND_128_MAP2_BIGEND;
      case 5:  return GFX_LFMT_SWIZZLING_SAND_128_MAP5_BIGEND;
      default: unreachable();
      }
   }
#else
   if (stripeWidth == 256)
   {
      switch (ddrMap)
      {
      case 2:  return GFX_LFMT_SWIZZLING_SAND_256_MAP2;
      case 5:  return GFX_LFMT_SWIZZLING_SAND_256_MAP5;
      default: unreachable();
      }
   }
   else
   {
      assert(stripeWidth == 128);
      switch (ddrMap)
      {
      case 2:  return GFX_LFMT_SWIZZLING_SAND_128_MAP2;
      case 5:  return GFX_LFMT_SWIZZLING_SAND_128_MAP5;
      default: unreachable();
      }
   }
#endif
}

static uint32_t lfmts_from_surface_info(
   GFX_LFMT_T *lfmts, GFX_LFMT_T *api_fmt,
   const BEGL_SurfaceInfo *info, bool flipY)
{
   /* TODO - once SWVC5-454 is resolved, this function drops the api_fmt */
   uint32_t num_planes = 1;

   switch (info->format)
   {
   case BEGL_BufferFormat_eA8B8G8R8:
      *api_fmt = GFX_LFMT_R8_G8_B8_A8_UNORM;
      lfmts[0] = GFX_LFMT_R8_G8_B8_A8_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eX8B8G8R8:
      *api_fmt = GFX_LFMT_R8_G8_B8_X8_UNORM;
      lfmts[0] = GFX_LFMT_R8_G8_B8_X8_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eR5G6B5:
      *api_fmt = GFX_LFMT_B5G6R5_UNORM;
      lfmts[0] = GFX_LFMT_B5G6R5_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eR4G4B4A4:
      *api_fmt = GFX_LFMT_R4G4B4A4_UNORM;
      lfmts[0] = GFX_LFMT_R4G4B4A4_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eR4G4B4X4:
      *api_fmt = GFX_LFMT_R4G4B4X4_UNORM;
      lfmts[0] = GFX_LFMT_R4G4B4X4_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eA4B4G4R4:
      *api_fmt = GFX_LFMT_A4B4G4R4_UNORM;
      lfmts[0] = GFX_LFMT_A4B4G4R4_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eX4B4G4R4:
      *api_fmt = GFX_LFMT_X4B4G4R4_UNORM;
      lfmts[0] = GFX_LFMT_X4B4G4R4_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eR5G5B5A1:
      *api_fmt = GFX_LFMT_R5G5B5A1_UNORM;
      lfmts[0] = GFX_LFMT_R5G5B5A1_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eR5G5B5X1:
      *api_fmt = GFX_LFMT_R5G5B5X1_UNORM;
      lfmts[0] = GFX_LFMT_R5G5B5X1_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eA1B5G5R5:
      *api_fmt = GFX_LFMT_A1B5G5R5_UNORM;
      lfmts[0] = GFX_LFMT_A1B5G5R5_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eX1B5G5R5:
      *api_fmt = GFX_LFMT_X1B5G5R5_UNORM;
      lfmts[0] = GFX_LFMT_X1B5G5R5_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eR8G8B8:
      *api_fmt = GFX_LFMT_R8_G8_B8_UNORM;
      lfmts[0] = GFX_LFMT_R8_G8_B8_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eA16B16G16R16_FP:
      *api_fmt = GFX_LFMT_R16_G16_B16_A16_FLOAT;
      lfmts[0] = GFX_LFMT_R16_G16_B16_A16_FLOAT_2D_RSO;
      break;
   case BEGL_BufferFormat_eA2B10G10R10:
      *api_fmt = GFX_LFMT_R10G10B10A2_UNORM;
      lfmts[0] = GFX_LFMT_R10G10B10A2_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eYUV422:
      /* TODO, khrn_image requires at this time an api_fmt. There is no api_fmt for YUV422
         use GFX_LFMT_R8_G8_B8_A8_UNORM for the moment till we remove api_fmt from khrn_image */
      *api_fmt = GFX_LFMT_R8_G8_B8_A8_UNORM;
      lfmts[0] = GFX_LFMT_Y8_U8_Y8_V8_2X1_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eYV12:
      /* TODO, khrn_image requires at this time an api_fmt. There is no api_fmt for YV12
         use GFX_LFMT_R8_G8_B8_A8_UNORM for the moment till we remove api_fmt from khrn_image */
      *api_fmt = GFX_LFMT_R8_G8_B8_A8_UNORM;
      /* YV12 is provided by Android and BSG as YCrCb/YVU */
      lfmts[0] = GFX_LFMT_Y8_UNORM_2D_RSO;
      lfmts[1] = GFX_LFMT_V8_2X2_UNORM_2D_RSO;
      lfmts[2] = GFX_LFMT_U8_2X2_UNORM_2D_RSO;
      num_planes = 3;
      break;
#if V3D_VER_AT_LEAST(3,3,0,0)
   case BEGL_BufferFormat_eBSTC:
      *api_fmt = GFX_LFMT_BSTC_RGBA_UNORM;
      lfmts[0] = GFX_LFMT_BSTCYFLIP_RGBA_UNORM_2D_RSO;
      break;
#endif
   case BEGL_BufferFormat_eTILED:
      *api_fmt = GFX_LFMT_R8_G8_B8_A8_UNORM;
      lfmts[0] = GFX_LFMT_R8_G8_B8_A8_UNORM_2D_UIF;
      break;
   case BEGL_BufferFormat_eY8:
   case BEGL_BufferFormat_eY10:
   {
      *api_fmt = GFX_LFMT_R8_G8_B8_A8_UNORM;
      num_planes = 2;
      lfmts[0] = sand_lfmt(info[0].format);
      lfmts[1] = sand_lfmt(info[1].format);
      // Set the swizzle
      GFX_LFMT_SWIZZLING_T swiz = sand_swizzling(info->stripeWidth);
      gfx_lfmt_set_swizzling(&lfmts[0], swiz);
      gfx_lfmt_set_swizzling(&lfmts[1], swiz);
      break;
   }
   default:
      unreachable();
      break;
   }

   if (flipY)
   {
      for (uint32_t i = 0; i < num_planes; i++)
         gfx_lfmt_set_yflip(&lfmts[i], GFX_LFMT_YFLIP_YFLIP);
   }

   return num_planes;
}

static gfx_buffer_colorimetry_t convert_colorimetry(BEGL_Colorimetry cp)
{
   switch (cp)
   {
   case BEGL_Colorimetry_RGB                  : return GFX_BUFFER_COLORIMETRY_DEFAULT;
   case BEGL_Colorimetry_BT_709               : return GFX_BUFFER_COLORIMETRY_BT_709;
   case BEGL_Colorimetry_Unknown              : return GFX_BUFFER_COLORIMETRY_UNKNOWN;
   case BEGL_Colorimetry_Dvi_Full_Range_RGB   : return GFX_BUFFER_COLORIMETRY_DVI_FULL_RANGE_RGB;
   case BEGL_Colorimetry_FCC                  : return GFX_BUFFER_COLORIMETRY_FCC;
   case BEGL_Colorimetry_BT_470_2_BG          : return GFX_BUFFER_COLORIMETRY_BT_470_2_BG;
   case BEGL_Colorimetry_Smpte_170M           : return GFX_BUFFER_COLORIMETRY_SMPTE_170M;
   case BEGL_Colorimetry_Smpte_240M           : return GFX_BUFFER_COLORIMETRY_SMPTE_240M;
   case BEGL_Colorimetry_XvYCC_709            : return GFX_BUFFER_COLORIMETRY_XVYCC_709;
   case BEGL_Colorimetry_XvYCC_601            : return GFX_BUFFER_COLORIMETRY_XVYCC_601;
   case BEGL_Colorimetry_BT_2020_NCL          : return GFX_BUFFER_COLORIMETRY_BT_2020_NCL;
   case BEGL_Colorimetry_BT_2020_CL           : return GFX_BUFFER_COLORIMETRY_BT_2020_CL;
   case BEGL_Colorimetry_Hdmi_Full_Range_YCbCr: return GFX_BUFFER_COLORIMETRY_HDMI_FULL_RANGE_YCBCR;
   default                                    : unreachable();
   }
}

static void buffer_desc_from_surface_info(GFX_BUFFER_DESC_T *descs, GFX_LFMT_T *api_fmt,
   gfx_buffer_usage_t *usage, const BEGL_SurfaceInfo *info, bool flipY)
{
   memset(descs, 0, sizeof(*descs) * info->miplevels);

   *usage = GFX_BUFFER_USAGE_V3D_RENDER_TARGET;

   /* Fill in the basic information for level 0 */
   descs->width  = info->width;
   descs->height = info->height;
   descs->depth  = 1;

   GFX_LFMT_T lfmts[GFX_BUFFER_MAX_PLANES];
   descs->num_planes = lfmts_from_surface_info(lfmts, api_fmt, info, flipY);
   for (uint32_t i = 0; i != descs->num_planes; ++i)
      descs->planes[i].lfmt = lfmts[i];

   if (info->format == BEGL_BufferFormat_eTILED && info->miplevels > 1)
   {
      assert(descs->num_planes == 1);
      assert(info->miplevels <= V3D_MAX_MIP_COUNT);

      /* Strip the swizzling from the lfmt passed to gfx_buffer_desc_gen so
       * it can make its own choices
       */
      gfx_lfmt_set_swizzling(&lfmts[0], GFX_LFMT_SWIZZLING_NONE);

      *usage |= GFX_BUFFER_USAGE_V3D_TEXTURE;
      size_t size, align;
      gfx_buffer_desc_gen(descs, &size, &align, *usage,
         info->width, info->height, /* depth */ 1, info->miplevels, /* num_planes */ 1, lfmts);

      assert(size <= info->byteSize);
   }
   else if (info->format == BEGL_BufferFormat_eYV12)
   {
      // Note we ignore info->pitchBytes here
      assert(descs->num_planes == 3);
      GFX_BUFFER_DESC_PLANE_T *p = descs->planes;
      p[0].pitch  = gfx_uround_up_p2(info->width, 16);
      p[1].pitch  = p[2].pitch = gfx_uround_up_p2(p[0].pitch / 2, 16);
      p[1].offset = info->height * p[0].pitch;
      p[2].offset = p[1].offset + ((info->height / 2) * p[1].pitch);
   }
   else if (info[0].format == BEGL_BufferFormat_eY8 || info[0].format == BEGL_BufferFormat_eY10)
   {
      assert(descs->num_planes == 2);
      GFX_BUFFER_DESC_PLANE_T *p = descs->planes;
      GFX_LFMT_BASE_DETAIL_T   bd;

      gfx_lfmt_base_detail(&bd, descs->planes[0].lfmt);
      p[0].pitch = info[0].stripedHeight * bd.bytes_per_block;

      gfx_lfmt_base_detail(&bd, descs->planes[1].lfmt);
      p[1].pitch = info[1].stripedHeight * bd.bytes_per_block;

      p[0].offset = 0;
      p[1].offset = 0;
      p[0].region = 0;
      p[1].region = 1;
   }
   else
   {
      assert(descs->num_planes == 1);
      assert(info->miplevels == 1);
      descs->planes[0].pitch = info->pitchBytes;

      if (info->format == BEGL_BufferFormat_eTILED)
         *usage |= GFX_BUFFER_USAGE_V3D_TEXTURE;
   }

   // Color primaries
   descs->colorimetry = convert_colorimetry(info->colorimetry);
}

struct NativeBuffer
{
   uint32_t type;
   uint32_t plane;
   BEGL_NativeBuffer buffer;
};

static void *acquire(uint32_t type, void *eglObject, uint32_t plane,
      BEGL_SurfaceInfo *surfaceInfo, EGLint *error)
{
   struct NativeBuffer *acquired = calloc(1, sizeof(*acquired));
   if (!acquired)
   {
      *error = EGL_BAD_ALLOC;
      return NULL;
   }

   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;
   assert(platform->SurfaceAcquire);
   acquired->type = type;
   acquired->plane = plane;
   acquired->buffer = platform->SurfaceAcquire(platform->context, type,
         eglObject, plane, surfaceInfo);
   if (!acquired->buffer)
   {
      *error = type == BEGL_PIXMAP_BUFFER ?
            EGL_BAD_NATIVE_PIXMAP : EGL_BAD_PARAMETER;
      free(acquired);
      acquired = NULL;
   }
   else
   {
      *error = EGL_SUCCESS;
   }
   return acquired;
}

static void release(void *nativeBuffer)
{
   struct NativeBuffer *acquired = (struct NativeBuffer *)nativeBuffer;
   if (!acquired)
      return;

   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;
   assert(platform->SurfaceRelease);
   platform->SurfaceRelease(platform->context, acquired->type, acquired->plane,
         acquired->buffer);
   free(acquired);
}

khrn_image *image_from_surface_abstract(uint32_t type, void *object, bool flipY,
      const char *description, unsigned *num_mip_levels, EGLint *error)
{
   uint32_t          num_handles = 1;
   BEGL_SurfaceInfo  surfaceInfo[GFX_BUFFER_MAX_PLANES];
   gmem_handle_t     gmem_handles[GFX_BUFFER_MAX_PLANES];
   void             *acquired[GFX_BUFFER_MAX_PLANES];
   memset(surfaceInfo, 0, sizeof(BEGL_SurfaceInfo) * GFX_BUFFER_MAX_PLANES);
   memset(gmem_handles, 0, sizeof(gmem_handle_t) * GFX_BUFFER_MAX_PLANES);
   memset(acquired, 0, sizeof(*acquired) * GFX_BUFFER_MAX_PLANES);

   EGLint dummy;
   if (!error)
      error = &dummy; /* to avoid checking for NULL */

   acquired[0] = acquire(type, object, /*plane=*/0, &surfaceInfo[0], error);
   if (!acquired[0])
      goto oom;

   /* We must be given locked pointers */
   assert(surfaceInfo[0].physicalOffset != 0);

   gmem_handles[0] = gmem_from_external_memory(release, acquired[0],
                                               surfaceInfo[0].physicalOffset, surfaceInfo[0].cachedAddr,
                                               surfaceInfo[0].byteSize, surfaceInfo[0].secure,
                                               surfaceInfo[0].contiguous, description);
   if (gmem_handles[0] == GMEM_HANDLE_INVALID)
   {
      *error = EGL_BAD_ALLOC;
      goto oom;
   }
   acquired[0] = NULL; /* now owned by gmem handle */

   // Ensure we extract both planes for sand format surfaces
   // which may have a separate alloc per plane
   if (surfaceInfo[0].format == BEGL_BufferFormat_eY8 ||
       surfaceInfo[0].format == BEGL_BufferFormat_eY10)
   {
      acquired[1] = acquire(type, object, /*plane=*/1, &surfaceInfo[1], error);
      if (!acquired[1])
         goto oom;

      /* We must be given locked pointers */
      assert(surfaceInfo[1].physicalOffset != 0);

      assert(surfaceInfo[0].miplevels == surfaceInfo[1].miplevels);

      num_handles = 2;
      gmem_handles[1] = gmem_from_external_memory(release, acquired[1],
                                                  surfaceInfo[1].physicalOffset, surfaceInfo[1].cachedAddr,
                                                  surfaceInfo[1].byteSize, surfaceInfo[1].secure,
                                                  surfaceInfo[1].contiguous, description);
      if (gmem_handles[1] == GMEM_HANDLE_INVALID)
      {
         *error = EGL_BAD_ALLOC;
         goto oom;
      }
      acquired[1] = NULL; /* now owned by gmem handle */
   }

   GFX_BUFFER_DESC_T  buffer_descs[V3D_MAX_MIP_COUNT];
   gfx_buffer_usage_t usage;
   GFX_LFMT_T         api_fmt;
   buffer_desc_from_surface_info(buffer_descs, &api_fmt, &usage, surfaceInfo, flipY);

   khrn_blob *image_blob = khrn_blob_create_from_handles(num_handles, gmem_handles, buffer_descs,
                                                         surfaceInfo[0].miplevels,
                                                         /*num_array_elems=*/1, /*array_pitch=*/0,
                                                         usage);
   if (!image_blob)
   {
      *error = EGL_BAD_ALLOC;
      goto oom;
   }

   // image_blob now owns gmem_handle

   khrn_image *image = khrn_image_create(image_blob, /*start_elem=*/0, /*num_array_elems=*/1,
                                         /*level=*/0, api_fmt);
   KHRN_MEM_ASSIGN(image_blob, NULL);

   if (image && num_mip_levels)
      *num_mip_levels = surfaceInfo[0].miplevels;

   *error = EGL_SUCCESS;
   return image;

oom:
   for (uint32_t p = 0; p < GFX_BUFFER_MAX_PLANES; p++)
   {
      gmem_free(gmem_handles[p]);
      release(acquired[p]);
   }
   return NULL;
}

BEGL_BufferFormat get_pixmap_format(void *nativePixmap)
{
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;
   if (platform->GetPixmapFormat)
      return platform->GetPixmapFormat(platform->context, nativePixmap);
   else
      return BEGL_BufferFormat_INVALID; /* pixmaps unsupported */
}
