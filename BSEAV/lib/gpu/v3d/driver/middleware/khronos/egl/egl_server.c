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
#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/egl/egl_client_config.h"
#include "interface/khronos/ext/egl_khr_sync_client.h"
#include "interface/khronos/ext/egl_brcm_driver_monitor_client.h"
#include "interface/khronos/egl/egl_int_impl.h"

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
      if (driverInterfaces->hwInterface == NULL ||
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

         if (driverInterfaces->displayInterface)
         {
            vcos_demand(driverInterfaces->displayInterface->BufferDequeue != NULL);
            vcos_demand(driverInterfaces->displayInterface->BufferQueue != NULL);
            vcos_demand(driverInterfaces->displayInterface->BufferCancel != NULL);
            vcos_demand(driverInterfaces->displayInterface->SurfaceGetInfo != NULL);
            vcos_demand(driverInterfaces->displayInterface->WindowPlatformStateCreate != NULL);
            vcos_demand(driverInterfaces->displayInterface->WindowPlatformStateDestroy != NULL);
         }

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
               get_core_revision_internal(16, revStr);
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

static bool write_would_block(KHRN_INTERLOCK_T *interlock)
{
   /* Write will block if interlock would block, OR if the current interlock is still in flight in hardware */
   return khrn_interlock_write_would_block(interlock) ||
      interlock->pos > khrn_get_last_done_seq();
}

void egl_server_surface_term(void *p)
{
   EGL_SURFACE_T *surface = p;

   if (surface->native_window_state) {
      if (surface->active_image) {

         /* complete any pending writes to the buffer */
         KHRN_IMAGE_T *image = surface->active_image;

         bool deps = write_would_block(&image->interlock);
         khrn_interlock_write_immediate(&image->interlock);

         void *platform_pixmap = image->platform_pixmap;
         uint64_t v3dfence = image->v3dfence;
         image->v3dfence = 0;

         KHRN_MEM_ASSIGN(surface->active_image, NULL);

         if (v3dfence)
            khrn_issue_fence_wait_job(v3dfence);

         int fd = -1;
         uint64_t internal = 0;

         if (deps)
            khrn_create_fence(&fd, &internal, 'C');

         // Issue a job to sync the pipe and actually display the buffer
         khrn_issue_swapbuffers_job(fd, internal, 'C');

         egl_server_platform_cancel(surface->native_window_state, platform_pixmap, fd);
      }

      egl_server_platform_destroy_window_state(surface->native_window_state);
   }

   khrn_hw_common_finish();

   if (surface->color[0] != NULL)
      surface->color[0]->flags &= ~IMAGE_FLAG_BOUND_CLIENTBUFFER;

   for (int i = 0; i < EGL_MAX_BUFFERS; i++)
      KHRN_MEM_ASSIGN(surface->color[i], NULL);

   KHRN_MEM_ASSIGN(surface->depth, NULL);
   KHRN_MEM_ASSIGN(surface->color_multi, NULL);
   KHRN_MEM_ASSIGN(surface->ds_multi, NULL);
   KHRN_MEM_ASSIGN(surface->bound_texture, NULL);
}

EGL_SERVER_STATE_T egl_server_state;

static VCOS_ONCE_T    once;
static bool           once_ok;

static void init_once(void)
{
   once_ok = true;
   // Create the gl context lock for that process
   glxx_context_gl_create_lock();
}

static bool ensure_init_once(void)
{
   vcos_demand(vcos_once(&once, init_once) == VCOS_SUCCESS);
   return once_ok;
}

bool egl_process_init(void)
{
   if (!ensure_init_once())
      return false;

   return true;
}

bool server_process_state_init(void)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   if (!state->inited) {
      state->next_context = 1;
      state->next_surface = 1;
      state->next_eglimage = 1;
      state->next_sync = 1;

      verify(khrn_map_init(&state->contexts, 64));
      verify(khrn_map_init(&state->surfaces, 64)); /* todo: handle failure */
      verify(khrn_map_init(&state->eglimages, 64)); /* todo: handle failure */
      verify(khrn_map_init(&state->syncs, 64));

      state->inited = true;

      if (!egl_process_init())
         goto end;

      /* legacy mode - display initialised before eglInitialize() */
      if (!state->display)
         state->display = (EGLDisplay)1;
   }
   return true;

end:
   server_process_state_term();
   return false;
}

static void server_process_attach(void)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   khrn_init_options();

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

   khrn_init_thread_pool();

   /* TODO : pass the "master event" as an argument here, so its clear when it can be deleted */
   /* or better - make the module create its own resources */
   khrn_hw_common_init(); /* todo: handle failure */
}

EGL_SERVER_STATE_T *egl_get_process_state(CLIENT_THREAD_STATE_T *thread, EGLDisplay dpy, EGLBoolean check_inited)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   if (state->display == dpy) {
      if (check_inited && !state->inited) {
         thread->error = EGL_NOT_INITIALIZED;
         return NULL;
      }
      else
         return state;
   }
   else {
      thread->error = EGL_BAD_DISPLAY;
      return NULL;
   }
}

void server_process_state_term(void)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   if (state->inited) {
      khrn_map_term(&state->contexts);
      khrn_map_term(&state->syncs);

#if EGL_BRCM_driver_monitor
      egl_driver_monitor_term(state);
#endif

      khrn_map_term(&state->surfaces);
      khrn_map_term(&state->eglimages);

      state->inited = false;
   }
}

static void egl_server_shutdown(void);

static void server_process_detach(void)
{
   server_process_state_term();
   egl_server_shutdown();
}

/*
 * Normally only called when egl_server_is_empty is true, but if it isn't then
 * we free all our stuff.
 * This shuts down everything set up by egl_server_startup_hack. At present this
 * includes the entire memory system.
 */
static void egl_server_shutdown(void)
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

static KHRN_IMAGE_T *get_back_buffer(void *p)
{
   EGL_SURFACE_T *surface = (EGL_SURFACE_T *)p;
   return surface->color[surface->back_buffer_index];
}

static KHRN_IMAGE_T *get_back_buffer_window_surface(void *p)
{
   EGL_SURFACE_T *surface = (EGL_SURFACE_T *)p;

   KHRN_IMAGE_T *image = NULL;
   if (surface->active_image == NULL)
   {
      int fd = -1;
#ifdef ANDROID
      /* TODO: At some point isolate the various object types so egl can be re-entrant on itself */
      CLIENT_UNLOCK();
#endif
      void *platform_pixmap = egl_server_platform_dequeue(surface->native_window_state, surface->colorformat, &fd);
#ifdef ANDROID
      CLIENT_LOCK();
#endif

      image = egl_server_platform_create_pixmap_info(platform_pixmap, true);

      KHRN_MEM_ASSIGN(surface->active_image, image);

      /* image not wrapped to KHRN_IMAGE_T somehow, just return */
      if (surface->active_image == NULL)
      {
         egl_server_platform_cancel(surface->native_window_state, platform_pixmap, fd);
         goto error;
      }

      KHRN_IMAGE_FORMAT_T format = image->format;
      if (!khrn_image_is_ok_for_render_target(format, false))
      {
         egl_server_platform_cancel(surface->native_window_state, platform_pixmap, fd);
         KHRN_MEM_ASSIGN(surface->active_image, NULL);
      }

      /* do this late when everything is OK */
      /* turns native fence into a v3d async job which can be waited on in the scheduler */
      image->v3dfence = khrn_fence_wait_async(fd);
      image->platform_pixmap = platform_pixmap;
   }

error:
   KHRN_MEM_ASSIGN(image, NULL);

   return surface->active_image;
}

static bool create_surface_internal(
   EGL_SURFACE_T *surface,
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
   KHRN_IMAGE_T *pixmap_image,
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

   if (mipmap) {
      assert(width > 0 && height > 0);
      assert(width <= EGL_CONFIG_MAX_WIDTH && height <= EGL_CONFIG_MAX_HEIGHT);
      buffers = _max(_msb(width), _msb(height)) + 1;
   }

   assert(buffers > 0);
   assert(buffers <= EGL_MAX_BUFFERS);
   assert(!mipmap || win == EGL_PLATFORM_WIN_NONE);
   assert(mipmap || win != EGL_PLATFORM_WIN_NONE || buffers == 1);

   surface->mipmap = mipmap;
   surface->buffers = buffers;
   surface->back_buffer_index = 0;
   assert(surface->bound_texture == NULL);
   surface->swap_interval = 1;
   surface->config_depth_bits = config_depth_bits;
   surface->config_stencil_bits = config_stencil_bits;
   surface->native_window_state = NULL;
   surface->get_back_buffer = get_back_buffer;
   surface->active_image = NULL;
   surface->colorformat = colorformat;    /* keep to validate the dequeue is OK */

   bool result = false;
   MEM_HANDLE_T hcommon_storage = MEM_HANDLE_INVALID;
   if (pixmap_image != NULL) {
      /* Can't combine pixmap wrapping with any of the following features */
      assert(type == PIXMAP);
      assert(buffers == 1);
      assert(!mipmap);
      assert(win == EGL_PLATFORM_WIN_NONE);
      KHRN_MEM_ASSIGN(surface->color[0], pixmap_image);
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
            "EGL_SURFACE_T mipmapped storage", MEM_COMPACT_DISCARD);
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
            if (mipmap) {
               w = _max(width >> i, 1);
               h = _max(height >> i, 1);
            }
            else {
               w = width;
               h = height;
            }

            KHRN_IMAGE_T *image = NULL;
            if (hcommon_storage != MEM_HANDLE_INVALID) {
               image = khrn_image_create_from_storage(
                  colorformat, w, h,
                  khrn_image_get_stride(colorformat, w),
                  MEM_HANDLE_INVALID,
                  hcommon_storage,
                  offset,
                  IMAGE_CREATE_FLAG_TEXTURE | IMAGE_CREATE_FLAG_RENDER_TARGET, secure); /* todo: are these flags right? should IMAGE_CREATE_FLAG_INVALID be included? */
               offset += khrn_image_get_size(colorformat, w, h);
            }
            else
               image = khrn_image_create(colorformat, w, h, color_image_create_flags, secure);

            if (image == NULL)
               goto final;

            KHRN_MEM_ASSIGN(surface->color[i], image);
            KHRN_MEM_ASSIGN(image, NULL);
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

      KHRN_IMAGE_T *(*image_create)(KHRN_IMAGE_FORMAT_T format,
         uint32_t width, uint32_t height, KHRN_IMAGE_CREATE_FLAG_T flags, bool secure);
      image_create = (multisampleformat != IMAGE_FORMAT_INVALID) ? glxx_image_create_ms : khrn_image_create;
      KHRN_IMAGE_T *image = image_create(depthstencilformat, width, height,
         image_create_flags | IMAGE_CREATE_FLAG_NO_STORAGE, secure);

      if (image == NULL)
         goto final;

      KHRN_MEM_ASSIGN(surface->depth, image);
      KHRN_MEM_ASSIGN(image, NULL);
   } else
      assert(surface->depth == NULL);

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
         KHRN_IMAGE_T *image = glxx_image_create_ms(COL_32_TLBD,
            width, height,
            (color_image_create_flags | IMAGE_CREATE_FLAG_NO_STORAGE) & ~IMAGE_CREATE_FLAG_TEXTURE, secure);
         if (image == NULL)
            goto final;

         KHRN_MEM_ASSIGN(surface->color_multi, image);
         KHRN_MEM_ASSIGN(image, NULL);
      }

      if (depth) {
         KHRN_IMAGE_T *image = glxx_image_create_ms(DEPTH_32_TLBD,
            width, height,
            image_create_flags | IMAGE_CREATE_FLAG_NO_STORAGE, secure);
         if (image == NULL)
            goto final;

         KHRN_MEM_ASSIGN(surface->ds_multi, image);
         KHRN_MEM_ASSIGN(image, NULL);
      }
   }
   else
   {
      assert(surface->color_multi == NULL);
      assert(surface->ds_multi == NULL);
   }

   result = true;

final:
   if (hcommon_storage != MEM_HANDLE_INVALID)
      mem_release(hcommon_storage);

   return result;
}

bool egl_create_surface(
   EGL_SURFACE_T *surface,
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
    return create_surface_internal(surface, win, buffers, width, height, secure, colorformat,
      depthstencilformat, maskformat, multisampleformat,
      mipmap, config_depth_bits, config_stencil_bits, NULL, type);
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

bool egl_create_wrapped_surface(
   EGL_SURFACE_T *surface,
   void *platform_pixmap,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   KHRN_IMAGE_FORMAT_T multisample,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits)
{
   KHRN_IMAGE_T *pixmap_image = egl_server_platform_create_pixmap_info(platform_pixmap, false);
   if (pixmap_image == NULL)
      return false;

   bool result = create_surface_internal(
      surface,
      EGL_PLATFORM_WIN_NONE,
      1,
      pixmap_image->width,
      pixmap_image->height,
      false,
      pixmap_image->format,
      depthstencilformat,
      maskformat,
      multisample,
      0,
      config_depth_bits,
      config_stencil_bits,
      pixmap_image,
      PIXMAP);

   KHRN_MEM_ASSIGN(pixmap_image, NULL);
   return result;
}

static GLXX_SHARED_T *create_shared_context(void)
{
   GLXX_SHARED_T *shared = KHRN_MEM_ALLOC_STRUCT(GLXX_SHARED_T);                                // check, glxx_shared_term

   if (shared != NULL) {
      khrn_mem_set_term(shared, glxx_shared_term);

      if (!glxx_shared_init(shared))
         KHRN_MEM_ASSIGN(shared, NULL);
   }

   return shared;
}

// Create server states. To actually use these, call eglIntMakeCurrent.
void *egl_create_glxx_server_state(void *sc, EGL_CONTEXT_TYPE_T share_type, bool secure)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   GLXX_SERVER_STATE_T *share_context = sc;

   /*
      create and initialize shared state structure
      or obtain one from the specified context
   */

   GLXX_SHARED_T *shared;
   if (share_context != NULL) {
      khrn_mem_acquire(share_context->shared);
      shared = share_context->shared;
   }
   else
      shared = create_shared_context();

   if (shared == NULL)
      return NULL;

   /*
      create and initialize state structure
   */

   GLXX_SERVER_STATE_T *glxx_state = KHRN_MEM_ALLOC_STRUCT(GLXX_SERVER_STATE_T);                     // check, glxx_server_state_term
   if (glxx_state == NULL) {
      KHRN_MEM_ASSIGN(shared, NULL);
      return NULL;
   }

   khrn_mem_set_term(glxx_state, glxx_server_state_term);

   bool (*server_state_init)(GLXX_SERVER_STATE_T *state, GLXX_SHARED_T *shared, bool secure) =
      (share_type == OPENGL_ES_11) ? gl11_server_state_init : gl20_server_state_init;

   if (!server_state_init(glxx_state, shared, secure))
      KHRN_MEM_ASSIGN(glxx_state, NULL);

   KHRN_MEM_ASSIGN(shared, NULL);

   return glxx_state;
}

static void attach_buffers_to_gl(
   KHRN_IMAGE_T *draw,
   KHRN_IMAGE_T *read,
   KHRN_IMAGE_T *depth,
   KHRN_IMAGE_T *color_multi,
   KHRN_IMAGE_T *ds_multi,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (state)
   {
      glxx_server_state_set_buffers(state, draw, read, depth, color_multi, ds_multi, config_depth_bits, config_stencil_bits);
      glxx_unlock_server_state(OPENGL_ES_ANY);
   }
}

void egl_update_gl_buffers(EGL_CURRENT_T *opengl)
{
   EGL_SURFACE_T *draw = opengl->draw;
   EGL_SURFACE_T *read = opengl->read;

   if (draw && read) {
      KHRN_IMAGE_T *_draw = draw->get_back_buffer(draw);
      KHRN_IMAGE_T *_read = read->get_back_buffer(read);

      EGL_CONTEXT_T *context = opengl->context;

      attach_buffers_to_gl(
         _draw,
         _read,
         draw->depth,
         draw->color_multi,
         draw->ds_multi,
         draw->config_depth_bits,
         draw->config_stencil_bits);
   }
}

void eglIntFlush_impl(bool flushgl)
{
   if (flushgl)
      flush_internal(false);
}

void eglIntFinish_impl(bool finishgl)
{
   if (finishgl)
      flush_internal(true);
}

bool egl_back_buffer_dims(EGL_SURFACE_T *surface, uint32_t *width, uint32_t *height)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   KHRN_IMAGE_T *color = surface->get_back_buffer(surface);
   if (color != NULL) {
      if (width)
         *width = color->width;
      if (height)
         *height = color->height;
      return true;
   }
   return false;
}

void egl_swapbuffers(EGL_SURFACE_T *surface)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   INCR_DRIVER_COUNTER(num_swaps);

   vcos_demand(surface->buffers >= 1);

   if (surface->thread->opengl.context != NULL)   /* This is a swap-chain */
   {
      EGL_CONTEXT_T *context = surface->thread->opengl.context;
      if (context->state != NULL)
      {
         GLXX_SERVER_STATE_T *glstate = glxx_lock_server_state(OPENGL_ES_ANY);
         glxx_hw_invalidate_frame(glstate, false, true, true, true, true);
         glxx_unlock_server_state(OPENGL_ES_ANY);
      }
   }

   KHRN_IMAGE_T *back_buffer = surface->get_back_buffer(surface);
   uint64_t v3dfence = 0;
   void *platform_pixmap = NULL;
   bool deps = false;
   if (back_buffer != NULL)
   {
      /* We're going to write this buffer */
      deps = write_would_block(&back_buffer->interlock);
      glxx_context_gl_lock();
      khrn_interlock_write(&back_buffer->interlock, KHRN_INTERLOCK_USER_NONE);
      glxx_context_gl_unlock();
      platform_pixmap = back_buffer->platform_pixmap;
      v3dfence = back_buffer->v3dfence;
      back_buffer->v3dfence = 0;
   }

   if (surface->color_multi)
   {
      KHRN_IMAGE_T *color_multi = surface->color_multi;
      glxx_context_gl_lock();
      khrn_interlock_invalidate(&color_multi->interlock);
      glxx_context_gl_unlock();
   }

   if (surface->ds_multi)
   {
      KHRN_IMAGE_T *ds_multi = surface->ds_multi;
      glxx_context_gl_lock();
      khrn_interlock_invalidate(&ds_multi->interlock);
      glxx_context_gl_unlock();
   }

   if (surface->depth)
   {
      KHRN_IMAGE_T *depth = surface->depth;
      glxx_context_gl_lock();
      khrn_interlock_invalidate(&depth->interlock);
      glxx_context_gl_unlock();
   }

   if (v3dfence)
      khrn_issue_fence_wait_job(v3dfence);

   int fd = -1;
   uint64_t internal = 0;

   if (deps)
      khrn_create_fence(&fd, &internal, 'S');

   // Issue a job to sync the pipe and actually display the buffer
   khrn_issue_swapbuffers_job(fd, internal, 'S');

#ifdef ANDROID
   /* TODO: At some point isolate the various object types so egl can be re-entrant on itself */
   CLIENT_UNLOCK();
#endif
   egl_server_platform_queue(surface->native_window_state, platform_pixmap, surface->swap_interval, fd);
#ifdef ANDROID
   CLIENT_LOCK();
#endif

   KHRN_MEM_ASSIGN(surface->active_image, NULL);

   // Buffers have swapped around, so need to make sure we're rendering into
   // the right ones.
   egl_update_gl_buffers(&surface->thread->opengl);
}

void egl_select_mipmap(EGL_SURFACE_T *surface, int level)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   assert(surface->mipmap);

   // If the value of this attribute is outside the range of supported
   // mipmap levels, the closest valid mipmap level is selected for rendering.
   if (level < 0)
      level = 0;
   else if ((uint32_t)level >= surface->buffers)
      level = surface->buffers - 1;

   surface->back_buffer_index = level;

   // Buffers have swapped around, so need to make sure we're rendering into
   // the right ones.
   egl_update_gl_buffers(&surface->thread->opengl);
}

int egl_copybuffers(EGL_SURFACE_T *surface, void *platform_pixmap)
{
   KHRN_IMAGE_T *dst = egl_server_platform_create_pixmap_info(platform_pixmap, false);
   if (dst == NULL)
      return EGL_BAD_NATIVE_PIXMAP;

   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   int error = EGL_SUCCESS;

   KHRN_IMAGE_T *src = surface->get_back_buffer(surface);

   if (src == NULL) {
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
   KHRN_MEM_ASSIGN(dst, NULL);

   return error;
}

void egl_get_color_data(EGL_SURFACE_T *surface, KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height, int32_t stride, uint32_t y_offset, void *data)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

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
   KHRN_IMAGE_T *src = surface->get_back_buffer(surface);

   khrn_interlock_read_immediate(&src->interlock);
   khrn_image_lock_wrap(src, &src_wrap);
   khrn_image_wrap_copy_region(
      &dst_wrap, 0, 0,
      width, height,
      &src_wrap, 0, y_offset,
      IMAGE_CONV_GL);
   khrn_image_unlock_wrap(src);
}

void egl_set_color_data(EGL_SURFACE_T *surface, KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height, int32_t stride, uint32_t y_offset, const void *data)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

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
   KHRN_IMAGE_T *dst = surface->get_back_buffer(surface);
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
}

static GLXX_TEXTURE_T *get_active_gl_texture_2d(void)
{
   GLXX_TEXTURE_T *result = NULL;
   GLXX_SERVER_STATE_T *glstate = glxx_lock_server_state(OPENGL_ES_ANY);
   if (glstate)
   {
      result = glstate->bound_texture[glstate->active_texture - GL_TEXTURE0].twod;
      glxx_unlock_server_state(OPENGL_ES_ANY);
   }
   return result;
}

bool egl_bind_tex_image(EGL_SURFACE_T *surface)
{
   GLXX_TEXTURE_T *texture = get_active_gl_texture_2d();
   KHRN_IMAGE_T *image0 = surface->color[0];

   /* We permit IMAGE_FLAG_BOUND_CLIENTBUFFER */
   assert(!(image0->flags & IMAGE_FLAG_BOUND_EGLIMAGE));
   bool result = false;
   if (!(image0->flags & IMAGE_FLAG_BOUND_TEXIMAGE)) {
      if (texture != NULL) {
         assert(texture->target != GL_TEXTURE_CUBE_MAP);
         glxx_texture_bind_images(texture, surface->buffers, surface->color, TEXTURE_STATE_BOUND_TEXIMAGE, surface->back_buffer_index);

         KHRN_MEM_ASSIGN(surface->bound_texture, texture);
      }
      result = true;
   }

   return result;
}

void egl_release_tex_image(EGL_SURFACE_T *surface)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   if (surface->bound_texture != NULL)
   {
      GLXX_TEXTURE_T *texture = surface->bound_texture;
      glxx_texture_release_teximage(texture);   // this will remove IMAGE_FLAG_BOUND_TEXIMAGE from all images
      KHRN_MEM_ASSIGN(surface->bound_texture, NULL);
   }
}
