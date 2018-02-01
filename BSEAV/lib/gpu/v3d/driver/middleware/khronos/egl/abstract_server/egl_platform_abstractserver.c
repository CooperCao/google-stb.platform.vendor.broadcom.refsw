/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/vcos/vcos.h"

#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/common/abstract/khrn_client_platform_abstract.h"

#include <stdio.h>
#include <assert.h>

#ifdef ANDROID
#include <cutils/log.h>
#endif

static BEGL_InitInterface     s_initInterface;

/* Register application level overrides for any or all of the abstract API calls made by the 3D driver. */
EGLAPI void EGLAPIENTRY BEGL_RegisterInitInterface(BEGL_InitInterface *iface)
{
   if (iface)
   {
      vcos_demand(iface->GetDisplay != NULL);
      vcos_demand(iface->Initialize != NULL);
      vcos_demand(iface->Terminate != NULL);
      s_initInterface = *iface;
   }
   else
   {
      memset(&s_initInterface, 0, sizeof(s_initInterface));
   }
}

BEGL_BufferFormat format_to_abstract_bufferformat(KHRN_IMAGE_FORMAT_T format)
{
   BEGL_BufferFormat ret;
   switch (format)
   {
   case ABGR_8888_RSO:                       ret = BEGL_BufferFormat_eA8B8G8R8;                                break;
   case XBGR_8888_RSO:                       ret = BEGL_BufferFormat_eX8B8G8R8;                                break;
   case RGBA_8888_RSO:                       ret = BEGL_BufferFormat_eR8G8B8A8;                                break;
   case RGBX_8888_RSO:                       ret = BEGL_BufferFormat_eR8G8B8X8;                                break;
   case RGB_565_RSO:                         ret = BEGL_BufferFormat_eR5G6B5;                                  break;
   case YV12_RSO:                            ret = BEGL_BufferFormat_eYV12;                                    break;

   case (ABGR_8888_RSO | IMAGE_FORMAT_PRE):  ret = BEGL_BufferFormat_eA8B8G8R8_sRGB_PRE;                       break;
   case (XBGR_8888_RSO | IMAGE_FORMAT_PRE):  ret = BEGL_BufferFormat_eX8B8G8R8_sRGB_PRE;                       break;
   case (RGBA_8888_RSO | IMAGE_FORMAT_PRE):  ret = BEGL_BufferFormat_eR8G8B8A8_sRGB_PRE;                       break;
   case (RGBX_8888_RSO | IMAGE_FORMAT_PRE):  ret = BEGL_BufferFormat_eR8G8B8X8_sRGB_PRE;                       break;
   case (RGB_565_RSO | IMAGE_FORMAT_PRE):    ret = BEGL_BufferFormat_eR5G6B5_sRGB_PRE;                         break;

   case (ABGR_8888_RSO | IMAGE_FORMAT_LIN):  ret = BEGL_BufferFormat_eA8B8G8R8_LIN_NON;                        break;
   case (XBGR_8888_RSO | IMAGE_FORMAT_LIN):  ret = BEGL_BufferFormat_eX8B8G8R8_LIN_NON;                        break;
   case (RGBA_8888_RSO | IMAGE_FORMAT_LIN):  ret = BEGL_BufferFormat_eR8G8B8A8_LIN_NON;                        break;
   case (RGBX_8888_RSO | IMAGE_FORMAT_LIN):  ret = BEGL_BufferFormat_eR8G8B8X8_LIN_NON;                        break;
   case (RGB_565_RSO | IMAGE_FORMAT_LIN):    ret = BEGL_BufferFormat_eR5G6B5_LIN_NON;                          break;

   case (ABGR_8888_RSO | IMAGE_FORMAT_LIN | IMAGE_FORMAT_PRE):  ret = BEGL_BufferFormat_eA8B8G8R8_LIN_PRE;     break;
   case (XBGR_8888_RSO | IMAGE_FORMAT_LIN | IMAGE_FORMAT_PRE):  ret = BEGL_BufferFormat_eX8B8G8R8_LIN_PRE;     break;
   case (RGBA_8888_RSO | IMAGE_FORMAT_LIN | IMAGE_FORMAT_PRE):  ret = BEGL_BufferFormat_eR8G8B8A8_LIN_PRE;     break;
   case (RGBX_8888_RSO | IMAGE_FORMAT_LIN | IMAGE_FORMAT_PRE):  ret = BEGL_BufferFormat_eR8G8B8X8_LIN_PRE;     break;
   case (RGB_565_RSO | IMAGE_FORMAT_LIN | IMAGE_FORMAT_PRE):    ret = BEGL_BufferFormat_eR5G6B5_LIN_PRE;       break;

   default:                                  ret = BEGL_BufferFormat_INVALID;                                  break;
   }

   return ret;
}

KHRN_IMAGE_FORMAT_T abstract_bufferformat_to_format(BEGL_BufferFormat format)
{
   KHRN_IMAGE_FORMAT_T ret;
   switch (format)
   {
   case BEGL_BufferFormat_eA8B8G8R8:            ret = ABGR_8888_RSO;                                           break;
   case BEGL_BufferFormat_eX8B8G8R8:            ret = XBGR_8888_RSO;                                           break;
   case BEGL_BufferFormat_eR8G8B8A8:            ret = RGBA_8888_RSO;                                           break;
   case BEGL_BufferFormat_eR8G8B8X8:            ret = RGBX_8888_RSO;                                           break;
   case BEGL_BufferFormat_eR5G6B5:              ret = RGB_565_RSO;                                             break;
   case BEGL_BufferFormat_eYV12:                ret = YV12_RSO;                                                break;

   case BEGL_BufferFormat_eA8B8G8R8_sRGB_PRE:   ret = (ABGR_8888_RSO | IMAGE_FORMAT_PRE);                      break;
   case BEGL_BufferFormat_eX8B8G8R8_sRGB_PRE:   ret = (XBGR_8888_RSO | IMAGE_FORMAT_PRE);                      break;
   case BEGL_BufferFormat_eR8G8B8A8_sRGB_PRE:   ret = (RGBA_8888_RSO | IMAGE_FORMAT_PRE);                      break;
   case BEGL_BufferFormat_eR8G8B8X8_sRGB_PRE:   ret = (RGBX_8888_RSO | IMAGE_FORMAT_PRE);                      break;
   case BEGL_BufferFormat_eR5G6B5_sRGB_PRE:     ret = (RGB_565_RSO | IMAGE_FORMAT_PRE);                        break;

   case BEGL_BufferFormat_eA8B8G8R8_LIN_NON:    ret = (ABGR_8888_RSO | IMAGE_FORMAT_LIN);                      break;
   case BEGL_BufferFormat_eX8B8G8R8_LIN_NON:    ret = (XBGR_8888_RSO | IMAGE_FORMAT_LIN);                      break;
   case BEGL_BufferFormat_eR8G8B8A8_LIN_NON:    ret = (RGBA_8888_RSO | IMAGE_FORMAT_LIN);                      break;
   case BEGL_BufferFormat_eR8G8B8X8_LIN_NON:    ret = (RGBX_8888_RSO | IMAGE_FORMAT_LIN);                      break;
   case BEGL_BufferFormat_eR5G6B5_LIN_NON:      ret = (RGB_565_RSO | IMAGE_FORMAT_LIN);                        break;

   case BEGL_BufferFormat_eA8B8G8R8_LIN_PRE:    ret = (ABGR_8888_RSO | IMAGE_FORMAT_LIN | IMAGE_FORMAT_PRE);   break;
   case BEGL_BufferFormat_eX8B8G8R8_LIN_PRE:    ret = (XBGR_8888_RSO | IMAGE_FORMAT_LIN | IMAGE_FORMAT_PRE);   break;
   case BEGL_BufferFormat_eR8G8B8A8_LIN_PRE:    ret = (RGBA_8888_RSO | IMAGE_FORMAT_LIN | IMAGE_FORMAT_PRE);   break;
   case BEGL_BufferFormat_eR8G8B8X8_LIN_PRE:    ret = (RGBX_8888_RSO | IMAGE_FORMAT_LIN | IMAGE_FORMAT_PRE);   break;
   case BEGL_BufferFormat_eR5G6B5_LIN_PRE:      ret = (RGB_565_RSO | IMAGE_FORMAT_LIN | IMAGE_FORMAT_PRE);     break;

   default:                                     ret = IMAGE_FORMAT_INVALID;                                    break;
   }

   return ret;
}

void BEGLint_intBufferGetRequirements(const BEGL_PixmapInfoEXT *bufferRequirements, BEGL_BufferSettings *bufferConstrainedRequirements)
{
   KHRN_IMAGE_CREATE_FLAG_T flags = 0;
   KHRN_IMAGE_FORMAT_T format;
   uint32_t align, wd, ht;
   uint32_t overrun;
   /* needs to be max of CPU cache line and the GCACHE on the v3d */
   uint32_t cacheline_size = mem_cacheline_size();
   cacheline_size = (cacheline_size > BCG_GCACHE_LINE_SIZE) ? cacheline_size : BCG_GCACHE_LINE_SIZE;

   memset(bufferConstrainedRequirements, 0, sizeof(BEGL_BufferSettings));

   flags = IMAGE_CREATE_FLAG_RENDER_TARGET | IMAGE_CREATE_FLAG_DISPLAY;

   format = abstract_bufferformat_to_format(bufferRequirements->format);

   bufferConstrainedRequirements->secure = bufferRequirements->secure;

   wd = bufferRequirements->width;
   ht = bufferRequirements->height;
   align = 0;

   khrn_image_platform_fudge(&format, &wd, &ht, &align, flags);

   bufferConstrainedRequirements->format = bufferRequirements->format;
   bufferConstrainedRequirements->width = bufferRequirements->width;
   bufferConstrainedRequirements->height = bufferRequirements->height;
   bufferConstrainedRequirements->alignment = (align > cacheline_size) ? align : cacheline_size;
   bufferConstrainedRequirements->pitchBytes = khrn_image_get_stride(format, wd);
   bufferConstrainedRequirements->totalByteSize = khrn_image_get_size(format, wd, ht);
   /* make sure the last line can always complete a tile line (with the enables set) */
   overrun = (KHRN_HW_TILE_WIDTH - (wd % KHRN_HW_TILE_WIDTH)) * (khrn_image_get_bpp(format) / 8);
   bufferConstrainedRequirements->totalByteSize += overrun;
   /* align to a cache line */
   bufferConstrainedRequirements->totalByteSize = (bufferConstrainedRequirements->totalByteSize + (cacheline_size - 1)) & ~(cacheline_size - 1);
}

EGLAPI void EGLAPIENTRY BEGLint_BufferGetRequirements(const BEGL_PixmapInfoEXT *bufferRequirements, BEGL_BufferSettings *bufferConstrainedRequirements)
{
   BEGLint_intBufferGetRequirements(bufferRequirements, bufferConstrainedRequirements);
}

KHRN_IMAGE_T *egl_server_platform_create_pixmap_info(void *platform_pixmap, bool invalid)
{
   BEGL_BufferSettings bufferSettings;
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   memset(&bufferSettings, 0, sizeof(BEGL_BufferSettings));

   BEGL_Error ret = driverInterfaces->displayInterface->SurfaceGetInfo(driverInterfaces->displayInterface->context,
      BEGL_DEFAULT_BUFFER, (BEGL_BufferHandle)platform_pixmap, &bufferSettings);

   if (ret != BEGL_Success)
      return NULL;

   KHRN_IMAGE_FORMAT_T format = IMAGE_FORMAT_INVALID;
   KHRN_IMAGE_CREATE_FLAG_T flags;

   uint32_t w = bufferSettings.width;
   uint32_t h = bufferSettings.height;

   flags = IMAGE_CREATE_FLAG_RENDER_TARGET | IMAGE_CREATE_FLAG_DISPLAY;

   if (invalid)
      flags |= IMAGE_CREATE_FLAG_INVALID;

   format = abstract_bufferformat_to_format(bufferSettings.format);

   if (format == YV12_RSO)
      flags = IMAGE_CREATE_FLAG_TEXTURE | IMAGE_CREATE_FLAG_RSO_TEXTURE;  /* Not a valid render target */

   KHRN_IMAGE_T *image = NULL;
   if ((((uintptr_t)bufferSettings.cachedAddr & ~0xFFF) == (uintptr_t)bufferSettings.cachedAddr) &&
         (((uintptr_t)bufferSettings.physOffset & ~0xFFF) == (uintptr_t)bufferSettings.physOffset) &&
         (format != IMAGE_FORMAT_INVALID))
   {
      uint32_t align = 0;
      khrn_image_platform_fudge(&format, &w, &h, &align, flags);

      MEM_HANDLE_T mh_wrap = mem_wrap(bufferSettings.secure ? NULL : bufferSettings.cachedAddr,
                              bufferSettings.physOffset,
                              bufferSettings.pitchBytes * h,
                              align, MEM_FLAG_DIRECT | (bufferSettings.secure ? MEM_FLAG_SECURE : 0),
                              "wrapped pixmap");

      image = khrn_image_create_from_storage(format,
         bufferSettings.width, bufferSettings.height, bufferSettings.pitchBytes,
         MEM_HANDLE_INVALID, mh_wrap, 0,
         flags, bufferSettings.secure);

      /* Note: Don't store the opaque buffer handle here, as we may not always want to destroy it.
         * We'll store it explicitly for our swap chain created buffers. Remember, this is used for pixmaps too.*/

      mem_release(mh_wrap);
   }

   return image;
}

static EGLDisplay egl_server_platform_default_display(void)
{
   /* return the first display */
   return (EGLDisplay)1;
}

EGLDisplay egl_server_platform_get_display(EGLenum platform,
      void *native_display, const EGLint *attrib_list, EGLint *error)
{
   EGLDisplay display;

   if (s_initInterface.GetDisplay)
   {
      BEGL_DisplayHandle handle;
      BEGL_Error res;

      res = s_initInterface.GetDisplay(s_initInterface.context, platform,
            native_display, attrib_list, &handle);
      if (res == BEGL_Success)
      {
         display = (EGLDisplay)handle;
         *error = EGL_SUCCESS;
      }
      else /* BEGL_Fail means that platform parameter was invalid */
      {
         display = EGL_NO_DISPLAY;
         *error = EGL_BAD_PARAMETER;
      }
   }
   else if (platform == BEGL_DEFAULT_PLATFORM)
   {
      /* legacy mode for default platform: only default display is available */
      if (native_display == EGL_DEFAULT_DISPLAY)
         display = egl_server_platform_default_display();
      else
         display = EGL_NO_DISPLAY;
      *error = EGL_SUCCESS;
   }
   else
   {
      /* legacy mode for non-default platform */
      display = EGL_NO_DISPLAY;
      *error = EGL_BAD_PARAMETER;
   }
   return display;
}

EGLint egl_server_platform_init(EGLDisplay display)
{
   EGLint error;

   if (s_initInterface.Initialize)
   {
      BEGL_DisplayHandle handle = (BEGL_DisplayHandle)display;
      if (handle)
      {
         if (s_initInterface.Initialize(s_initInterface.context,
               handle) == BEGL_Success)
            error = EGL_SUCCESS;
         else
            error = EGL_NOT_INITIALIZED;
      }
      else
         error = EGL_BAD_DISPLAY;
   }
   else if (display == egl_server_platform_default_display())
   {
      /* legacy mode for default display: platform was already initialised */
      error = EGL_SUCCESS;
   }
   else
   {
      /* legacy mode for non-default display */
      error = EGL_BAD_DISPLAY;
   }
   return error;
}

EGLint egl_server_platform_term(EGLDisplay display)
{
   EGLint error;

   if (s_initInterface.Terminate)
   {
      BEGL_DisplayHandle handle = (BEGL_DisplayHandle)display;
      if (s_initInterface.Terminate(s_initInterface.context, handle) == BEGL_Success)
         error = EGL_SUCCESS;
      else
         error = EGL_BAD_DISPLAY;
   }
   else if (display == egl_server_platform_default_display())
   {
      /* legacy mode for default display: platform will be terminated later */
      error = EGL_SUCCESS;
   }
   else
   {
      /* legacy mode for non-default display */
      error = EGL_BAD_DISPLAY;
   }
   return error;
}

bool egl_server_platform_create_window_state(BEGL_WindowState **windowState, uintptr_t window, bool secure)
{
   BEGL_WindowState        *platformState = NULL;
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();
   bool                    ok = true;

   platformState = (BEGL_WindowState *)malloc(sizeof(BEGL_WindowState));
   memset(platformState, 0, sizeof(BEGL_WindowState));
   platformState->window = (BEGL_WindowHandle)window;

   if (platformState)
   {
      platformState->platformState = driverInterfaces->displayInterface->WindowPlatformStateCreate(
                                             driverInterfaces->displayInterface->context,
                                             (BEGL_WindowHandle)window, secure);
      if (platformState->platformState == NULL)
         ok = false;
   }

   *windowState = platformState;

   return ok;
}

void egl_server_platform_destroy_window_state(BEGL_WindowState  *windowState)
{
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   driverInterfaces->displayInterface->WindowPlatformStateDestroy(
      driverInterfaces->displayInterface->context,
      windowState->platformState);
   free(windowState);
}

void *egl_server_platform_dequeue(
   BEGL_WindowState *window_state, KHRN_IMAGE_FORMAT_T format, int *fd)
{
   BEGL_BufferHandle pixmap = NULL;
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   BEGL_BufferFormat abstract_format = format_to_abstract_bufferformat(format);

   pixmap = driverInterfaces->displayInterface->BufferDequeue(driverInterfaces->displayInterface->context,
      window_state->platformState, abstract_format, fd);

   return (void *)pixmap;
}

bool egl_server_platform_queue(BEGL_WindowState *windowState, void *opaque_buffer_handle, int swap_interval, int fd)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   driverInterfaces->displayInterface->BufferQueue(
      driverInterfaces->displayInterface->context,
      windowState->platformState,
      opaque_buffer_handle,
      swap_interval,
      fd);

   return BEGL_Success;
}

bool egl_server_platform_cancel(BEGL_WindowState *windowState, void *opaque_buffer_handle, int fd)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   driverInterfaces->displayInterface->BufferCancel(
      driverInterfaces->displayInterface->context,
      windowState->platformState,
      opaque_buffer_handle,
      fd);

   return BEGL_Success;
}

bool egl_server_platform_get_info(EGLenum target, void *native_buffer, uint32_t *w, uint32_t *h, uint32_t *stride, KHRN_IMAGE_FORMAT_T *format, uint32_t *offset, void **p)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   if ((driverInterfaces->displayInterface != NULL) &&
      (driverInterfaces->displayInterface->SurfaceGetInfo != NULL))
   {
      BEGL_Error res;
      BEGL_BufferSettings settings;

      res = driverInterfaces->displayInterface->SurfaceGetInfo(driverInterfaces->displayInterface->context,
         target, native_buffer,
         &settings);

      if (res == BEGL_Success)
      {
         *w = settings.width;
         *h = settings.height;
         *stride = settings.pitchBytes;
         *offset = settings.physOffset;
         *p = settings.cachedAddr;

         KHRN_IMAGE_FORMAT_T f = abstract_bufferformat_to_format(settings.format);

         /* only convert to lt for tf images.  RSO should travel through as normal */
         if (khrn_image_is_tformat(f) && khrn_image_prefer_lt(f, *w, *h))
            *format = khrn_image_to_lt_format(f);
         else
            *format = f;

         return true;
      }
   }

   return false;
}

static void egl_server_platform_inc_refcnt(EGLenum target, void *native_buffer)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   if ((driverInterfaces->displayInterface != NULL) &&
      (driverInterfaces->displayInterface->SurfaceChangeRefCount != NULL))
   {
      driverInterfaces->displayInterface->SurfaceChangeRefCount(driverInterfaces->displayInterface->context,
            target, native_buffer, BEGL_Increment);
   }
}

static void egl_server_platform_dec_refcnt(EGLenum target, void *native_buffer)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   if ((driverInterfaces->displayInterface != NULL) &&
      (driverInterfaces->displayInterface->SurfaceChangeRefCount != NULL))
   {
      driverInterfaces->displayInterface->SurfaceChangeRefCount(driverInterfaces->displayInterface->context,
            target, native_buffer, BEGL_Decrement);
   }
}

typedef struct
{
   EGLenum target;
   void *native_buffer;
} EGLBUFINFO_T;

static void egl_server_platform_image_wrap_term(MEM_HANDLE_T handle)
{
   EGLBUFINFO_T *info = (EGLBUFINFO_T *)mem_get_param(handle);
   egl_server_platform_dec_refcnt(info->target, info->native_buffer);
   free(info);
}

KHRN_IMAGE_T *egl_server_platform_image_wrap(EGLenum target, void *native_buffer)
{
   MEM_HANDLE_T handle;
   void *p;
   uint32_t offset, w, h, stride;
   KHRN_IMAGE_FORMAT_T format = ABGR_8888_RSO;

   if (!egl_server_platform_get_info(target, native_buffer, &w, &h, &stride, &format, &offset, &p))
      return NULL;

   handle = mem_wrap(p, offset,
      h * stride,
      1, MEM_FLAG_DIRECT,
      "EGL_NATIVE_BUFFER_ANDROID");
   if (handle == MEM_HANDLE_INVALID)
      return NULL;

   EGLBUFINFO_T *info = malloc(sizeof(*info));
   if (!info)
   {
      mem_release(handle); /* this can't trigger our term handle - it's not set yet */
      return NULL;
   }
   info->target = target;
   info->native_buffer = native_buffer;

   egl_server_platform_inc_refcnt(target, native_buffer);

   mem_set_term(handle, egl_server_platform_image_wrap_term, info);

   KHRN_IMAGE_T *image = khrn_image_create_from_storage(format,
      w, h, stride,
      MEM_HANDLE_INVALID, handle, 0, IMAGE_CREATE_FLAG_TEXTURE | IMAGE_CREATE_FLAG_RSO_TEXTURE, false);

   mem_release(handle);

   return image;
}

void *egl_server_platform_get_native_buffer(EGLenum target, EGLClientBuffer *egl_buffer)
{
   void *native_buffer;
   BEGL_DriverInterfaces  *driverInterfaces = BEGL_GetDriverInterfaces();
   if ((driverInterfaces->displayInterface != NULL) &&
      (driverInterfaces->displayInterface->GetNativeBuffer != NULL))
   {
      if (driverInterfaces->displayInterface->GetNativeBuffer(
            driverInterfaces->displayInterface->context, target, egl_buffer,
            &native_buffer) != BEGL_Success)
         native_buffer = NULL;
   }
   else /* use egl_buffer as is */
   {
      native_buffer = (void*)egl_buffer;
   }
   return native_buffer;

}

KHRN_IMAGE_T *egl_server_platform_image_new(EGLenum target, void *native_buffer, EGLint *error)
{
   if (native_buffer)
      return egl_server_platform_image_wrap(target, native_buffer);
   else
   {
      *error = EGL_BAD_MATCH;
      return NULL;
   }
}

uint32_t egl_server_platform_get_color_format(KHRN_IMAGE_FORMAT_T format)
{
   uint32_t res = 0;
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   /* remove any colorspace flags */
   format = khrn_image_no_colorspace_format(format);

   if (khrn_image_is_tformat(format) || khrn_image_is_rso(format))
   {
      if ((driverInterfaces->displayInterface != NULL) &&
         (driverInterfaces->displayInterface->GetNativeFormat != NULL))
      {
         format = khrn_image_to_rso_format(format);

         BEGL_BufferFormat bufferFormat = format_to_abstract_bufferformat(format);

         driverInterfaces->displayInterface->GetNativeFormat(driverInterfaces->displayInterface->context,
            bufferFormat, &res);
      }
   }

   return res;
}
