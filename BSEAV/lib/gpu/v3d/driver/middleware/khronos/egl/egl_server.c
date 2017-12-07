/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
#include "middleware/khronos/glxx/glxx_texture.h"
#include "interface/khronos/common/khrn_int_ids.h"
#include "middleware/khronos/gl11/gl11_server.h"
#include "middleware/khronos/glxx/glxx_hw.h"
#include "middleware/khronos/gl20/gl20_server.h"
#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/gl20/gl20_shader.h"
#include "middleware/khronos/glxx/glxx_framebuffer.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/common/2708/khrn_render_state_4.h"
#include "interface/khronos/egl/egl_client_surface.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/common/khrn_api_interposer.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/egl/egl_client_config.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "interface/khronos/egl/egl_int_config.h"

#if EGL_KHR_image
   #include "middleware/khronos/ext/egl_khr_image.h"
#endif

static BEGL_DriverInterfaces  s_driverInterfacesStruct;

static void server_process_attach(void);
static void server_process_detach(void);

/* register the driver interface */
EGLAPI void EGLAPIENTRY BEGL_RegisterDriverInterfaces(BEGL_DriverInterfaces *driverInterfaces)
{
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
         /* get it to break instantly to simplify error conditions later */
         vcos_demand(driverInterfaces->hwInterface->GetInfo != NULL);
         vcos_demand(driverInterfaces->hwInterface->SendJob != NULL);
         vcos_demand(driverInterfaces->hwInterface->GetNotification != NULL);
         vcos_demand(driverInterfaces->hwInterface->SendSync != NULL);
         vcos_demand(driverInterfaces->hwInterface->GetBinMemory != NULL);
         vcos_demand(driverInterfaces->displayInterface->BufferDequeue != NULL);
         vcos_demand(driverInterfaces->displayInterface->BufferQueue != NULL);
         vcos_demand(driverInterfaces->displayInterface->BufferCancel != NULL);
         vcos_demand(driverInterfaces->displayInterface->SurfaceGetInfo != NULL);
         vcos_demand(driverInterfaces->displayInterface->WindowPlatformStateCreate != NULL);
         vcos_demand(driverInterfaces->displayInterface->WindowPlatformStateDestroy != NULL);

         /* See if we already have a valid memory interface.
            * If not, we need to bring up the driver now.
            * If we do, and this is the same, we do nothing.
            * If we do, but this is different - try to close & restart driver */
         if (s_driverInterfacesStruct.memInterface == NULL)
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
         else if (s_driverInterfacesStruct.memInterface != driverInterfaces->memInterface)
         {
            /* Uh oh, memory interface has changed - better start again.
               I wouldn't normally expect any applications to hit this path! */
            client_process_detach();
            server_process_detach();

            s_driverInterfacesStruct = *driverInterfaces;

            server_process_attach();
            client_process_attach();
         }
         else if (s_driverInterfacesStruct.displayInterface != driverInterfaces->displayInterface)
         {
            /* install RSO configs (default) */
            egl_config_install_configs(0);

            s_driverInterfacesStruct = *driverInterfaces;
         }

         s_driverInterfacesStruct = *driverInterfaces;
      }
   }

   if (term)
   {
      /* Time to take down the driver */
      client_process_detach();
      server_process_detach();

      memset(&s_driverInterfacesStruct, 0, sizeof(BEGL_DriverInterfaces));
   }
}

EGLAPI BEGL_DriverInterfaces* EGLAPIENTRY BEGL_GetDriverInterfaces(void)
{
   return &s_driverInterfacesStruct;
}

extern BEGL_BufferHandle BEGLint_PixmapCreateCompatiblePixmap(BEGL_PixmapInfoEXT *pixmapInfo);
extern void BEGLint_intBufferGetRequirements(const BEGL_PixmapInfoEXT *bufferRequirements, BEGL_BufferSettings *bufferConstrainedRequirements);
extern void khrn_job_callback(void);

EGLAPI void EGLAPIENTRY BEGL_GetDefaultDriverInterfaces(BEGL_DriverInterfaces *driverInterfaces)
{
   memset(driverInterfaces, 0, sizeof(BEGL_DriverInterfaces));

   driverInterfaces->displayCallbacks.PixmapCreateCompatiblePixmap = BEGLint_PixmapCreateCompatiblePixmap;
   driverInterfaces->displayCallbacks.BufferGetRequirements = BEGLint_intBufferGetRequirements;

   driverInterfaces->hardwareCallbacks.JobCallback = khrn_job_callback;
}

/*
   void egl_server_surface_term(MEM_HANDLE_T handle)

   Terminator for EGL_SERVER_SURFACE_T

   Preconditions:

   handle is an (possibly uninitialised or partially initialised*) object of type EGL_SERVER_SURFACE_T

   Postconditions:

   Frees any references to resources that are referred to by the object of type EGL_SERVER_SURFACE_T
*/

void egl_server_surface_term(MEM_HANDLE_T handle)
{
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(handle, NULL);

   if (surface != NULL)
   {
      khrn_hw_common_finish();

      if (surface->mh_color[0] != MEM_HANDLE_INVALID) {
         ((KHRN_IMAGE_T *)mem_lock(surface->mh_color[0], NULL))->flags &= ~IMAGE_FLAG_BOUND_CLIENTBUFFER;
         mem_unlock(surface->mh_color[0]);
      }

      for (int i = 0; i < EGL_MAX_BUFFERS; i++)
         MEM_ASSIGN(surface->mh_color[i], MEM_HANDLE_INVALID);

      MEM_ASSIGN(surface->mh_depth, MEM_HANDLE_INVALID);
      MEM_ASSIGN(surface->mh_color_multi, MEM_HANDLE_INVALID);
      MEM_ASSIGN(surface->mh_ds_multi, MEM_HANDLE_INVALID);
      MEM_ASSIGN(surface->mh_bound_texture, MEM_HANDLE_INVALID);
   }

   mem_unlock(handle);
}

EGL_SERVER_STATE_T egl_server_state;

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

   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   if (driverInterfaces->memInterface != NULL)
      mem_init(driverInterfaces->memInterface);
   else
      return;

   /* install RSO configs (default) */
   egl_config_install_configs(0);

   if ((driverInterfaces->displayInterface != NULL) &&
      (driverInterfaces->displayInterface->DefaultOrientation != NULL) &&
      (driverInterfaces->displayInterface->DefaultOrientation(driverInterfaces->displayInterface) == BEGL_Success))
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

   MEM_ASSIGN(state->glcontext, MEM_HANDLE_INVALID);
   MEM_ASSIGN(state->gldrawsurface, MEM_HANDLE_INVALID);
   MEM_ASSIGN(state->glreadsurface, MEM_HANDLE_INVALID);
   MEM_ASSIGN(state->vgcontext, MEM_HANDLE_INVALID);
   MEM_ASSIGN(state->vgsurface, MEM_HANDLE_INVALID);

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
}

static MEM_HANDLE_T get_back_buffer(void *p)
{
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)p;
   return surface->mh_color[surface->back_buffer_index];
}

static MEM_HANDLE_T get_back_buffer_window_surface(void *p)
{
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)p;
   MEM_HANDLE_T himage = MEM_HANDLE_INVALID;

   if (surface->mh_active_image == MEM_HANDLE_INVALID)
   {
      int fd = -1;
      void *platform_pixmap = egl_server_platform_dequeue(surface->native_window_state, surface->colorformat, &fd);

      himage = egl_server_platform_create_pixmap_info(platform_pixmap, true);

      MEM_ASSIGN(surface->mh_active_image, himage);

      /* image not wrapped to KHRN_IMAGE_T somehow, just return */
      if (surface->mh_active_image == MEM_HANDLE_INVALID)
      {
         egl_server_platform_cancel(surface->native_window_state, platform_pixmap, fd);
         goto error;
      }

      KHRN_IMAGE_T *image = (KHRN_IMAGE_T *)mem_lock(surface->mh_active_image, NULL);
      KHRN_IMAGE_FORMAT_T format = image->format;
      if (!khrn_image_is_ok_for_render_target(format, false))
      {
         egl_server_platform_cancel(surface->native_window_state, platform_pixmap, fd);
         mem_unlock(surface->mh_active_image);
         MEM_ASSIGN(surface->mh_active_image, MEM_HANDLE_INVALID);
      }

      /* do this late when everything is OK */
      /* turns native fence into a v3d async job which can be waited on in the scheduler */
      image->v3dfence = khrn_fence_wait_async(fd);
      image->platform_pixmap = platform_pixmap;

      mem_unlock(surface->mh_active_image);
   }

error:
   if (himage != MEM_HANDLE_INVALID)
      mem_release(himage);

   return surface->mh_active_image;
}

static EGL_SURFACE_ID_T create_surface_internal(
   uintptr_t win,
   uint32_t buffers,
   uint32_t width,
   uint32_t height,
   bool secure,
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
   UNUSED(maskformat);

   /* todo: these flags aren't entirely correct... */

   /* clear, but also mark as invalid */
   KHRN_IMAGE_CREATE_FLAG_T color_image_create_flags =
      IMAGE_CREATE_FLAG_ONE | IMAGE_CREATE_FLAG_INVALID;

   /* if we're a window surface, might rotate/display */
   if (win != EGL_PLATFORM_WIN_NONE)
      color_image_create_flags |= IMAGE_CREATE_FLAG_PAD_ROTATE | IMAGE_CREATE_FLAG_DISPLAY;

   /* support rendering to if format is ok (don't force the format) */
   if (khrn_image_is_ok_for_render_target(colorformat, false)) {
      color_image_create_flags |= IMAGE_CREATE_FLAG_RENDER_TARGET;

      /* also support use as texture if not window surface (todo: this is hacky, and i don't think it's right) */
      if (win == EGL_PLATFORM_WIN_NONE)
         color_image_create_flags |= IMAGE_CREATE_FLAG_TEXTURE;
   }

   /* clear, but also mark as invalid */
   /* aux buffers are just there for rendering... */
   KHRN_IMAGE_CREATE_FLAG_T image_create_flags =
      IMAGE_CREATE_FLAG_ONE | IMAGE_CREATE_FLAG_INVALID | IMAGE_CREATE_FLAG_RENDER_TARGET;

   /* if we're a window surface, might rotate */
   if (win != EGL_PLATFORM_WIN_NONE)
      image_create_flags |= IMAGE_CREATE_FLAG_PAD_ROTATE;

   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   MEM_HANDLE_T shandle = MEM_ALLOC_STRUCT_EX(EGL_SERVER_SURFACE_T, MEM_COMPACT_DISCARD);                   // check, egl_server_surface_term
   if (shandle == MEM_HANDLE_INVALID)
      return 0;

   mem_set_term(shandle, egl_server_surface_term, NULL);

   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);

   if (mipmap) {
      assert(width > 0 && height > 0);
      assert(width <= EGL_CONFIG_MAX_WIDTH && height <= EGL_CONFIG_MAX_HEIGHT);
      buffers = _max(_msb(width), _msb(height)) + 1;
   }

   assert(buffers > 0);
   assert(buffers <= EGL_MAX_BUFFERS);
   assert(!mipmap || win == EGL_PLATFORM_WIN_NONE);
   assert(mipmap || win != EGL_PLATFORM_WIN_NONE || buffers == 1);

   surface->name = state->next_surface;
   surface->mipmap = mipmap;
   surface->buffers = buffers;
   surface->back_buffer_index = 0;
   assert(surface->mh_bound_texture == MEM_HANDLE_INVALID);
   surface->swap_interval = 1;
   surface->config_depth_bits = config_depth_bits;
   surface->config_stencil_bits = config_stencil_bits;
   surface->native_window_state = NULL;
   surface->get_back_buffer = get_back_buffer;
   surface->mh_active_image = MEM_HANDLE_INVALID;
   surface->colorformat = colorformat;    /* keep to validate the dequeue is OK */

   EGL_SURFACE_ID_T result = 0;
   MEM_HANDLE_T hcommon_storage = MEM_HANDLE_INVALID;
   if (hpixmapimage != MEM_HANDLE_INVALID) {
      /* Can't combine pixmap wrapping with any of the following features */
      assert(type == PIXMAP);
      assert(buffers == 1);
      assert(!mipmap);
      assert(win == EGL_PLATFORM_WIN_NONE);
      UNUSED(vgimagehandle);
      MEM_ASSIGN(surface->mh_color[0], hpixmapimage);
   } else {
      /* PBUFFER or WINDOW */
      if (mipmap) {
         uint32_t size = 0;
         for (uint32_t i = 0; i < buffers; i++) {
            uint32_t w, h;
            w = _max(width >> i, 1);
            h = _max(height >> i, 1);
            size += khrn_image_get_size(colorformat, w, h);
         }
         MEM_FLAG_T flags = MEM_FLAG_DIRECT;
         if (secure)
            flags |= MEM_FLAG_SECURE;
         hcommon_storage = mem_alloc_ex(size, 4096, flags,
            "EGL_SERVER_SURFACE_T mipmapped storage", MEM_COMPACT_DISCARD);
         if (hcommon_storage == MEM_HANDLE_INVALID)
            goto final;
      }

      if (type == WINDOW) {
         if (!egl_server_platform_create_window_state(&surface->native_window_state, win, secure))
            goto final;
         surface->get_back_buffer = get_back_buffer_window_surface;
      } else {

         /* NOTE: PBUFFER only create surfaces upfront */

         uint32_t offset = 0;
         for (uint32_t i = 0; i < buffers; i++) {

            uint32_t w, h;
            MEM_HANDLE_T himage;
            if (mipmap) {
               w = _max(width >> i, 1);
               h = _max(height >> i, 1);
            }
            else {
               w = width;
               h = height;
            }

            if (hcommon_storage != MEM_HANDLE_INVALID) {
               himage = khrn_image_create_from_storage(
                  colorformat, w, h,
                  khrn_image_get_stride(colorformat, w),
                  MEM_HANDLE_INVALID,
                  hcommon_storage,
                  offset,
                  IMAGE_CREATE_FLAG_TEXTURE | IMAGE_CREATE_FLAG_RENDER_TARGET, secure); /* todo: are these flags right? should IMAGE_CREATE_FLAG_INVALID be included? */
               offset += khrn_image_get_size(colorformat, w, h);
            }
            else
               himage = khrn_image_create(colorformat, w, h, color_image_create_flags, secure);

            if (himage == MEM_HANDLE_INVALID)
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

      MEM_HANDLE_T(*image_create)(KHRN_IMAGE_FORMAT_T format,
         uint32_t width, uint32_t height, KHRN_IMAGE_CREATE_FLAG_T flags, bool secure);
      image_create = (multisampleformat != IMAGE_FORMAT_INVALID) ? glxx_image_create_ms : khrn_image_create;
      himage = image_create(depthstencilformat, width, height,
         image_create_flags | IMAGE_CREATE_FLAG_NO_STORAGE, secure);

      if (himage == MEM_HANDLE_INVALID)
         goto final;

      MEM_ASSIGN(surface->mh_depth, himage);
      mem_release(himage);
   } else
      assert(surface->mh_depth == MEM_HANDLE_INVALID);

   if (multisampleformat != IMAGE_FORMAT_INVALID) {
      bool color, depth;
      switch (multisampleformat)
      {
      case DEPTH_COL_64_TLBD:
         color = true;
         depth = true;
         break;
      case COL_32_TLBD:
         color = true;
         depth = false;
         break;
      case DEPTH_32_TLBD:
      default:
         color = false;
         depth = true;
         break;
      }

      if (color) {
         MEM_HANDLE_T himage = glxx_image_create_ms(COL_32_TLBD,
            width, height,
            (color_image_create_flags | IMAGE_CREATE_FLAG_NO_STORAGE) & ~IMAGE_CREATE_FLAG_TEXTURE, secure);
         if (himage == MEM_HANDLE_INVALID)
            goto final;

         MEM_ASSIGN(surface->mh_color_multi, himage);
         mem_release(himage);
      }

      if (depth) {
         MEM_HANDLE_T himage = glxx_image_create_ms(DEPTH_32_TLBD,
            width, height,
            image_create_flags | IMAGE_CREATE_FLAG_NO_STORAGE, secure);
         if (himage == MEM_HANDLE_INVALID)
            goto final;

         MEM_ASSIGN(surface->mh_ds_multi, himage);
         mem_release(himage);
      }
   }
   else
   {
      assert(surface->mh_color_multi == MEM_HANDLE_INVALID);
      assert(surface->mh_ds_multi == MEM_HANDLE_INVALID);
   }

   if (khrn_map_insert(&state->surfaces, state->next_surface, shandle))
      result = state->next_surface++;

final:
   mem_unlock(shandle);
   mem_release(shandle);
   if (hcommon_storage != MEM_HANDLE_INVALID)
      mem_release(hcommon_storage);

   return result;
}

EGL_SURFACE_ID_T eglIntCreateSurface_impl(
   uintptr_t win,
   uint32_t buffers,
   uint32_t width,
   uint32_t height,
   bool secure,
   KHRN_IMAGE_FORMAT_T colorformat,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   KHRN_IMAGE_FORMAT_T multisampleformat,
   uint32_t mipmap,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits,
   uint32_t type)
{
    return create_surface_internal(win, buffers, width, height, secure, colorformat,
      depthstencilformat, maskformat, multisampleformat,
      mipmap, config_depth_bits, config_stencil_bits, MEM_HANDLE_INVALID,
      MEM_HANDLE_INVALID, type);
}

/*
   EGL_SURFACE_ID_T eglIntCreateWrappedSurface_impl(
      void *pixmap,
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
   void *platform_pixmap,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   KHRN_IMAGE_FORMAT_T multisample,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits)
{
   MEM_HANDLE_T himage = egl_server_platform_create_pixmap_info(platform_pixmap, false);
   if (himage != MEM_HANDLE_INVALID) {
      KHRN_IMAGE_T *image = (KHRN_IMAGE_T *)mem_lock(himage, NULL);
      EGL_SURFACE_ID_T result = create_surface_internal(
         EGL_PLATFORM_WIN_NONE,
         1,
         image->width,
         image->height,
         false,
         image->format,
         depthstencilformat,
         maskformat,
         multisample,
         0,
         config_depth_bits,
         config_stencil_bits,
         MEM_HANDLE_INVALID,
         himage,
         PIXMAP);
      mem_unlock(himage);
      mem_release(himage);
      return result;
   }
   else
      return 0;
}

static MEM_HANDLE_T create_shared_context(void)
{
   MEM_HANDLE_T handle = MEM_ALLOC_STRUCT_EX(GLXX_SHARED_T, MEM_COMPACT_DISCARD);                                // check, glxx_shared_term

   if (handle != MEM_HANDLE_INVALID) {
      mem_set_term(handle, glxx_shared_term, NULL);

      if (glxx_shared_init((GLXX_SHARED_T *)mem_lock(handle, NULL)))
         mem_unlock(handle);
      else {
         mem_unlock(handle);
         mem_release(handle);
         handle = MEM_HANDLE_INVALID;
      }
   }

   return handle;
}

static MEM_HANDLE_T get_shared_context(EGL_SERVER_STATE_T *state, EGL_GL_CONTEXT_ID_T share_id, EGL_CONTEXT_TYPE_T share_type)
{
   MEM_HANDLE_T result = MEM_HANDLE_INVALID;

   switch (share_type) {
   case OPENGL_ES_11:
   {
      GLXX_SERVER_STATE_T *server;
      MEM_HANDLE_T handle = khrn_map_lookup(&state->glcontexts, share_id);
      assert(handle != MEM_HANDLE_INVALID);
      server = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);
      result = server->mh_shared;
      mem_unlock(handle);
      break;
   }
   case OPENGL_ES_20:
   {
      GLXX_SERVER_STATE_T *server;
      MEM_HANDLE_T handle = khrn_map_lookup(&state->glcontexts, share_id);
      assert(handle != MEM_HANDLE_INVALID);
      server = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);
      result = server->mh_shared;
      mem_unlock(handle);
      break;
   }
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
      assert(share_type == OPENGL_ES_11 || share_type == OPENGL_ES_20);
      shandle = get_shared_context(state, share_id, share_type);
      mem_acquire(shandle);
   } else
      shandle = create_shared_context();

   if (shandle == MEM_HANDLE_INVALID)
      return 0;

   /*
      create and initialize state structure
   */

   handle = MEM_ALLOC_STRUCT_EX(GLXX_SERVER_STATE_T, MEM_COMPACT_DISCARD);                     // check, glxx_server_state_term
   if (handle == MEM_HANDLE_INVALID) {
      mem_release(shandle);
      return 0;
   }

   mem_set_term(handle, glxx_server_state_term, NULL);

   server = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);

   if (!gl11_server_state_init(server, state->next_context, shandle)) {
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
      assert(share_type == OPENGL_ES_11 || share_type == OPENGL_ES_20);
      shandle = get_shared_context(state, share, share_type);
      mem_acquire(shandle);
   } else
      shandle = create_shared_context();

   if (shandle == MEM_HANDLE_INVALID)
      return 0;

   /*
      create and initialize state structure
   */

   handle = MEM_ALLOC_STRUCT_EX(GLXX_SERVER_STATE_T, MEM_COMPACT_DISCARD);                     // check, glxx_server_state_term
   if (handle == MEM_HANDLE_INVALID) {
      mem_release(shandle);
      return 0;
   }

   mem_set_term(handle, glxx_server_state_term, NULL);

   if (!gl20_server_state_init((GLXX_SERVER_STATE_T *)mem_lock(handle, NULL), state->next_context, shandle)) {
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

static bool write_would_block(KHRN_INTERLOCK_T *interlock)
{
   /* Write will block if interlock would block, OR if the current interlock is still in flight in hardware */
   return khrn_interlock_write_would_block(interlock) ||
      interlock->pos > khrn_get_last_done_seq();
}

// Disassociates surface or context objects from their handles. The objects
// themselves still exist as long as there is a reference to them. In
// particular, if you delete part of a triple buffer group, the remaining color
// buffers plus the ancillary buffers all survive.

int eglIntDestroySurface_impl(EGL_SURFACE_ID_T s)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   MEM_HANDLE_T mh_surface = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = mem_lock(mh_surface, NULL);

   if (surface->native_window_state) {
      if (surface->mh_active_image) {

         /* complete any pending writes to the buffer */
         KHRN_IMAGE_T *image = mem_lock(surface->mh_active_image, NULL);

         bool deps = write_would_block(&image->interlock);
         khrn_interlock_write_immediate(&image->interlock);

         void *platform_pixmap = image->platform_pixmap;
         void *v3dfence = (void *)(uintptr_t)vcos_atomic_exchange((unsigned int *)&image->v3dfence, (unsigned int)((uintptr_t)NULL));

         mem_unlock(surface->mh_active_image);
         MEM_ASSIGN(surface->mh_active_image, MEM_HANDLE_INVALID);

         if (v3dfence)
            khrn_issue_fence_wait_job(v3dfence);

         int fd = -1;
         void *internal = NULL;

         if (deps)
            khrn_create_fence(&fd, &internal, 'C');

         // Issue a job to sync the pipe and actually display the buffer
         khrn_issue_swapbuffers_job(fd, internal, 'C');

         egl_server_platform_cancel(surface->native_window_state, platform_pixmap, fd);
      }

      egl_server_platform_destroy_window_state(surface->native_window_state);
   }

   mem_unlock(mh_surface);

   khrn_map_delete(&state->surfaces, s);

   return 0;
}

void eglIntDestroyGL_impl(EGL_GL_CONTEXT_ID_T c)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   khrn_map_delete(&state->glcontexts, c);
}

static void detach_buffers_from_gl(uint32_t glversion, MEM_HANDLE_T handle)
{
   if (handle != MEM_HANDLE_INVALID) {
      switch (glversion) {
      case EGL_SERVER_GL11:
      case EGL_SERVER_GL20:
      {
         GLXX_SERVER_STATE_T *state = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);
         glxx_server_state_set_buffers(state, MEM_HANDLE_INVALID, MEM_HANDLE_INVALID, MEM_HANDLE_INVALID,
            MEM_HANDLE_INVALID, MEM_HANDLE_INVALID, 0, 0);
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
   MEM_HANDLE_T read,
   MEM_HANDLE_T depth,
   MEM_HANDLE_T color_multi,
   MEM_HANDLE_T ds_multi,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits)
{
   if (handle != MEM_HANDLE_INVALID) {
      switch (glversion) {
      case EGL_SERVER_GL11:
      case EGL_SERVER_GL20:
      {
         GLXX_SERVER_STATE_T *state = (GLXX_SERVER_STATE_T *)mem_lock(handle, NULL);
         glxx_server_state_set_buffers(state, draw, read, depth, color_multi, ds_multi, config_depth_bits, config_stencil_bits);
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
   if (state) {
      EGL_SERVER_SURFACE_T *dsurface = (EGL_SERVER_SURFACE_T *)mem_lock(state->gldrawsurface, NULL);
      EGL_SERVER_SURFACE_T *rsurface = (EGL_SERVER_SURFACE_T *)mem_lock(state->glreadsurface, NULL);

      if (dsurface && rsurface) {

         MEM_HANDLE_T draw = dsurface->get_back_buffer(dsurface);
         MEM_HANDLE_T read = rsurface->get_back_buffer(rsurface);

         attach_buffers_to_gl(
            state->glversion,
            state->glcontext,
            draw,
            read,
            dsurface->mh_depth,
            dsurface->mh_color_multi,
            dsurface->mh_ds_multi,
            dsurface->config_depth_bits,
            dsurface->config_stencil_bits);
      }

      mem_unlock(state->gldrawsurface);
      mem_unlock(state->glreadsurface);
   }
}

// Selects the given process id for all operations. Most resource creation is
//  associated with the currently selected process id
// Selects the given context, draw and read surfaces for GL operations.
// Selects the given context and surface for VG operations.
// Any of the surfaces may be identical to each other.
// Contexts will not be flushed. (eglMakeCurrent must flush but this is handled
// by the client. It is not necessary to flush when switching threads
// If any of the surfaces have been resized then the color and ancillary buffers
//  are freed and recreated in the new size.
void eglIntMakeCurrent_impl(uint32_t glversion, EGL_GL_CONTEXT_ID_T gc,
   EGL_SURFACE_ID_T gdraw, EGL_SURFACE_ID_T gread)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   /* uint32_t thread_id = rtos_get_thread_id(); */

   //GL
   if (gc != EGL_SERVER_NO_GL_CONTEXT) {
      MEM_HANDLE_T chandle = khrn_map_lookup(&state->glcontexts, gc);
      MEM_HANDLE_T dhandle = khrn_map_lookup(&state->surfaces, gdraw);
      MEM_HANDLE_T rhandle = khrn_map_lookup(&state->surfaces, gread);

      assert(chandle != MEM_HANDLE_INVALID && dhandle != MEM_HANDLE_INVALID && rhandle != MEM_HANDLE_INVALID);

      if (chandle != state->glcontext || dhandle != state->gldrawsurface || rhandle != state->glreadsurface)
      {
         detach_buffers_from_gl(state->glversion, state->glcontext);

         assert(glversion == EGL_SERVER_GL11 || glversion == EGL_SERVER_GL20);

         GL11_FORCE_UNLOCK_SERVER_STATE();
         GL20_FORCE_UNLOCK_SERVER_STATE();
         state->glversion = glversion;
         MEM_ASSIGN(state->glcontext, chandle);
         MEM_ASSIGN(state->gldrawsurface, dhandle);
         MEM_ASSIGN(state->glreadsurface, rhandle);

         update_gl_buffers(state);
      }

   } else {
      assert(gdraw == EGL_SERVER_NO_SURFACE);
      assert(gread == EGL_SERVER_NO_SURFACE);
      if (state->glcontext != MEM_HANDLE_INVALID) {
         detach_buffers_from_gl(state->glversion, state->glcontext);
         GL11_FORCE_UNLOCK_SERVER_STATE();
         GL20_FORCE_UNLOCK_SERVER_STATE();
         state->glversion = 0;
         MEM_ASSIGN(state->glcontext, MEM_HANDLE_INVALID);
         MEM_ASSIGN(state->gldrawsurface, MEM_HANDLE_INVALID);
         MEM_ASSIGN(state->glreadsurface, MEM_HANDLE_INVALID);
      }
   }
}

static void flush_gl(uint32_t glversion, MEM_HANDLE_T handle, bool wait)
{
   if (handle != MEM_HANDLE_INVALID) {
      switch (glversion) {
      case EGL_SERVER_GL11:
      case EGL_SERVER_GL20:
      {
         glxx_server_state_flush(wait);
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
   UNUSED(flushvg);

   if (flushgl)
      flush_gl(state->glversion, state->glcontext, true);

   return 0;
}

void eglIntFlush_impl(uint32_t flushgl, uint32_t flushvg)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   UNUSED(flushvg);

   if (flushgl)
      flush_gl(state->glversion, state->glcontext, false);
}

bool eglIntBackBufferDims_impl(EGL_SURFACE_ID_T s, uint32_t *width, uint32_t *height)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T shandle = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);

   MEM_HANDLE_T mh_color = surface->get_back_buffer(surface);
   if (mh_color != MEM_HANDLE_INVALID) {
      KHRN_IMAGE_T *color = (KHRN_IMAGE_T *)mem_lock(mh_color, NULL);
      if (width)
         *width = color->width;
      if (height)
         *height = color->height;
      mem_unlock(mh_color);
      return true;
   }
   return false;
}

void eglIntSwapBuffers_impl(EGL_SURFACE_ID_T s)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T shandle = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);

   INCR_DRIVER_COUNTER(num_swaps);

   vcos_demand(surface->buffers >= 1);

   if (state->glcontext != MEM_HANDLE_INVALID)   /* This is a swap-chain */
   {
      GLXX_SERVER_STATE_T *glstate = (GLXX_SERVER_STATE_T *)mem_lock(state->glcontext, NULL);
      glxx_hw_invalidate_frame(glstate, false, true, true, true, true);
      mem_unlock(state->glcontext);
   }

   MEM_HANDLE_T mh_back_buffer = surface->get_back_buffer(surface);
   void *v3dfence = NULL;
   void *platform_pixmap = NULL;
   bool deps = false;
   if (mh_back_buffer != MEM_HANDLE_INVALID)
   {
      KHRN_IMAGE_T *color = (KHRN_IMAGE_T *)mem_lock(mh_back_buffer, NULL);
      /* We're going to write this buffer */
      deps = write_would_block(&color->interlock);
      khrn_interlock_write(&color->interlock, KHRN_INTERLOCK_USER_NONE);
      platform_pixmap = color->platform_pixmap;
      v3dfence = (void *)(uintptr_t)vcos_atomic_exchange((unsigned int *)&color->v3dfence, (unsigned int)((uintptr_t)NULL));
      mem_unlock(mh_back_buffer);
   }

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

   mem_unlock(shandle);

   if (v3dfence)
      khrn_issue_fence_wait_job(v3dfence);

   int fd = -1;
   void *internal = NULL;

   if (deps)
      khrn_create_fence(&fd, &internal, 'S');

   // Issue a job to sync the pipe and actually display the buffer
   khrn_issue_swapbuffers_job(fd, internal, 'S');

   egl_server_platform_queue(surface->native_window_state, platform_pixmap, surface->swap_interval, fd);

   MEM_ASSIGN(surface->mh_active_image, MEM_HANDLE_INVALID);

   // Buffers have swapped around, so need to make sure we're rendering into
   // the right ones.
   if (state->glcontext != MEM_HANDLE_INVALID)
      update_gl_buffers(state);
}

void eglIntSelectMipmap_impl(EGL_SURFACE_ID_T s, int level)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T shandle = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);

   assert(surface->mipmap);

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
   if (state->glcontext != MEM_HANDLE_INVALID)
      update_gl_buffers(state);
}

int eglIntCopyBuffers_impl(EGL_SURFACE_ID_T s, void *platform_pixmap)
{
   MEM_HANDLE_T himage = egl_server_platform_create_pixmap_info(platform_pixmap, false);
   if (himage == MEM_HANDLE_INVALID)
      return EGL_BAD_NATIVE_PIXMAP;

   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T shandle = khrn_map_lookup(&state->surfaces, s);
   EGL_SERVER_SURFACE_T *surface = (EGL_SERVER_SURFACE_T *)mem_lock(shandle, NULL);
   int error = EGL_SUCCESS;

   MEM_HANDLE_T mh_back_buffer = surface->get_back_buffer(surface);
   KHRN_IMAGE_T *src = (KHRN_IMAGE_T *)mem_lock(mh_back_buffer, NULL);
   KHRN_IMAGE_T *dst = (KHRN_IMAGE_T *)mem_lock(himage, NULL);

   if (mh_back_buffer == MEM_HANDLE_INVALID) {
      error = EGL_BAD_MATCH;
      goto end;
   }

   if (src->width != dst->width || src->height != dst->height) {
      error = EGL_BAD_MATCH;
      goto end;
   }

   KHRN_IMAGE_WRAP_T dst_wrap, src_wrap;
   khrn_image_lock_wrap(src, &src_wrap);
   khrn_image_lock_wrap(dst, &dst_wrap);
   khrn_image_wrap_copy_region(
      &dst_wrap, 0, 0,
      src->width, dst->width,
      &src_wrap, 0, 0,
      IMAGE_CONV_GL);
   khrn_image_unlock_wrap(src);
   khrn_image_unlock_wrap(dst);

end:
   mem_unlock(mh_back_buffer);
   mem_unlock(shandle);

   mem_unlock(himage);
   mem_release(himage);

   return error;
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
   khrn_image_wrap(&dst_wrap, format, width, height, stride, flags, false, data);

   //TODO: we currently expect the client to flush before calling this
   //I've added a khrn_interlock_read_immediate below. Can we remove the flush on the client?

   // TODO will this handle all necessary conversions correctly?
   // Will it handle images of different sizes?
   MEM_HANDLE_T mh_back_buffer = surface->get_back_buffer(surface);
   src = (KHRN_IMAGE_T *)mem_lock(mh_back_buffer, NULL);
   khrn_interlock_read_immediate(&src->interlock);
   khrn_image_lock_wrap(src, &src_wrap);
   khrn_image_wrap_copy_region(
      &dst_wrap, 0, 0,
      width, height,
      &src_wrap, 0, y_offset,
      IMAGE_CONV_GL);
   khrn_image_unlock_wrap(src);
   mem_unlock(mh_back_buffer);

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
   khrn_image_wrap(&src_wrap, format, width, height, stride, flags, false, (void *)data); /* casting away constness here, but we won't actually modify */

   // TODO will this handle all necessary conversions correctly?
   // Will it handle images of different sizes?
   MEM_HANDLE_T mh_back_buffer = surface->get_back_buffer(surface);
   dst = (KHRN_IMAGE_T *)mem_lock(mh_back_buffer, NULL);
   if (!dst->secure)
   {
      khrn_interlock_write_immediate(&dst->interlock);
      khrn_image_lock_wrap(dst, &dst_wrap);
      khrn_image_wrap_copy_region(
         &dst_wrap, 0, y_offset,
         width, height,
         &src_wrap, 0, 0,
         IMAGE_CONV_GL);
      khrn_image_unlock_wrap(dst);
   }
   mem_unlock(mh_back_buffer);

   mem_unlock(shandle);
}

static MEM_HANDLE_T get_active_gl_texture_2d(EGL_SERVER_STATE_T *state)
{
   MEM_HANDLE_T result = MEM_HANDLE_INVALID;
   if (state->glcontext != MEM_HANDLE_INVALID) {
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
   assert(!(image0->flags & IMAGE_FLAG_BOUND_EGLIMAGE));
   if (!(image0->flags & IMAGE_FLAG_BOUND_TEXIMAGE)) {
      if (thandle != MEM_HANDLE_INVALID) {
         GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)mem_lock(thandle, NULL);
         assert(texture->target != GL_TEXTURE_CUBE_MAP);
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

   if (surface->mh_bound_texture != MEM_HANDLE_INVALID)
   {
      MEM_HANDLE_T thandle = surface->mh_bound_texture;
      GLXX_TEXTURE_T *texture = (GLXX_TEXTURE_T *)mem_lock(thandle, NULL);
      glxx_texture_release_teximage(texture);   // this will remove IMAGE_FLAG_BOUND_TEXIMAGE from all images
      mem_unlock(thandle);

      MEM_ASSIGN(surface->mh_bound_texture, MEM_HANDLE_INVALID);
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
   usage += get_mem_usage(image->mh_palette);

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

   usage += get_mem_usage(state->mh_cache);

   return usage;
}

static int gl20_program_walk(GL20_PROGRAM_T *program, int size)
{
   int usage = 0;
   int i;

   UNUSED(size);

   usage += get_mem_usage(program->mh_vertex);
   usage += get_mem_usage(program->mh_fragment);
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

static void egl_server_sync_term(MEM_HANDLE_T handle);

USAGE_RESOLVE_T resolves[] = {
   {khrn_image_term, (USAGE_WALK_T)image_walk},

   {glxx_texture_term, (USAGE_WALK_T)texture_walk},
   {glxx_buffer_term, (USAGE_WALK_T)buffer_walk},
   {glxx_shared_term, (USAGE_WALK_T)shared_walk},

   {egl_server_surface_term, (USAGE_WALK_T)egl_server_surface_walk},

   {glxx_server_state_term, (USAGE_WALK_T)gl11_server_state_walk},

   {glxx_server_state_term, (USAGE_WALK_T)gl20_server_state_walk},
   {gl20_program_term, (USAGE_WALK_T)gl20_program_walk},
   {glxx_renderbuffer_term, (USAGE_WALK_T)glxx_renderbuffer_walk},
   {glxx_framebuffer_term, (USAGE_WALK_T)glxx_framebuffer_walk},
   {egl_server_sync_term, NULL},

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

static void egl_server_sync_term(MEM_HANDLE_T handle)
{
   EGL_SERVER_SYNC_T *sync = (EGL_SERVER_SYNC_T *)mem_lock(handle, NULL);
   vcos_semaphore_delete(&sync->sem);
   mem_unlock(handle);
}

EGL_SYNC_ID_T eglIntCreateSync_impl(uint32_t type, int32_t condition, int32_t status)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGL_SERVER_SYNC_T     *sync;
   EGL_SYNC_ID_T         result = 0;
   EGL_SERVER_STATE_T    *state = EGL_GET_SERVER_STATE();

   MEM_HANDLE_T shandle = MEM_ALLOC_STRUCT_EX(EGL_SERVER_SYNC_T, MEM_COMPACT_DISCARD);                   // check
   if (shandle == MEM_HANDLE_INVALID)
      return result;

   sync = (EGL_SERVER_SYNC_T *)mem_lock(shandle, NULL);

   if (vcos_semaphore_create(&sync->sem, "egl sync object", 0) != VCOS_SUCCESS) {
      mem_unlock(shandle);
      mem_release(shandle);
      return MEM_HANDLE_INVALID;
   }

   mem_set_term(shandle, egl_server_sync_term, NULL);

   sync->type = type;
   sync->condition = condition;
   sync->status = status;

   if (khrn_map_insert(&state->syncs, state->next_sync, shandle))
      result = state->next_sync++;

   mem_unlock(shandle);
   mem_release(shandle);

   if (type == EGL_SYNC_FENCE_KHR) {
      if (state->glcontext != MEM_HANDLE_INVALID) {
         GLXX_SERVER_STATE_T *glstate = GLXX_LOCK_SERVER_STATE();
         bool inserted = glxx_hw_insert_sync(glstate, shandle);
         GLXX_UNLOCK_SERVER_STATE();
         if (!inserted)
            goto end;
      }
   }

   return result;

end:
   khrn_map_delete(&state->syncs, result);
   return 0;
}

void eglIntDestroySync_impl(EGL_SYNC_ID_T s)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   khrn_map_delete(&state->syncs, s);
}

void eglSyncGetAttrib_impl(EGL_SYNC_ID_T s, int32_t attrib, int32_t * value)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   MEM_HANDLE_T handle;
   EGL_SERVER_SYNC_T *sync;

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
}

int eglIntSyncWaitTimeout_impl(EGL_SYNC_ID_T s, uint32_t timeout)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   glxx_server_state_flush(false);

   MEM_HANDLE_T handle = khrn_map_lookup(&state->syncs, s);
   VCOS_SEMAPHORE_T *sem = NULL;
   if (handle) {
      EGL_SERVER_SYNC_T *sync = (EGL_SERVER_SYNC_T *)mem_lock(handle, NULL);
      /* no wait whilst callbacks are locked */
      sem = &sync->sem;
   }
   mem_unlock(handle);

   int res = VCOS_EINVAL;
   if (sem) {
      if (timeout == 0xFFFFFFFF)
         res = vcos_semaphore_wait(sem);
      else
         res = vcos_semaphore_wait_timeout(sem, timeout);
   }

   return res;
}