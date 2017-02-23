/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
abstract platform layer
=============================================================================*/
#include <stdlib.h>

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_parallel.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/egl/egl_client_config.h"
#include "middleware/khronos/egl/egl_server.h"
#include "interface/vcos/vcos.h"
#include "middleware/khronos/common/khrn_hw.h"
#ifndef V3D_LEAN
#include "middleware/khronos/dispatch/khrn_dispatch_rpc.h" /* khdispatch_send_async prototype */
#endif
#include "interface/khronos/common/khrn_int_ids.h"

#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/common/abstract/khrn_client_platform_abstract.h"

// Client is single-threaded on direct platform so no mutex stuff necessary

// TODO: find somewhere better to put this function?

void khdispatch_send_async(uint32_t command, uint64_t pid, uint32_t sem)
{
   if (sem != KHRN_NO_SEMAPHORE) {
      uint32_t name[3];

      name[0] = (uint32_t)pid; name[1] = (uint32_t)(pid >> 32); name[2] = (uint32_t)sem;

      if (command == ASYNC_COMMAND_DESTROY) {
         /* todo: destroy */
      } else {
         PLATFORM_SEMAPHORE_T s;
         if (khronos_platform_semaphore_create(&s, name, 1) == KHR_SUCCESS) {
            switch (command) {
            case ASYNC_COMMAND_WAIT:
               /* todo: i don't understand what ASYNC_COMMAND_WAIT is for, so this
                * might be completely wrong */
               khronos_platform_semaphore_acquire(&s);
               break;
            case ASYNC_COMMAND_POST:
               khronos_platform_semaphore_release(&s);
               break;
            default:
               UNREACHABLE();
            }
            khronos_platform_semaphore_destroy(&s);
         }
      }
   }
}

VCOS_STATUS_T khronos_platform_semaphore_create(PLATFORM_SEMAPHORE_T *sem, uint32_t *name, int count)
{
   char buf[64];
   // should be KhanSemaphore but we are limited to 31 characters
   vcos_snprintf(buf,sizeof(buf),"KhanSem%08x%08x%08x", name[0], name[1], name[2]);
   return vcos_named_semaphore_create(sem, buf, count);
}

void khronos_platform_semaphore_destroy(PLATFORM_SEMAPHORE_T *sem)
{
   vcos_named_semaphore_delete(sem);
}

void khronos_platform_semaphore_acquire(PLATFORM_SEMAPHORE_T *sem)
{
   vcos_named_semaphore_wait(sem);
}

VCOS_STATUS_T khronos_platform_semaphore_acquire_timeout(PLATFORM_SEMAPHORE_T *sem, int timeout)
{
   return vcos_named_semaphore_wait_timeout(sem, timeout);
}

void khronos_platform_semaphore_release(PLATFORM_SEMAPHORE_T *sem)
{
   vcos_named_semaphore_post(sem);
}

VCOS_STATUS_T platform_tls_create(PLATFORM_TLS_T *tls, void (*destructor)(void*))
{
   return vcos_tls_create(tls, destructor);
}

void platform_tls_destroy(PLATFORM_TLS_T tls)
{
   vcos_tls_delete(tls);
}

void platform_tls_set(PLATFORM_TLS_T tls, void *v)
{
   if (vcos_tls_set(tls, v) != VCOS_SUCCESS)
   {
      ANDROID_LOGD("platform_tls_set() : FAILED (key %p), (v %p)\n", (void *)tls, v);
   }
}

void *platform_tls_get(PLATFORM_TLS_T tls)
{
   void * tsd = vcos_tls_get(tls);
   if (tsd == NULL)
   {
      ANDROID_LOGD("platform_tls_get() : creating TLS block, (tid = %d) (pid = %d)\n", gettid(), getpid());

      if (client_thread_attach(tls))
         tsd = vcos_tls_get(tls);
      else
         ANDROID_LOGD("client_thread_attach() : FAILED\n");
   }

   if (tsd == NULL)
   {
      ANDROID_LOGD("***************************************************************************\n");
      ANDROID_LOGD("ERROR : TLS = NULL, (tid = %d) (pid = %d)\n", gettid(), getpid());
      ANDROID_LOGD("***************************************************************************\n");
   }

   return tsd;
}

void platform_tls_remove(PLATFORM_TLS_T tls)
{
   vcos_tls_set(tls, NULL);
}

/* ----------------------------------------------------------------------
 * workaround for broken platforms which don't detect threads exiting
 * -------------------------------------------------------------------- */
void platform_hint_thread_finished(void)
{
   client_thread_detach(NULL);
}

void *khrn_platform_malloc(size_t size, const char *desc)
{
   UNUSED(desc);

   return malloc(size);
}

void khrn_platform_free(void *v)
{
   free(v);
}

void platform_get_info(EGLNativeWindowType win, unsigned int flags, CLIENT_PLATFORM_INFO_T * info)
{
   BEGL_WindowInfo         windowInfo;
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   if (info)
   {
      memset(info, 0, sizeof(CLIENT_PLATFORM_INFO_T));

      if ((driverInterfaces != NULL) &&
          (driverInterfaces->displayInterface != NULL) &&
          (driverInterfaces->displayInterface->WindowGetInfo) != NULL)
      {
         BEGL_Error ret;
         BEGL_WindowInfoFlags wiFlags = 0;
         memset(&windowInfo, 0, sizeof(BEGL_WindowInfo));

         if (flags & GI_WIDTH)
            wiFlags |= BEGL_WindowInfoWidth;
         if (flags & GI_HEIGHT)
            wiFlags |= BEGL_WindowInfoHeight;
         if (flags & GI_FMT)
            wiFlags |= BEGL_WindowInfoFormat;
         if (flags & GI_SWAPCC)
            wiFlags |= BEGL_WindowInfoSwapChainCount;
         if (flags & GI_REFRESH)
            wiFlags |= BEGL_WindowInfoRefreshRate;

         ret = driverInterfaces->displayInterface->WindowGetInfo(driverInterfaces->displayInterface->context,
                                                                 win, wiFlags, &windowInfo);
         if (ret == BEGL_Success)
         {
            info->width = windowInfo.width;
            info->height = windowInfo.height;
            info->swapchain_count = windowInfo.swapchain_count;
            info->format = windowInfo.format;
            info->colorFormat = windowInfo.colorFormat;
            info->refreshRate = windowInfo.refreshRateMilliHertz;
         }
      }
   }
}

bool platform_supported_format(KHRN_IMAGE_FORMAT_T format)
{
   bool res;

   /* a platform is always required to provide RSO outputs */
   if (khrn_image_is_rso(format))
      res = true;
   else
   {
      BEGL_DriverInterfaces * driverInterfaces = BEGL_GetDriverInterfaces();

      if ((driverInterfaces != NULL) &&
          (driverInterfaces->displayInterface != NULL) &&
          (driverInterfaces->displayInterface->IsSupported != NULL))
      {
         BEGL_BufferFormat bufferFormat;

         format = khrn_image_no_colorspace_format(format);

         switch (format)
         {
         case ABGR_8888_TF:      bufferFormat = BEGL_BufferFormat_eA8B8G8R8_TFormat;      break;
         case XBGR_8888_TF:      bufferFormat = BEGL_BufferFormat_eX8B8G8R8_TFormat;      break;
         case RGB_565_TF:        bufferFormat = BEGL_BufferFormat_eR5G6B5_TFormat;        break;
         default:                bufferFormat = BEGL_BufferFormat_INVALID;                break;
         }

         if (driverInterfaces->displayInterface->IsSupported(driverInterfaces->displayInterface->context, bufferFormat) == BEGL_Success)
            res = true;
         else
            res = false;
      }
      else
         res = false;
   }

   return res;
}

bool platform_get_buffersettings(void * opaque_buffer_handle, BEGL_BufferSettings * bufferSettings)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_Error ret = BEGL_Fail;

   if ((driverInterfaces != NULL) &&
       (driverInterfaces->displayInterface != NULL) &&
       (driverInterfaces->displayInterface->BufferGetCreateSettings != NULL) &&
       (bufferSettings != NULL))
   {
      ret = driverInterfaces->displayInterface->BufferGetCreateSettings(
         driverInterfaces->displayInterface->context,
         opaque_buffer_handle,
         bufferSettings);
   }

   return (ret == BEGL_Success);
}

bool platform_lock_buffer(void * opaque_buffer_handle, BEGL_WindowState *windowState, int *fence)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();
   BEGL_Error ret = BEGL_Fail;

   if ((driverInterfaces != NULL) &&
       (driverInterfaces->displayInterface != NULL) &&
       (driverInterfaces->displayInterface->BufferAccess != NULL))
   {
      BEGL_BufferAccessState bufferAccess;
      bufferAccess.buffer = opaque_buffer_handle;
      if (windowState != NULL)
         bufferAccess.windowState = *windowState;
      bufferAccess.fence = -1;

      ret = driverInterfaces->displayInterface->BufferAccess(driverInterfaces->displayInterface->context,
                                                             &bufferAccess);

      *fence = bufferAccess.fence;
   }

   return (ret == BEGL_Success);
}

bool platform_get_pixmap_info(EGLNativePixmapType pixmap, KHRN_IMAGE_WRAP_T *image)
{
   MEM_HANDLE_T handle = MEM_INVALID_HANDLE;
   BEGL_BufferSettings bufferSettings;
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   if ((driverInterfaces != NULL) &&
       (driverInterfaces->displayInterface != NULL) &&
       (driverInterfaces->displayInterface->BufferGetCreateSettings != NULL))
   {
      BEGL_Error ret;
      ret = driverInterfaces->displayInterface->BufferGetCreateSettings(driverInterfaces->displayInterface->context,
         (BEGL_BufferHandle)pixmap, &bufferSettings);

      if (ret == BEGL_Success)
      {
         uint32_t w, h;
         uint32_t align = 0;
         KHRN_IMAGE_FORMAT_T format;
         uint32_t flags;

         w = bufferSettings.width;
         h = bufferSettings.height;

         flags = IMAGE_FLAG_RENDER_TARGET | IMAGE_FLAG_DISPLAY;

         if (bufferSettings.format == BEGL_BufferFormat_eR5G6B5 ||
             bufferSettings.format == BEGL_BufferFormat_eR5G6B5_Texture)
         {
            format = RGB_565_RSO;
         }
         else if (bufferSettings.format == BEGL_BufferFormat_eYUV422_Texture ||
                  bufferSettings.format == BEGL_BufferFormat_eVUY224_Texture)
         {
            format = YUV_422_RSO;
            flags =  0;
         }
         else if (bufferSettings.format == BEGL_BufferFormat_eA8B8G8R8_Texture ||
                  bufferSettings.format == BEGL_BufferFormat_eR8G8B8A8_Texture)
         {
            /* Internally, both of these are the same - any endian switching is handled by the platform layer,
               not by the driver. e.g. A8B8G8R8 and R8G8B8A8 map to different (reversed) Nexus surface formats so the
               blitter will do the swapping, since we can't do it in our h/w. */
            format = ABGR_8888_RSO;
            flags =  IMAGE_FLAG_TEXTURE | IMAGE_FLAG_RSO_TEXTURE | IMAGE_FLAG_RENDER_TARGET;
         }
         else if (bufferSettings.format == BEGL_BufferFormat_eA8R8G8B8_Texture ||
                  bufferSettings.format == BEGL_BufferFormat_eB8G8R8A8_Texture)
         {
            /* Internally, both of these are the same - any endian switching is handled by the platform layer,
               not by the driver. e.g. A8B8G8R8 and R8G8B8A8 map to different (reversed) Nexus surface formats so the
               blitter will do the swapping, since we can't do it in our h/w. */
            format = ARGB_8888_RSO;
            flags =  IMAGE_FLAG_TEXTURE | IMAGE_FLAG_RSO_TEXTURE | IMAGE_FLAG_RENDER_TARGET;
         }
         else
         {
            format = ABGR_8888_RSO;
         }

         if (bufferSettings.openvg)
         {
            format = abstract_colorformat_to_format(format, bufferSettings.colorFormat);
            format = khrn_image_to_openvg_format(format);
         }

         KHRN_IMAGE_CREATE_FLAG_T image_create_flags =
            ((flags & IMAGE_FLAG_TEXTURE) ? IMAGE_CREATE_FLAG_TEXTURE : 0) |
            ((flags & IMAGE_FLAG_RSO_TEXTURE) ? IMAGE_CREATE_FLAG_RSO_TEXTURE : 0) |
            ((flags & IMAGE_FLAG_RENDER_TARGET) ? IMAGE_CREATE_FLAG_RENDER_TARGET : 0) |
            ((flags & IMAGE_FLAG_DISPLAY) ? IMAGE_CREATE_FLAG_DISPLAY : 0);

         khrn_image_platform_fudge(&format, &w, &h, &align, image_create_flags);

         /* Check that this looks like a validly sized pixmap buffer */
         if (khrn_image_is_rso(format) &&
            (bufferSettings.pitchBytes == khrn_image_get_stride(format, w)) &&
            (bufferSettings.alignment == align))
         {
            khrn_image_interlock_wrap(image, format, bufferSettings.width, bufferSettings.height,
               bufferSettings.pitchBytes, flags, bufferSettings.secure, bufferSettings.cachedAddr, NULL);
            return true;
         }
      }
   }

   /* EGL_BAD_NATIVE_PIXMAP */
   return false;
}

bool platform_match_pixmap_api_support(EGLNativePixmapType pixmap, uint32_t api_support)
{
   UNUSED(pixmap);
   UNUSED(api_support);

   return true;
}

uint64_t khronos_platform_get_process_id(void)
{
   return 0;
}

void platform_client_lock(void)
{
   platform_mutex_acquire(&client_mutex);
}

void platform_client_release(void)
{
   platform_mutex_release(&client_mutex);
}

void platform_init_rpc(struct CLIENT_THREAD_STATE *state) { UNUSED(state); }
void platform_term_rpc(struct CLIENT_THREAD_STATE *state) { UNUSED(state); }
void platform_maybe_free_process(void) { }
void platform_destroy_winhandle(void *a, uint32_t b) { UNUSED(a); UNUSED(b); }

void platform_surface_update(uint32_t handle) { UNUSED(handle); }
void egl_gce_win_change_image(void) { UNREACHABLE(); }


void platform_retrieve_pixmap_completed(EGLNativePixmapType pixmap) { UNUSED(pixmap); UNREACHABLE(); }
void platform_send_pixmap_completed(EGLNativePixmapType pixmap) { UNUSED(pixmap); UNREACHABLE(); }
uint32_t platform_memcmp(const void * aLeft, const void * aRight, size_t aLen) { return khrn_par_memcmp(aLeft, aRight, aLen); }
void platform_memcpy(void * aTrg, const void * aSrc, size_t aLength) { khrn_par_memcpy(aTrg, aSrc, aLength); }

bool khrn_platform_decode_native(EGLClientBuffer buffer,
                                 uint32_t *w,
                                 uint32_t *h,
                                 uint32_t *stride,
                                 KHRN_IMAGE_FORMAT_T *format,
                                 uint32_t *offset,
                                 void **p)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   if ((driverInterfaces != NULL) &&
       (driverInterfaces->displayInterface != NULL) &&
       (driverInterfaces->displayInterface->DecodeNativeFormat != NULL))
   {
      BEGL_Error res;
      BEGL_BufferSettings settings;

      res = driverInterfaces->displayInterface->DecodeNativeFormat(driverInterfaces->displayInterface->context,
         (void *)buffer,
         &settings);

      if (res == BEGL_Success)
      {
         *w = settings.width;
         *h = settings.height;
         *stride = settings.pitchBytes;
         *offset = settings.physOffset;
         *p = settings.cachedAddr;

         KHRN_IMAGE_FORMAT_T f = IMAGE_FORMAT_INVALID;
         switch (settings.format)
         {
         case BEGL_BufferFormat_eA8B8G8R8:         f = ABGR_8888_RSO;   break;
         case BEGL_BufferFormat_eX8B8G8R8:         f = XBGR_8888_RSO;   break;
         case BEGL_BufferFormat_eR5G6B5:           f = RGB_565_RSO;     break;
         case BEGL_BufferFormat_eYV12_Texture:     f = YV12_RSO;        break;
         case BEGL_BufferFormat_eA8B8G8R8_TFormat: f = ABGR_8888_TF;    break;
         case BEGL_BufferFormat_eX8B8G8R8_TFormat: f = XBGR_8888_TF;    break;
         case BEGL_BufferFormat_eR5G6B5_TFormat:   f = RGB_565_TF;      break;
         case BEGL_BufferFormat_eR5G5B5A1_TFormat: f = RGBA_5551_TF;    break;
         case BEGL_BufferFormat_eR4G4B4A4_TFormat: f = RGBA_4444_TF;    break;

         default:
            break;
         }

         if (khrn_image_prefer_lt(f, *w, *h))
            *format = khrn_image_to_lt_format(f);
         else
            *format = f;

         return true;
      }
   }

   return false;
}

void khrn_platform_image_wrap_term(MEM_HANDLE_T handle)
{
   EGLClientBuffer buffer = (EGLClientBuffer)mem_get_param(handle);
   khrn_platform_dec_refcnt(buffer);
}

MEM_HANDLE_T khrn_platform_image_wrap(EGLClientBuffer buffer)
{
   MEM_HANDLE_T handle;
   void * p;
   uint32_t offset, w, h, stride;
   KHRN_IMAGE_FORMAT_T format = ABGR_8888_RSO;

   khrn_platform_decode_native(buffer, &w, &h, &stride, &format, &offset, &p);

   khrn_platform_inc_refcnt(buffer);

   handle = mem_wrap(p, offset,
      h * stride,
      1, MEM_FLAG_DIRECT,
      "EGL_NATIVE_BUFFER_ANDROID");

   mem_set_term(handle, khrn_platform_image_wrap_term, (void *)buffer);

   MEM_HANDLE_T res = khrn_image_create_from_storage(format,
      w, h, stride,
      MEM_INVALID_HANDLE, handle, 0, IMAGE_CREATE_FLAG_TEXTURE | IMAGE_CREATE_FLAG_RSO_TEXTURE, false);

   mem_release(handle);

   return res;
}

MEM_HANDLE_T khrn_platform_image_new(EGLClientBuffer buffer, EGLenum target, EGLint *error)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   if ((driverInterfaces != NULL) &&
      (driverInterfaces->displayInterface != NULL) &&
      (driverInterfaces->displayInterface->SurfaceVerifyImageTarget != NULL) &&
      (driverInterfaces->displayInterface->SurfaceVerifyImageTarget(driverInterfaces->displayInterface->context, (void *)buffer, target) != BEGL_Success))
   {
      *error = EGL_BAD_PARAMETER;
      return MEM_HANDLE_INVALID;
   }

   return khrn_platform_image_wrap(buffer);
}

EGLDisplay khrn_platform_set_display_id(EGLNativeDisplayType display_id)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   if ((driverInterfaces != NULL) &&
       (driverInterfaces->displayInterface != NULL) &&
       (driverInterfaces->displayInterface->SetDisplayID != NULL))
   {
      EGLDisplay egl_display_id;
      BEGL_Error res;

      res = driverInterfaces->displayInterface->SetDisplayID(driverInterfaces->displayInterface->context,
         (uint32_t)display_id,
         (uint32_t *)&egl_display_id);
      if (res != BEGL_Success)
         return EGL_NO_DISPLAY;
      return egl_display_id;
   }

   if (display_id == EGL_DEFAULT_DISPLAY)
      return (EGLDisplay)1;
   else
      return EGL_NO_DISPLAY;
}

void khrn_platform_inc_refcnt(EGLClientBuffer buffer)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   if ((driverInterfaces != NULL) &&
      (driverInterfaces->displayInterface != NULL) &&
      (driverInterfaces->displayInterface->SurfaceChangeRefCount != NULL))
   {
      driverInterfaces->displayInterface->SurfaceChangeRefCount(driverInterfaces->displayInterface->context,
         (void *)buffer, BEGL_Increment);
   }
}

void khrn_platform_dec_refcnt(EGLClientBuffer buffer)
{
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   if ((driverInterfaces != NULL) &&
      (driverInterfaces->displayInterface != NULL) &&
      (driverInterfaces->displayInterface->SurfaceChangeRefCount != NULL))
   {
      driverInterfaces->displayInterface->SurfaceChangeRefCount(driverInterfaces->displayInterface->context,
         (void *)buffer, BEGL_Decrement);
   }
}

uint32_t khrn_platform_get_window_position(EGLNativeWindowType win)
{
   UNUSED(win);
   return 0;
}

uint32_t khrn_platform_get_color_format(KHRN_IMAGE_FORMAT_T format)
{
   uint32_t res = 0;
   BEGL_DriverInterfaces   *driverInterfaces = BEGL_GetDriverInterfaces();

   /* remove any colorspace flags */
   format = khrn_image_no_colorspace_format(format);

   if (khrn_image_is_tformat(format) || khrn_image_is_rso(format))
   {
      if ((driverInterfaces != NULL) &&
         (driverInterfaces->displayInterface != NULL) &&
         (driverInterfaces->displayInterface->GetNativeFormat != NULL))
      {
         BEGL_BufferFormat bufferFormat = BEGL_BufferFormat_INVALID;
         format = khrn_image_to_rso_format(format);
         if (format == ABGR_8888)
            bufferFormat = BEGL_BufferFormat_eA8B8G8R8;
         else if (format == XBGR_8888)
            bufferFormat = BEGL_BufferFormat_eX8B8G8R8;
         else if (format == RGB_565)
            bufferFormat = BEGL_BufferFormat_eR5G6B5;

         driverInterfaces->displayInterface->GetNativeFormat(driverInterfaces->displayInterface->context,
            bufferFormat, &res);
      }
   }

   return res;
}

void khrn_platform_release_pixmap_info(EGLNativePixmapType pixmap, KHRN_IMAGE_WRAP_T *image)
{
   UNUSED(pixmap);
   UNUSED(image);
   /* Nothing to do */
}

void khrn_platform_unbind_pixmap_from_egl_image(EGLImageKHR egl_image)
{
   UNUSED(egl_image);
   /* todo */
}
