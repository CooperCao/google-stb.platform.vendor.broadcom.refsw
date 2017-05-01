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

BEGL_BufferFormat get_begl_format_abstract(GFX_LFMT_T fmt)
{
   BEGL_BufferFormat ret = BEGL_BufferFormat_INVALID;

   switch (fmt)
   {
   case GFX_LFMT_R8_G8_B8_A8_UNORM :
      ret = BEGL_BufferFormat_eA8B8G8R8;
      break;
   case GFX_LFMT_R8_G8_B8_X8_UNORM :
   case GFX_LFMT_R8_G8_B8_UNORM    :
      ret = BEGL_BufferFormat_eX8B8G8R8;
      break;
   case GFX_LFMT_B5G6R5_UNORM      :
      ret = BEGL_BufferFormat_eR5G6B5;
      break;
   case GFX_LFMT_R4G4B4A4_UNORM:
      ret = BEGL_BufferFormat_eR4G4B4A4;
      break;
   case GFX_LFMT_R4G4B4X4_UNORM:
      ret = BEGL_BufferFormat_eR4G4B4X4;
      break;
   case GFX_LFMT_A4B4G4R4_UNORM:
      ret = BEGL_BufferFormat_eA4B4G4R4;
      break;
   case GFX_LFMT_X4B4G4R4_UNORM:
      ret = BEGL_BufferFormat_eX4B4G4R4;
      break;
   case GFX_LFMT_R5G5B5A1_UNORM:
      ret = BEGL_BufferFormat_eR5G5B5A1;
      break;
   case GFX_LFMT_R5G5B5X1_UNORM:
      ret = BEGL_BufferFormat_eR5G5B5X1;
      break;
   case GFX_LFMT_A1B5G5R5_UNORM:
      ret = BEGL_BufferFormat_eA1B5G5R5;
      break;
   case GFX_LFMT_X1B5G5R5_UNORM:
      ret = BEGL_BufferFormat_eX1B5G5R5;
      break;
   case GFX_LFMT_Y8_U8_Y8_V8_2X1_UNORM:
      ret = BEGL_BufferFormat_eYUV422;
      break;
#if V3D_VER_AT_LEAST(3,3,0,0)
   case GFX_LFMT_BSTC_RGBA_UNORM:
      ret = BEGL_BufferFormat_eBSTC;
      break;
#endif
   default :
      break;
   }

   return ret;
}

static uint32_t get_formats_abstract(BEGL_BufferFormat format,
                                       GFX_LFMT_T *lfmt,
                                       GFX_LFMT_T *api_fmt,
                                       bool flipY)
{
   /* TODO - once SWVC5-454 is resolved, this function drops the api_fmt */
   uint32_t num_planes = 1;

   switch (format)
   {
   case BEGL_BufferFormat_eA8B8G8R8:
      *api_fmt = GFX_LFMT_R8_G8_B8_A8_UNORM;
      lfmt[0] = GFX_LFMT_R8_G8_B8_A8_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eX8B8G8R8:
      *api_fmt = GFX_LFMT_R8_G8_B8_X8_UNORM;
      lfmt[0] = GFX_LFMT_R8_G8_B8_X8_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eR5G6B5:
      *api_fmt = GFX_LFMT_B5G6R5_UNORM;
      lfmt[0] = GFX_LFMT_B5G6R5_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eR4G4B4A4:
      *api_fmt = GFX_LFMT_R4G4B4A4_UNORM;
      lfmt[0] = GFX_LFMT_R4G4B4A4_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eR4G4B4X4:
      *api_fmt = GFX_LFMT_R4G4B4X4_UNORM;
      lfmt[0] = GFX_LFMT_R4G4B4X4_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eA4B4G4R4:
      *api_fmt = GFX_LFMT_A4B4G4R4_UNORM;
      lfmt[0] = GFX_LFMT_A4B4G4R4_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eX4B4G4R4:
      *api_fmt = GFX_LFMT_X4B4G4R4_UNORM;
      lfmt[0] = GFX_LFMT_X4B4G4R4_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eR5G5B5A1:
      *api_fmt = GFX_LFMT_R5G5B5A1_UNORM;
      lfmt[0] = GFX_LFMT_R5G5B5A1_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eR5G5B5X1:
      *api_fmt = GFX_LFMT_R5G5B5X1_UNORM;
      lfmt[0] = GFX_LFMT_R5G5B5X1_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eA1B5G5R5:
      *api_fmt = GFX_LFMT_A1B5G5R5_UNORM;
      lfmt[0] = GFX_LFMT_A1B5G5R5_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eX1B5G5R5:
      *api_fmt = GFX_LFMT_X1B5G5R5_UNORM;
      lfmt[0] = GFX_LFMT_X1B5G5R5_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eYUV422:
      /* TODO, khrn_image requires at this time an api_fmt. There is no api_fmt for YUV422
         use GFX_LFMT_R8_G8_B8_A8_UNORM for the moment till we remove api_fmt from khrn_image */
      *api_fmt = GFX_LFMT_R8_G8_B8_A8_UNORM;
      lfmt[0] = GFX_LFMT_Y8_U8_Y8_V8_2X1_UNORM_2D_RSO;
      break;
   case BEGL_BufferFormat_eYV12:
      /* TODO, khrn_image requires at this time an api_fmt. There is no api_fmt for YV12
         use GFX_LFMT_R8_G8_B8_A8_UNORM for the moment till we remove api_fmt from khrn_image */
      *api_fmt = GFX_LFMT_R8_G8_B8_A8_UNORM;
      /* YV12 is provided by Android and BSG as YCrCb/YVU */
      lfmt[0] = GFX_LFMT_Y8_UNORM_2D_RSO;
      lfmt[1] = GFX_LFMT_V8_2X2_UNORM_2D_RSO;
      lfmt[2] = GFX_LFMT_U8_2X2_UNORM_2D_RSO;
      num_planes = 3;
      break;
#if V3D_VER_AT_LEAST(3,3,0,0)
   case BEGL_BufferFormat_eBSTC:
      *api_fmt = GFX_LFMT_BSTC_RGBA_UNORM;
      lfmt[0] = GFX_LFMT_BSTCYFLIP_RGBA_UNORM_2D_RSO;
      break;
#endif
   default:
      unreachable();
      break;
   }

   if (flipY)
   {
      for (uint32_t i = 0; i < num_planes; i++)
         gfx_lfmt_set_yflip(&lfmt[i], GFX_LFMT_YFLIP_YFLIP);
   }

   return num_planes;
}

/* The function gfx_buffer_desc_gen creates the buffer description using 3
   individual planes with no knowledge of the overall buffer format.
   This function is called knowing that it is a YV12 buffer and corrects
   the pitches to be 16 pixel aligned and calculates the correct offsets. */
static void fix_YV12_planes_pitch_and_offset(GFX_BUFFER_DESC_T *buffer_desc)
{
   assert((buffer_desc->width & 0x1) == 0 && (buffer_desc->height & 0x1) == 0);

   buffer_desc->planes[0].pitch = gfx_uround_up_p2(buffer_desc->width, 16);
   buffer_desc->planes[1].pitch = gfx_uround_up_p2(buffer_desc->planes[0].pitch / 2, 16);
   buffer_desc->planes[2].pitch = buffer_desc->planes[1].pitch;

   buffer_desc->planes[0].offset = 0;
   buffer_desc->planes[1].offset = buffer_desc->planes[0].pitch * buffer_desc->height;
   buffer_desc->planes[2].offset = buffer_desc->planes[1].pitch * buffer_desc->height / 2 + buffer_desc->planes[1].offset;
}

static void image_term_abstract(void *nativeSurface)
{
   BEGL_DisplayInterface   *platform = &g_bcgPlatformData.displayInterface;

   if (platform->SurfaceChangeRefCount)
      platform->SurfaceChangeRefCount(platform->context, nativeSurface, BEGL_Decrement);
}

khrn_image *image_from_surface_abstract(void *nativeSurface, bool flipY)
{
   khrn_image            *image = NULL;
   khrn_blob             *image_blob = NULL;
   gmem_handle_t           gmem_handle = GMEM_HANDLE_INVALID;
   GFX_BUFFER_DESC_T       buffer_desc;
   BEGL_DisplayInterface   *platform = &g_bcgPlatformData.displayInterface;
   BEGL_SurfaceInfo        surfaceInfo;
   GFX_LFMT_T              api_fmt;
   GFX_LFMT_T              lfmt[GFX_BUFFER_MAX_PLANES];

   memset(&buffer_desc, 0, sizeof(GFX_BUFFER_DESC_T));
   memset(&surfaceInfo, 0, sizeof(BEGL_SurfaceInfo));

   if (!platform->SurfaceGetInfo)
      goto error;

   if (platform->SurfaceGetInfo(platform->context, nativeSurface, &surfaceInfo) != BEGL_Success)
      goto error;

   /* Construct a GFX_BUFFER_DESC_T */
   uint32_t num_planes = get_formats_abstract(surfaceInfo.format, lfmt, &api_fmt, flipY);

   size_t size, align;
   gfx_buffer_desc_gen(&buffer_desc,
      &size, &align, GFX_BUFFER_USAGE_V3D_RENDER_TARGET,
      surfaceInfo.width, surfaceInfo.height, 1, 1, num_planes, lfmt);

   if (surfaceInfo.format == BEGL_BufferFormat_eYV12)
      /* Horizontal stride multiple of 16 pixels for YV12 */
      fix_YV12_planes_pitch_and_offset(&buffer_desc);

   /* We must be given locked pointers */
   assert(surfaceInfo.physicalOffset != 0);

   if (platform->SurfaceChangeRefCount)
      platform->SurfaceChangeRefCount(platform->context, nativeSurface, BEGL_Increment);

   gmem_handle = gmem_from_external_memory(image_term_abstract, nativeSurface,
                                           surfaceInfo.physicalOffset, surfaceInfo.cachedAddr,
                                           surfaceInfo.byteSize, "display_surface");
   if (gmem_handle == GMEM_HANDLE_INVALID)
      goto error;

   image_blob = khrn_blob_create_from_storage(gmem_handle,
                                              &buffer_desc,
                                              1, 1, surfaceInfo.byteSize,
                                              GFX_BUFFER_USAGE_V3D_RENDER_TARGET);

   if (!image_blob)
      goto error;

   image = khrn_image_create(image_blob, 0, 1, 0, api_fmt);

   if (!image)
      goto error;

   KHRN_MEM_ASSIGN(image_blob, NULL);

   return image;

error:
   KHRN_MEM_ASSIGN(image, NULL);
   KHRN_MEM_ASSIGN(image_blob, NULL);
   gmem_free(gmem_handle);

   return NULL;
}