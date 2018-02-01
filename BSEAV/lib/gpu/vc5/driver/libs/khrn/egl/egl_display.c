/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "libs/util/demand.h" /* TODO Shouldn't be using demand in production code... */
#include "egl_thread.h"
#include "egl_display.h"
#include "egl_context.h"
#include "egl_surface.h"
#include "egl_platform.h"
#include "egl_process.h"
#include "egl_image.h"
#include "egl_types.h"
#include "egl_surface_base.h"
#include "egl_attrib_list.h"
#include <EGL/eglext_brcm.h>
#include "egl_sync.h"
#include "egl_client_exts.h"
#include "egl_display_exts.h"
#include "../common/khrn_vector.h"
#include <assert.h>

#define PLATFORM_EXTS_STR_MAX_SIZE (1024)

LOG_DEFAULT_CAT("egl_display")

/*
 * A list of client extensions, independent of any display.
 *
 * Client extensions are introduced with EGL_EXT_client_extensions and became
 * core functionality in EGL 1.5.
 */
static char client_extension_string[EGL_CLIENT_EXTS_STR_MAX_SIZE + PLATFORM_EXTS_STR_MAX_SIZE];

static struct
{
   EGLDisplay     display;
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
   char           extension_string[EGL_DISPLAY_EXTS_STR_MAX_SIZE + PLATFORM_EXTS_STR_MAX_SIZE];
}
egl_display;

typedef struct platform_display
{
   void *handle;
   int refs;
} platform_display;

typedef struct platform_displays
{
   khrn_vector displays;
   VCOS_MUTEX_T lock;
} platform_displays;

static EGLDisplay add_platform_display(platform_displays *displays,
      void *handle)
{
   vcos_mutex_lock(&displays->lock);

   platform_display *display = khrn_vector_data(platform_display,
         &displays->displays);
   size_t i;
   for (i = 0; i < displays->displays.size; i++)
      if (display[i].handle == handle)
         break;
   if (i == displays->displays.size) /* not found */
   {
      platform_display *display = khrn_vector_emplace_back(platform_display,
            &displays->displays);
      if (display)
      {
         /* vector extended, now i == displays->displays.size - 1 */
         display->handle = handle;

         /* Reference count == 0 is not a mistake, it's EGL display weirdness.
          *
          * An app can eglGetDisplay() and the display handle remains valid
          * for the entire lifetime of a process. A call to eglInitialize()
          * initialises the display but a call eglTerminate() only invalidates
          * resource handles and marks display as uninitialised. Actual release
          * of resources is done by calling both eglTerminate(dpy) and
          * eglMakeCurrent(dpy, EGL_NO_CONTEXT, EGL_NO_SURFACE, EGL_NO_SURFACE)
          * in any order. Once none of display resources is in use the platform
          * display can be terminated. But the display handle remains valid and
          * it's allowed to eglInitialise() it again, hence the reference count.
          */
         display->refs = 0;
      }
   }
   vcos_mutex_unlock(&displays->lock);

   return i < displays->displays.size ? (EGLDisplay)(i + 1) : EGL_NO_DISPLAY;
}

static inline platform_display *to_platform_display(platform_displays *displays,
      EGLDisplay egl_display)
{
   assert(vcos_mutex_is_locked(&displays->lock));

   size_t i = (size_t)egl_display - 1;
   if (i < displays->displays.size)
      return khrn_vector_data(platform_display, &displays->displays) + i;
   else
      return NULL;
}

static bool get_platform_display(platform_displays *displays,
      EGLDisplay egl_display)
{
   vcos_mutex_lock(&displays->lock);

   platform_display *display = to_platform_display(displays, egl_display);
   bool got = false;
   if (display &&
         (display->refs > 0 || egl_platform_initialize(display->handle)))
   {
     ++display->refs;
     got = true;
   }
   vcos_mutex_unlock(&displays->lock);
   return got;
}

static bool put_platform_display(platform_displays *displays,
      EGLDisplay egl_display)
{
   vcos_mutex_lock(&displays->lock);

   platform_display *display = to_platform_display(displays, egl_display);
   bool put = false;
   if (display)
   {
      assert(display->refs > 0);
      if (display->refs == 1)
         egl_platform_terminate(display->handle);
      --display->refs;
      put = true;
   }
   vcos_mutex_unlock(&displays->lock);
   return put;
}

static platform_displays s_platform_displays;

static void term_platform_displays(void)
{
   platform_display *ptr = khrn_vector_data(platform_display,
         &s_platform_displays.displays);
   platform_display *end = ptr + s_platform_displays.displays.size;
   while (ptr < end)
   {
      if (ptr->refs > 0)
      {
#ifndef NDEBUG
         fprintf(stderr,
               "Warning: Program exit with EGL display still in use!\n"
               "         Forcing platform display termination.\n");
#endif
         egl_platform_terminate(ptr->handle);
      }
      ++ptr;
   }
   vcos_mutex_delete(&s_platform_displays.lock);
}

static bool init_platform_displays(void)
{
   memset(&s_platform_displays, 0, sizeof(s_platform_displays));
   if (vcos_mutex_create(&s_platform_displays.lock, "EGL displays lock")
         != VCOS_SUCCESS)
      return false;
   atexit(term_platform_displays);
   return true;
}

extern bool egl_display_refinc(EGLDisplay dpy)
{
   return get_platform_display(&s_platform_displays, dpy);
}

extern bool egl_display_refdec(EGLDisplay dpy)
{
   return put_platform_display(&s_platform_displays, dpy);
}

extern void *egl_display_platform_handle(EGLDisplay dpy)
{
   vcos_mutex_lock(&s_platform_displays.lock);
   platform_display *display = to_platform_display(&s_platform_displays, dpy);
   void *handle = display ? display->handle : NULL;
   vcos_mutex_unlock(&s_platform_displays.lock);
   return handle;
}

static void init_extension_string(char *dest, size_t size,
   char *(*platform_independent)(char *), const char *platform_specific);

static void init_once(void)
{
   egl_display.once_ok = (vcos_mutex_create(&egl_display.lock,
         "egl_display.lock") == VCOS_SUCCESS) && init_platform_displays();
   if (!egl_display.once_ok)
   {
      log_error("Fatal: unable to init once egl_display");
   }
   init_extension_string(client_extension_string,
         sizeof(client_extension_string), egl_client_exts_str,
         egl_platform_get_client_extensions());
}

static bool ensure_init_once(void)
{
   demand(vcos_once(&egl_display.once, init_once) == VCOS_SUCCESS);
   return egl_display.once_ok;
}

static void init_extension_string(char *dest, size_t size,
   char *(*platform_independent)(char *), const char *platform_specific)
{
   char *s = platform_independent(dest);
   if (platform_specific && *platform_specific)
   {
      if (s != dest)
         *(s++) = ' ';
      assert((s + strlen(platform_specific) - dest) < (ptrdiff_t)size);
      strcpy(s, platform_specific);
   }
   else
      assert((s - dest) < (ptrdiff_t)size);
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

   if (check_display && dpy != egl_display.display)
      err = EGL_BAD_DISPLAY;

   vcos_mutex_unlock(&egl_display.lock);

   thread->error = err;
   return err == EGL_SUCCESS;
}

bool egl_is_valid_display(EGLDisplay dpy)
{
   vcos_mutex_lock(&s_platform_displays.lock);
   bool valid = (to_platform_display(&s_platform_displays, dpy) != NULL);
   vcos_mutex_unlock(&s_platform_displays.lock);
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

static EGLDisplay egl_get_platform_display_impl(EGLenum platform,
      void *native_display, const void *attrib_list, EGL_AttribType type)
{
   EGLDisplay egl_display = EGL_NO_DISPLAY;
   EGLint error = EGL_NOT_INITIALIZED;
   if (!ensure_init_once())
      goto end;

   /*
    * In comparison to other functions, eglGetDisplay can be called
    * without egl been initialised therefore don't call egl_initialized
    * and set the thread error state here.
    */
   void *handle;
   error = egl_platform_get_display(platform, native_display,
         attrib_list, type, &handle);
   if (error == EGL_SUCCESS)
      egl_display = add_platform_display(&s_platform_displays, handle);
end:
   egl_thread_set_error(error);
   return egl_display;
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id)
{
   return egl_get_platform_display_impl(BEGL_DEFAULT_PLATFORM, display_id, NULL,
         attrib_EGLint);
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetPlatformDisplay(EGLenum platform,
      void *native_display, const EGLAttrib *attrib_list)
{
   return egl_get_platform_display_impl(platform, native_display, attrib_list,
         attrib_EGLAttrib);
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetPlatformDisplayEXT(EGLenum platform,
      void *native_display, const EGLint *attrib_list)
{
   return egl_get_platform_display_impl(platform, native_display, attrib_list,
         attrib_EGLint);
}

EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy,
      EGLint *major, EGLint *minor)
{
   EGL_THREAD_T *thread = egl_thread_get();
   if (!thread)
   {
      return EGL_FALSE;
   }

   if (!ensure_init_once())
   {
      thread->error = EGL_NOT_INITIALIZED;
      return EGL_FALSE;
   }

   vcos_mutex_lock(&egl_display.lock);

   if (egl_display.initialized &&  dpy != egl_display.display)
   {
      thread->error = EGL_BAD_DISPLAY;
      vcos_mutex_unlock(&egl_display.lock);
      return EGL_FALSE;
   }

   if (!egl_display.initialized)
   {
      bool plat_init = false;
      bool proc_init = false;

      plat_init = egl_display_refinc(dpy);
      if (!plat_init)
         goto end;

      egl_map_init(&egl_display.contexts);
      egl_map_init(&egl_display.surfaces);
      egl_map_init(&egl_display.images);
      egl_map_init(&egl_display.syncs);

      proc_init = egl_process_init();
      if (!proc_init)
         goto end;

      egl_display.next_handle = 1;
      egl_display.display = dpy;
      init_extension_string(egl_display.extension_string,
            sizeof(egl_display.extension_string), egl_display_exts_str,
            egl_platform_get_display_extensions());
      egl_display.initialized = true;

   end:
      if (!egl_display.initialized)
      {
         if (proc_init) egl_process_release();
         egl_map_destroy(&egl_display.surfaces);
         egl_map_destroy(&egl_display.contexts);
         egl_map_destroy(&egl_display.images);
         egl_map_destroy(&egl_display.syncs);
         if (plat_init) egl_display_refdec(dpy);
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
   EGLDisplay display;
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
   display = egl_display.display;
   egl_display.display = EGL_NO_DISPLAY;
   init_extension_string(egl_display.extension_string,
         sizeof(egl_display.extension_string), egl_display_exts_str, "");
   egl_display.initialized = false;
   vcos_mutex_unlock(&egl_display.lock);

   destroy_map(&contexts, (destructor_t) egl_context_try_delete);
   destroy_map(&surfaces, (destructor_t) egl_surface_try_delete);
   destroy_map(&images, (destructor_t) egl_image_refdec);
   destroy_map(&syncs, (destructor_t) egl_sync_refdec);

   egl_process_release();
   egl_display_refdec(display);
}

EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy)
{
   EGLint err = EGL_SUCCESS;

   if (!egl_is_valid_display(dpy))
      err = EGL_BAD_DISPLAY;
   else if (ensure_init_once())
      egl_terminate();

   EGL_THREAD_T *thread = egl_thread_get();
   if (thread)
      thread->error = err;
   return err == EGL_SUCCESS;
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
