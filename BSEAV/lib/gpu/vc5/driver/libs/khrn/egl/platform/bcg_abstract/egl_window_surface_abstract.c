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
#include "libs/core/lfmt/lfmt_translate_v3d.h"

#include "libs/platform/gmem.h"
#include "libs/platform/v3d_scheduler.h"

#include "egl_platform_abstract.h"
#include "egl_surface_common_abstract.h"

LOG_DEFAULT_CAT("egl_window_surface_abstract")

/* Our own representation of an egl_window_surface */
struct egl_window_surface
{
   EGL_SURFACE_T        base;                /* The driver's internal surface data */
   int                  interval;            /* pushed as part of buffer queue */
   khrn_image          *active_image;        /* The current back buffer as a khrn_image */
   BEGL_SwapchainBuffer native_back_buffer;  /* The current back buffer surface (opaque) */
   void                *native_window_state; /* Opaque data that the platform ties to the native window */
};

static EGL_SURFACE_METHODS_T fns;

static BEGL_BufferFormat get_begl_format(GFX_LFMT_T api_fmt, bool x_padded)
{
   switch (api_fmt)
   {
   case GFX_LFMT_R8_G8_B8_A8_UNORM:    return BEGL_BufferFormat_eA8B8G8R8;
   case GFX_LFMT_B5G6R5_UNORM:         return BEGL_BufferFormat_eR5G6B5;
   case GFX_LFMT_A4B4G4R4_UNORM:       return BEGL_BufferFormat_eA4B4G4R4;
   case GFX_LFMT_A1B5G5R5_UNORM:       return BEGL_BufferFormat_eA1B5G5R5;
   case GFX_LFMT_R8_G8_B8_UNORM:       return x_padded ? BEGL_BufferFormat_eX8B8G8R8 : BEGL_BufferFormat_eR8G8B8;
   case GFX_LFMT_R16_G16_B16_A16_FLOAT:return BEGL_BufferFormat_eA16B16G16R16_FP;
   case GFX_LFMT_R10G10B10A2_UNORM:    return BEGL_BufferFormat_eA2B10G10R10;
#if V3D_VER_AT_LEAST(3,3,0,0)
   case GFX_LFMT_BSTC_RGBA_UNORM:      return BEGL_BufferFormat_eBSTC;
#endif
   default:                         unreachable(); return BEGL_BufferFormat_INVALID;
   }
}

static EGLint dequeue_buffer(EGL_WINDOW_SURFACE_T *surf)
{
   BEGL_DisplayInterface *platform = &g_bcgPlatformData.displayInterface;

   /* What color format does the config request? */
   BEGL_BufferFormat format = get_begl_format(surf->base.config->color_api_fmt, surf->base.config->x_padded);

   /* Get our initial buffer */
   int fence = -1;
   int age = 0;

   assert(platform->GetNextSurface);
   surf->native_back_buffer = platform->GetNextSurface(platform->context,
         surf->native_window_state, format, surf->base.secure,
         &age, &fence);
   if (!surf->native_back_buffer)
      return EGL_BAD_NATIVE_WINDOW;

   surf->active_image = image_from_surface_abstract(BEGL_SWAPCHAIN_BUFFER,
         surf->native_back_buffer, /*flipY=*/true, "swapchain buffer",
         /*num_mip_levels=*/NULL, /*error=*/NULL);

   if (surf->active_image == NULL)
   {
      assert(platform->CancelSurface);
      platform->CancelSurface(platform->context, surf->native_window_state,
            surf->native_back_buffer, fence);
      return EGL_BAD_ALLOC;
   }

   /* window surfaces need to be renderable */
   GFX_LFMT_T gfx_format = khrn_image_get_lfmt(surf->active_image, 0);
   if (!gfx_lfmt_can_render_format(gfx_format))
   {
      platform->CancelSurface(platform->context, surf->native_window_state,
            surf->native_back_buffer, fence);
      KHRN_MEM_ASSIGN(surf->active_image, NULL);
      return EGL_BAD_NATIVE_WINDOW;
   }

   /* resize aux buffers if necessary */
   unsigned width, height;
   khrn_image_get_dimensions(surf->active_image, &width, &height, NULL, NULL);
   if (!egl_surface_base_resize(&surf->base, width, height))
   {
      platform->CancelSurface(platform->context, surf->native_window_state,
            surf->native_back_buffer, fence);
      surf->native_back_buffer = NULL;
      KHRN_MEM_ASSIGN(surf->active_image, NULL);
      return EGL_BAD_ALLOC;
   }

   egl_surface_base_update_buffer_age_heuristics(&surf->base,
         surf->active_image, age);

   if (fence == -1)
      return EGL_SUCCESS;

   uint64_t job_id = v3d_scheduler_submit_wait_fence(fence);
   khrn_resource_job_replace(khrn_image_get_resource(surf->active_image), job_id);

   return EGL_SUCCESS;
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

static EGLint swap_buffers(EGL_SURFACE_T *surface)
{
   EGL_WINDOW_SURFACE_T          *surf = (EGL_WINDOW_SURFACE_T *) surface;
   BEGL_DisplayInterface         *platform = &g_bcgPlatformData.displayInterface;

   if (surf->active_image)
   {
      const v3d_scheduler_deps *out_deps = egl_surface_flush_back_buffer_writer(surface);
      int fence = v3d_scheduler_create_fence(out_deps, V3D_SCHED_DEPS_COMPLETED, /*force_create=*/false);

      assert(platform->DisplaySurface);

      if (platform->DisplaySurface(platform->context, surf->native_window_state, surf->native_back_buffer, fence, surf->interval) != BEGL_Success)
         return EGL_BAD_NATIVE_WINDOW;

      KHRN_MEM_ASSIGN(surf->active_image, NULL);

      dump_gmem_status();
   }

   return EGL_SUCCESS;
}

/* Get the buffer to draw to */
static khrn_image *get_back_buffer(EGL_SURFACE_T *surface)
{
   EGL_WINDOW_SURFACE_T *surf = (EGL_WINDOW_SURFACE_T *) surface;

   if (surf->active_image == NULL)
      dequeue_buffer(surf);

   return surf->active_image;
}

EGLint get_attrib(EGL_SURFACE_T *surface, EGLint attrib, EGLint *value)
{
   EGL_WINDOW_SURFACE_T *surf = (EGL_WINDOW_SURFACE_T *) surface;

   switch (attrib)
   {
   case EGL_WIDTH:
   case EGL_HEIGHT:
      get_back_buffer(surface); /* force dequeue, this will handle resize */
      break;

   case EGL_BUFFER_AGE_EXT:
      if (!khrn_options.disable_buffer_age)
      {
         EGL_CONTEXT_T *context = egl_thread_get_context();
         if (!context || context->draw != surface)
            return EGL_BAD_SURFACE;

         if (!get_back_buffer(surface))
            return EGL_BAD_ALLOC;

         *value = egl_surface_base_query_buffer_age(surface);
      }
      else
         *value = 0;
      log_trace("Buffer age query = %d", *value);
      return EGL_SUCCESS;

   default:
      break;
   }
   return egl_surface_base_get_attrib(&surf->base, attrib, value);
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
   BEGL_DisplayInterface      *platform = &g_bcgPlatformData.displayInterface;

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

   surface->base.type = EGL_SURFACE_TYPE_NATIVE_WINDOW;

   error = egl_surface_base_init(&surface->base, &fns, config, attrib_list,
         attrib_type, /*width=*/0, /*height=*/0, win, NULL);
   if (error != EGL_SUCCESS)
      goto end;

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

   /* get the colour buffer and create auxiliary buffers */
   error = dequeue_buffer(surface);
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
   .get_attrib = get_attrib,
   .delete_fn = delete_fn,
};
