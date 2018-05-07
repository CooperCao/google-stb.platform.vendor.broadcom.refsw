/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/vcos/vcos.h"

#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/common/abstract/khrn_client_platform_abstract.h"

#include <EGL/begl_platform.h>

#include <stdio.h>
#include <assert.h>

#ifdef ANDROID
#include <cutils/log.h>
#endif

static BEGL_InitInterface     s_initInterface;

/* Register application level overrides for any or all of the abstract API calls made by the 3D driver. */
BEGLAPI void BEGLAPIENTRY BEGL_RegisterInitInterface(BEGL_InitInterface *iface)
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
   switch ((int)format)
   {
   case ABGR_8888_RSO:                       ret = BEGL_BufferFormat_eA8B8G8R8;                                break;
   case XBGR_8888_RSO:                       ret = BEGL_BufferFormat_eX8B8G8R8;                                break;
   case RGBA_8888_RSO:                       ret = BEGL_BufferFormat_eR8G8B8A8;                                break;
   case RGBX_8888_RSO:                       ret = BEGL_BufferFormat_eR8G8B8X8;                                break;
   case RGB_565_RSO:                         ret = BEGL_BufferFormat_eR5G6B5;                                  break;
   case YV12_RSO:                            ret = BEGL_BufferFormat_eYV12;                                    break;

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

   default:                                     ret = IMAGE_FORMAT_INVALID;                                    break;
   }

   return ret;
}

typedef struct
{
   EGLenum target;
   void *native_buffer;
} EGLBUFINFO_T;

static void egl_server_platform_image_wrap_term(MEM_HANDLE_T handle)
{
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   EGLBUFINFO_T *info = (EGLBUFINFO_T *)mem_get_param(handle);

   driverInterfaces->displayInterface->ReleaseNativeBuffer(
         driverInterfaces->displayInterface->context, info->target,
         info->native_buffer);

   free(info);
}

static KHRN_IMAGE_T *egl_server_platform_create_image(uint32_t target,
      void *buffer, bool invalid, const char *desc)
{
   KHRN_IMAGE_FORMAT_T format = IMAGE_FORMAT_INVALID;
   KHRN_IMAGE_CREATE_FLAG_T flags;

   EGLBUFINFO_T *info = malloc(sizeof(*info));
   if (!info)
      goto end;

   BEGL_BufferSettings bufferSettings = { 0 };
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_NativeBuffer native_buffer =
         driverInterfaces->displayInterface->AcquireNativeBuffer(
               driverInterfaces->displayInterface->context, target, buffer,
               &bufferSettings);
   if (!native_buffer)
      goto cleanup_info;

   uint32_t w = bufferSettings.width;
   uint32_t h = bufferSettings.height;

   flags = IMAGE_CREATE_FLAG_RENDER_TARGET | IMAGE_CREATE_FLAG_DISPLAY;

   if (invalid)
      flags |= IMAGE_CREATE_FLAG_INVALID;

   format = abstract_bufferformat_to_format(bufferSettings.format);

   if (format == YV12_RSO)
      flags = IMAGE_CREATE_FLAG_TEXTURE | IMAGE_CREATE_FLAG_RSO_TEXTURE;  /* Not a valid render target */

   if ((((uintptr_t)bufferSettings.cachedAddr & ~0xFFF) != (uintptr_t)bufferSettings.cachedAddr) ||
         (((uintptr_t)bufferSettings.physOffset & ~0xFFF) != (uintptr_t)bufferSettings.physOffset) ||
         (format == IMAGE_FORMAT_INVALID))
      goto cleanup_native_buffer;

   uint32_t align = 0;
   khrn_image_platform_fudge(&format, &w, &h, &align, flags);

   MEM_HANDLE_T mh_wrap = mem_wrap(bufferSettings.secure ? NULL : bufferSettings.cachedAddr,
                           bufferSettings.physOffset,
                           bufferSettings.pitchBytes * h,
                           align, MEM_FLAG_DIRECT | (bufferSettings.secure ? MEM_FLAG_SECURE : 0),
                           desc);
   if (mh_wrap == MEM_HANDLE_INVALID)
      goto cleanup_native_buffer;

   info->target = target;
   info->native_buffer = native_buffer;
   mem_set_term(mh_wrap, egl_server_platform_image_wrap_term, info);

   KHRN_IMAGE_T *image = khrn_image_create_from_storage(format,
      bufferSettings.width, bufferSettings.height, bufferSettings.pitchBytes,
      MEM_HANDLE_INVALID, mh_wrap, 0,
      flags, bufferSettings.secure);

   mem_release(mh_wrap);
   return image;

cleanup_native_buffer:
   driverInterfaces->displayInterface->ReleaseNativeBuffer(
         driverInterfaces->displayInterface->context, target, buffer);
cleanup_info:
   free(info);
end:
   return NULL;
}

KHRN_IMAGE_T *egl_server_platform_create_pixmap(EGLNativePixmapType platform_pixmap)
{

   return egl_server_platform_create_image(BEGL_PIXMAP_BUFFER, platform_pixmap,
         /*invalid=*/false, "wrapped pixmap");
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

bool egl_server_platform_create_window_state(void **native_window_state, uintptr_t window, bool secure)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   *native_window_state = driverInterfaces->displayInterface->WindowPlatformStateCreate(
                                          driverInterfaces->displayInterface->context,
                                          (BEGL_WindowHandle)window, secure);
   return *native_window_state != NULL;
}

void egl_server_platform_destroy_window_state(void *native_window_state)
{
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   driverInterfaces->displayInterface->WindowPlatformStateDestroy(
      driverInterfaces->displayInterface->context,
      native_window_state);
}

KHRN_IMAGE_T *egl_server_platform_dequeue(
   void *native_window_state, KHRN_IMAGE_FORMAT_T format, void **swapchain_buffer, int *fd)
{
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   BEGL_BufferFormat abstract_format = format_to_abstract_bufferformat(format);

   *fd = -1;
   *swapchain_buffer = driverInterfaces->displayInterface->BufferDequeue(driverInterfaces->displayInterface->context,
      native_window_state, abstract_format, fd);
   if (!*swapchain_buffer)
      return NULL;

   return egl_server_platform_create_image(BEGL_SWAPCHAIN_BUFFER,
         *swapchain_buffer, /*invalid=*/true, "wrapped swapchain buffer");
}

bool egl_server_platform_queue(void *native_window_state, void *swapchain_buffer, int swap_interval, int fd)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   driverInterfaces->displayInterface->BufferQueue(
      driverInterfaces->displayInterface->context,
      native_window_state,
      swapchain_buffer,
      swap_interval,
      fd);

   return BEGL_Success;
}

bool egl_server_platform_cancel(void *native_window_state, void *swapchain_buffer, int fd)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   driverInterfaces->displayInterface->BufferCancel(
      driverInterfaces->displayInterface->context,
      native_window_state,
      swapchain_buffer,
      fd);

   return BEGL_Success;
}

static BEGL_NativeBuffer egl_server_platform_get_native_buffer(EGLenum target,
      void *egl_object, uint32_t *w, uint32_t *h, uint32_t *stride,
      KHRN_IMAGE_FORMAT_T *format, uint32_t *offset, void **p)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   if ((driverInterfaces->displayInterface != NULL) &&
       (driverInterfaces->displayInterface->AcquireNativeBuffer != NULL))
   {
      BEGL_NativeBuffer native_buffer;
      BEGL_BufferSettings settings;

      native_buffer = driverInterfaces->displayInterface->AcquireNativeBuffer(
            driverInterfaces->displayInterface->context, target, egl_object,
            &settings);

      if (native_buffer)
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
      }
      return native_buffer;
   }

   return NULL;
}

static KHRN_IMAGE_T *egl_server_platform_image_wrap(EGLenum target, EGLClientBuffer client_buffer)
{
   MEM_HANDLE_T handle;
   void *p;
   uint32_t offset, w, h, stride;
   KHRN_IMAGE_FORMAT_T format = ABGR_8888_RSO;

   BEGL_NativeBuffer native_buffer = egl_server_platform_get_native_buffer(
         target, client_buffer, &w, &h, &stride, &format, &offset, &p);

   if (!native_buffer)
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

   mem_set_term(handle, egl_server_platform_image_wrap_term, info);

   KHRN_IMAGE_T *image = khrn_image_create_from_storage(format,
      w, h, stride,
      MEM_HANDLE_INVALID, handle, 0, IMAGE_CREATE_FLAG_TEXTURE | IMAGE_CREATE_FLAG_RSO_TEXTURE, false);

   mem_release(handle);

   return image;
}

KHRN_IMAGE_T *egl_server_platform_image_new(EGLenum target, EGLClientBuffer client_buffer, EGLint *error)
{
   if (client_buffer)
      return egl_server_platform_image_wrap(target, client_buffer);
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
