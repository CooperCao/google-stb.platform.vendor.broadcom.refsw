/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

static uint32_t lfmts_from_buffer_format(
   GFX_LFMT_T *lfmts, GFX_LFMT_T *api_fmt,
   BEGL_BufferFormat format, bool flipY)
{
   /* TODO - once SWVC5-454 is resolved, this function drops the api_fmt */
   uint32_t num_planes = 1;

   switch (format)
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

static void buffer_desc_from_surface_info(GFX_BUFFER_DESC_T *descs, GFX_LFMT_T *api_fmt,
   gfx_buffer_usage_t *usage, const BEGL_SurfaceInfo *info, bool flipY)
{
   memset(descs, 0, sizeof(*descs) * info->miplevels);

   *usage = GFX_BUFFER_USAGE_V3D_RENDER_TARGET;

   /* Fill in the basic information for level 0 */
   descs->width = info->width;
   descs->height = info->height;
   descs->depth = 1;

   GFX_LFMT_T lfmts[GFX_BUFFER_MAX_PLANES];
   descs->num_planes = lfmts_from_buffer_format(lfmts, api_fmt, info->format, flipY);
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
      p[0].pitch = gfx_uround_up_p2(info->width, 16);
      p[1].pitch = p[2].pitch = gfx_uround_up_p2(p[0].pitch / 2, 16);
      p[1].offset = info->height * p[0].pitch;
      p[2].offset = p[1].offset + ((info->height / 2) * p[1].pitch);
   }
   else
   {
      assert(descs->num_planes == 1);
      assert(info->miplevels == 1);
      descs->planes[0].pitch = info->pitchBytes;

      if (info->format == BEGL_BufferFormat_eTILED)
         *usage |= GFX_BUFFER_USAGE_V3D_TEXTURE;
   }
}

static void image_term_abstract(void *nativeSurface)
{
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;

   if (platform->SurfaceChangeRefCount)
      platform->SurfaceChangeRefCount(platform->context, nativeSurface, BEGL_Decrement);
}

khrn_image *image_from_surface_abstract(void *nativeSurface, bool flipY, unsigned *num_mip_levels)
{
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;
   if (!platform->SurfaceGetInfo)
      return NULL;

   BEGL_SurfaceInfo surfaceInfo;
   memset(&surfaceInfo, 0, sizeof(BEGL_SurfaceInfo));
   if (platform->SurfaceGetInfo(platform->context, nativeSurface, &surfaceInfo) != BEGL_Success)
      return NULL;

   GFX_BUFFER_DESC_T buffer_descs[V3D_MAX_MIP_COUNT];
   GFX_LFMT_T api_fmt;
   gfx_buffer_usage_t usage;
   buffer_desc_from_surface_info(buffer_descs, &api_fmt, &usage, &surfaceInfo, flipY);

   /* We must be given locked pointers */
   assert(surfaceInfo.physicalOffset != 0);

   if (platform->SurfaceChangeRefCount)
      platform->SurfaceChangeRefCount(platform->context, nativeSurface, BEGL_Increment);

   gmem_handle_t gmem_handle = gmem_from_external_memory(image_term_abstract, nativeSurface,
                                           surfaceInfo.physicalOffset, surfaceInfo.cachedAddr,
                                           surfaceInfo.byteSize, "display_surface");
   if (gmem_handle == GMEM_HANDLE_INVALID)
      return NULL;

   khrn_blob *image_blob = khrn_blob_create_from_storage(gmem_handle, buffer_descs,
                                           surfaceInfo.miplevels, 1,
                                           surfaceInfo.byteSize, usage);

   if (!image_blob)
   {
      gmem_free(gmem_handle);
      return NULL;
   }
   // image_blob now owns gmem_handle

   khrn_image *image = khrn_image_create(image_blob,
      /*start_elem=*/0, /*num_array_elems=*/1, /*level=*/0, api_fmt);
   KHRN_MEM_ASSIGN(image_blob, NULL);

   *num_mip_levels = surfaceInfo.miplevels;
   return image;
}
