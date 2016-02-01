/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION

=============================================================================*/

#include "vcos.h"
#include "helpers/demand.h" /* TODO Shouldn't be using demand in production code... */
#include "middleware/khronos/egl/egl_thread.h"
#include "middleware/khronos/egl/egl_display.h"
#include "middleware/khronos/egl/egl_context.h"
#include "middleware/khronos/egl/egl_surface.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/egl/egl_process.h"
#include "middleware/khronos/egl/egl_image.h"
#include "middleware/khronos/egl/egl_types.h"
#include "middleware/khronos/egl/egl_surface_base.h"
#include "middleware/khronos/egl/egl_attrib_list.h"
#include "interface/khronos/include/EGL/eglext_brcm.h"
#include "middleware/khronos/egl/egl_sync.h"

#define MAX_EXT_STRING (1024)

#define VCOS_LOG_CATEGORY (&egl_display_log)
VCOS_LOG_CAT_T egl_display_log = VCOS_LOG_INIT("egl_display", VCOS_LOG_INFO);

/*
 * A list of client extensions, independent of any display.
 *
 * Client extensions are introduced with EGL_EXT_client_extensions and became
 * core functionality in EGL 1.5.
 */
static char client_extension_string[MAX_EXT_STRING];

static struct
{
   VCOS_ONCE_T    once;
   bool           once_ok;
   VCOS_MUTEX_T   lock;
   EGL_MAP_T      contexts;
   EGL_MAP_T      surfaces;
   EGL_MAP_T      images;
   EGL_MAP_T      syncs;
   uintptr_t      next_handle;
   bool           initialized;

   /*
    * A list of display extensions.
    *
    * A display extension adds functionality to an individual EGLDisplay. This
    * type of extension has always existed but, until EGL_EXT_client_extensions,
    * lacked an identifying name.
    *
    * Note: client and display extension lists are disjoint.
    */
   char           extension_string[MAX_EXT_STRING];
}
egl_display;

static void init_extension_strings(void);

static void init_once(void)
{
   egl_display.once_ok = vcos_mutex_create(&egl_display.lock, "egl_display.lock") == VCOS_SUCCESS;
   if (!egl_display.once_ok)
   {
      vcos_log_error("Fatal: unable to init once egl_display");
   }
   init_extension_strings();
}

static bool ensure_init_once(void)
{
   demand(vcos_once(&egl_display.once, init_once) == VCOS_SUCCESS);
   return egl_display.once_ok;
}

static void init_extension_string(char *dest, size_t size, const char *common,
      const char *platform_specific)
{
   if ((platform_specific == NULL) || (*platform_specific == '\0'))
   {
      assert(strlen(common) < size);
      strncpy(dest, common, size - 1);
   }
   else
   {
      /* +1 for the space separating them */
      assert(strlen(common) + 1 + strlen(platform_specific) < size);
      vcos_safe_sprintf(dest, size, 0, "%s %s", common, platform_specific);
   }
}

static void init_extension_strings(void)
{
   static const char *client_extensions =
      "EGL_EXT_client_extensions " /* always included */
      "EGL_EXT_platform_base "
      "EGL_KHR_client_get_all_proc_addresses";

   static const char *display_extensions =
      "EGL_KHR_image "
      "EGL_KHR_image_base "
# if EGL_BRCM_performance_counters
      "EGL_BRCM_performance_counters "
# endif
# if EGL_BRCM_event_monitor
      "EGL_BRCM_event_monitor "
# endif
# if EGL_BRCM_gl_framebuffer_image
      "EGL_BRCM_gl_framebuffer_image "
# endif
# if EGL_EXT_create_context_robustness
      "EGL_EXT_create_context_robustness "
# endif
      "EGL_KHR_cl_event2 "
      "EGL_KHR_gl_colorspace "
      "EGL_KHR_gl_texture_2D_image "
      "EGL_KHR_gl_texture_cubemap_image "
      "EGL_KHR_gl_renderbuffer_image "
      "EGL_KHR_fence_sync "
      "EGL_KHR_wait_sync "
      "EGL_KHR_create_context "
      "EGL_KHR_surfaceless_context "
      "EGL_KHR_get_all_proc_addresses";

   init_extension_string(client_extension_string,
         sizeof(client_extension_string), client_extensions,
         egl_platform_get_client_extensions());

   init_extension_string(egl_display.extension_string,
         sizeof(egl_display.extension_string), display_extensions,
         egl_platform_get_display_extensions());
}

/*
 * Invent a handle for obj and store handle->obj in map. Return the handle or
 * 0 for no memory
 */
static const void *map_obj(EGL_MAP_T *map, void *obj)
{
   bool ok;
   const void *ret;

   vcos_mutex_lock(&egl_display.lock);

   do
      ret = (const void *) egl_display.next_handle++;
   while (ret == NULL);    /* because eventually it will wrap */

   ok = egl_map_insert(map, ret, obj);

   vcos_mutex_unlock(&egl_display.lock);

   return ok ? ret : 0;
}

static void* map_remove(EGL_MAP_T *map, const void *handle)
{
   void *value;

   vcos_mutex_lock(&egl_display.lock);
   value = egl_map_remove(map, handle);
   vcos_mutex_unlock(&egl_display.lock);

   return value;
}

/*
 * Look something up in a map, getting the display lock, which protects all
 * the maps.
 */
static void *map_lookup(const EGL_MAP_T *map, const void *handle)
{
   void *ret;

   vcos_mutex_lock(&egl_display.lock);
   ret = egl_map_lookup(map, handle);
   vcos_mutex_unlock(&egl_display.lock);

   return ret;
}

/* Like map_lookup only in reverse */
static const void *map_reverse_lookup(const EGL_MAP_T *map, void *value)
{
   const void *ret = NULL;

   vcos_mutex_lock(&egl_display.lock);
   ret = egl_map_reverse_lookup(map, value);
   vcos_mutex_unlock(&egl_display.lock);

   return ret;
}

EGL_CONTEXT_T *egl_get_context(EGLContext context)
{
   return map_lookup(&egl_display.contexts, context);
}

EGLContext egl_get_context_handle(EGL_CONTEXT_T *context)
{
   return (EGLContext) map_reverse_lookup(&egl_display.contexts, context);
}

EGL_SURFACE_T *egl_get_surface(EGLSurface surface)
{
   return map_lookup(&egl_display.surfaces, surface);
}

bool egl_any_surfaces_using_native_win(EGLNativeWindowType win)
{
   size_t      i = 0;
   EGL_PAIR_T  *pair;

   do
   {
      pair = egl_map_items(&egl_display.surfaces, &i);

      if (pair)
      {
         EGL_SURFACE_T *surf = pair->value;
         if (surf->native_window == win)
            return true;
      }
   }
   while (pair != NULL);

   return false;
}

bool egl_any_surfaces_using_native_pixmap(EGLNativePixmapType pixmap)
{
   size_t      i = 0;
   EGL_PAIR_T  *pair;

   do
   {
      pair = egl_map_items(&egl_display.surfaces, &i);

      if (pair)
      {
         EGL_SURFACE_T *surf = pair->value;
         if (surf->native_pixmap == pixmap)
            return true;
      }
   }
   while (pair != NULL);

   return false;
}

EGLSurface egl_get_surface_handle(EGL_SURFACE_T *surface)
{
   return (EGLSurface) map_reverse_lookup(&egl_display.surfaces, surface);
}

EGL_IMAGE_T *egl_get_image_refinc(EGLImageKHR image)
{
   EGL_IMAGE_T *egl_image;
   vcos_mutex_lock(&egl_display.lock);
   egl_image = egl_map_lookup(&egl_display.images, image);
   if (egl_image)
      egl_image_refinc(egl_image);
   vcos_mutex_unlock(&egl_display.lock);
   return egl_image;
}

EGLImageKHR egl_get_image_handle(EGL_IMAGE_T *image)
{
   return (EGLImageKHR) map_reverse_lookup(&egl_display.images, image);
}

EGL_SYNC_T *egl_get_sync_refinc(EGLSyncKHR sync)
{
   EGL_SYNC_T *egl_sync;
   vcos_mutex_lock(&egl_display.lock);
   egl_sync = egl_map_lookup(&egl_display.syncs, sync);
   if (egl_sync)
      egl_sync_refinc(egl_sync);
   vcos_mutex_unlock(&egl_display.lock);
   return egl_sync;
}

bool egl_initialized(EGLDisplay dpy, bool check_display)
{
   if (!ensure_init_once())
   {
      return false;
   }

   EGLint err = EGL_SUCCESS;
   EGL_THREAD_T *thread = egl_thread_get();

   if (thread == NULL)
      return false;

   vcos_mutex_lock(&egl_display.lock);

   if (!egl_display.initialized)
      err = EGL_NOT_INITIALIZED;

   if (check_display && dpy != egl_platform_get_default_display())
      err = EGL_BAD_DISPLAY;

   vcos_mutex_unlock(&egl_display.lock);

   thread->error = err;
   return err == EGL_SUCCESS;
}

bool egl_is_valid_display(EGLDisplay dpy)
{
   if (!ensure_init_once())
      return false;

   vcos_mutex_lock(&egl_display.lock);

   bool valid = dpy == egl_platform_get_default_display();

   vcos_mutex_unlock(&egl_display.lock);

   return valid;
}

EGLContext egl_map_context(EGL_CONTEXT_T *context)
{
   return (EGLContext) map_obj(&egl_display.contexts, context);
}

EGL_CONTEXT_T* egl_unmap_context(EGLContext context)
{
   return map_remove(&egl_display.contexts, context);
}

EGLSurface egl_map_surface(EGL_SURFACE_T *surface)
{
   return (EGLSurface) map_obj(&egl_display.surfaces, surface);
}

EGL_SURFACE_T* egl_unmap_surface(EGLSurface surface)
{
   return map_remove(&egl_display.surfaces, surface);
}

EGLImageKHR egl_map_image(EGL_IMAGE_T *image)
{
   return (EGLImageKHR) map_obj(&egl_display.images, image);
}

EGL_IMAGE_T* egl_unmap_image(EGLImageKHR image)
{
   return map_remove(&egl_display.images, image);
}

EGLSyncKHR egl_map_sync(EGL_SYNC_T *sync)
{
   return (EGLSyncKHR) map_obj(&egl_display.syncs, sync);
}

EGL_SYNC_T* egl_unmap_sync(EGLImageKHR sync_id)
{
   return map_remove(&egl_display.syncs, sync_id);
}

static EGLDisplay egl_get_display(EGLNativeDisplayType display_id)
{
   if (display_id == EGL_DEFAULT_DISPLAY)
      return egl_platform_get_default_display();

   if (egl_platform_set_default_display(display_id))
      return display_id;

   return EGL_NO_DISPLAY;
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id)
{
   return egl_get_display(display_id);
}

static EGLDisplay egl_get_platform_display_impl(EGLenum platform,
      void *native_display)
{
   EGLDisplay display;
   EGLNativeDisplayType display_id = (EGLNativeDisplayType)native_display;

   if (egl_platform_supported(platform))
   {
      display = egl_get_display(display_id);
   }
   else
   {
      egl_thread_set_error(EGL_BAD_PARAMETER);
      display = EGL_NO_DISPLAY;
   }
   return display;
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetPlatformDisplay(EGLenum platform,
      void *native_display, const EGLAttrib *attrib_list)
{
   UNUSED(attrib_list);
   return egl_get_platform_display_impl(platform, native_display);
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetPlatformDisplayEXT(EGLenum platform,
      void *native_display, const EGLint *attrib_list)
{
   UNUSED(attrib_list);
   return egl_get_platform_display_impl(platform, native_display);
}

EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy,
      EGLint *major, EGLint *minor)
{
   EGL_THREAD_T *thread = egl_thread_get();
   if (!thread)
   {
      return EGL_FALSE;
   }

   if (dpy != egl_platform_get_default_display())
   {
      thread->error = EGL_BAD_DISPLAY;
      return EGL_FALSE;
   }

   if (!ensure_init_once())
   {
      thread->error = EGL_NOT_INITIALIZED;
      return EGL_FALSE;
   }

   vcos_mutex_lock(&egl_display.lock);

   if (!egl_display.initialized)
   {
      bool proc_init = false;
      bool slock_init = false;

      egl_map_init(&egl_display.contexts);
      egl_map_init(&egl_display.surfaces);
      egl_map_init(&egl_display.images);
      egl_map_init(&egl_display.syncs);

      slock_init = egl_syncs_lock_init();
      if (!slock_init)
         goto end;

      proc_init = egl_process_init();
      if (!proc_init)
         goto end;

      egl_display.next_handle = 1;
      egl_display.initialized = true;

   end:
      if (!egl_display.initialized)
      {
         if (proc_init) egl_process_release();
         if (slock_init) egl_syncs_lock_deinit();
         egl_map_destroy(&egl_display.surfaces);
         egl_map_destroy(&egl_display.contexts);
         egl_map_destroy(&egl_display.images);
         egl_map_destroy(&egl_display.syncs);
         thread->error = EGL_NOT_INITIALIZED;
      }
   }

   if (egl_display.initialized)
   {
      if (major) *major = 1;
      if (minor) *minor = 5;
      thread->error = EGL_SUCCESS;
   }

   vcos_mutex_unlock(&egl_display.lock);
   return EGL_TRUE;
}

typedef void (*destructor_t)(void *);

/* Call destroy for every value in the map, and then destroy the map itself */
static void destroy_map(EGL_MAP_T *map, destructor_t destroy)
{
   EGL_PAIR_T *pair;

   while ((pair = egl_map_pop(map)) != NULL)
      destroy(pair->value);

   egl_map_destroy(map);
}

void egl_terminate(void)
{
   EGL_MAP_T contexts, surfaces, images, syncs;
   vcos_mutex_lock(&egl_display.lock);
   if (!egl_display.initialized)
   {
      vcos_mutex_unlock(&egl_display.lock);
      return;
   }
   egl_map_move(&contexts, &egl_display.contexts);
   egl_map_move(&surfaces, &egl_display.surfaces);
   egl_map_move(&images, &egl_display.images);
   egl_map_move(&syncs, &egl_display.syncs);
   egl_syncs_lock_deinit();
   egl_process_release();
   egl_display.initialized = false;
   vcos_mutex_unlock(&egl_display.lock);

   destroy_map(&contexts, (destructor_t) egl_context_try_delete);
   destroy_map(&surfaces, (destructor_t) egl_surface_try_delete);
   destroy_map(&images, (destructor_t) egl_image_refdec);
   destroy_map(&syncs, (destructor_t) egl_sync_refdec);
}

EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy)
{
   EGL_THREAD_T *thread = egl_thread_get();

   if (thread == NULL || !egl_initialized(dpy, false))
   {
      /* Special case for eglTerminate :
         "Termination of a display that has already been terminated, or has not yet been
          initialized, is allowed, but the only effect of such a call is to return EGL_TRUE"
       */
      if (thread == NULL || thread->error == EGL_NOT_INITIALIZED)
      {
         if (thread != NULL)
            thread->error = EGL_SUCCESS;
         return EGL_TRUE;
      }
   }

   if (dpy != egl_platform_get_default_display())
   {
      thread->error = EGL_BAD_DISPLAY;
      return EGL_FALSE;
   }

   egl_terminate();

   thread->error = EGL_SUCCESS;
   return EGL_TRUE;
}

EGLAPI const char *EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name)
{
   char const* ret = NULL;

   if (!ensure_init_once())
      goto end;

   if (dpy == EGL_NO_DISPLAY && name == EGL_EXTENSIONS)
   {
      // Since we support EGL_EXT_client_extensions, it is valid to query using
      // EGL_NO_DISPLAY. In this case, the only valid name is EGL_EXTENSIONS.
      // The resulting string must include "EGL_EXT_client_extensions"
      // at minimum.
      ret = client_extension_string;
      goto end;
   }

   if (!egl_initialized(dpy, true))
      return NULL;

   switch (name)
   {
   case EGL_CLIENT_APIS:
      ret = "OpenGL_ES";
      break;
   case EGL_EXTENSIONS:
      ret = egl_display.extension_string;
      break;
   case EGL_VENDOR:
      ret = "Broadcom";
      break;
   case EGL_VERSION:
      ret = "1.5";
      break;
   default:
      break;
   }

end:
   egl_thread_set_error(ret != NULL ? EGL_SUCCESS : EGL_BAD_PARAMETER);
   return ret;
}
