/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Minimal EGL server, handling creation of buffers and contexts and "make
current". Most of the EGL state machine now lives on the client side.
=============================================================================*/
#define KHRN_LATE_COLOR_BUFFER_ALLOC

#include "middleware/khronos/glxx/glxx_buffer.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_parallel.h"
#include "interface/khronos/common/khrn_options.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/common/khrn_interlock.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/common/khrn_client.h"
extern void khdispatch_send_async(uint32_t command, uint64_t pid, uint32_t sem);
#include "middleware/khronos/glxx/glxx_texture.h"
#include "interface/khronos/common/khrn_int_ids.h"
#include "middleware/khronos/gl11/gl11_server.h"
#include "middleware/khronos/glxx/glxx_hw.h"
#include "middleware/khronos/gl20/gl20_server.h"
#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/gl20/gl20_shader.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#ifndef NO_OPENVG
#include "middleware/khronos/vg/vg_server.h"
#include "middleware/khronos/vg/vg_font.h"
#include "middleware/khronos/vg/vg_image.h"
#include "middleware/khronos/vg/vg_mask_layer.h"
#include "middleware/khronos/vg/vg_paint.h"
#include "middleware/khronos/vg/vg_path.h"
#include "middleware/khronos/vg/vg_ramp.h"
#include "middleware/khronos/vg/vg_hw.h"
#endif /* NO_OPENVG */
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/egl/egl_disp.h"
#include "middleware/khronos/common/2708/khrn_render_state_4.h"
#include "middleware/khronos/common/2708/khrn_copy_buffer_4.h"
#include "interface/khronos/egl/egl_client_surface.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/common/khrn_api_interposer.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/egl/egl_client_config.h"

#define LOGGING_GENERAL 0
static INLINE void logging_message(int x, ...) { UNUSED(x); }

#include "helpers/vc_image/vc_image.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "interface/khronos/egl/egl_int_config.h"

#if EGL_KHR_image
   #include "middleware/khronos/ext/egl_khr_image.h"
#endif

static BEGL_DriverInterfaces  s_driverInterfacesStruct;
static BEGL_DriverInterfaces *s_driverInterfaces = &s_driverInterfacesStruct;

static void server_process_attach(void);
static void server_process_detach(void);

/* register the driver interface */
EGLAPI void EGLAPIENTRY BEGL_RegisterDriverInterfaces(BEGL_DriverInterfaces *driverInterfaces)
{
   bool bad = false;
   bool term = false;

   /* Sanity check that the relevant driver interfaces are present */
   if (driverInterfaces == NULL)
      term = true;
   else
   {
      if (driverInterfaces->displayInterface == NULL ||
         driverInterfaces->hwInterface == NULL ||
         driverInterfaces->memInterface == NULL)
      {
         term = true;
      }
      else
      {
         /* Must have these */
         if (driverInterfaces->hwInterface->GetInfo == NULL ||
             driverInterfaces->hwInterface->SendJob == NULL ||
             driverInterfaces->hwInterface->GetNotification == NULL ||
             driverInterfaces->hwInterface->SendSync == NULL ||
             driverInterfaces->hwInterface->GetBinMemory == NULL ||
             driverInterfaces->displayInterface->BufferCreate == NULL ||
             driverInterfaces->displayInterface->BufferDestroy == NULL ||
             driverInterfaces->displayInterface->BufferDisplay == NULL ||
             driverInterfaces->displayInterface->BufferGetCreateSettings == NULL ||
             driverInterfaces->displayInterface->BufferGet == NULL ||
             driverInterfaces->displayInterface->BufferAccess == NULL ||
             driverInterfaces->displayInterface->WindowGetInfo == NULL ||
             driverInterfaces->displayInterface->WindowUndisplay == NULL)
            bad = true;

         if (driverInterfaces->displayInterface->WindowPlatformStateCreate != NULL &&
             driverInterfaces->displayInterface->WindowPlatformStateDestroy == NULL)
             bad = true;

         if (driverInterfaces->displayInterface->WindowPlatformStateCreate == NULL &&
            driverInterfaces->displayInterface->WindowPlatformStateDestroy != NULL)
            bad = true;

         if (bad)
         {
            printf("******* ERROR : BEGL_RegisterDriverInterfaces: Platform layer interfaces are incomplete or are an older version. ******\n");
            exit(1);
         }
         else
         {
            /* See if we already have a valid memory interface.
             * If not, we need to bring up the driver now.
             * If we do, and this is the same, we do nothing.
             * If we do, but this is different - try to close & restart driver */
            if (s_driverInterfaces->memInterface == NULL)
            {
               s_driverInterfacesStruct = *driverInterfaces;

               /* Now we can bring up the driver proper */
               server_process_attach();
               client_process_attach();
#ifndef NDEBUG
               {
                  GLubyte revStr[16];
                  glintGetCoreRevision_impl(16, revStr);
                  printf("VideoCore IV HW (V3D-%s)\n", revStr);
               }
#endif
            }
            else if (s_driverInterfaces->memInterface != driverInterfaces->memInterface)
            {
               /* Uh oh, memory interface has changed - better start again.
                  I wouldn't normally expect any applications to hit this path! */
               client_process_detach();
               server_process_detach();

               s_driverInterfacesStruct = *driverInterfaces;

               server_process_attach();
               client_process_attach();
            }
            else if (s_driverInterfaces->displayInterface != driverInterfaces->displayInterface)
            {
               /* display interface has changed, re-register the eglconfig types */
               if ((driverInterfaces != NULL) &&
                   (driverInterfaces->displayInterface != NULL) &&
                   (driverInterfaces->displayInterface->IsSupported != NULL) &&
                   (driverInterfaces->displayInterface->IsSupported(driverInterfaces->displayInterface, BEGL_BufferFormat_eA8B8G8R8_TFormat) == BEGL_Success))
                  /* install TFormat configs */
                  egl_config_install_configs(1);
               else
                  /* install RSO configs (default) */
                  egl_config_install_configs(0);

               s_driverInterfacesStruct = *driverInterfaces;
            }
               s_driverInterfacesStruct = *driverInterfaces;
         }
      }
   }

   if (term)
   {
      /* Time to take down the driver */
      client_process_detach();
      server_process_detach();

      memset(s_driverInterfaces, 0, sizeof(BEGL_DriverInterfaces));
   }
}

extern BEGL_DriverInterfaces* BEGLint_GetDriverInterfaces(void)
{
   return s_driverInterfaces;
}

EGLAPI BEGL_DriverInterfaces* EGLAPIENTRY BEGL_GetDriverInterfaces(void)
{
   return BEGLint_GetDriverInterfaces();
}

extern BEGL_BufferHandle BEGLint_PixmapCreateCompatiblePixmap(BEGL_PixmapInfo *pixmapInfo);
extern void BEGLint_intBufferGetRequirements(BEGL_PixmapInfo *bufferRequirements, BEGL_BufferSettings * bufferConstrainedRequirements);
extern void khrn_job_callback(void);

EGLAPI void EGLAPIENTRY BEGL_GetDefaultDriverInterfaces(BEGL_DriverInterfaces *driverInterfaces)
{
   memset(driverInterfaces, 0, sizeof(BEGL_DriverInterfaces));

   driverInterfaces->displayCallbacks.PixmapCreateCompatiblePixmap = BEGLint_PixmapCreateCompatiblePixmap;
   driverInterfaces->displayCallbacks.BufferGetRequirements = BEGLint_intBufferGetRequirements;

   driverInterfaces->hardwareCallbacks.JobCallback = khrn_job_callback;
}

/*
   void egl_server_surface_term(void *v, uint32_t size)

   Terminator for EGL_SERVER_SURFACE_T

   Preconditions:

   v is a valid pointer to a (possibly uninitialised or partially initialised*) object of type EGL_SERVER_SURFACE_T

   Postconditions:

   Frees any references to resources that are referred to by the object of type EGL_SERVER_SURFACE_T
*/

void egl_server_surface_term(void *v, uint32_t size)
{
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)v;

   int i;
   BEGL_WindowState  *windowState = NULL;

   if (surface != NULL)
   {
      UNUSED_NDEBUG(size);
      if (surface->display_thread_state.platform_state != NULL)
      {
         khrn_hw_common_finish();
         egl_disp_destroy_display_thread(&surface->display_thread_state);
      }

      vcos_assert(size == sizeof(EGL_SERVER_SURFACE_T));

      if (surface->mh_color[0] != MEM_INVALID_HANDLE) {
         ((KHRN_IMAGE_T *)mem_lock(surface->mh_color[0], NULL))->flags &= ~IMAGE_FLAG_BOUND_CLIENTBUFFER;
         mem_unlock(surface->mh_color[0]);
      }

      for (i = 0; i < EGL_MAX_BUFFERS; i++)
      {
         if (surface->mh_color[i] != MEM_INVALID_HANDLE)
         {
            KHRN_IMAGE_T *image = mem_lock(surface->mh_color[i], NULL);
            if (image != NULL && image->window_state != NULL && image->window_state->window == (BEGL_WindowHandle)surface->win)
            {
               vcos_assert(windowState == NULL || windowState == image->window_state);
               windowState = image->window_state;
            }
            mem_unlock(surface->mh_color[i]);
         }
         MEM_ASSIGN(surface->mh_color[i], MEM_INVALID_HANDLE);
      }

      MEM_ASSIGN(surface->mh_depth, MEM_INVALID_HANDLE);
      MEM_ASSIGN(surface->mh_color_multi, MEM_INVALID_HANDLE);
      MEM_ASSIGN(surface->mh_ds_multi, MEM_INVALID_HANDLE);
      MEM_ASSIGN(surface->mh_mask, MEM_INVALID_HANDLE);
      MEM_ASSIGN(surface->mh_preserve, MEM_INVALID_HANDLE);

      MEM_ASSIGN(surface->mh_bound_texture, MEM_INVALID_HANDLE);

      if (windowState != NULL)
      {
         BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();
         if (driverInterfaces != NULL &&
             driverInterfaces->displayInterface != NULL &&
             driverInterfaces->displayInterface->WindowPlatformStateDestroy != NULL)
         {
            driverInterfaces->displayInterface->WindowPlatformStateDestroy(s_driverInterfaces->displayInterface->context,
               windowState->platformState);
         }
         free((void *)windowState);
      }
   }
}

EGL_SERVER_STATE_T egl_server_state;

static uint32_t override_size = 0;
static uint32_t override_handles_size = 0;

extern void khrn_init_api_interposer(void);

static void server_process_attach(void)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   khrn_init_options();
   khrn_init_api_interposer();

   memset(state, MEM_HANDLE_INVALID, sizeof(EGL_SERVER_STATE_T));       /* do not remove, sets all handles to invalid */
   memset(&khrn_workarounds, (int)true, sizeof(khrn_workarounds));      /* ensure all workarounds are on until explicitly determined */

   khrn_init_workarounds();

   /* TODO : ADD DEINIT or we leak */
   vcos_demand(VCOS_SUCCESS == vcos_init());

   if ((s_driverInterfaces != NULL) && (s_driverInterfaces->memInterface != NULL))
      mem_init(s_driverInterfaces->memInterface);
   else
      return;

   /* now we have a handle to the platform layer, grab the correct format for the egl context */
   if ((s_driverInterfaces != NULL) &&
       (s_driverInterfaces->displayInterface != NULL) &&
       (s_driverInterfaces->displayInterface->IsSupported != NULL) &&
       (s_driverInterfaces->displayInterface->IsSupported(s_driverInterfaces->displayInterface, BEGL_BufferFormat_eA8B8G8R8_TFormat) == BEGL_Success))
      /* install TFormat configs */
      egl_config_install_configs(1);
   else
      /* install RSO configs (default) */
      egl_config_install_configs(0);

   if ((s_driverInterfaces != NULL) &&
      (s_driverInterfaces->displayInterface != NULL) &&
      (s_driverInterfaces->displayInterface->DefaultOrientation != NULL) &&
      (s_driverInterfaces->displayInterface->DefaultOrientation(s_driverInterfaces->displayInterface) == BEGL_Success))
   {
      /* reset any workarounds for orientation, reset to default (bottom up) */
      khrn_workarounds.FB_BOTTOM_UP = false;
      khrn_workarounds.FB_TOP_DOWN = false;
   }

   state->glversion = 0;
   state->next_context = 1;
   state->next_surface = 1;
   state->next_eglimage = 1;
#if EGL_KHR_sync
   state->next_sync = 1;
#endif

   state->locked_glcontext = NULL;
   state->locked_vgcontext = NULL;

   verify(khrn_map_init(&state->glcontexts, 64)); /* todo: handle failure */
   verify(khrn_map_init(&state->vgcontexts, 64)); /* todo: handle failure */
   verify(khrn_map_init(&state->surfaces, 64)); /* todo: handle failure */
   verify(khrn_map_init(&state->eglimages, 64)); /* todo: handle failure */

#if EGL_KHR_sync
   verify(khrn_map_init(&state->syncs, 8)); /* todo: handle failure */
#endif

   khrn_init_thread_pool();

   egl_server_platform_init();

   /* TODO : pass the "master event" as an argument here, so its clear when it can be deleted */
   /* or better - make the module create its own resources */
   khrn_hw_common_init(); /* todo: handle failure */
}

static void server_process_detach(void)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   khrn_map_term(&state->surfaces);
   khrn_map_term(&state->glcontexts);
   khrn_map_term(&state->vgcontexts);
   khrn_map_term(&state->eglimages);

#if EGL_KHR_sync
   khrn_map_term(&state->syncs);
#endif

   egl_server_shutdown();
}

/*
 * Normally only called when egl_server_is_empty is true, but if it isn't then
 * we free all our stuff.
 * This shuts down everything set up by egl_server_startup_hack. At present this
 * includes the entire memory system.
 */
void egl_server_shutdown(void)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   /*
      we have to be careful here about shutting things down in the right order!

      the first thing we do is flush all queued stuff and wait for it to finish
      (we might still have queued stuff as we are quite lazy about flushing it)

      (this doesn't include display stuff: we'll wait for each egl surface
      individually before it is destroyed)
   */

   khrn_hw_common_finish();

   /*
      now we destroy all egl objects
   */

   MEM_ASSIGN(state->glcontext, MEM_INVALID_HANDLE);
   MEM_ASSIGN(state->gldrawsurface, MEM_INVALID_HANDLE);
   MEM_ASSIGN(state->glreadsurface, MEM_INVALID_HANDLE);
   MEM_ASSIGN(state->vgcontext, MEM_INVALID_HANDLE);
   MEM_ASSIGN(state->vgsurface, MEM_INVALID_HANDLE);

#ifndef NO_OPENVG
   /*
      vg might be marked for termination...
   */

   vg_maybe_term();
#endif

   /*
      at this point, the system is completely idle and empty

      shutdown everything from top to bottom...
   */

   glxx_hw_term();
   khrn_hw_common_term();
   khrn_term_thread_pool();

   /*
      finally, tear down the heap
   */

   mem_term();
}

void egl_server_unlock()
{
   GL11_FORCE_UNLOCK_SERVER_STATE();
   GL20_FORCE_UNLOCK_SERVER_STATE();
#ifndef NO_OPENVG
   vg_unlock();
#endif /* NO_OPENVG */
}

/*
   EGL_SURFACE_ID_T create_surface_internal(
      uint32_t win,
      uint32_t buffers,
      uint32_t width,
      uint32_t height,
      uint32_t swapchainc,
      KHRN_IMAGE_FORMAT_T colorformat,
      KHRN_IMAGE_FORMAT_T depthstencilformat,
      KHRN_IMAGE_FORMAT_T maskformat,
      uint32_t multisample,
      uint32_t mipmap,
      uint32_t config_depth_bits,
      uint32_t config_stencil_bits,
      MEM_HANDLE_T vgimagehandle,
      MEM_HANDLE_T hpixmapimage,
      uint32_t type)

   Preconditions:
      win is a valid server-side window handle or PLATFORM_WIN_NONE
      1 <= buffers <= EGL_MAX_BUFFERS
      0 < width <= EGL_CONFIG_MAX_WIDTH
      0 < height <= EGL_CONFIG_MAX_HEIGHT
      swapchainc is the number of external buffers in the swapchain
      colorformat is a hardware framebuffer-supported uncompressed color format
      depthstencilformat is a hardware framebuffer-supported depth and/or stencil format, or IMAGE_FORMAT_INVALID
      maskformat is a hardware framebuffer-supported mask format, or IMAGE_FORMAT_INVALID
      multisample in {0,1}
      mipmap in {0,1}
      config_depth_bits is number of depth bits in the selected config
      config_stencil_bits is number of stencil bits in the selected config
      vgimagehandle is MEM_INVALID_HANDLE or a handle to a VG_IMAGE
      hpixmapimage is MEM_INVALID_HANDLE or a handle to a KHRN_IMAGE_T

      Exactly one of the following is true:
          win && !mipmap && !vgimagehandle && !hpixmapimage
         !win && !mipmap && !vgimagehandle && !hpixmapimage && buffers == 1
         !win &&  mipmap && !vgimagehandle && !hpixmapimage
         !win && !mipmap &&  vgimagehandle && !hpixmapimage && buffers == 1
         !win && !mipmap && !vgimagehandle &&  hpixmapimage && buffers == 1
      where win, vgimagehandle, hpixmapimage are "true" if they are not equal to EGL_PLATFORM_WIN_NONE, MEM_INVALID_HANDLE, MEM_INVALID_HANDLE

   Postconditions:

      Return value is 0 or
*/

static EGL_SURFACE_ID_T create_surface_internal(
   uint32_t win,
   uint32_t buffers,
   uint32_t width,
   uint32_t height,
   uint32_t swapchainc,
   KHRN_IMAGE_FORMAT_T colorformat,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   KHRN_IMAGE_FORMAT_T multisampleformat,
   uint32_t mipmap,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits,
   MEM_HANDLE_T vgimagehandle,
   MEM_HANDLE_T hpixmapimage,      /*floating KHRN_IMAGE_T*/
   uint32_t type)
{
   EGL_SERVER_SURFACE_T *surface;
   /* todo: these flags aren't entirely correct... */
   KHRN_IMAGE_CREATE_FLAG_T color_image_create_flags = (KHRN_IMAGE_CREATE_FLAG_T)(
      /* clear, but also mark as invalid */
      IMAGE_CREATE_FLAG_ONE | IMAGE_CREATE_FLAG_INVALID |
      /* if we're a window surface, might rotate/display */
      ((win != EGL_PLATFORM_WIN_NONE) ? (IMAGE_CREATE_FLAG_PAD_ROTATE | IMAGE_CREATE_FLAG_DISPLAY) : 0) |
      /* support rendering to if format is ok (don't force the format) */
      (khrn_image_is_ok_for_render_target(colorformat, false) ? (IMAGE_CREATE_FLAG_RENDER_TARGET |
      /* also support use as texture if not window surface (todo: this is hacky, and i don't think it's right) */
      ((win == EGL_PLATFORM_WIN_NONE) ? IMAGE_CREATE_FLAG_TEXTURE : 0)) : 0));
   KHRN_IMAGE_CREATE_FLAG_T image_create_flags = (KHRN_IMAGE_CREATE_FLAG_T)(
      /* clear, but also mark as invalid */
      IMAGE_CREATE_FLAG_ONE | IMAGE_CREATE_FLAG_INVALID |
      /* if we're a window surface, might rotate */
      ((win != EGL_PLATFORM_WIN_NONE) ? IMAGE_CREATE_FLAG_PAD_ROTATE : 0) |
      /* aux buffers are just there for rendering... */
      IMAGE_CREATE_FLAG_RENDER_TARGET);
   MEM_HANDLE_T hcommon_storage = MEM_INVALID_HANDLE;
   EGL_SURFACE_ID_T result = 0;

   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   MEM_HANDLE_T shandle = MEM_ALLOC_STRUCT_EX(EGL_SERVER_SURFACE_T, MEM_COMPACT_DISCARD);                   // check, egl_server_surface_term
   if (shandle == MEM_INVALID_HANDLE)
      return result;

   mem_set_term(shandle, egl_server_surface_term);

   surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);

   if (mipmap) {
      vcos_assert(width > 0 && height > 0);
      vcos_assert(width <= EGL_CONFIG_MAX_WIDTH && height <= EGL_CONFIG_MAX_HEIGHT);
      buffers = _max(_msb(width), _msb(height)) + 1;
   }

   vcos_assert(buffers > 0);
   vcos_assert(buffers <= EGL_MAX_BUFFERS);
   vcos_assert(!mipmap || win == EGL_PLATFORM_WIN_NONE);
   vcos_assert(mipmap || win != EGL_PLATFORM_WIN_NONE || buffers == 1);

   surface->name = state->next_surface;
   surface->mipmap = mipmap;
   surface->win = win;
   surface->swapchainc = swapchainc;
   surface->buffers = buffers;
   surface->back_buffer_index = 0;
   vcos_assert(surface->mh_bound_texture == MEM_INVALID_HANDLE);
   surface->swap_interval = 1;
   surface->pid = state->pid;
   surface->config_depth_bits = config_depth_bits;
   surface->config_stencil_bits = config_stencil_bits;

   if (hpixmapimage != MEM_INVALID_HANDLE) {
      // Can't combine pixmap wrapping with any of the following features
      vcos_assert(buffers == 1);
      vcos_assert(!mipmap);
      vcos_assert(win == EGL_PLATFORM_WIN_NONE);
#ifndef NO_OPENVG
      vcos_assert(vgimagehandle == MEM_INVALID_HANDLE);
#else
      UNUSED(vgimagehandle);
#endif /* NO_OPENVG */

      /*TODO: accept handle-based images as well as wrapped ones? */

      MEM_ASSIGN(surface->mh_color[0], hpixmapimage);
   } else {
      uint32_t i;
      uint32_t offset = 0;
      BEGL_WindowState *platformState = NULL;

      if (mipmap)
      {
         uint32_t size = 0;
         for (i = 0; i < buffers; i++)
         {
            uint32_t w,h;
            w = _max(width >> i, 1);
            h = _max(height >> i, 1);
            size += khrn_image_get_size(colorformat, w, h);
         }
         hcommon_storage = mem_alloc_ex(size, 4096, MEM_FLAG_DIRECT, "EGL_SERVER_SURFACE_T mipmapped storage", MEM_COMPACT_DISCARD);
         if (hcommon_storage == MEM_INVALID_HANDLE)
            goto final;
      }

      if (type == WINDOW)
      {
         if (!egl_server_platform_create_window_state(&platformState, win))
            goto final;

         egl_disp_create_display_thread(&surface->display_thread_state, platformState);
      }

      for (i = 0; i < buffers; i++) {
#ifndef NO_OPENVG
         if (i == 0 && vgimagehandle != MEM_INVALID_HANDLE) {
            VG_IMAGE_T *vgimage = (VG_IMAGE_T *)mem_lock(vgimagehandle, NULL);
            MEM_ASSIGN(surface->mh_color[i], vgimage->image);
            mem_unlock(vgimagehandle);
         } else
#endif /* NO_OPENVG */
         {
            uint32_t w,h;
            MEM_HANDLE_T himage;
            if (mipmap) {
               w = _max(width >> i, 1);
               h = _max(height >> i, 1);
            } else {
               w = width;
               h = height;
            }

            if (hcommon_storage != MEM_INVALID_HANDLE)
            {
               himage = khrn_image_create_from_storage(
                  colorformat, w, h,
                  khrn_image_get_stride(colorformat, w),
                  MEM_INVALID_HANDLE,
                  hcommon_storage,
                  offset,
                  (KHRN_IMAGE_CREATE_FLAG_T)(IMAGE_CREATE_FLAG_TEXTURE | IMAGE_CREATE_FLAG_RENDER_TARGET)); /* todo: are these flags right? should IMAGE_CREATE_FLAG_INVALID be included? */
               offset += khrn_image_get_size(colorformat, w, h);
            }
            else
            {
               /* We must only use the abstract API to make our buffer if the buffer is genuinely going to stay
                * an RSO buffer. We need to check whether that's actually the case. */
               KHRN_IMAGE_FORMAT_T final_format = colorformat;
               uint32_t            pw = w;
               uint32_t            ph = h;
               uint32_t            align = 0;

               khrn_image_platform_fudge(&final_format, &pw, &ph, &align, color_image_create_flags);

               if (platform_supported_format(final_format))
               {
                  KHRN_IMAGE_T *image;
                  BEGL_BufferUsage usage = BEGL_BufferUsage_eSwapChain0;
                  uint32_t pixmap;
                  EGLint error;

                  if (type == WINDOW)
                  {
                     if (i < 3)
                        usage = BEGL_BufferUsage_eSwapChain0 + i;
                     else
                        /* the platform layer only supports up to triple buffering ATM */
                        vcos_assert(0);
                  }
                  else
                     usage = BEGL_BufferUsage_ePixmap;

                  /* wrap surfaces, if possible, when creating a window */
                  if (type == WINDOW)
                  {
                     pixmap = egl_server_platform_get_buffer((EGLNativeWindowType)win, colorformat, w, h, color_image_create_flags,
                                                              platformState, usage);
                  }
                  else
                  {
                     ANDROID_LOGD("%s: swapchain_count = %d, type = %d\n", __FUNCTION__, swapchainc, type);

                     pixmap = egl_server_platform_create_buffer(colorformat, w, h, color_image_create_flags,
                                                                platformState, usage);
                  }
                  himage = egl_server_platform_create_pixmap_info(pixmap, &error);
                  /* Store the opaque buffer handle */
                  image = mem_lock(himage, NULL);
                  image->opaque_buffer_handle = (void*)pixmap;
                  image->window_state = platformState;

                  if (type == WINDOW)
                     image->display_thread_state = &surface->display_thread_state;

                  mem_unlock(himage);
               }
               else
               {
                  himage = khrn_image_create(colorformat, w, h, color_image_create_flags);
               }
            }
            if (himage == MEM_INVALID_HANDLE)
               goto final;

            MEM_ASSIGN(surface->mh_color[i], himage);
            mem_release(himage);
         }
      }
   }

   /*
       "The contents of the depth and stencil buffers may not be preserved when rendering
       an OpenGL ES texture to the pbuffer and switching which image of the
       texture is rendered to (e.g., switching from rendering one mipmap level to rendering
       another)."
       So in the mipmapped case, we have just one depth/stencil buffer which is
       big enough for the largest mipmap but which we reuse for all mipmaps.
   */

   if (depthstencilformat != IMAGE_FORMAT_INVALID) {
      MEM_HANDLE_T himage;
      if (multisampleformat != IMAGE_FORMAT_INVALID)
      {
         himage = khrn_image_create(depthstencilformat, 2*width, 2*height,
            (KHRN_IMAGE_CREATE_FLAG_T)(image_create_flags | IMAGE_CREATE_FLAG_NO_STORAGE));
      }
      else
      {
         himage = khrn_image_create(depthstencilformat, width, height,
            (KHRN_IMAGE_CREATE_FLAG_T)(image_create_flags | IMAGE_CREATE_FLAG_NO_STORAGE));
      }

      if (himage == MEM_INVALID_HANDLE)
         goto final;

      MEM_ASSIGN(surface->mh_depth, himage);
      mem_release(himage);
   } else
      vcos_assert(surface->mh_depth == MEM_INVALID_HANDLE);

   if (multisampleformat != IMAGE_FORMAT_INVALID)
   {
      bool color = false;
      bool depth = false;

      if (multisampleformat == DEPTH_COL_64_TLBD)
      {
         color = true;
         depth = true;
      }
      else if (multisampleformat == COL_32_TLBD)
      {
         color = true;
         depth = false;
      }
      else // DEPTH_32_TLBD
      {
         color = false;
         depth = true;
      }

      if (color)
      {
         MEM_HANDLE_T himage = khrn_image_create(COL_32_TLBD, 2*width, 2*height,
            (KHRN_IMAGE_CREATE_FLAG_T)(image_create_flags | IMAGE_CREATE_FLAG_NO_STORAGE)
            );
         if (himage == MEM_INVALID_HANDLE)
            goto final;

         MEM_ASSIGN(surface->mh_color_multi, himage);
         mem_release(himage);
      }

      if (depth)
      {
         MEM_HANDLE_T himage = khrn_image_create(DEPTH_32_TLBD, 2*width, 2*height,
            (KHRN_IMAGE_CREATE_FLAG_T)(image_create_flags | IMAGE_CREATE_FLAG_NO_STORAGE)
            );
         if (himage == MEM_INVALID_HANDLE)
            goto final;

         MEM_ASSIGN(surface->mh_ds_multi, himage);
         mem_release(himage);
      }
   }
   else
   {
      vcos_assert(surface->mh_color_multi == MEM_INVALID_HANDLE);
      vcos_assert(surface->mh_ds_multi == MEM_INVALID_HANDLE);
   }

   if (maskformat != IMAGE_FORMAT_INVALID) {
      MEM_HANDLE_T himage = khrn_image_create(maskformat, width, height, image_create_flags);
      if (himage == MEM_INVALID_HANDLE)
         goto final;

      MEM_ASSIGN(surface->mh_mask, himage);
      mem_release(himage);
   } else
      vcos_assert(surface->mh_mask == MEM_INVALID_HANDLE);

   if (khrn_map_insert(&state->surfaces, state->next_surface, shandle))
      result = state->next_surface++;

final:
   mem_unlock(shandle);
   mem_release(shandle);
   if (hcommon_storage != MEM_INVALID_HANDLE)
      mem_release(hcommon_storage);

   return result;
}

static int euclidean_gcd(int a, int b)
{
   if (b > a) {
      int t = a;
      a = b;
      b = t;
   }

   while (b != 0) {
      int m = a % b;
      a = b;
      b = m;
   }

   return a;
}

/*
   int eglIntCreateSurface_impl(
   uint32_t win,
   uint32_t buffers,
   uint32_t width,
   uint32_t height,
   KHRN_IMAGE_FORMAT_T colorformat,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   uint32_t largest_pbuffer,
   uint32_t mipmap,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits,
   uint32_t type,
   uint32_t *results)

   Implementation notes:

   If largest_pbuffer is true, we try creating surfaces of various different
   sizes (with the correct aspect ratio) until the allocation succeeds.
   Otherwise we just attempt to create one of the correct size (create_surface_internal
   does the actual work).

   Preconditions:
      win is a valid server-side window handle or PLATFORM_WIN_NONE
      1 <= buffers <= EGL_MAX_BUFFERS
      0 < width, 0 < height
      If !largest_pbuffer then width <= EGL_CONFIG_MAX_WIDTH, height <= EGL_CONFIG_MAX_HEIGHT
      colorformat is a hardware framebuffer-supported uncompressed color format
      depthstencilformat is a hardware framebuffer-supported depth and/or stencil format, or IMAGE_FORMAT_INVALID
      maskformat is a hardware framebuffer-supported mask format, or IMAGE_FORMAT_INVALID
      multisampleformat is a hardware framebuffer-supported multisample color format or IMAGE_FORMAT_INVALID
      largest_pbuffer in {0,1}
      mipmap in {0,1}
      config_depth_bits is number of depth bits in the selected config
      config_stencil_bits is number of stencil bits in the selected config
      results is a valid pointer to three elements

   Postconditions:
      Return value is 3 (i.e. the number of items returned)
      On success:
         results[0] is the width of the surface
         results[1] is the height of the surface
         results[2] is the server-side surface handle and is nonzero
      On failure:
         results[2] == 0
*/

int eglIntCreateSurface_impl(
   uint32_t win,
   uint32_t buffers,
   uint32_t width,
   uint32_t height,
   uint32_t swapchainc,
   KHRN_IMAGE_FORMAT_T colorformat,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   KHRN_IMAGE_FORMAT_T multisampleformat,
   uint32_t largest_pbuffer,
   uint32_t mipmap,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits,
   uint32_t type,
   uint32_t *results)
{
   uint32_t max;
   uint32_t i;
   EGL_SURFACE_ID_T surface;

   if (largest_pbuffer && width && height) {
      uint32_t gcd = euclidean_gcd(width, height);

      width /= gcd;
      height /= gcd;

      max = _min(_min(EGL_CONFIG_MAX_WIDTH / width, EGL_CONFIG_MAX_HEIGHT / height), gcd);
   } else
      max = 1;

   surface = 0;

   for (i = max; i > 0 && !surface; i--) {
      surface = create_surface_internal(win, buffers, width*i, height*i, swapchainc, colorformat,
                                        depthstencilformat, maskformat, multisampleformat,
                                        mipmap, config_depth_bits, config_stencil_bits, MEM_INVALID_HANDLE,
                                        MEM_INVALID_HANDLE, type);

      results[0] = width*i;
      results[1] = height*i;
   }

   results[2] = surface;

   return 3;
}

#ifndef NO_OPENVG

/*
   return true if vgimage_format is compatible with config_format, ie the pixel
   format (ignoring premultiplied/linear flags) is the same
*/

static bool compatible_formats(KHRN_IMAGE_FORMAT_T config_format, KHRN_IMAGE_FORMAT_T vgimage_format)
{
   return (config_format & ~(IMAGE_FORMAT_MEM_LAYOUT_MASK | IMAGE_FORMAT_PRE | IMAGE_FORMAT_LIN | IMAGE_FORMAT_OVG)) ==
      (vgimage_format & ~(IMAGE_FORMAT_MEM_LAYOUT_MASK | IMAGE_FORMAT_PRE | IMAGE_FORMAT_LIN | IMAGE_FORMAT_OVG));
}

/*
   results
   0  EGL_SURFACE_ID_T  Handle to new surface (0 on failure)
   1  EGLInt            Error code
   2  EGLInt            Width
   3  EGLInt            Height
   4  KHRN_IMAGE_FORMAT_T    Format (may differ from the requested one in PRE or LIN)

   We always return 5 so that the RPC knows to transmit 5 return values.
*/

int eglIntCreatePbufferFromVGImage_impl(
   VGImage vg_handle,
   KHRN_IMAGE_FORMAT_T colorformat,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   KHRN_IMAGE_FORMAT_T multisampleformat,
   uint32_t mipmap,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits,
   uint32_t *results)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   results[0] = 0;

   if (state->vgcontext != MEM_INVALID_HANDLE) {
      VG_SERVER_STATE_T *server = VG_LOCK_SERVER_STATE();
      MEM_HANDLE_T imagehandle = vg_get_image(server, vg_handle, (EGLint *)(results + 1) /* error code */);

      if (imagehandle != MEM_INVALID_HANDLE) {
         if (vg_is_image(imagehandle)) {
            /* at this point, if the pixel format of the image is ok for
             * rendering to, the whole image is ok for rendering to and has the
             * IMAGE_FLAG_RENDER_TARGET flag set. so if the compatible_formats
             * call below succeeds, we know the image is ok to use as the color
             * buffer of a surface (which is the whole point of this function) */

            VG_IMAGE_T *vgimage = (VG_IMAGE_T *)mem_lock(imagehandle, NULL);
            if (vg_image_leak(vgimage)) {
               KHRN_IMAGE_T *image = (KHRN_IMAGE_T *)mem_lock(vgimage->image, NULL);
               uint32_t width = vgimage->image_width;
               uint32_t height = vgimage->image_height;
               uint32_t buffers = mipmap ? _msb(_max(width, height)) + 1 : 1;
               KHRN_IMAGE_FORMAT_T actualformat = vgimage->image_format;

               if (!compatible_formats(colorformat, actualformat)) {
                  results[1] = EGL_BAD_MATCH;
               } else if (image->flags & (IMAGE_FLAG_BOUND_CLIENTBUFFER|IMAGE_FLAG_BOUND_EGLIMAGE)) {
                  /*
                   * TODO: we are forbidden from binding to a pbuffer when it's
                   * already bound to a different pbuffer.
                   * But what about binding to a pbuffer when it's already an EGLImage
                   * sibling?
                   */
                  results[1] = EGL_BAD_ACCESS;
               } else {
                  results[0] = create_surface_internal(
                     EGL_PLATFORM_WIN_NONE,
                     buffers,
                     width,
                     height,
                     0,
                     colorformat,
                     depthstencilformat,
                     maskformat,
                     multisampleformat,
                     mipmap,
                     config_depth_bits,
                     config_stencil_bits,
                     imagehandle,
                     MEM_INVALID_HANDLE,
                     PIXMAP);

                  if (results[0]) {
                     image->flags |= IMAGE_FLAG_BOUND_CLIENTBUFFER;
                     results[2] = width;
                     results[3] = height;
                     results[4] = actualformat;
                  }
                  else
                     results[1] = EGL_BAD_ALLOC;
               }

               mem_unlock(vgimage->image);
            } else
               results[1] = EGL_BAD_ALLOC;
            mem_unlock(imagehandle);
         } else {
            /* child images cannot be render targets */
            results[1] = EGL_BAD_MATCH;
         }
      }

      VG_UNLOCK_SERVER_STATE();
   } else {
      results[1] = EGL_BAD_PARAMETER;       //TODO: what error to return if we don't have an OpenVG context at all?
   }

   return 5;
}

#endif /* NO_OPENVG */

/*
   EGL_SURFACE_ID_T eglIntCreateWrappedSurface_impl(
      uint32_t handle_0, uint32_t handle_1,
      KHRN_IMAGE_FORMAT_T depthstencilformat,
      KHRN_IMAGE_FORMAT_T maskformat,
      uint32_t multisample)

   Implementation notes:

   TODO: multisample is being ignored

   Preconditions:
      [handle_0, handle_1] is a valid server pixmap handle
      depthstencilformat is a hardware framebuffer-supported depth and/or stencil format, or IMAGE_FORMAT_INVALID
      maskformat is a hardware framebuffer-supported mask format, or IMAGE_FORMAT_INVALID
*/

EGL_SURFACE_ID_T eglIntCreateWrappedSurface_impl(
   uint32_t handle_0, uint32_t handle_1,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   KHRN_IMAGE_FORMAT_T multisample,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits)
{
   MEM_HANDLE_T himage;
   EGLint error;
   vcos_assert(handle_1 == -1);
   {
      himage = egl_server_platform_create_pixmap_info(handle_0, &error);
   }

   if (himage != MEM_INVALID_HANDLE) {
      KHRN_IMAGE_T *image = (KHRN_IMAGE_T *)mem_lock(himage, NULL);
      EGL_SURFACE_ID_T result = create_surface_internal(
         EGL_PLATFORM_WIN_NONE,
         1,
         image->width,
         image->height,
         0,
         image->format,
         depthstencilformat,
         maskformat,
         multisample,
         0,
         config_depth_bits,
         config_stencil_bits,
         MEM_INVALID_HANDLE,
         himage,
         PIXMAP);
      mem_unlock(himage);
      {
      mem_release(himage);
      }
      return result;
   } else {
      return 0;
   }
}

static MEM_HANDLE_T create_shared_context(void)
{
   MEM_HANDLE_T handle = MEM_ALLOC_STRUCT_EX(GLXX_SHARED_T, MEM_COMPACT_DISCARD);                                // check, glxx_shared_term

   if (handle != MEM_INVALID_HANDLE) {
      mem_set_term(handle, glxx_shared_term);

      if (glxx_shared_init((GLXX_SHARED_T *)mem_lock(handle, NULL)))
         mem_unlock(handle);
      else {
         mem_unlock(handle);
         mem_release(handle);
         handle = MEM_INVALID_HANDLE;
      }
   }

   return handle;
}

static MEM_HANDLE_T get_shared_context(EGL_SERVER_STATE_T *state, EGL_GL_CONTEXT_ID_T share_id, EGL_CONTEXT_TYPE_T share_type)
{
   MEM_HANDLE_T result = MEM_INVALID_HANDLE;

   switch (share_type) {
   case OPENGL_ES_11:
   {
      GLXX_SERVER_STATE_T *server;
      MEM_HANDLE_T handle = khrn_map_lookup(&state->glcontexts, share_id);
      vcos_assert(handle != MEM_INVALID_HANDLE);
      server = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);
      result = server->mh_shared;
      mem_unlock(handle);
      break;
   }
   case OPENGL_ES_20:
   {
      GLXX_SERVER_STATE_T *server;
      MEM_HANDLE_T handle = khrn_map_lookup(&state->glcontexts, share_id);
      vcos_assert(handle != MEM_INVALID_HANDLE);
      server = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);
      result = server->mh_shared;
      mem_unlock(handle);
      break;
   }
#ifndef NO_OPENVG
   case OPENVG:
   {
      VG_SERVER_STATE_T *server;
      MEM_HANDLE_T handle = khrn_map_lookup(&state->vgcontexts, share_id);
      vcos_assert(handle != MEM_INVALID_HANDLE);
      server = (VG_SERVER_STATE_T *)mem_lock(handle, NULL);
      result = server->shared_state;
      mem_unlock(handle);
      break;
   }
#endif /* NO_OPENVG */
   default:
      UNREACHABLE();
   }
   return result;
}

// Create server states. To actually use these, call eglIntMakeCurrent.
EGL_GL_CONTEXT_ID_T eglIntCreateGLES11_impl(EGL_GL_CONTEXT_ID_T share_id, EGL_CONTEXT_TYPE_T share_type)
{
   GLXX_SERVER_STATE_T *server;
   EGL_GL_CONTEXT_ID_T result = 0;

   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   /*
      create and initialize shared state structure
      or obtain one from the specified context
   */

   MEM_HANDLE_T shandle, handle;
   if (share_id) {
      vcos_assert(share_type == OPENGL_ES_11 || share_type == OPENGL_ES_20);
      shandle = get_shared_context(state, share_id, share_type);
      mem_acquire(shandle);
   } else
      shandle = create_shared_context();

   if (shandle == MEM_INVALID_HANDLE)
      return 0;

   /*
      create and initialize state structure
   */

   handle = MEM_ALLOC_STRUCT_EX(GLXX_SERVER_STATE_T, MEM_COMPACT_DISCARD);                     // check, glxx_server_state_term
   if (handle == MEM_INVALID_HANDLE) {
      mem_release(shandle);
      return 0;
   }

   mem_set_term(handle, glxx_server_state_term);

   server = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);

   if (!gl11_server_state_init(server, state->next_context, state->pid, shandle)) {
      mem_unlock(handle);
      mem_release(handle);
      mem_release(shandle);
      return 0;
   }

   if (khrn_map_insert(&state->glcontexts, state->next_context, handle))
      result = state->next_context++;

   mem_unlock(handle);
   mem_release(handle);
   mem_release(shandle);

   return result;
}

EGL_GL_CONTEXT_ID_T eglIntCreateGLES20_impl(EGL_GL_CONTEXT_ID_T share, EGL_CONTEXT_TYPE_T share_type)
{
   EGL_GL_CONTEXT_ID_T result = 0;

   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   /*
      create and initialize shared state structure
      or obtain one from the specified context
   */

   MEM_HANDLE_T shandle, handle;
   if (share) {
      vcos_assert(share_type == OPENGL_ES_11 || share_type == OPENGL_ES_20);
      shandle = get_shared_context(state, share, share_type);
      mem_acquire(shandle);
   } else
      shandle = create_shared_context();

   if (shandle == MEM_INVALID_HANDLE)
      return 0;

   /*
      create and initialize state structure
   */

   handle = MEM_ALLOC_STRUCT_EX(GLXX_SERVER_STATE_T, MEM_COMPACT_DISCARD);                     // check, glxx_server_state_term
   if (handle == MEM_INVALID_HANDLE) {
      mem_release(shandle);
      return 0;
   }

   mem_set_term(handle, glxx_server_state_term);

   if (!gl20_server_state_init((GLXX_SERVER_STATE_T *)mem_lock(handle, NULL), state->next_context, state->pid, shandle)) {
      mem_unlock(handle);
      mem_release(handle);
      mem_release(shandle);
      return 0;
   }

   if (khrn_map_insert(&state->glcontexts, state->next_context, handle))
      result = state->next_context++;

   mem_unlock(handle);
   mem_release(handle);
   mem_release(shandle);

   return result;
}

#ifndef NO_OPENVG

EGL_VG_CONTEXT_ID_T eglIntCreateVG_impl(EGL_VG_CONTEXT_ID_T share, EGL_CONTEXT_TYPE_T share_type)
{
   EGL_GL_CONTEXT_ID_T result = 0;

   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   /*
      create and initialize shared state structure
      or obtain one from the specified context
   */

   MEM_HANDLE_T shared_state_handle, handle;
   if (share) {
      vcos_assert(share_type == OPENVG);
      shared_state_handle = get_shared_context(state, share, share_type);
      mem_acquire(shared_state_handle);
   } else
      shared_state_handle = vg_server_shared_state_alloc();

   if (shared_state_handle == MEM_INVALID_HANDLE)
      return 0;

   handle = vg_server_state_alloc(shared_state_handle, state->pid);
   mem_release(shared_state_handle);

   if (handle == MEM_INVALID_HANDLE)
      return 0;

   if (khrn_map_insert(&state->vgcontexts, state->next_context, handle))
      result = state->next_context++;

   mem_release(handle);

   return result;
}

#endif /* NO_OPENVG */

// Disassociates surface or context objects from their handles. The objects
// themselves still exist as long as there is a reference to them. In
// particular, if you delete part of a triple buffer group, the remaining color
// buffers plus the ancillary buffers all survive.


int eglIntDestroySurface_impl(EGL_SURFACE_ID_T s)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   khrn_map_delete(&state->surfaces, s);

   return 0;
}

void eglIntDestroyGL_impl(EGL_GL_CONTEXT_ID_T c)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   khrn_map_delete(&state->glcontexts, c);
}

#ifndef NO_OPENVG
void eglIntDestroyVG_impl(EGL_VG_CONTEXT_ID_T c)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   khrn_map_delete(&state->vgcontexts, c);
}
#endif /* NO_OPENVG */

void destroy_glcontext_callback(KHRN_MAP_T *map, uint32_t key, MEM_HANDLE_T value, void *pid)
{
   MEM_TERM_T term = mem_get_term(value);

   bool todestroy = false;

   UNUSED(map);

   if (term == glxx_server_state_term) {
      todestroy = ((GLXX_SERVER_STATE_T *)mem_lock(value, NULL))->pid == *(uint64_t *)pid;
      mem_unlock(value);
   } else
      UNREACHABLE();

   if (todestroy) {
      EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
      khrn_map_delete(&state->glcontexts, key);
   }
}

#ifndef NO_OPENVG
void destroy_vgcontext_callback(KHRN_MAP_T *map, uint32_t key, MEM_HANDLE_T value, void *pid)
{
   bool todestroy;
   UNUSED(map);

   todestroy = ((VG_SERVER_STATE_T *)mem_lock(value, NULL))->pid == *(uint64_t *)pid;
   mem_unlock(value);

   if (todestroy) {
      EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
      khrn_map_delete(&state->vgcontexts, key);
   }
}
#endif /* NO_OPENVG */

#if EGL_KHR_image
void destroy_eglimage_callback(KHRN_MAP_T *map, uint32_t key, MEM_HANDLE_T value, void *pid)
{
   bool todestroy;
   UNUSED(map);

   todestroy = ((EGL_IMAGE_T *)mem_lock(value, NULL))->pid == *(uint64_t *)pid;
   mem_unlock(value);

   if (todestroy) {
      EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
      khrn_map_delete(&state->eglimages, key);
   }
}
#endif

static void detach_buffers_from_gl(uint32_t glversion, MEM_HANDLE_T handle)
{
   if (handle != MEM_INVALID_HANDLE) {
      switch (glversion) {
      case EGL_SERVER_GL11:
      {
         GLXX_SERVER_STATE_T *state = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);

         MEM_ASSIGN(state->mh_draw, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->mh_read, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->mh_depth, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->mh_color_multi, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->mh_ds_multi, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->mh_preserve, MEM_INVALID_HANDLE);

         mem_unlock(handle);
         break;
      }
      case EGL_SERVER_GL20:
      {
         GLXX_SERVER_STATE_T *state = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);

         MEM_ASSIGN(state->mh_draw, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->mh_read, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->mh_depth, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->mh_color_multi, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->mh_ds_multi, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->mh_preserve, MEM_INVALID_HANDLE);

         mem_unlock(handle);
         break;
      }
      default:
         UNREACHABLE();
      }
   }
}

static void attach_buffers_to_gl(uint32_t glversion,
                                 MEM_HANDLE_T handle,
                                 MEM_HANDLE_T draw,
                                 uint32_t swapchainc,
                                 MEM_HANDLE_T read,
                                 MEM_HANDLE_T depth,
                                 MEM_HANDLE_T color_multi,
                                 MEM_HANDLE_T ds_multi,
                                 MEM_HANDLE_T preserveBuf,
                                 uint32_t config_depth_bits,
                                 uint32_t config_stencil_bits)
{
   if (handle != MEM_INVALID_HANDLE) {
      switch (glversion) {
      case EGL_SERVER_GL11:
      case EGL_SERVER_GL20:
      {
         GLXX_SERVER_STATE_T *state = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);

         state->config_depth_bits = config_depth_bits;
         state->config_stencil_bits = config_stencil_bits;

         glxx_server_state_set_buffers(state, draw, swapchainc, read, depth, color_multi, ds_multi, preserveBuf);

         mem_unlock(handle);
         break;
      }
      default:
         UNREACHABLE();
      }
   }
}

static void update_gl_buffers(EGL_SERVER_STATE_T *state)
{
   if (state->glcontext != MEM_INVALID_HANDLE) {
      EGL_SERVER_SURFACE_T *dsurface = (EGL_SERVER_SURFACE_T *)mem_lock(state->gldrawsurface, NULL);
      EGL_SERVER_SURFACE_T *rsurface = (EGL_SERVER_SURFACE_T *)mem_lock(state->glreadsurface, NULL);

      attach_buffers_to_gl(
         state->glversion,
         state->glcontext,
         dsurface->mh_color[dsurface->back_buffer_index],
         dsurface->swapchainc,
         rsurface->mh_color[rsurface->back_buffer_index],
         dsurface->mh_depth,
         dsurface->mh_color_multi,
         dsurface->mh_ds_multi,
         dsurface->mh_preserve,
         dsurface->config_depth_bits,
         dsurface->config_stencil_bits);

      mem_unlock(state->gldrawsurface);
      mem_unlock(state->glreadsurface);
   }
}

#ifndef NO_OPENVG

static void update_vg_buffers(EGL_SERVER_STATE_T *state)
{
   if (state->vgsurface == MEM_INVALID_HANDLE) {
      vg_buffers_changed(MEM_INVALID_HANDLE, 0, MEM_INVALID_HANDLE);
   } else {
      EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(state->vgsurface, NULL);
      vg_buffers_changed(surface->mh_color[surface->back_buffer_index], surface->swapchainc, surface->mh_mask);
      mem_unlock(state->vgsurface);
   }
}

#endif /* NO_OPENVG */

// Selects the given process id for all operations. Most resource creation is
//  associated with the currently selected process id
// Selects the given context, draw and read surfaces for GL operations.
// Selects the given context and surface for VG operations.
// Any of the surfaces may be identical to each other.
// Contexts will not be flushed. (eglMakeCurrent must flush but this is handled
// by the client. It is not necessary to flush when switching threads
// If any of the surfaces have been resized then the color and ancillary buffers
//  are freed and recreated in the new size.
void eglIntMakeCurrent_impl(uint32_t pid_0, uint32_t pid_1, uint32_t glversion, EGL_GL_CONTEXT_ID_T gc, EGL_SURFACE_ID_T gdraw, EGL_SURFACE_ID_T gread
#ifndef NO_OPENVG
   , EGL_VG_CONTEXT_ID_T vc, EGL_SURFACE_ID_T vsurf
#endif /* NO_OPENVG */
   )
{
#ifndef NO_OPENVG
   MEM_HANDLE_T chandle, shandle;
   bool surface_changed, context_changed;
#endif /* NO_OPENVG */
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   /* uint32_t thread_id = rtos_get_thread_id(); */

#ifdef WANT_ARM_LINUX
   khrn_misc_set_connection_pid(pid_0, pid_1);
#endif
   state->pid = ((uint64_t)pid_1 << 32) | pid_0;

   //GL
   if (gc != EGL_SERVER_NO_GL_CONTEXT) {
      MEM_HANDLE_T chandle = khrn_map_lookup(&state->glcontexts, gc);
      MEM_HANDLE_T dhandle = khrn_map_lookup(&state->surfaces, gdraw);
      MEM_HANDLE_T rhandle = khrn_map_lookup(&state->surfaces, gread);

      vcos_assert(chandle != MEM_INVALID_HANDLE && dhandle != MEM_INVALID_HANDLE && rhandle != MEM_INVALID_HANDLE);

      if (chandle != state->glcontext || dhandle != state->gldrawsurface || rhandle != state->glreadsurface)
      {
         detach_buffers_from_gl(state->glversion, state->glcontext);

         vcos_assert(glversion == EGL_SERVER_GL11 || glversion == EGL_SERVER_GL20);

         GL11_FORCE_UNLOCK_SERVER_STATE();
         GL20_FORCE_UNLOCK_SERVER_STATE();
         state->glversion = glversion;
         MEM_ASSIGN(state->glcontext, chandle);
         MEM_ASSIGN(state->gldrawsurface, dhandle);
         MEM_ASSIGN(state->glreadsurface, rhandle);

         update_gl_buffers(state);
      }

   } else {
      vcos_assert(gdraw == EGL_SERVER_NO_SURFACE);
      vcos_assert(gread == EGL_SERVER_NO_SURFACE);
      if (state->glcontext != MEM_INVALID_HANDLE) {
         detach_buffers_from_gl(state->glversion, state->glcontext);
         GL11_FORCE_UNLOCK_SERVER_STATE();
         GL20_FORCE_UNLOCK_SERVER_STATE();
         state->glversion = 0;
         MEM_ASSIGN(state->glcontext, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->gldrawsurface, MEM_INVALID_HANDLE);
         MEM_ASSIGN(state->glreadsurface, MEM_INVALID_HANDLE);
      }
   }

#ifndef NO_OPENVG
   /*VG */
   chandle = (vc == EGL_SERVER_NO_VG_CONTEXT) ? MEM_INVALID_HANDLE : khrn_map_lookup(&state->vgcontexts, vc);
   shandle = (vsurf == EGL_SERVER_NO_SURFACE) ? MEM_INVALID_HANDLE : khrn_map_lookup(&state->surfaces, vsurf);

   vcos_assert((chandle == MEM_INVALID_HANDLE) == (shandle == MEM_INVALID_HANDLE));

   surface_changed = state->vgsurface != shandle;
   context_changed = state->vgcontext != chandle;

   if (context_changed) {
      VG_FORCE_UNLOCK_SERVER_STATE();
      MEM_ASSIGN(state->vgcontext, chandle);
   }
   if (surface_changed) {
      MEM_ASSIGN(state->vgsurface, shandle);
   }

   if (surface_changed) {
      update_vg_buffers(state);
   } else if (context_changed) {
      vg_state_changed();
   }
#endif
}

static void flush_gl(uint32_t glversion, MEM_HANDLE_T handle, bool wait)
{
   if (handle != MEM_INVALID_HANDLE) {
      switch (glversion) {
      case EGL_SERVER_GL11:
      case EGL_SERVER_GL20:
      {
         GLXX_SERVER_STATE_T *state = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);

         glxx_server_state_flush(state, wait);

         mem_unlock(handle);
         break;
      }
      default:
         UNREACHABLE();
      }
   }
}

// Flushes one or both context, and waits for the flushes to complete before returning.
// Equivalent to:
// if (flushgl) glFinish();
// if (flushvg) vgFinish();
// (todo: actually, we now also wait for the display stuff to finish. should
// gl/vgFinish also do this?)
int eglIntFlushAndWait_impl(uint32_t flushgl, uint32_t flushvg)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
#ifdef NO_OPENVG
   UNUSED(flushvg);
#endif

   if (flushgl)
      flush_gl(state->glversion, state->glcontext, true);
#ifndef NO_OPENVG
   if (flushvg && (state->vgcontext != MEM_INVALID_HANDLE)) {
      vgFinish_impl();
   }
#endif /* NO_OPENVG */

   return 0;
}

void eglIntFlush_impl(uint32_t flushgl, uint32_t flushvg)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
#ifdef NO_OPENVG
   UNUSED(flushvg);
#endif

   if (flushgl)
      flush_gl(state->glversion, state->glcontext, false);
#ifndef NO_OPENVG
   if (flushvg && (state->vgcontext != MEM_INVALID_HANDLE)) {
      vgFlush_impl();
   }
#endif /* NO_OPENVG */
}

void eglIntSwapBuffers_impl(EGL_SURFACE_ID_T s, uint32_t width, uint32_t height, uint32_t swapchainc, uint32_t handle, uint32_t preserve, uint32_t position)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T shandle = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);
   MEM_HANDLE_T ihandle;
   KHRN_IMAGE_T *image;
   bool         swapIntervalAllowed = true;
   uint32_t     swapInterval = 1;
   MEM_HANDLE_T currentImageHandle;
   KHRN_IMAGE_T *currentImage;
   bool         multisampled = (surface->mh_color_multi != MEM_INVALID_HANDLE);

   INCR_DRIVER_COUNTER(num_swaps);

   /* BCG_EGLIMAGE_CONVERTER : Increment the nominal frame counter. It's used for sync'ing EGLimage conversions. */
   if (state->glcontext != MEM_INVALID_HANDLE)
   {
      GLXX_SERVER_STATE_T *glstate = (GLXX_SERVER_STATE_T *)mem_lock(state->glcontext, NULL);
      glstate->frame_number++;
      mem_unlock(state->glcontext);
   }

   egl_server_platform_set_position(handle, position, width, height);

   vcos_assert(surface->win != EGL_PLATFORM_WIN_NONE);   //TODO: is having this EGL_PLATFORM_WIN_NONE thing messy?
   vcos_assert(surface->buffers >= 1);

   if (surface->buffers > 1 && state->glcontext != MEM_INVALID_HANDLE)   /* This is a swap-chain */
   {
      bool         invalidateMS;
      GLXX_SERVER_STATE_T *glstate;
      KHRN_IMAGE_T *src = (KHRN_IMAGE_T *)mem_lock(surface->mh_color[surface->back_buffer_index], NULL);

      /* Do we need to destroy the preserve buffer? */
      if (!(preserve && !multisampled) || src->width != width || src->height != height)
         MEM_ASSIGN(surface->mh_preserve, MEM_INVALID_HANDLE);

      /* Check if we need to create the preserve buffer */
      if (preserve && !multisampled)
      {
         /* We need a preserve buffer */
         if (surface->mh_preserve == MEM_INVALID_HANDLE)
         {
            MEM_HANDLE_T himage;

            himage = khrn_image_create(src->format, src->width, src->height,
               ((src->flags & IMAGE_FLAG_TEXTURE) ? IMAGE_CREATE_FLAG_TEXTURE : 0) |
               ((src->flags & IMAGE_FLAG_RSO_TEXTURE) ? IMAGE_CREATE_FLAG_RSO_TEXTURE : 0) |
               ((src->flags & IMAGE_FLAG_RENDER_TARGET) ? IMAGE_CREATE_FLAG_RENDER_TARGET : 0) |
               ((src->flags & IMAGE_FLAG_DISPLAY) ? IMAGE_CREATE_FLAG_DISPLAY : 0));

            MEM_ASSIGN(surface->mh_preserve, himage);
            mem_release(himage);
         }
      }

      mem_unlock(surface->mh_color[surface->back_buffer_index]);

      /* Auxiliary buffers are not usually preserved. The exception is the multisample buffer which is
         preserved when in preserve mode. */
      invalidateMS = !(multisampled && preserve);

      glstate = (GLXX_SERVER_STATE_T *)mem_lock(state->glcontext, NULL);
      glxx_hw_invalidate_frame(glstate, false, true, true, invalidateMS, !preserve || multisampled, true);
      mem_unlock(state->glcontext);
   }
#ifndef NO_OPENVG
   else if (surface->buffers > 1 && state->vgcontext != MEM_HANDLE_INVALID)
   {
      vg_be_install_framebuffer();
   }
#endif /* NO_OPENVG */

   currentImageHandle = surface->mh_color[surface->back_buffer_index];
   currentImage = (KHRN_IMAGE_T *)mem_lock(currentImageHandle, NULL);
   vcos_assert(currentImage->alreadyLocked > 0);
   vcos_atomic_decrement(&currentImage->alreadyLocked);
   mem_unlock(surface->mh_color[surface->back_buffer_index]);

   /* No swapInterval support for single buffering */
   if (surface->buffers == 1)
      swapIntervalAllowed = false;

   /* We're going to write this buffer */
   khrn_interlock_write(&currentImage->interlock, KHRN_INTERLOCK_USER_NONE);

   if (swapIntervalAllowed)
      swapInterval = surface->swap_interval;

   surface->win = handle;
   surface->back_buffer_index = (surface->back_buffer_index + 1) % surface->buffers;
   /* (non-single-buffered surfaces) don't wait for the next buffer to come off
    * the display -- the interlock system will handle this */

   ihandle = surface->mh_color[surface->back_buffer_index];
   image = (KHRN_IMAGE_T *)mem_lock(ihandle, NULL);

   /* Update the swap interval in the image */
   image->swap_interval = swapInterval;

   /* on an abstract platform, the buffers ARE provided from the underlying windowing system.
      Don't provide any resize in this case, as its not appropriate.  It's up to the window manager to do it */

   /* The contents of the buffers becomes undefined so mark the interlocks to prevent loads */
   if (surface->buffers > 1)
   {
      if (!multisampled || !preserve)
         khrn_interlock_invalidate(&image->interlock);

      if (surface->mh_color_multi)
      {
         KHRN_IMAGE_T *color_multi = (KHRN_IMAGE_T *)mem_lock(surface->mh_color_multi, NULL);
         khrn_interlock_invalidate(&color_multi->interlock);
         mem_unlock(surface->mh_color_multi);
      }
      if (surface->mh_ds_multi)
      {
         KHRN_IMAGE_T *ds_multi = (KHRN_IMAGE_T *)mem_lock(surface->mh_ds_multi, NULL);
         khrn_interlock_invalidate(&ds_multi->interlock);
         mem_unlock(surface->mh_ds_multi);
      }
      if (surface->mh_depth)
      {
         KHRN_IMAGE_T *depth = (KHRN_IMAGE_T *)mem_lock(surface->mh_depth, NULL);
         khrn_interlock_invalidate(&depth->interlock);
         mem_unlock(surface->mh_depth);
      }
   }

   mem_unlock(ihandle);
   mem_unlock(shandle);

   if (surface->buffers > 1)
   {
      // Setup a delayed copy which will copy the rendered frame to the preserve buffer as soon as it
      // completes rendering.
      if (preserve && !multisampled)
         khrn_delayed_copy_buffer(surface->mh_preserve, currentImageHandle);

      // Issue a job to sync the pipe and actually display the buffer
      khrn_issue_swapbuffers_job(currentImageHandle);

      // Buffers have swapped around, so need to make sure we're rendering into
      // the right ones.
      update_gl_buffers(state);
#ifndef NO_OPENVG
      if (state->vgsurface == shandle)
         update_vg_buffers(state);
#endif /* NO_OPENVG */
   }
}

void eglIntSelectMipmap_impl(EGL_SURFACE_ID_T s, int level)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T shandle = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);

   vcos_assert(surface->mipmap);

   // If the value of this attribute is outside the range of supported
   // mipmap levels, the closest valid mipmap level is selected for rendering.
   if (level < 0)
      level = 0;
   else if ((uint32_t)level >= surface->buffers)
      level = surface->buffers - 1;

   surface->back_buffer_index = level;

   mem_unlock(shandle);

   // Buffers have swapped around, so need to make sure we're rendering into
   // the right ones.
   update_gl_buffers(state);
#ifndef NO_OPENVG
   if (state->vgsurface == shandle) { update_vg_buffers(state); }
#endif /* NO_OPENVG */
}

void eglIntGetColorData_impl(EGL_SURFACE_ID_T s, KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height, int32_t stride, uint32_t y_offset, void *data)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T shandle = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);
   KHRN_IMAGE_T *src;
   KHRN_IMAGE_WRAP_T dst_wrap, src_wrap;
   uint32_t flags = 0;

   if (stride < 0) {
      /* we use stride < 0 to signal upside down images, flip it back and mark the flag */
      flags |= IMAGE_FLAG_DISPLAY;
      stride = -stride;
   }
   khrn_image_interlock_wrap(&dst_wrap, format, width, height, stride, flags, data, NULL);

   //TODO: we currently expect the client to flush before calling this
   //I've added a khrn_interlock_read_immediate below. Can we remove the flush on the client?

   // TODO will this handle all necessary conversions correctly?
   // Will it handle images of different sizes?
   src = (KHRN_IMAGE_T *)mem_lock(surface->mh_color[surface->back_buffer_index], NULL);
   khrn_interlock_read_immediate(&src->interlock);
   khrn_image_lock_interlock_wrap(src, &src_wrap, NULL);
   khrn_image_wrap_copy_region(
      &dst_wrap, 0, 0,
      width, height,
      &src_wrap, 0, y_offset,
      IMAGE_CONV_GL);
   khrn_image_unlock_wrap(src);
   mem_unlock(surface->mh_color[surface->back_buffer_index]);

   mem_unlock(shandle);
}

void eglIntSetColorData_impl(EGL_SURFACE_ID_T s, KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height, int32_t stride, uint32_t y_offset, const void *data)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T shandle = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);
   KHRN_IMAGE_T *dst;
   KHRN_IMAGE_WRAP_T dst_wrap, src_wrap;
   uint32_t flags = 0;

   if (stride < 0) {
      /* we use stride < 0 to signal upside down images, flip it back and mark the flag */
      flags |= IMAGE_FLAG_DISPLAY;
      stride = -stride;
   }
   khrn_image_interlock_wrap(&src_wrap, format, width, height, stride, flags, (void *)data, NULL); /* casting away constness here, but we won't actually modify */

   // TODO will this handle all necessary conversions correctly?
   // Will it handle images of different sizes?
   dst = (KHRN_IMAGE_T *)mem_lock(surface->mh_color[surface->back_buffer_index], NULL);
   khrn_interlock_write_immediate(&dst->interlock);
   khrn_image_lock_interlock_wrap(dst, &dst_wrap, NULL);
   khrn_image_wrap_copy_region(
      &dst_wrap, 0, y_offset,
      width, height,
      &src_wrap, 0, 0,
      IMAGE_CONV_GL);
   khrn_image_unlock_wrap(dst);
   mem_unlock(surface->mh_color[surface->back_buffer_index]);

   mem_unlock(shandle);
}

static MEM_HANDLE_T get_active_gl_texture_2d(EGL_SERVER_STATE_T *state)
{
   MEM_HANDLE_T result = MEM_INVALID_HANDLE;
   if (state->glcontext != MEM_INVALID_HANDLE) {
      switch (state->glversion) {
      case EGL_SERVER_GL11:
      case EGL_SERVER_GL20:
      {
         GLXX_SERVER_STATE_T *glstate = (GLXX_SERVER_STATE_T *)mem_lock(state->glcontext, NULL);
         result = glstate->bound_texture[glstate->active_texture - GL_TEXTURE0].mh_twod;
         mem_unlock(state->glcontext);
         break;
      }
      default:
         UNREACHABLE();
      }
   }

   return result;
}

bool eglIntBindTexImage_impl(EGL_SURFACE_ID_T s)
{
   bool result;
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T shandle = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);
   MEM_HANDLE_T thandle = get_active_gl_texture_2d(state);
   KHRN_IMAGE_T *image0 = (KHRN_IMAGE_T *)mem_lock(surface->mh_color[0], NULL);

   /* We permit IMAGE_FLAG_BOUND_CLIENTBUFFER */
   vcos_assert(!(image0->flags & IMAGE_FLAG_BOUND_EGLIMAGE));
   if (!(image0->flags & IMAGE_FLAG_BOUND_TEXIMAGE)) {
      if (thandle != MEM_INVALID_HANDLE) {
         GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)mem_lock(thandle, NULL);
         vcos_assert(texture->target != GL_TEXTURE_CUBE_MAP);
         glxx_texture_bind_images(texture, surface->buffers, surface->mh_color, TEXTURE_STATE_BOUND_TEXIMAGE, shandle, surface->back_buffer_index);
         mem_unlock(thandle);

         MEM_ASSIGN(surface->mh_bound_texture, thandle);
      }
      result = true;
   } else {
      result = false;
   }

   mem_unlock(surface->mh_color[0]);
   mem_unlock(shandle);

   return result;
}

void eglIntReleaseTexImage_impl(EGL_SURFACE_ID_T s)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T shandle = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);

   if (surface->mh_bound_texture != MEM_INVALID_HANDLE)
   {
      MEM_HANDLE_T thandle = surface->mh_bound_texture;
      GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)mem_lock(thandle, NULL);
      glxx_texture_release_teximage(texture);   // this will remove IMAGE_FLAG_BOUND_TEXIMAGE from all images
      mem_unlock(thandle);

      MEM_ASSIGN(surface->mh_bound_texture, MEM_INVALID_HANDLE);
   }

   mem_unlock(shandle);
}

void eglIntSwapInterval_impl(EGL_SURFACE_ID_T s, uint32_t swap_interval)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T shandle = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);

   surface->swap_interval = swap_interval;
   mem_unlock(shandle);
}

typedef int (*USAGE_WALK_T)(void *v, int);

typedef struct {
   MEM_TERM_T term;
   USAGE_WALK_T walk;
} USAGE_RESOLVE_T;

static int get_mem_usage(MEM_HANDLE_T handle);

static int image_walk(KHRN_IMAGE_T *image, int size)
{
   int usage = 0;
   UNUSED(size);

   usage += get_mem_usage(image->mh_storage);
   usage += get_mem_usage(image->mh_aux);

   return usage;
}

static int texture_walk(GLXX_TEXTURE_T *texture, int size)
{
   int usage = 0;
   int i, j;
   UNUSED(size);

   for (i = 0; i < TEXTURE_BUFFER_COUNT; i++)
      for (j = 0; j <= LOG2_MAX_TEXTURE_SIZE; j++)
         usage += get_mem_usage(texture->mh_mipmaps[i][j]);

   for(i = 0; i < GLXX_TEXTURE_POOL_SIZE; i++)
      usage += get_mem_usage(texture->blob_pool[i].mh_storage);
   usage += get_mem_usage(texture->mh_depaletted_blob);

   return usage;
}

static int buffer_walk(GLXX_BUFFER_T *buffer, int size)
{
   int usage = 0;
   int i;
   UNUSED(size);

   for(i = 0; i < GLXX_BUFFER_POOL_SIZE; i++)
      usage += get_mem_usage(buffer->pool[i].mh_storage);

   return usage;
}

static void map_iterator(KHRN_MAP_T *map, uint32_t key, MEM_HANDLE_T value, void *v)
{
   UNUSED(map);
   UNUSED(key);

   *(int *)v += get_mem_usage(value);
}

static int shared_walk(GLXX_SHARED_T *shared, int size)
{
   int usage = 0;
   UNUSED(size);

   khrn_map_iterate(&shared->pobjects, map_iterator, &usage);
   khrn_map_iterate(&shared->textures, map_iterator, &usage);
   khrn_map_iterate(&shared->buffers, map_iterator, &usage);
   khrn_map_iterate(&shared->renderbuffers, map_iterator, &usage);
   khrn_map_iterate(&shared->framebuffers, map_iterator, &usage);

   return usage;
}

static int egl_server_surface_walk(EGL_SERVER_SURFACE_T *surface, int size)
{
   int usage = 0;
   int i;
   UNUSED(size);

   for (i = 0; i < EGL_MAX_BUFFERS; i++)
      usage += get_mem_usage(surface->mh_color[i]);

   usage += get_mem_usage(surface->mh_depth);
   usage += get_mem_usage(surface->mh_color_multi);
   usage += get_mem_usage(surface->mh_ds_multi);
   usage += get_mem_usage(surface->mh_mask);
   usage += get_mem_usage(surface->mh_preserve);

   usage += get_mem_usage(surface->mh_bound_texture);

   return usage;
}

static int gl11_server_state_walk(GLXX_SERVER_STATE_T *state, int size)
{
   int usage = 0;
   int i;
   UNUSED(size);

   usage += get_mem_usage(state->bound_buffer.mh_array);
   usage += get_mem_usage(state->bound_buffer.mh_element_array);

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      usage += get_mem_usage(state->bound_buffer.mh_attrib_array[i]);

   for (i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      usage += get_mem_usage(state->bound_texture[i].mh_twod);
      usage += get_mem_usage(state->bound_texture[i].mh_external);
   }

   usage += get_mem_usage(state->mh_shared);
   usage += get_mem_usage(state->mh_default_texture_twod);
   usage += get_mem_usage(state->mh_default_texture_external);

   usage += get_mem_usage(state->mh_draw);
   usage += get_mem_usage(state->mh_read);
   usage += get_mem_usage(state->mh_depth);
   usage += get_mem_usage(state->mh_color_multi);
   usage += get_mem_usage(state->mh_ds_multi);
   usage += get_mem_usage(state->mh_preserve);

   usage += get_mem_usage(state->mh_cache);

   return usage;
}

static int gl20_server_state_walk(GLXX_SERVER_STATE_T *state, int size)
{
   int usage = 0;
   int i;
   UNUSED(size);

   usage += get_mem_usage(state->mh_program);

   usage += get_mem_usage(state->bound_buffer.mh_array);
   usage += get_mem_usage(state->bound_buffer.mh_element_array);

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      usage += get_mem_usage(state->bound_buffer.mh_attrib_array[i]);

   for (i = 0; i < GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS; i++) {
      usage += get_mem_usage(state->bound_texture[i].mh_twod);
      usage += get_mem_usage(state->bound_texture[i].mh_external);
      usage += get_mem_usage(state->bound_texture[i].mh_cube);
   }

   usage += get_mem_usage(state->mh_bound_renderbuffer);
   usage += get_mem_usage(state->mh_bound_framebuffer);

   usage += get_mem_usage(state->mh_shared);
   usage += get_mem_usage(state->mh_default_texture_twod);
   usage += get_mem_usage(state->mh_default_texture_external);
   usage += get_mem_usage(state->mh_default_texture_cube);

   usage += get_mem_usage(state->mh_draw);
   usage += get_mem_usage(state->mh_read);
   usage += get_mem_usage(state->mh_depth);
   usage += get_mem_usage(state->mh_color_multi);
   usage += get_mem_usage(state->mh_ds_multi);
   usage += get_mem_usage(state->mh_preserve);

   usage += get_mem_usage(state->mh_cache);

   return usage;
}

static int gl20_bindings_walk(GL20_BINDING_T *base, int size)
{
   int usage = 0;
   int i, count = size / sizeof(GL20_BINDING_T);

   vcos_assert(size % sizeof(GL20_BINDING_T) == 0);

   for (i = 0; i < count; i++)
      usage += get_mem_usage(base[i].mh_name);

   return usage;
}

static int gl20_uniform_info_walk(GL20_UNIFORM_INFO_T *base, int size)
{
   int usage = 0;
   int i, count = size / sizeof(GL20_UNIFORM_INFO_T);

   vcos_assert(size % sizeof(GL20_UNIFORM_INFO_T) == 0);

   for (i = 0; i < count; i++)
      usage += get_mem_usage(base[i].mh_name);

   return usage;
}

static int gl20_attrib_info_walk(GL20_ATTRIB_INFO_T *base, int size)
{
   int usage = 0;
   int i, count = size / sizeof(GL20_ATTRIB_INFO_T);

   vcos_assert(size % sizeof(GL20_ATTRIB_INFO_T) == 0);

   for (i = 0; i < count; i++)
      usage += get_mem_usage(base[i].mh_name);

   return usage;
}

static int gl20_shader_sources_walk(MEM_HANDLE_T *base, int size)
{
   int usage = 0;
   int i, count = size / sizeof(MEM_HANDLE_T);

   vcos_assert(size % sizeof(MEM_HANDLE_T) == 0);

   for (i = 0; i < count; i++)
      usage += get_mem_usage(base[i]);

   return usage;
}

static int gl20_program_walk(GL20_PROGRAM_T *program, int size)
{
   int usage = 0;
   int i;

   UNUSED(size);

   usage += get_mem_usage(program->mh_vertex);
   usage += get_mem_usage(program->mh_fragment);
   usage += get_mem_usage(program->mh_bindings);
   usage += get_mem_usage(program->result.mh_blob);

   for (i = 0; i < GL20_LINK_RESULT_CACHE_SIZE; i++)
   {
      usage += get_mem_usage(program->result.cache[i].data.mh_vcode);
      usage += get_mem_usage(program->result.cache[i].data.mh_ccode);
      usage += get_mem_usage(program->result.cache[i].data.mh_fcode);
      usage += get_mem_usage(program->result.cache[i].data.mh_vuniform_map);
      usage += get_mem_usage(program->result.cache[i].data.mh_cuniform_map);
      usage += get_mem_usage(program->result.cache[i].data.mh_funiform_map);
   }

   usage += get_mem_usage(program->mh_sampler_info);
   usage += get_mem_usage(program->mh_uniform_info);
   usage += get_mem_usage(program->mh_uniform_data);
   usage += get_mem_usage(program->mh_attrib_info);
   usage += get_mem_usage(program->mh_info);

   return usage;
}

static int gl20_shader_walk(GL20_SHADER_T *shader, int size)
{
   int usage = 0;
   UNUSED(size);

   usage += get_mem_usage(shader->mh_sources_current);
   usage += get_mem_usage(shader->mh_sources_compile);

   usage += get_mem_usage(shader->mh_info);

   return usage;
}

static int glxx_renderbuffer_walk(GLXX_RENDERBUFFER_T *renderbuffer, int size)
{
   int usage = 0;
   UNUSED(size);

   usage += get_mem_usage(renderbuffer->mh_storage);

   return usage;
}

static int glxx_framebuffer_walk(GLXX_FRAMEBUFFER_T *framebuffer, int size)
{
   int usage = 0;
   UNUSED(size);

   usage += get_mem_usage(framebuffer->attachments.color.mh_object);
   usage += get_mem_usage(framebuffer->attachments.depth.mh_object);
   usage += get_mem_usage(framebuffer->attachments.stencil.mh_object);

   return usage;
}

#ifndef NO_OPENVG

static int vg_font_walk(VG_FONT_T *font, int size)
{
   int usage = 0;
   uint32_t i;

   VG_FONT_LOCKED_T font_locked;

   vg_font_lock(font, &font_locked);
   for (i = 0; i != (font_locked.capacity * 2); ++i)
      if (font_locked.entries[i] != SLOT_NONE)
         usage += get_mem_usage(font_locked.slots[font_locked.entries[i]].object);
   vg_font_unlock(font);

   usage += get_mem_usage(font->entries);
   usage += get_mem_usage(font->slots);

   return usage;
}

static int vg_image_walk(VG_IMAGE_T *image, int size)
{
   int usage = 0;

   usage += get_mem_usage(image->image);

   return usage;
}

static int vg_child_image_walk(VG_CHILD_IMAGE_T *child_image, int size)
{
   int usage = 0;

   usage += get_mem_usage(child_image->parent);

   return usage;
}

static int vg_mask_layer_walk(VG_MASK_LAYER_T *mask_layer, int size)
{
   int usage = 0;

   usage += get_mem_usage(mask_layer->image);

   return usage;
}

static int vg_paint_walk(VG_PAINT_T *paint, int size)
{
   int usage = 0;

   usage += get_mem_usage(paint->pattern);

   if (paint->ramp != MEM_INVALID_HANDLE) {
      VG_RAMP_T *ramp = (VG_RAMP_T *)mem_lock(paint->ramp, NULL);
      usage += get_mem_usage(ramp->data);
      mem_unlock(paint->ramp);
   }

   usage += get_mem_usage(paint->ramp_stops);

   return usage;
}

static int vg_path_walk(VG_PATH_T *path, int size)
{
   int usage = 0;

   usage += get_mem_usage(path->coords);
   usage += get_mem_usage(path->segments);

   return usage;
}

static void set_iterator(VG_SET_T *map, MEM_HANDLE_T handle, void *p)
{
   *(int *)p += get_mem_usage(handle);
}

static int vg_server_shared_state_walk(VG_SERVER_SHARED_STATE_T *shared_state, int size)
{
   int usage = 0;

   vg_set_iterate(&shared_state->objects, set_iterator, &usage);

   return usage;
}

static int vg_server_state_walk(VG_SERVER_STATE_T *state, int size)
{
   int usage = 0;

   usage += get_mem_usage(state->shared_state);

   usage += get_mem_usage(state->stroke_paint);
   usage += get_mem_usage(state->fill_paint);

   usage += get_mem_usage(state->scissor.scissor);

   return usage;
}

#endif /* NO_OPENVG */

USAGE_RESOLVE_T resolves[] = {
   {khrn_image_term, (USAGE_WALK_T)image_walk},

   {glxx_texture_term, (USAGE_WALK_T)texture_walk},
   {glxx_buffer_term, (USAGE_WALK_T)buffer_walk},
   {glxx_shared_term, (USAGE_WALK_T)shared_walk},

   {egl_server_surface_term, (USAGE_WALK_T)egl_server_surface_walk},

   {glxx_server_state_term, (USAGE_WALK_T)gl11_server_state_walk},

   {glxx_server_state_term, (USAGE_WALK_T)gl20_server_state_walk},
   {gl20_bindings_term, (USAGE_WALK_T)gl20_bindings_walk},
   {gl20_uniform_info_term, (USAGE_WALK_T)gl20_uniform_info_walk},
   {gl20_attrib_info_term, (USAGE_WALK_T)gl20_attrib_info_walk},
   {gl20_shader_sources_term, (USAGE_WALK_T)gl20_shader_sources_walk},
   {gl20_program_term, (USAGE_WALK_T)gl20_program_walk},
   {gl20_shader_term, (USAGE_WALK_T)gl20_shader_walk},
   {glxx_renderbuffer_term, (USAGE_WALK_T)glxx_renderbuffer_walk},
   {glxx_framebuffer_term, (USAGE_WALK_T)glxx_framebuffer_walk},

#ifndef NO_OPENVG
   {vg_font_bprint_term, NULL},
   {vg_font_term, (USAGE_WALK_T)vg_font_walk},
   {vg_image_bprint_term, NULL},
   {vg_image_term, (USAGE_WALK_T)vg_image_walk},
   {vg_child_image_bprint_term, (USAGE_WALK_T)vg_child_image_walk},
   {vg_child_image_term, (USAGE_WALK_T)vg_child_image_walk},
   {vg_mask_layer_bprint_term, NULL},
   {vg_mask_layer_term, (USAGE_WALK_T)vg_mask_layer_walk},
   {vg_paint_bprint_term, NULL},
   {vg_paint_term, (USAGE_WALK_T)vg_paint_walk},
   {vg_path_bprint_term, NULL},
   {vg_path_term, (USAGE_WALK_T)vg_path_walk},
   {vg_server_shared_state_term, (USAGE_WALK_T)vg_server_shared_state_walk},
   {vg_server_state_term, (USAGE_WALK_T)vg_server_state_walk},
#endif /* NO_OPENVG */

   {NULL, NULL}
};

static int flag;

static int get_mem_usage(MEM_HANDLE_T handle)
{
   int usage = 0;

   if (handle && (mem_get_flags(handle) & MEM_FLAG_USER) != (MEM_FLAG_T)flag) {
      MEM_TERM_T term = mem_get_term(handle);
      int size = mem_get_size(handle);

      usage = size;

      mem_set_user_flag(handle, flag);

      if (term)
      {
         USAGE_RESOLVE_T *resolve;
         for (resolve = resolves; resolve->term; resolve++) {
            if (resolve->term == term) {
               if(resolve->walk) {
                  usage += resolve->walk(mem_lock(handle, NULL), size);
                  mem_unlock(handle);
               }
               break;
            }
         }
      }
   }

   return usage;
}

typedef struct {
   uint64_t pid;
   uint32_t private_usage;
   uint32_t shared_usage;
} USAGE_RECORD_T;

typedef struct {
   USAGE_RECORD_T *data;
   uint32_t count;
} USAGE_USERDATA_T;

static USAGE_RECORD_T *find_record(USAGE_USERDATA_T *user, uint64_t pid)
{
   uint32_t i;
   for (i = 0; i < user->count; i++) {
      if (!user->data[i].pid || user->data[i].pid == pid) {
         user->data[i].pid = pid;

         return &user->data[i];
      }
   }

   return NULL;
}

static void glcontext_callback(KHRN_MAP_T *map, uint32_t key, MEM_HANDLE_T value, void *user)
{
   MEM_TERM_T term = mem_get_term(value);

   int usage = get_mem_usage(value);

   UNUSED(map);
   UNUSED(key);

   if (flag == MEM_FLAG_USER) {
      USAGE_RECORD_T *record = NULL;

      if (term == glxx_server_state_term) {
         record = find_record((USAGE_USERDATA_T *)user, ((GLXX_SERVER_STATE_T *)mem_lock(value, NULL))->pid);
         mem_unlock(value);
      } else
         UNREACHABLE();

      if (record)
         record->private_usage += usage;
   }
}

#ifndef NO_OPENVG
static void vgcontext_callback(KHRN_MAP_T *map, uint32_t key, MEM_HANDLE_T value, void *user)
{
   VG_SERVER_STATE_T *state = (VG_SERVER_STATE_T *)mem_lock(value, NULL);

   int usage = get_mem_usage(value);

   UNUSED(map);
   UNUSED(key);

   if (flag == MEM_FLAG_USER) {
      USAGE_RECORD_T *record = find_record((USAGE_USERDATA_T *)user, state->pid);

      if (record)
         record->private_usage += usage;
   }

   mem_unlock(value);
}
#endif /* NO_OPENVG */

static void surface_callback(KHRN_MAP_T *map, uint32_t key, MEM_HANDLE_T value, void *user)
{
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(value, NULL);

   int usage = get_mem_usage(value);

   UNUSED(map);
   UNUSED(key);

   if (flag == MEM_FLAG_USER) {
      USAGE_RECORD_T *record = find_record((USAGE_USERDATA_T *)user, surface->pid);

      if (record)
         record->private_usage += usage;
   }

   mem_unlock(value);
}

EGL_SYNC_ID_T eglIntCreateSync_impl(uint32_t type, int32_t condition, int32_t status, uint32_t sem)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGL_SERVER_SYNC_T     *sync;
   EGL_SYNC_ID_T         result = 0;
   EGL_SERVER_STATE_T    *state = EGL_GET_SERVER_STATE();

   MEM_HANDLE_T shandle = MEM_ALLOC_STRUCT_EX(EGL_SERVER_SYNC_T, MEM_COMPACT_DISCARD);                   // check
   if (shandle == MEM_INVALID_HANDLE)
      return result;

   sync = (EGL_SERVER_SYNC_T *)mem_lock(shandle, NULL);

   sync->type = type;
   sync->condition = condition;
   sync->status = status;
   sync->pid = state->pid;
   sync->sem = sem;
   /* grab HW counter of when the sync was created (NOT tied to interlock system, but uses the same counter */
   sync->sequence = khrn_get_last_issued_seq() + 1;

   lockCallback();
   if (khrn_map_insert(&state->syncs, state->next_sync, shandle))
      result = state->next_sync++;
   unlockCallback();

   mem_unlock(shandle);
   mem_release(shandle);

   if (type == EGL_SYNC_FENCE_KHR) {
#ifndef NO_OPENVG
      if (thread->bound_api != EGL_OPENVG_API)
#endif /* NO_OPENVG */
      {
         GLXX_HW_FRAMEBUFFER_T fb;
         GLXX_HW_RENDER_STATE_T *rs;
         GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
         rs = glxx_install_framebuffer(state, &fb, false);
         if (rs) {
            if (rs->drawn)
               rs->fence_active = true;
            else {
               /* nothing in the pipeline, so just signal */
               sync->status = EGL_SIGNALED_KHR;
               khdispatch_send_async(ASYNC_COMMAND_POST, sync->pid, sync->sem);
               /* just kill what we have just created, so it doesnt create render jobs with nothing in */
               glxx_hw_discard_frame(rs);
            }
         } else {
            sync->status = EGL_SIGNALED_KHR;
            khdispatch_send_async(ASYNC_COMMAND_POST, sync->pid, sync->sem);
         }
         GLXX_UNLOCK_SERVER_STATE();
      }
#ifndef NO_OPENVG
      else {
         EGL_SERVER_SURFACE_T *surface = mem_lock(state->vgsurface, NULL);

         vg_be_fence_sync_used(surface->mh_color[surface->back_buffer_index]);

         mem_unlock(state->vgsurface);
      }
#endif /* NO_OPENVG */
   }

   return result;
}

void eglIntDestroySync_impl(EGL_SYNC_ID_T s)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T handle;
   EGL_SERVER_SYNC_T *sync;

   lockCallback();
   handle = khrn_map_lookup(&state->syncs, s);
   sync = (EGL_SERVER_SYNC_T *)mem_lock(handle, NULL);
   khdispatch_send_async(ASYNC_COMMAND_DESTROY, sync->pid, sync->sem);  //tell host to delete semaphore
   mem_unlock(handle);

   khrn_map_delete(&state->syncs, s);
   unlockCallback();
}

void eglSyncGetAttrib_impl(EGL_SYNC_ID_T s, int32_t attrib, int32_t * value)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T handle;
   EGL_SERVER_SYNC_T *sync;

   lockCallback();
   handle = khrn_map_lookup(&state->syncs, s);
   sync = (EGL_SERVER_SYNC_T *)mem_lock(handle, NULL);

   switch (attrib) {
   case EGL_SYNC_STATUS_KHR:
      *value = sync->status;
      break;
   case EGL_SYNC_CONDITION_KHR:
      *value = sync->condition;
      break;
   default:
      UNREACHABLE();
      break;
   }

   mem_unlock(handle);
   unlockCallback();
}

static void egl_khr_sync_callback(KHRN_MAP_T *map, uint32_t key, MEM_HANDLE_T value, void *used)
{
   EGL_SERVER_SYNC_T *sync = (EGL_SERVER_SYNC_T *)mem_lock(value, NULL);
   uint64_t *sequence = (uint64_t *)used;

   UNUSED(map);
   UNUSED(key);

   if ((sync->status != EGL_SIGNALED_KHR) &&
       (sync->sequence <= *sequence)) {
      khdispatch_send_async(ASYNC_COMMAND_POST, sync->pid, sync->sem);
      sync->status = EGL_SIGNALED_KHR;
   }

   mem_unlock(value);
}

void egl_khr_fence_update(uint64_t job_sequence)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   if (state->syncs.entries) {
      khrn_map_iterate(&state->syncs, egl_khr_sync_callback, &job_sequence);
   }
}
