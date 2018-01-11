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

/* Our own representation of an egl_window_surface */
struct egl_window_surface
{
   EGL_SURFACE_T  base;                      /* The driver's internal surface data */
   int            interval;                  /* pushed as part of buffer queue */
   khrn_image    *active_image;              /* The current back buffer as a khrn_image */
   void          *native_back_buffer;        /* The current back buffer surface (opaque) */
   void          *native_window_state;       /* Opaque data that the platform ties to the native window */
};

static EGL_SURFACE_METHODS_T fns;

static BEGL_BufferFormat get_begl_format(GFX_LFMT_T fmt)
{
   switch (fmt)
   {
   case GFX_LFMT_R8_G8_B8_A8_UNORM: return BEGL_BufferFormat_eA8B8G8R8;
   case GFX_LFMT_R8_G8_B8_X8_UNORM:
   case GFX_LFMT_R8_G8_B8_UNORM:    return BEGL_BufferFormat_eX8B8G8R8;
   case GFX_LFMT_B5G6R5_UNORM:      return BEGL_BufferFormat_eR5G6B5;
   case GFX_LFMT_A4B4G4R4_UNORM:    return BEGL_BufferFormat_eA4B4G4R4;
   case GFX_LFMT_A1B5G5R5_UNORM:    return BEGL_BufferFormat_eA1B5G5R5;
#if V3D_VER_AT_LEAST(3,3,0,0)
   case GFX_LFMT_BSTC_RGBA_UNORM:   return BEGL_BufferFormat_eBSTC;
#endif
   default:                         unreachable(); return BEGL_BufferFormat_INVALID;
   }
}

static egl_result_t dequeue_buffer(EGL_WINDOW_SURFACE_T *surf)
{
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;

   /* What color format does the config request? */
   BEGL_BufferFormat format = get_begl_format(surf->base.config->color_api_fmt);

   /* Get our initial buffer */
   int fence = -1;
   assert(platform->GetNextSurface);
   if (platform->GetNextSurface(platform->context, surf->native_window_state, format,
                                &format, &surf->native_back_buffer, surf->base.secure, &fence) != BEGL_Success)
      return EGL_RES_BAD_NATIVE_WINDOW;

   unsigned num_mip_levels;
   surf->active_image = image_from_surface_abstract(surf->native_back_buffer, true, &num_mip_levels);

   if (surf->active_image == NULL)
   {
      assert(platform->CancelSurface);
      platform->CancelSurface(platform->context, surf->native_window_state,
            surf->native_back_buffer, fence);
      return EGL_RES_NO_MEM;
   }

   /* window surfaces need to be renderable */
   GFX_LFMT_T gfx_format = khrn_image_get_lfmt(surf->active_image, 0);
   if (!egl_can_render_format(gfx_format))
   {
      platform->CancelSurface(platform->context, surf->native_window_state,
            surf->native_back_buffer, fence);
      KHRN_MEM_ASSIGN(surf->active_image, NULL);
      return EGL_RES_BAD_NATIVE_WINDOW;
   }

   /* Get age of dequeued buffer */
   surf->base.buffer_age = 0;
   if (platform->WindowGetInfo)
   {
      BEGL_WindowInfo info;
      if (platform->WindowGetInfo(platform->context, surf->native_window_state,
                                  BEGL_WindowInfoBackBufferAge, &info) == BEGL_Success)
         surf->base.buffer_age = info.backBufferAge;
   }

   if (fence == -1)
      return EGL_RES_SUCCESS;

   uint64_t job_id = v3d_scheduler_submit_wait_fence(fence);
   khrn_resource_job_replace(khrn_image_get_resource(surf->active_image), job_id);

   return EGL_RES_SUCCESS;
}

static void dump_gmem_status(void)
{
   static int  s_count = -1;
   static int  s_limit = 30;

   if (s_count == -1)
   {
      char value[VCOS_PROPERTY_VALUE_MAX];
      vcos_property_get("debug.gmem.dump_every_n_frames", value, sizeof(value), "30");
      if (strlen(value))
         s_limit = atoi(value);

      s_count = s_limit;   /* Trigger dump on this first swap */
   }

   s_count++;
   if (s_count >= s_limit)
   {
      gmem_print_level(LOG_TRACE); /* Dump the gmem status if trace level is enabled */
      s_count = 0;
   }
}

static egl_result_t swap_buffers(EGL_SURFACE_T *surface)
{
   EGL_WINDOW_SURFACE_T          *surf = (EGL_WINDOW_SURFACE_T *) surface;
   BEGL_DisplayInterface         *platform = &g_bcgPlatformData.displayInterface;

   if (surf->active_image)
   {
      const v3d_scheduler_deps *out_deps = egl_surface_flush_back_buffer_writer(surface);
      int fence = v3d_scheduler_create_fence(out_deps, V3D_SCHED_DEPS_COMPLETED, /*force_create=*/false);

      assert(platform->DisplaySurface);

      if (platform->DisplaySurface(platform->context, surf->native_window_state, surf->native_back_buffer, fence, surf->interval) != BEGL_Success)
         return EGL_RES_BAD_NATIVE_WINDOW;

      KHRN_MEM_ASSIGN(surf->active_image, NULL);

      dump_gmem_status();
   }

   return dequeue_buffer(surf);
}

static void get_dimensions(EGL_SURFACE_T *surface, unsigned *width, unsigned *height)
{
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;
   BEGL_WindowInfo       winInfo;

   if (platform->WindowGetInfo)
   {
      EGL_WINDOW_SURFACE_T *surf = (EGL_WINDOW_SURFACE_T *)surface;

      platform->WindowGetInfo(platform->context, surf->native_window_state, BEGL_WindowInfoWidth | BEGL_WindowInfoHeight, &winInfo);
      *width = winInfo.width;
      *height = winInfo.height;
   }
   else
   {
      *width = 0;
      *height = 0;
   }
}

/* Get the buffer to draw to */
static khrn_image *get_back_buffer(const EGL_SURFACE_T *surface)
{
   EGL_WINDOW_SURFACE_T *surf = (EGL_WINDOW_SURFACE_T *) surface;

   if (surf->active_image == NULL)
      dequeue_buffer(surf);

   return surf->active_image;
}

static void delete_fn(EGL_SURFACE_T *surface)
{
   EGL_WINDOW_SURFACE_T    *surf = (EGL_WINDOW_SURFACE_T *) surface;
   BEGL_DisplayInterface   *platform = &g_bcgPlatformData.displayInterface;

   if (!surface)
      return;

   if (surf->native_window_state)
   {
      if (surf->native_back_buffer)
      {
         int fence =  -1;

         /* we need to cancel with a fence for already flushed operations  + original
          * fence when we dequeued the surface */
         if (surf->active_image)
         {
            const v3d_scheduler_deps   *deps = NULL;

            egl_context_gl_lock();

            khrn_resource *resource = khrn_image_get_resource(surf->active_image);
            /* we should have flushed by now (when the surface stopped being current) */
            assert(!khrn_resource_has_reader_or_writer(resource));
            deps = &resource->pre_write;

            egl_context_gl_unlock();

            if (deps->n != 0)
               fence = v3d_scheduler_create_fence(deps, V3D_SCHED_DEPS_COMPLETED, /*force_create=*/false);

            /* release the image only after we used the deps, since deps is a
             * pointer inside the image */
            KHRN_MEM_ASSIGN(surf->active_image, NULL);
         }

         assert(platform->CancelSurface);
         platform->CancelSurface(platform->context, surf->native_window_state,
            surf->native_back_buffer, fence);
      }
      if (platform->WindowPlatformStateDestroy != NULL)
         platform->WindowPlatformStateDestroy(platform->context, surf->native_window_state);
   }

   assert(surf->active_image == NULL);

   egl_surface_base_destroy(surface);
   free(surface);
}

static EGLSurface egl_create_window_surface_impl(EGLDisplay dpy, EGLConfig config_in,
      EGLNativeWindowType win, const void *attrib_list,
      EGL_AttribType attrib_type)
{
   EGLint                     error = EGL_BAD_ALLOC;
   EGL_WINDOW_SURFACE_T       *surface;
   EGLSurface                 ret = EGL_NO_SURFACE;
   BEGL_WindowInfo            winInfo;
   unsigned int               width, height;
   BEGL_DisplayInterface      *platform = &g_bcgPlatformData.displayInterface;

   memset(&winInfo, 0, sizeof(BEGL_WindowInfo));

   if (!egl_initialized(dpy, true))
      return EGL_NO_SURFACE;

   surface = calloc(1, sizeof(*surface));
   if (!surface)
      goto end;

   const EGL_CONFIG_T *config = egl_config_validate(config_in);
   if (!config)
   {
      error = EGL_BAD_CONFIG;
      goto end;
   }

   /* Validate the window surface - this is an opaque structure, so just check the pointer right now */
   if (win == NULL || win == (EGLNativeWindowType)0xFFFFFFFF)
   {
      error = EGL_BAD_NATIVE_WINDOW;
      goto end;
   }

   surface->base.fns = &fns;

   /* set default swap interval */
   surface->interval = 1;

   /* Let the platform attach it's own data to the native window if required */
   if (platform->WindowPlatformStateCreate != NULL)
      surface->native_window_state = platform->WindowPlatformStateCreate(platform->context, (void*)win);
   else
      surface->native_window_state = (void*)win;

   if (surface->native_window_state == NULL)
   {
      error = EGL_BAD_NATIVE_WINDOW;
      goto end;
   }

   /* Determine size of the underlying native window */
   get_dimensions(&surface->base, &width, &height);

   if (width == 0 || height == 0)
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   surface->base.type = EGL_SURFACE_TYPE_NATIVE_WINDOW;

   error = egl_surface_base_init(&surface->base, &fns, config, attrib_list,
         attrib_type, width, height, win, NULL);
   if (error != EGL_SUCCESS)
      goto end;

   ret = egl_map_surface((EGL_SURFACE_T *) surface);
   if (!ret || ret == EGL_NO_SURFACE)
      goto end;

   error = EGL_SUCCESS;
end:
   if (error != EGL_SUCCESS)
   {
      egl_unmap_surface(ret);
      delete_fn((EGL_SURFACE_T *) surface);
      ret = EGL_NO_SURFACE;
   }
   egl_thread_set_error(error);
   return ret;
}

EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy,
      EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
   return egl_create_window_surface_impl(dpy, config, win, attrib_list,
         attrib_EGLint);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurface(EGLDisplay dpy,
      EGLConfig config, void *native_window, const EGLAttrib *attrib_list)
{
   return egl_create_window_surface_impl(dpy, config,
         (EGLNativeWindowType)native_window, attrib_list, attrib_EGLAttrib);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurfaceEXT(EGLDisplay dpy,
      EGLConfig config, void *native_window, const EGLint *attrib_list)
{
   return egl_create_window_surface_impl(dpy, config,
         (EGLNativeWindowType)native_window, attrib_list, attrib_EGLint);
}

/* Set the swap interval (see eglSwapInterval). It's up to the implementation to clamp
 * interval to whatever range it can support. */
static void swap_interval(EGL_SURFACE_T *surface, int interval)
{
   EGL_WINDOW_SURFACE_T          *surf = (EGL_WINDOW_SURFACE_T *)surface;

   /* EGL spec. 3.10.3, silently clamped to min/max */
   if (interval < EGL_CONFIG_MIN_SWAP_INTERVAL)
      interval = EGL_CONFIG_MIN_SWAP_INTERVAL;
   if (interval > EGL_CONFIG_MAX_SWAP_INTERVAL)
      interval = EGL_CONFIG_MAX_SWAP_INTERVAL;

   surf->interval = interval;
}

static EGL_SURFACE_METHODS_T fns =
{
   .get_back_buffer = get_back_buffer,
   .swap_buffers = swap_buffers,
   .swap_interval = swap_interval,
   .get_dimensions = get_dimensions,
   .delete_fn = delete_fn,
};
