/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
/*
   Global preconditions?

   Server is up (needed by RPC_CALL[n]_RES)


   Spec ambiguity:

   What should we do if eglGetError is called twice? Currently we reset the error to EGL_SUCCESS.

   eglGetConfigs:
   "The number of configurations is returned in num config"
   We assume this refers to the number of configurations returned, rather than the total number of
   configurations available. (These are different values if config_size is too small).

   eglChooseConfig:
   Fail if the same attribute is specified more than once?
   (I can't find anything in the spec saying what to do in this case)

   In general, which attribute values are accepted and which raise
   EGL_BAD_ATTRIBUTE is vague.

   In particular, what do we do about the "ignored" and conditionally
   ignored ones?

   Currently ignoring ("ignored"):
   case EGL_MAX_PBUFFER_HEIGHT:
   case EGL_MAX_PBUFFER_PIXELS:
   case EGL_MAX_PBUFFER_WIDTH:
   case EGL_NATIVE_VISUAL_ID:

   Currently not ignoring ("conditionally ignored")
   case EGL_TRANSPARENT_BLUE_VALUE:
   case EGL_TRANSPARENT_GREEN_VALUE:
   case EGL_TRANSPARENT_RED_VALUE:

   Currently ignoring ("conditionally ignored")
   case EGL_NATIVE_VISUAL_TYPE:

   The following sentences in the spec seem to contradict each other:
   "If EGL_MATCH_NATIVE_PIXMAP is specified in attrib list, it must be followed
by an attribute value"

   "If EGL_DONT_CARE is specified as an attribute value, then the
   attribute will not be checked. EGL_DONT_CARE may be specified for all attributes
   except EGL_LEVEL."

   In addition, EGL_NONE is listed as the default match value for EGL_MATCH_NATIVE_PIXMAP.
   What happens if EGL_DONT_CARE or EGL_NONE is a valid native pixmap value?

   What we actually do is we always treat the value supplied with EGL_MATCH_NATIVE_PIXMAP
   as a valid handle (and fail if it's invalid), and ignore it only if it's not in the list
   at all.


   EGL_MATCH_NATIVE_PIXMAP: todo: we'll set thread->error to something like EGL_BAD_ATTRIBUTE; should we be setting it to EGL_BAD_NATIVE_PIXMAP?
   What is EGL_PRESERVED_RESOURCES?



   Do we need to do anything for EGL_LEVEL?

   What is EGL_PRESERVED_RESOURCES?
   What exactly are EGL_NATIVE_VISUAL_ID, EGL_NATIVE_VISUAL_TYPE?


   Implementation notes:

   We only support one display. This is assumed to have a native display_id
   of 0 (==EGL_DEFAULT_DISPLAY) and an EGLDisplay id of 1

   All EGL client functions preserve the invariant (CLIENT_THREAD_STATE_ERROR)

   It would be nice for the EGL version to only be defined in one place (rather than both eglInitialize and eglQueryString).

   We allow implicit casts from bool to EGLint


   We make the following assumptions:

      EGL_CONFIG_CAVEAT (all EGL_NONE)
      EGL_COLOR_BUFFER_TYPE (all EGL_RGB_BUFFER)
      EGL_SAMPLES (if EGL_SAMPLES is 1 then all 4 else all 0)
      EGL_NATIVE_RENDERABLE is true
      EGL_MAX_PBUFFER_WIDTH, EGL_MAX_PBUFFER_HEIGHT, EGL_MIN_SWAP_INTERVAL, EGL_MAX_SWAP_INTERVAL are the same for all configs

      All configs support all of:
         (EGL_PBUFFER_BIT | EGL_PIXMAP_BIT | EGL_WINDOW_BIT | EGL_VG_COLORSPACE_LINEAR_BIT | EGL_VG_ALPHA_FORMAT_PRE_BIT | EGL_MULTISAMPLE_RESOLVE_BOX_BIT | EGL_SWAP_BEHAVIOR_PRESERVED_BIT);


   EGL_OPTIMAL_FORMAT_BIT_KHR: Considered optimal if no format conversion needs doing

   EGL_TRANSPARENT_TYPE is always EGL_NONE because we don't support EGL_TRANSPARENT_RGB. Should there be an EGL_TRANSPARENT_ALPHA?
*/

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_options.h"
#include "interface/khronos/egl/egl_client_surface.h"
#include "interface/khronos/egl/egl_client_context.h"
#include "interface/khronos/egl/egl_client_config.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/egl/egl_int_impl.h"
#include "interface/khronos/common/khrn_client_platform.h"
#include "middleware/khronos/egl/egl_platform.h"
#include "middleware/khronos/egl/egl_server.h"
#include "middleware/khronos/common/khrn_mem.h"


#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#ifndef WIN32
#include <alloca.h>
#endif

static VCOS_ONCE_T    once;
static bool           once_ok;
static VCOS_MUTEX_T   lock;

static void init_once(void)
{
   once_ok = (vcos_mutex_create(&lock, "lock") == VCOS_SUCCESS);
   if (!once_ok)
      vcos_log(LOG_ERROR, "Fatal: unable to init once egl_display");
}

bool egl_ensure_init_once(void)
{
   vcos_demand(vcos_once(&once, init_once) == VCOS_SUCCESS);
   return once_ok;
}

void CLIENT_LOCK(void)
{
   vcos_mutex_lock(&lock);
}

void CLIENT_UNLOCK(void)
{
   assert(vcos_mutex_is_locked(&lock));
   vcos_mutex_unlock(&lock);
}

EGLAPI EGLint EGLAPIENTRY eglGetError(void)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   if (thread)
   {
      EGLint result;

      assert(thread != NULL);

      result = thread->error;
      thread->error = EGL_SUCCESS;

      return result;
   }
   else
      return EGL_SUCCESS;
}

static EGLDisplay eglGetDisplay_impl(EGLenum platform,
      void *native_display, const EGLint *attrib_list)
{
   EGLDisplay display;
   EGLint error;
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   if (!egl_ensure_init_once())
   {
      thread->error = EGL_NOT_INITIALIZED;
      return EGL_FALSE;
   }

   CLIENT_LOCK();

   display = egl_server_platform_get_display(platform, native_display, attrib_list, &error);
   if (thread)
      thread->error = error;

   CLIENT_UNLOCK();

   return display;
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id)
{
   return eglGetDisplay_impl(BEGL_DEFAULT_PLATFORM, display_id, NULL);
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetPlatformDisplayEXT(EGLenum platform,
      void *native_display, const EGLint *attrib_list)
{
   return eglGetDisplay_impl(platform, native_display, attrib_list);
}

/*
* Add the attribute to the list if it is not already there or
* change its value if the attribute is in the list
*/
static void add_attrib_value(EGLint *attrib_list, EGLint name, EGLint value)
{
   if (name == EGL_NONE)
      return;

   /* find attribute and update its value*/
   while (*attrib_list != EGL_NONE) {
      if (*attrib_list == name) {
         attrib_list++;
         *attrib_list = value;
         /* return because it might be the last pair before EGL_NONE */
         return;
      }

      attrib_list += 2;
   }

   /* if not found: add attribute and EGL_NONE to the end */
   if (*attrib_list == EGL_NONE) {
      *attrib_list++ = name;
      *attrib_list++ = value;
      *attrib_list = EGL_NONE;
   }
}

static void clean_attrib_list(EGLint *cleaned_attrib_list, const EGLint *attrib_list)
{
   while (attrib_list && *attrib_list != EGL_NONE) {
      EGLint name = *attrib_list++;
      EGLint value = *attrib_list++;
      add_attrib_value(cleaned_attrib_list, name, value);
   }
}

/*
* Counts the number of elements (name or value) in the attribute list
* including EGL_NONE
*/
static unsigned int get_size_of_attrib_list(const EGLint *attrib_list)
{
   unsigned int size = 0;

   while (attrib_list && *attrib_list != EGL_NONE) {
      size += 2;
      attrib_list += 2;
   }

   /* number of elements (name or value) + EGL_NONE */
   return size + 1;
}

static void update_sort_params(EGLint *cleaned_attrib_list, bool *use_red,
   bool *use_green, bool *use_blue, bool *use_alpha)
{
   while (*cleaned_attrib_list != EGL_NONE) {
      EGLint name = *cleaned_attrib_list++;
      EGLint value = *cleaned_attrib_list++;

      switch (name) {
      case EGL_RED_SIZE:
         *use_red = (value != 0 && value != EGL_DONT_CARE);
         break;
      case EGL_GREEN_SIZE:
         *use_green = (value != 0 && value != EGL_DONT_CARE);
         break;
      case EGL_BLUE_SIZE:
         *use_blue = (value != 0 && value != EGL_DONT_CARE);
         break;
      case EGL_ALPHA_SIZE:
         *use_alpha = (value != 0 && value != EGL_DONT_CARE);
         break;
      default:
         break;
      }
   }
}

static EGLBoolean egl_choose_config(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLBoolean result = EGL_FALSE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      if (!num_config) {
         thread->error = EGL_BAD_PARAMETER;
         result = EGL_FALSE;
         goto end;
      }

      /*
       * check for invalid attributes
      */

      if (!egl_config_check_attribs(attrib_list)) {
         thread->error = EGL_BAD_ATTRIBUTE;
         result = EGL_FALSE;
         goto end;
      }

      unsigned int num_attrib = get_size_of_attrib_list(attrib_list);

      EGLint *cleaned_attrib_list = (EGLint *)malloc(num_attrib * sizeof(EGLint));
      if (cleaned_attrib_list == NULL) {
         thread->error = EGL_BAD_ALLOC;
         result = EGL_FALSE;
         goto end;
      }
      cleaned_attrib_list[0] = EGL_NONE;
      clean_attrib_list(cleaned_attrib_list, attrib_list);

      bool use_red = false;
      bool use_green = false;
      bool use_blue = false;
      bool use_alpha = false;

      update_sort_params(cleaned_attrib_list, &use_red, &use_green, &use_blue, &use_alpha);

      /*
       * sort configs
      */
      int *ids = (int *)alloca(EGL_MAX_CONFIGS * sizeof(int));

      for (int i = 0; i < EGL_MAX_CONFIGS; i++)
         ids[i] = i;

      egl_config_sort(ids, use_red, use_green, use_blue, use_alpha);

      /*
       * return configs
      */

      int j = 0;
      for (int i = 0; i < EGL_MAX_CONFIGS; i++) {
         if (egl_config_filter(ids[i], cleaned_attrib_list)) {
            if (configs && j < config_size) {
               configs[j] = egl_config_from_id(ids[i]);
               j++;
            }
            else if (!configs) {
               /*
                * if configs==NULL then we count all configs
                * otherwise we only count the configs we return
               */
               j++;
            }
         }
      }

      free(cleaned_attrib_list);
      thread->error = EGL_SUCCESS;
      *num_config = j;
      result = EGL_TRUE;

end:
      CLIENT_UNLOCK();
      return result;
   }

   return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      if (!value) {
         thread->error = EGL_BAD_PARAMETER;
         result = EGL_FALSE;
      }
      else if (egl_config_to_id(config) < 0 || egl_config_to_id(config) >= EGL_MAX_CONFIGS) {
         thread->error = EGL_BAD_CONFIG;
         result = EGL_FALSE;
      }
      else if (!egl_config_get_attrib(egl_config_to_id(config), attribute, value)) {
         thread->error = EGL_BAD_ATTRIBUTE;
         result = EGL_FALSE;
      }
      else {
         thread->error = EGL_SUCCESS;
         result = EGL_TRUE;
      }
      CLIENT_UNLOCK();
   }
   else
      result = EGL_FALSE;

   return result;
}

/*
TODO: do an RPC call to make sure the Khronos vll is loaded (and that it stays loaded until eglTerminate)
Also affects global image (and possibly others?)
   EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)

   Khronos documentation:

   EGL may be initialized on a display by calling
   EGLBoolean eglInitialize(EGLDisplay dpy, EGLint
   *major, EGLint *minor);
   EGL TRUE is returned on success, and major and minor are updated with the major
   and minor version numbers of the EGL implementation (for example, in an EGL
   1.2 implementation, the values of *major and *minor would be 1 and 2, respectively).
   major and minor are not updated if they are specified as NULL.
   EGL FALSE is returned on failure and major and minor are not updated. An
   EGL BAD DISPLAY error is generated if the dpy argument does not refer to a valid
   EGLDisplay. An EGL NOT INITIALIZED error is generated if EGL cannot be
   initialized for an otherwise valid dpy.
   Initializing an already-initialized display is allowed, but the only effect of such
   a call is to return EGL TRUE and update the EGL version numbers. An initialized
   display may be used from other threads in the same address space without being
   initalized again in those threads.

   Implementation notes:

   client_egl_get_process_state sets some errors for us.

   Preconditions:

   major is NULL or a valid pointer
   minor is NULL or a valid pointer
   eglTerminate(dpy) must be called at some point after calling this function if we return EGL_TRUE.

   Postconditions:

   The following conditions cause error to assume the specified value

      EGL_BAD_DISPLAY               An EGLDisplay argument does not name a valid EGLDisplay
      EGL_NOT_INITIALIZED           EGL is not initialized, or could not be initialized, for the specified display.
      EGL_SUCCESS                   Function succeeded.

   if more than one condition holds, the first error is generated.

   Invariants preserved:

   -

   Invariants used:

   -
*/

EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   if (!thread)
      return EGL_FALSE;

   if (!egl_ensure_init_once())
   {
      thread->error = EGL_NOT_INITIALIZED;
      return EGL_FALSE;
   }

   EGLint error = EGL_SUCCESS;

   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();
   vcos_demand(state != NULL);

   CLIENT_LOCK();

   if (state->display != dpy && state->inited)
   {
      /* we don't support simultaneous displays */
      error = EGL_BAD_DISPLAY;
      state = NULL;
   }
   else if (!state->inited)
   {
      if (!state->platform_inited)
         error = egl_server_platform_init(dpy);
      else
         error = EGL_SUCCESS;
      if (error == EGL_SUCCESS)
      {
         state->platform_inited = true;
         state->display = dpy;

         if (!server_process_state_init())
         {
            error = EGL_NOT_INITIALIZED;
            state = NULL;
            if (state->platform_inited)
               egl_server_platform_term(dpy);
         }
      }
      else
         state = NULL;
   }
   /* else already initialised on this display */

   /* Can't get thread state before a call to egl_server_platform_init()
    * because that state may get destroyed and re-created in the process
    * of initialising the new display.
    */
   thread = CLIENT_GET_THREAD_STATE();
   thread->error = error;

   if (!state)
   {
      CLIENT_UNLOCK();
      return EGL_FALSE;
   }

   if (major)
      *major = 1;
   if (minor)
      *minor = 4;

   CLIENT_UNLOCK();

   return EGL_TRUE;
}

/*
   EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy)

   Khronos documentation:

   To release resources associated with use of EGL and client APIs on a display,
   call
   EGLBoolean eglTerminate(EGLDisplay dpy);
   Termination marks all EGL-specific resources associated with the specified display
   for deletion. If contexts or surfaces created with respect to dpy are current (see
   section 3.7.3) to any thread, then they are not actually released while they remain
   current. Such contexts and surfaces will be destroyed, and all future references to
   them will become invalid, as soon as any otherwise valid eglMakeCurrent call is
   made from the thread they are bound to.
   eglTerminate returns EGL TRUE on success.
   If the dpy argument does not refer to a valid EGLDisplay, EGL FALSE is
   returned, and an EGL BAD DISPLAY error is generated.
   Termination of a display that has already been terminated, or has not yet been
   initialized, is allowed, but the only effect of such a call is to return EGL TRUE, since
   there are no EGL resources associated with the display to release. A terminated
   display may be re-initialized by calling eglInitialize again. When re-initializing
   a terminated display, resources which were marked for deletion as a result of the
   earlier termination remain so marked, and references to them are not valid.

   Implementation notes:

   -

   Preconditions:

   -

   Postconditions:

   The following conditions cause error to assume the specified value

      EGL_BAD_DISPLAY               An EGLDisplay argument does not name a valid EGLDisplay
      EGL_SUCCESS                   Function succeeded.

   if more than one condition holds, the first error is generated.

   Invariants preserved:

   -

   Invariants used:

   -
*/

EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   if (!egl_ensure_init_once()) {
      thread->error = EGL_SUCCESS;
      return EGL_TRUE;
   }

   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   CLIENT_LOCK();

   vcos_demand(state != NULL);

   if (state->display == dpy) {
      /* TODO : what about client tls state, this needs removing */

      /* teardown process state and recreate back to initial condition */
      server_process_state_term();

      if (state->context_current_count == 0 && state->platform_inited)
      {
         thread->error = egl_server_platform_term(dpy);
         if (thread->error == EGL_SUCCESS)
            state->platform_inited = false;
      }
      else
         thread->error = EGL_SUCCESS;
   } else
      thread->error = EGL_BAD_DISPLAY;

   /* external egl images may still be in the pipeline */
   eglIntFinish_impl(true);

   CLIENT_UNLOCK();

   return (thread->error == EGL_SUCCESS);
}

/*
   EGLAPI const char EGLAPIENTRY * eglQueryString(EGLDisplay dpy, EGLint name)

   Khronos documentation:

   3.3 EGL Versioning
const char *eglQueryString(EGLDisplay dpy, EGLint
name);
eglQueryString returns a pointer to a static, zero-terminated string describing
some aspect of the EGL implementation running on the specified display.
name may be one of EGL CLIENT APIS, EGL EXTENSIONS, EGL VENDOR, or
EGL VERSION.
The EGL CLIENT APIS string describes which client rendering APIs are supported.
It is zero-terminated and contains a space-separated list of API names,
which must include at least one of OpenGL ES or OpenVG.
Version 1.3 - December 4, 2006
3.4. CONFIGURATION MANAGEMENT 13
The EGL EXTENSIONS string describes which EGL extensions are supported
by the EGL implementation running on the specified display. The string is zeroterminated
and contains a space-separated list of extension names; extension names
themselves do not contain spaces. If there are no extensions to EGL, then the empty
string is returned.
The format and contents of the EGL VENDOR string is implementation dependent.
The format of the EGL VERSION string is:
<major version.minor version><space><vendor specific info>
Both the major and minor portions of the version number are numeric. Their values
must match the major and minor values returned by eglInitialize (see section 3.2).
The vendor-specific information is optional; if present, its format and contents are
implementation specific.
On failure, NULL is returned. An EGL NOT INITIALIZED error is generated if
EGL is not initialized for dpy. An EGL BAD PARAMETER error is generated if name
is not one of the values described above.

   Implementation notes:

   We support the following extensions but they can be removed from the driver if defined to zero.
      EGL_KHR_image extensions
      EGL_KHR_lock_surface

   Preconditions:

   -

   Postconditions:

   The following conditions cause error to assume the specified value

      EGL_BAD_DISPLAY               An EGLDisplay argument does not name a valid EGLDisplay
      EGL_NOT_INITIALIZED           EGL is not initialized for the specified display.
      EGL_BAD_PARAMETER             name is not one of {EGL_CLIENT_APIS, EGL_EXTENSIONS, EGL_VENDOR, EGL_VERSION}
      EGL_SUCCESS                   Function succeeded.

   if more than one condition holds, the first error is generated.

   Return value is NULL or a pointer to a null-terminated string which is valid forever.

   Invariants preserved:

   -

   Invariants used:

   -
*/

EGLAPI const char * EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGL_SERVER_STATE_T *state;
   const char *result = NULL;

   if (!egl_ensure_init_once())
   {
      thread->error = EGL_BAD_PARAMETER;
      goto end;
   }

   if (dpy == EGL_NO_DISPLAY && name == EGL_EXTENSIONS)
   {
      // Since we support EGL_EXT_client_extensions, it is valid to query using
      // EGL_NO_DISPLAY. In this case, the only valid name is EGL_EXTENSIONS.
      // The resulting string must include "EGL_EXT_client_extensions"
      // at minimum.
      result = ""
            "EGL_EXT_client_extensions "
            "EGL_EXT_platform_base "
#ifdef WAYLAND
            "EGL_EXT_platform_wayland "
            "EGL_KHR_platform_wayland "
#endif
            ;
      return result;
   }

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      thread->error = EGL_SUCCESS;
      switch (name) {
      case EGL_CLIENT_APIS:
         result = "OpenGL_ES";
         break;
      case EGL_EXTENSIONS:
         //TODO: this list isn't quite correct
         result = ""
#if EGL_KHR_image
            "EGL_KHR_image EGL_KHR_image_base EGL_KHR_image_pixmap EGL_KHR_vg_parent_image EGL_KHR_gl_texture_2D_image EGL_KHR_gl_texture_cubemap_image "
#endif
#if EGL_KHR_lock_surface
            "EGL_KHR_lock_surface "
#endif
#if KHR_get_all_proc_addresses
            "KHR_get_all_proc_addresses "
#endif
#if EGL_ANDROID_swap_rectangle
            "EGL_ANDROID_swap_rectangle "
#endif
#ifdef ANDROID
            "EGL_ANDROID_image_native_buffer "
#endif
#if EGL_KHR_fence_sync
            "EGL_KHR_fence_sync "
#endif
#if EGL_BRCM_image_update_control
            "EGL_BRCM_image_update_control "
#endif
#if EGL_ANDROID_framebuffer_target
            "EGL_ANDROID_framebuffer_target "
#endif
#ifdef WAYLAND
            "EGL_WL_bind_wayland_display "
#endif
            ;
         break;
      case EGL_VENDOR:
         result = "Broadcom";
         break;
      case EGL_VERSION:
         result = "1.4";
         break;
      default:
         thread->error = EGL_BAD_PARAMETER;
         result = NULL;
      }
      CLIENT_UNLOCK();
   } else
      result = NULL;

end:
   return result;
}

typedef struct {
   EGL_SERVER_STATE_T *state;
   EGLNativeWindowType window;
   int is_dup;
} WINDOW_CHECK_DATA_T;

static void callback_check_duplicate_window(khrn_map *map, uint32_t key, void *value, void *p)
{
   WINDOW_CHECK_DATA_T *window_check_data = (WINDOW_CHECK_DATA_T *)p;
   EGL_SURFACE_T *surface = value;

   UNUSED_NDEBUG(map);
   UNUSED_NDEBUG(key);

   assert(map == &window_check_data->state->surfaces);
   assert(surface != NULL);
   assert((uintptr_t)key == (uintptr_t)surface->name);

   if (surface->type == WINDOW && surface->win == window_check_data->window)
      window_check_data->is_dup = 1;
}

/*
   EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)

   Khronos documentation:

   3.5.1 Creating On-Screen Rendering Surfaces
   To create an on-screen rendering surface, first create a native platform window
   with attributes corresponding to the desired EGLConfig (e.g. with the same color
   depth, with other constraints specific to the platform). Using the platform-specific
   type EGLNativeWindowType, which is the type of a handle to that native window,
   then call:

   EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win,
   const EGLint *attrib_list);

   eglCreateWindowSurface creates an onscreen EGLSurface and returns a handle
   to it. Any EGL context created with a compatible EGLConfig can be used to
   render into this surface.

   attrib list specifies a list of attributes for the window. The list has the same
   structure as described for eglChooseConfig. Attributes that can be specified in
   attrib list include EGL_RENDER_BUFFER, EGL_VG_COLORSPACE, and EGL_VG_ALPHA_FORMAT.
   It is possible that some platforms will define additional attributes specific to
   those environments, as an EGL extension.

   attrib list may be NULL or empty (first attribute is EGL_NONE), in which case
   all attributes assumes their default value as described below.
   EGL_RENDER_BUFFER specifies which buffer should be used for client API
   rendering to the window, as described in section 2.2.2. If its value is EGL_SINGLE_BUFFER,
   then client APIs should render directly into the visible window.

   If its value is EGL_BACK_BUFFER, then all client APIs should render into the back
   buffer. The default value of EGL_RENDER_BUFFER is EGL_BACK_BUFFER.

   Client APIs may not be able to respect the requested rendering buffer. To
   determine the actual buffer being rendered to by a context, call eglQueryContext
   (see section 3.7.4).

   EGL_VG_COLORSPACE specifies the color space used by OpenVG when
   rendering to the surface. If its value is EGL_VG_COLORSPACE_sRGB, then
   a non-linear, perceptually uniform color space is assumed, with a corresponding
   VGImageFormat of form VG_s*. If its value is EGL_VG_-
   COLORSPACE_LINEAR, then a linear color space is assumed, with a corresponding
   VGImageFormat of form VG_l*. The default value of EGL_VG_COLORSPACE
   is EGL_VG_COLORSPACE_sRGB.

   EGL_VG_ALPHA_FORMAT specifies how alpha values are interpreted by
   OpenVG when rendering to the surface. If its value is EGL_VG_ALPHA_FORMAT_-
   NONPRE, then alpha values are not premultipled. If its value is EGL_VG_ALPHA_-
   FORMAT_PRE, then alpha values are premultiplied. The default value of EGL_VG_-
   ALPHA_FORMAT is EGL_VG_ALPHA_FORMAT_NONPRE.

   Note that the EGL_VG_COLORSPACE and EGL_VG_ALPHA_FORMAT attributes
   are used only by OpenVG . EGL itself, and other client APIs such as OpenGL and
   OpenGL ES , do not distinguish multiple colorspace models. Refer to section 11.2
   of the OpenVG 1.0 specification for more information.

   Similarly, the EGL_VG_ALPHA_FORMAT attribute does not necessarily control
   or affect the window systems interpretation of alpha values, even when the window
   system makes use of alpha to composite surfaces at display time. The window system's
   use and interpretation of alpha values is outside the scope of EGL. However,
   the preferred behavior is for window systems to ignore the value of EGL_VG_-
   ALPHA_FORMAT when compositing window surfaces.

   On failure eglCreateWindowSurface returns EGL_NO_SURFACE. If the attributes
   of win do not correspond to config, then an EGL_BAD_MATCH error is generated.
   If config does not support rendering to windows (the EGL_SURFACE_TYPE
   attribute does not contain EGL_WINDOW_BIT), an EGL_BAD_MATCH error is generated.
   If config does not support the colorspace or alpha format attributes specified
   in attrib list (as defined for eglCreateWindowSurface), an EGL_BAD_MATCH error
   is generated. If config is not a valid EGLConfig, an EGL_BAD_CONFIG error
   is generated. If win is not a valid native window handle, then an EGL_BAD_NATIVE_WINDOW
   error should be generated. If there is already an EGLConfig
   associated with win (as a result of a previous eglCreateWindowSurface call), then
   an EGL_BAD_ALLOC error is generated. Finally, if the implementation cannot allocate
   resources for the new EGL window, an EGL_BAD_ALLOC error is generated.

   Implementation notes:

   -

   Preconditions:

   attrib_list is NULL or a pointer to an EGL_NONE-terminated list of attribute/value pairs

   Postconditions:

   The following conditions cause error to assume the specified value

      EGL_BAD_DISPLAY               An EGLDisplay argument does not name a valid EGLDisplay
      EGL_NOT_INITIALIZED           EGL is not initialized for the specified display.
      EGL_BAD_CONFIG                config is not a valid EGLConfig
      EGL_BAD_NATIVE_WINDOW         win is not a valid native window handle
      EGL_BAD_ATTRIBUTE             attrib_list contains an undefined EGL attribute or an attribute value that is unrecognized or out of range.
           (TODO: EGL_BAD_ATTRIBUTE not mentioned in spec)
      EGL_BAD_NATIVE_WINDOW         window is larger than EGL_CONFIG_MAX_WIDTH x EGL_CONFIG_MAX_HEIGHT
           (TODO: Maybe EGL_BAD_ALLOC might be more appropriate?)
      EGL_BAD_ALLOC                 implementation cannot allocate resources for the new EGL window
           (TODO: If there is already an EGLConfig associated with win)
      EGL_SUCCESS                   Function succeeded.

   if more than one condition holds, the first error is generated.

   Return value is EGL_NO_SURFACE or an EGLSurface handle which is valid until the EGL session ends or
   eglDestroySurface is called.

   Invariants preserved:

   (CLIENT_PROCESS_STATE_SURFACES)
   (CLIENT_PROCESS_STATE_NEXT_SURFACE)

   Invariants used:

   -
*/

static EGLSurface eglCreateWindowSurface_impl(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLSurface result;

   if (!egl_ensure_init_once())
      return EGL_NO_SURFACE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      uintptr_t handle = platform_get_handle(win);

      if ((int)(size_t)config < 1 || (int)(size_t)config > EGL_MAX_CONFIGS) {
         thread->error = EGL_BAD_CONFIG;
         result = EGL_NO_SURFACE;
      } else if (handle == PLATFORM_WIN_NONE) {
         // The platform reports that this is an invalid window handle
         thread->error = EGL_BAD_NATIVE_WINDOW;
         result = EGL_NO_SURFACE;
      } else {
         bool linear = false;
         bool premult = false;
         bool secure = false;

         EGLint error = egl_surface_check_attribs(WINDOW, attrib_list, &linear, &premult,
            0, 0, 0, 0, 0, 0, &secure);
         if (error != EGL_SUCCESS) {
            thread->error = error;
            result = EGL_NO_SURFACE;
         } else {
            /*
             * Check that we didn't already use this window in an
             * earlier call to eglCreateWindowSurface()
            */
            WINDOW_CHECK_DATA_T window_check_data;
            window_check_data.state = state;
            window_check_data.window = win;
            window_check_data.is_dup = 0;

            khrn_map_iterate(&state->surfaces, callback_check_duplicate_window, &window_check_data);

            EGL_SURFACE_T *surface;
            if (window_check_data.is_dup) {
               thread->error = EGL_BAD_ALLOC;
               result = EGL_NO_SURFACE;
            }
            else {
               surface = egl_surface_create(
                                 (EGLSurface)(size_t)state->next_surface,
                                 WINDOW,
                                 linear ? LINEAR : SRGB,
                                 premult ? PRE : NONPRE,
                                 secure,
                                 3 /* num_buffers */,
                                 0 /* width */, 0 /* height */,
                                 config,
                                 win,
                                 handle,
                                 false,
                                 false,
                                 EGL_NO_TEXTURE,
                                 EGL_NO_TEXTURE,
                                 0 /* pixmap */,
                                 NULL);

               if (surface != NULL) {
                  if (khrn_map_insert(&state->surfaces, state->next_surface, surface)) {
                     thread->error = EGL_SUCCESS;
                     result = (EGLSurface)(size_t)state->next_surface++;
                  } else {
                     thread->error = EGL_BAD_ALLOC;
                     result = EGL_NO_SURFACE;
                  }
                  KHRN_MEM_ASSIGN(surface, NULL);
               } else {
                  thread->error = EGL_BAD_ALLOC;
                  result = EGL_NO_SURFACE;
               }
            }
         }
      }
      CLIENT_UNLOCK();
   }
   else
      result = EGL_NO_SURFACE;

   return result;
}

EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
   return eglCreateWindowSurface_impl(dpy, config, win, attrib_list);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformWindowSurfaceEXT (EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list)
{
   return eglCreateWindowSurface_impl(dpy, config, native_window, attrib_list);
}

/*
   EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)

   Khronos documentation:

   3.5.2 Creating Off-Screen Rendering Surfaces
   EGL supports off-screen rendering surfaces in pbuffers. Pbuffers differ from windows
   in the following ways:

   1. Pbuffers are typically allocated in offscreen (non-visible) graphics memory
   and are intended only for accelerated offscreen rendering. Allocation can fail
   if there are insufficient graphics resources (implementations are not required
   to virtualize framebuffer memory). Clients should deallocate pbuffers when
   they are no longer in use, since graphics memory is often a scarce resource.

   2. Pbuffers are EGL resources and have no associated native window or native
   window type. It may not be possible to render to pbuffers using native
   rendering APIs.

   To create a pbuffer, call

      EGLSurface eglCreatePbufferSurface(EGLDisplay dpy,
         EGLConfig config, const EGLint
         *attrib_list);

   This creates a single pbuffer surface and returns a handle to it.
   attrib list specifies a list of attributes for the pbuffer. The list has the same
   structure as described for eglChooseConfig. Attributes that can be specified in
   attrib list include EGL_WIDTH, EGL_HEIGHT, EGL_LARGEST_PBUFFER, EGL_TEXTURE_FORMAT,
   EGL_TEXTURE_TARGET, EGL_MIPMAP_TEXTURE, EGL_VG_COLORSPACE, and EGL_VG_ALPHA_FORMAT.

   It is possible that some platforms will define additional attributes specific to
   those environments, as an EGL extension.

   attrib list may be NULL or empty (first attribute is EGL_NONE), in which case
   all the attributes assume their default values as described below.

   EGL_WIDTH and EGL_HEIGHT specify the pixel width and height of the rectangular
   pbuffer. If the value of EGLConfig attribute EGL_TEXTURE_FORMAT is
   not EGL_NO_TEXTURE, then the pbuffer width and height specify the size of the
   level zero texture image. The default values for EGL_WIDTH and EGL_HEIGHT are
   zero.

   EGL_TEXTURE_FORMAT specifies the format of the OpenGL ES texture that
   will be created when a pbuffer is bound to a texture map. It can be set to EGL_-
   TEXTURE_RGB, EGL_TEXTURE_RGBA, or EGL_NO_TEXTURE. The default value of
   EGL_TEXTURE_FORMAT is EGL_NO_TEXTURE.

   EGL_TEXTURE_TARGET specifies the target for the OpenGL ES texture that
   will be created when the pbuffer is created with a texture format of EGL_-
   TEXTURE_RGB or EGL_TEXTURE_RGBA. The target can be set to EGL_NO_-
   TEXTURE or EGL_TEXTURE_2D. The default value of EGL_TEXTURE_TARGET is
   EGL_NO_TEXTURE.

   EGL_MIPMAP_TEXTURE indicates whether storage for OpenGL ES mipmaps
   should be allocated. Space for mipmaps will be set aside if the attribute value
   is EGL_TRUE and EGL_TEXTURE_FORMAT is not EGL_NO_TEXTURE. The default
   value for EGL_MIPMAP_TEXTURE is EGL_FALSE.

   Use EGL_LARGEST_PBUFFER to get the largest available pbuffer when the allocation
   of the pbuffer would otherwise fail. The width and height of the allocated
   pbuffer will never exceed the values of EGL_WIDTH and EGL_HEIGHT, respectively.
   If the pbuffer will be used as a OpenGL ES texture (i.e., the value of
   EGL_TEXTURE_TARGET is EGL_TEXTURE_2D, and the value of EGL_TEXTURE_-
   FORMAT is EGL_TEXTURE_RGB or EGL_TEXTURE_RGBA), then the aspect ratio
   will be preserved and the new width and height will be valid sizes for the texture
   target (e.g. if the underlying OpenGL ES implementation does not support
   non-power-of-two textures, both the width and height will be a power of 2). Use
   eglQuerySurface to retrieve the dimensions of the allocated pbuffer. The default
   value of EGL_LARGEST_PBUFFER is EGL_FALSE.

   EGL_VG_COLORSPACE and EGL_VG_ALPHA_FORMAT have the same meaning
   and default values as when used with eglCreateWindowSurface.
   The resulting pbuffer will contain color buffers and ancillary buffers as specified
   by config.

   The contents of the depth and stencil buffers may not be preserved when rendering
   an OpenGL ES texture to the pbuffer and switching which image of the
   texture is rendered to (e.g., switching from rendering one mipmap level to rendering
   another).

   On failure eglCreatePbufferSurface returns EGL_NO_SURFACE. If the pbuffer
   could not be created due to insufficient resources, then an EGL_BAD_ALLOC error
   is generated. If config is not a valid EGLConfig, an EGL_BAD_CONFIG error is
   generated. If the value specified for either EGL_WIDTH or EGL_HEIGHT is less
   than zero, an EGL_BAD_PARAMETER error is generated. If config does not support
   pbuffers, an EGL_BAD_MATCH error is generated. In addition, an EGL_BAD_MATCH
   error is generated if any of the following conditions are true:

   The EGL_TEXTURE_FORMAT attribute is not EGL_NO_TEXTURE, and EGL_WIDTH
   and/or EGL_HEIGHT specify an invalid size (e.g., the texture size is
   not a power of two, and the underlying OpenGL ES implementation does not
   support non-power-of-two textures).

   The EGL_TEXTURE_FORMAT attribute is EGL_NO_TEXTURE, and EGL_TEXTURE_TARGET
   is something other than EGL_NO_TEXTURE; or, EGL_TEXTURE_FORMAT is something
   other than EGL_NO_TEXTURE, and EGL_TEXTURE_TARGET is EGL_NO_TEXTURE.

   Finally, an EGL_BAD_ATTRIBUTE error is generated if any of the EGL_TEXTURE_FORMAT,
   EGL_TEXTURE_TARGET, or EGL_MIPMAP_TEXTURE attributes
   are specified, but config does not support OpenGL ES rendering (e.g.
   the EGL_RENDERABLE_TYPE attribute does not include at least one of EGL_OPENGL_ES_BIT
   or EGL_OPENGL_ES2_BIT.

   Implementation notes:

   -

   attrib_list is NULL or a pointer to an EGL_NONE-terminated list of attribute/value pairs

   Postconditions:

   The following conditions cause error to assume the specified value

      EGL_BAD_DISPLAY               An EGLDisplay argument does not name a valid EGLDisplay
      EGL_NOT_INITIALIZED           EGL is not initialized for the specified display.
      EGL_BAD_CONFIG                config is not a valid EGLConfig
      EGL_BAD_ATTRIBUTE             attrib_list contains an undefined EGL attribute or an attribute value that is unrecognized or out of range.
           (TODO: EGL_BAD_ATTRIBUTE not mentioned in spec)
      EGL_BAD_MATCH                 config doesn't support EGL_BIND_TO_TEXTURE_RGB(A) and you specify EGL_TEXTURE_FORMAT=EGL_TEXTURE_RGB(A)
           (TODO: no mention of this in the spec)
      EGL_BAD_ALLOC                 requested dimensions are larger than EGL_CONFIG_MAX_WIDTH x EGL_CONFIG_MAX_HEIGHT
           (TODO: no mention of this in the spec)
      EGL_BAD_ALLOC                 implementation cannot allocate resources for the new EGL window
           (TODO: If there is already an EGLConfig associated with win)
      EGL_SUCCESS                   Function succeeded.

   if more than one condition holds, the first error is generated.

   Return value is EGL_NO_SURFACE or an EGLSurface handle which is valid until the EGL session ends or
   eglDestroySurface is called.

   Invariants preserved:

   (CLIENT_PROCESS_STATE_SURFACES)
   (CLIENT_PROCESS_STATE_NEXT_SURFACE)

   Invariants used:

   -
*/

EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config,
               const EGLint *attrib_list)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLSurface result;

   if (!egl_ensure_init_once())
      return EGL_NO_SURFACE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      if ((int)(size_t)config < 1 || (int)(size_t)config > EGL_MAX_CONFIGS) {
         thread->error = EGL_BAD_CONFIG;
         result = EGL_NO_SURFACE;
      } else {
         int width = 0;
         int height = 0;
         bool largest_pbuffer = 0;
         EGLenum texture_format = EGL_NO_TEXTURE;
         EGLenum texture_target = EGL_NO_TEXTURE;
         bool mipmap_texture = false;
         bool linear = false;
         bool premult = false;
         bool secure = false;

         EGLint error = egl_surface_check_attribs(PBUFFER, attrib_list, &linear, &premult, &width, &height,
            &largest_pbuffer, &texture_format, &texture_target, &mipmap_texture, &secure);
         if (error != EGL_SUCCESS) {
            thread->error = error;
            result = EGL_NO_SURFACE;
         } else if (
            (texture_format != EGL_NO_TEXTURE && (width == 0 || height == 0)) ||
            ((texture_format == EGL_NO_TEXTURE) != (texture_target == EGL_NO_TEXTURE)) ||
            !egl_config_bindable(egl_config_to_id(config), texture_format)
         ) {

         /*
         "In addition, an EGL_BAD_MATCH
         error is generated if any of the following conditions are true:
         - The EGL_TEXTURE_FORMAT attribute is not EGL_NO_TEXTURE, and
         EGL_WIDTH and/or EGL_HEIGHT specify an invalid size (e.g., the texture size
         is not a power of two, and the underlying OpenGL ES implementation does
         not support non-power-of-two textures).
         - The EGL_TEXTURE_FORMAT attribute is EGL_NO_TEXTURE, and
         EGL_TEXTURE_TARGET is something other than EGL_NO_TEXTURE; or,
         EGL_TEXTURE_FORMAT is something other than EGL_NO_TEXTURE, and
         EGL_TEXTURE_TARGET is EGL_NO_TEXTURE."
          */

         /*
         TODO It doesn't seem to explicitly say it in the spec, but I'm also
         generating EGL_BAD_MATCH if the config doesn't support EGL_BIND_TO_TEXTURE_RGB(A)
         and you specify EGL_TEXTURE_FORMAT=EGL_TEXTURE_RGB(A)
         */
            thread->error = EGL_BAD_MATCH;
            result = EGL_NO_SURFACE;
         } else if ((width > EGL_CONFIG_MAX_WIDTH || height > EGL_CONFIG_MAX_HEIGHT) && !largest_pbuffer) {
            /*
               TODO no mention of this in the spec, but clearly we fail if we try to allocate
               an oversize pbuffer without the largest_pbuffer stuff enabled
            */

            thread->error = EGL_BAD_ALLOC;
            result = EGL_NO_SURFACE;
         } else {
            EGL_SURFACE_T *surface = egl_surface_create(
                             (EGLSurface)(size_t)state->next_surface,
                             PBUFFER,
                             linear ? LINEAR : SRGB,
                             premult ? PRE : NONPRE,
                             secure,
                             1,
                             width, height,
                             config,
                             0,
                             PLATFORM_WIN_NONE,
                             largest_pbuffer,
                             mipmap_texture,
                             texture_format,
                             texture_target,
                             0,
                             NULL);

            if (surface != NULL) {
               if (khrn_map_insert(&state->surfaces, state->next_surface, surface)) {
                  thread->error = EGL_SUCCESS;
                  result = (EGLSurface)(size_t)state->next_surface++;
               } else {
                  thread->error = EGL_BAD_ALLOC;
                  result = EGL_NO_SURFACE;
               }
               KHRN_MEM_ASSIGN(surface, NULL);
            } else {
               thread->error = EGL_BAD_ALLOC;
               result = EGL_NO_SURFACE;
            }
         }
      }
      CLIENT_UNLOCK();
   }
   else
      result = EGL_NO_SURFACE;

   return result;
}

typedef struct {
   EGL_SERVER_STATE_T *state;
   EGLNativePixmapType pixmap;
   int is_dup;
} PIXMAP_CHECK_DATA_T;

static void callback_check_duplicate_pixmap(khrn_map *map, uint32_t key, void *value, void *p)
{
   PIXMAP_CHECK_DATA_T *pixmap_check_data = (PIXMAP_CHECK_DATA_T *)p;
   EGL_SURFACE_T *surface = value;

   UNUSED_NDEBUG(map);
   UNUSED_NDEBUG(key);

   assert(map == &pixmap_check_data->state->surfaces);
   assert(surface != NULL);
   assert((uintptr_t)key == (uintptr_t)surface->name);

   if ((surface->type == PIXMAP) && (surface->pixmap == pixmap_check_data->pixmap)) {
      pixmap_check_data->is_dup = 1;
   }
}

static EGLSurface eglCreatePixmapSurface_impl(EGLDisplay dpy, EGLConfig config,
              EGLNativePixmapType pixmap,
              const EGLint *attrib_list)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLSurface result = EGL_NO_SURFACE;
   EGLint error = EGL_BAD_ALLOC;

   if (!egl_ensure_init_once())
      return EGL_NO_SURFACE;

   if (!CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
      return EGL_NO_SURFACE;

   if ((int)(size_t)config < 1 || (int)(size_t)config > EGL_MAX_CONFIGS) {
      error = EGL_BAD_CONFIG;
      goto end;
   }

   bool linear = false, premult = false, secure = false;

   error = egl_surface_check_attribs(PIXMAP, attrib_list, &linear, &premult, 0, 0, 0, 0, 0, 0, &secure);
   if (error != EGL_SUCCESS)
      goto end;

   /*
      * Check that we didn't already use this pixmap in an
      * earlier call to eglCreatePixmapSurface()
      */
   PIXMAP_CHECK_DATA_T pixmap_check_data;
   pixmap_check_data.state = state;
   pixmap_check_data.pixmap = pixmap;
   pixmap_check_data.is_dup = 0;

   khrn_map_iterate(&state->surfaces, callback_check_duplicate_pixmap, &pixmap_check_data);

   if (pixmap_check_data.is_dup)
      goto end; /* EGL_BAD_ALLOC */

   int egl_surface_create_result = 0;
   EGL_SURFACE_T *surface = egl_surface_create(
                  (EGLSurface)(size_t)state->next_surface,
                  PIXMAP,
                  linear ? LINEAR : SRGB,
                  premult ? PRE : NONPRE,
                  secure,
                  1,
                  -1, -1,
                  config,
                  0,
                  PLATFORM_WIN_NONE,
                  EGL_FALSE,
                  EGL_FALSE,
                  EGL_NO_TEXTURE,
                  EGL_NO_TEXTURE,
                  pixmap,
                  &egl_surface_create_result);

   if (surface == NULL)
      goto end; /* EGL_BAD_ALLOC */

   if (egl_surface_create_result == -1) {
      error = EGL_BAD_NATIVE_PIXMAP;
      KHRN_MEM_ASSIGN(surface, NULL);
      goto end;
   }

   if (surface->width > EGL_CONFIG_MAX_WIDTH || surface->height > EGL_CONFIG_MAX_HEIGHT) {
      /* Maybe EGL_BAD_ALLOC might be more appropriate? */
      error = EGL_BAD_NATIVE_WINDOW;
      KHRN_MEM_ASSIGN(surface, NULL);
      goto end;
   }

   if (khrn_map_insert(&state->surfaces, state->next_surface, surface)) {
      error = EGL_SUCCESS;
      result = (EGLSurface)(size_t)state->next_surface++;
   } else
      error = EGL_BAD_ALLOC;

   KHRN_MEM_ASSIGN(surface, NULL);

end:
   if (error != EGL_SUCCESS)
      result = EGL_NO_SURFACE;

   thread->error = error;

   CLIENT_UNLOCK();

   return result;
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config,
              EGLNativePixmapType pixmap,
              const EGLint *attrib_list)
{
   return eglCreatePixmapSurface_impl(dpy, config, pixmap, attrib_list);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePlatformPixmapSurfaceEXT (EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLint *attrib_list)
{
   return eglCreatePixmapSurface_impl(dpy, config, native_pixmap, attrib_list);
}

//TODO: if this is a pixmap surface, should we make sure the pixmap gets
//updated?
//TODO: should this be a blocking call to the server, so that we know the EGL surface is really
//destroyed before subsequently destroying the associated window?
//TODO: is it safe for asynchronous swap notifications to come back after the surface has been
//destroyed, or do we need to wait for them? (and how?)
EGLAPI EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surf)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      thread->error = EGL_SUCCESS;

      EGL_SURFACE_T *surface = egl_get_surface(thread, state, surf);

      if (surface != NULL)
         khrn_map_delete(&state->surfaces, (uint32_t)(uintptr_t)surf);

      result = (thread->error == EGL_SUCCESS ? EGL_TRUE : EGL_FALSE );

      CLIENT_UNLOCK();
   } else
      result = EGL_FALSE;

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surf,
            EGLint attribute, EGLint *value)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      thread->error = EGL_SUCCESS;

      EGL_SURFACE_T *surface = egl_get_locked_surface(thread, state, surf);

      if (surface != NULL) {
#if EGL_KHR_lock_surface
         switch (attribute)
         {
         case EGL_BITMAP_POINTER_KHR:
         case EGL_BITMAP_PITCH_KHR:
         case EGL_BITMAP_ORIGIN_KHR:
         case EGL_BITMAP_PIXEL_RED_OFFSET_KHR:
         case EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR:
         case EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR:
         case EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR:
         case EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR:
            thread->error = egl_surface_get_mapped_buffer_attrib(surface, attribute, value);

            CLIENT_UNLOCK();
            return (thread->error == EGL_SUCCESS ? EGL_TRUE : EGL_FALSE );
         default:
            /* Other attributes can only be queried if the surface is unlocked */
            if (surface->is_locked) {
               thread->error = EGL_BAD_ACCESS;
               CLIENT_UNLOCK();
               return EGL_FALSE;
            }
         }
#endif
         if (!egl_surface_get_attrib(surface, attribute, value))
            thread->error = EGL_BAD_ATTRIBUTE;
      }

      result = (thread->error == EGL_SUCCESS ? EGL_TRUE : EGL_FALSE );

      CLIENT_UNLOCK();
   } else
      result = EGL_FALSE;

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   switch (api) {
   case EGL_OPENVG_API:
   case EGL_OPENGL_ES_API:
      thread->bound_api = api;

      thread->error = EGL_SUCCESS;
      return EGL_TRUE;
   default:
      thread->error = EGL_BAD_PARAMETER;
      return EGL_FALSE;
   }
}

EGLAPI EGLenum EGLAPIENTRY eglQueryAPI(void)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   return thread->bound_api;
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitClient(void)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   //TODO: "If the surface associated with the calling thread's current context is no
   //longer valid, EGL_FALSE is returned and an EGL_BAD_CURRENT_SURFACE error is
   //generated".

   eglIntFinish_impl(thread->bound_api == EGL_OPENGL_ES_API);

   thread->error = EGL_SUCCESS;

   CLIENT_UNLOCK();

   return EGL_TRUE;
}

//TODO: update pixmap surfaces?
EGLAPI EGLBoolean EGLAPIENTRY eglReleaseThread(void)
{
   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   client_thread_detach(NULL);

   CLIENT_UNLOCK();
   return EGL_TRUE;
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferFromClientBuffer(
         EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
         EGLConfig config, const EGLint *attrib_list)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLSurface result;

   if (!egl_ensure_init_once())
      return EGL_NO_SURFACE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      UNUSED(buftype);
      UNUSED(buffer);
      UNUSED(config);
      UNUSED(attrib_list);

      thread->error = EGL_BAD_PARAMETER;
      result = EGL_NO_SURFACE;

      CLIENT_UNLOCK();
   }
   else
      result = EGL_NO_SURFACE;

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surf,
             EGLint attribute, EGLint value)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      thread->error = EGL_SUCCESS;

      EGL_SURFACE_T *surface = egl_get_surface(thread, state, surf);

      if (surface)
         thread->error = egl_surface_set_attrib(surface, attribute, value);

      result = (thread->error == EGL_SUCCESS ? EGL_TRUE : EGL_FALSE );
      CLIENT_UNLOCK();
   } else
      result = EGL_FALSE;

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglBindTexImage(EGLDisplay dpy, EGLSurface surf, EGLint buffer)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
//TODO: is behaviour correct if there is no current rendering context?
      thread->error = EGL_SUCCESS;

      EGL_SURFACE_T *surface = egl_get_surface(thread, state, surf);

      if (surface == NULL)
         goto end;

      if (surface->texture_format != EGL_NO_TEXTURE) {
         if (buffer == EGL_BACK_BUFFER) {
            if (surface->type == PBUFFER && surface->texture_target == EGL_TEXTURE_2D) {
               result = (EGLBoolean)egl_bind_tex_image(surface);
               if (result != EGL_TRUE) {
                  // If buffer is already bound to a texture then an
                  // EGL_BAD_ACCESS error is returned.
                  // But we don't know whether it is or not until we call
                  // the server.
                  thread->error = EGL_BAD_ACCESS;
               }
            } else
               thread->error = EGL_BAD_SURFACE;
         } else
            thread->error = EGL_BAD_PARAMETER;
      } else
         thread->error = EGL_BAD_MATCH;

      result = (thread->error == EGL_SUCCESS ? EGL_TRUE : EGL_FALSE );

      CLIENT_UNLOCK();
   } else
      result = EGL_FALSE;

   return result;

end:
   CLIENT_UNLOCK();
   return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglReleaseTexImage(EGLDisplay dpy, EGLSurface surf, EGLint buffer)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      thread->error = EGL_SUCCESS;

      EGL_SURFACE_T *surface = egl_get_surface(thread, state, surf);

      if (surface == NULL)
         goto end;

      if (surface->texture_format != EGL_NO_TEXTURE) {
         if (buffer == EGL_BACK_BUFFER) {
            if (surface->type == PBUFFER) {
               //TODO: not a "bound" pbuffer?
               egl_release_tex_image(surface);
            } else
               thread->error = EGL_BAD_SURFACE;
         } else
            thread->error = EGL_BAD_PARAMETER;
      } else
         thread->error = EGL_BAD_MATCH;

      result = (thread->error == EGL_SUCCESS ? EGL_TRUE : EGL_FALSE );

      CLIENT_UNLOCK();
   } else
      result = EGL_FALSE;

   return result;

end:
   CLIENT_UNLOCK();
   return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLint error;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state)) {
      EGL_CURRENT_T *current = &thread->opengl;
      if (current->context == NULL) {
         error = EGL_BAD_CONTEXT;
         goto end;
      }

      EGL_SURFACE_T *surface = current->draw;
      if (surface == NULL) {
         error = EGL_BAD_SURFACE;
         goto end;
      }

      if (surface->type == WINDOW) {
         if (interval < EGL_CONFIG_MIN_SWAP_INTERVAL)
            interval = EGL_CONFIG_MIN_SWAP_INTERVAL;
         if (interval > EGL_CONFIG_MAX_SWAP_INTERVAL)
            interval = EGL_CONFIG_MAX_SWAP_INTERVAL;

         surface->swap_interval = interval;
      }

      error = EGL_SUCCESS;
end:
      thread->error = error;
      CLIENT_UNLOCK();

      return error == EGL_SUCCESS;
   }

   return EGL_FALSE;
}

EGLAPI EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_ctx, const EGLint *attrib_list)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLContext result;

   if (!egl_ensure_init_once())
      return EGL_NO_CONTEXT;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      if ((int)(size_t)config < 1 || (int)(size_t)config > EGL_MAX_CONFIGS) {
         thread->error = EGL_BAD_CONFIG;
         result = EGL_NO_CONTEXT;
      } else {
         EGLint max_version = (EGLint) (thread->bound_api == EGL_OPENGL_ES_API ? 2 : 1);
         EGLint version = 1;
         bool secure = false;

         EGLint error = egl_context_check_attribs(attrib_list, max_version, &version, &secure);
         if (error != EGL_SUCCESS) {
            thread->error = error;
            result = EGL_NO_CONTEXT;
         } else if (!(egl_config_get_api_support(egl_config_to_id(config)) &
            ((thread->bound_api == EGL_OPENVG_API) ? EGL_OPENVG_BIT :
            ((version == 1) ? EGL_OPENGL_ES_BIT : EGL_OPENGL_ES2_BIT)))) {
            thread->error = EGL_BAD_CONFIG;
            result = EGL_NO_CONTEXT;
         } else {
            EGL_CONTEXT_T *share_context;

            if (share_ctx != EGL_NO_CONTEXT) {
               share_context = egl_get_context(thread, state, share_ctx);

               if (share_context == NULL)
                  thread->error = EGL_BAD_CONTEXT;
            } else
               share_context = NULL;

            if ((share_ctx == EGL_NO_CONTEXT) || (share_context != NULL)) {

               EGL_CONTEXT_TYPE_T type = (version == 1) ? OPENGL_ES_11 : OPENGL_ES_20;

               EGL_CONTEXT_T *context = egl_context_create(
                  share_context,
                  (EGLContext)(size_t)state->next_context,
                  dpy, config, type,
                  secure);

               if (context != NULL) {
                  if (khrn_map_insert(&state->contexts, state->next_context, context)) {
                     thread->error = EGL_SUCCESS;
                     result = (EGLContext)(size_t)state->next_context++;
                  } else {
                     thread->error = EGL_BAD_ALLOC;
                     result = EGL_NO_CONTEXT;
                  }
                  KHRN_MEM_ASSIGN(context, NULL);
               } else {
                  thread->error = EGL_BAD_ALLOC;
                  result = EGL_NO_CONTEXT;
               }
            } else {
               /* thread->error set above */
               result = EGL_NO_CONTEXT;
            }
         }
      }
      CLIENT_UNLOCK();
   }
   else
      result = EGL_NO_CONTEXT;

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      thread->error = EGL_SUCCESS;

      EGL_CONTEXT_T *context = egl_get_context(thread, state, ctx);

      if (context != NULL)
         khrn_map_delete(&state->contexts, (uint32_t)(uintptr_t)ctx);

      result = thread->error == EGL_SUCCESS;
      CLIENT_UNLOCK();
   }
   else
      result = EGL_FALSE;

   return result;
}

void egl_current_release_surfaces(EGL_SERVER_STATE_T *state, CLIENT_THREAD_STATE_T *thread)
{
   if (thread->opengl.context != NULL) {
      EGL_CONTEXT_T *context = thread->opengl.context;
      context->renderbuffer = EGL_NONE;

      KHRN_MEM_ASSIGN(thread->opengl.context, NULL);

      assert(state->context_current_count > 0);
      state->context_current_count--;
   }

   KHRN_MEM_ASSIGN(thread->opengl.draw, NULL);
   KHRN_MEM_ASSIGN(thread->opengl.read, NULL);

   if (state->context_current_count == 0 && !state->inited)
      egl_server_platform_term(state->display);
}

static bool context_and_surface_are_compatible(EGL_CONTEXT_T *context, EGL_SURFACE_T *surface)
{
   /*
      from section 2.2 of the (1.3) spec, a context and surface are compatible
      if:
      1) they support the same type of color buffer (rgb or luminance). this is
         trivially true for us as we only support rgb color buffers
      2) they have color buffers and ancillary buffers of the same depth
      3) the surface was created with respect to an EGLConfig supporting client
         api rendering of the same type as the api type of the context
      4) they were created with respect to the same EGLDisplay. this is
         trivially true for us as we only have one EGLDisplay
   */

   uint32_t api_type = 0;
   switch (context->type) {
   case OPENGL_ES_11: api_type = EGL_OPENGL_ES_BIT; break;
   case OPENGL_ES_20: api_type = EGL_OPENGL_ES2_BIT; break;
   default:           UNREACHABLE();
   }

   uint32_t context_configid = egl_config_to_id(context->config);
   uint32_t surface_configid = egl_config_to_id(surface->config);

   return
      egl_config_bpps_match(context_configid, surface_configid) && /* (2) */
      (egl_config_get_api_support(context_configid) & api_type); /* (3) */
}

static bool egl_current_set_surfaces(EGL_SERVER_STATE_T *state, CLIENT_THREAD_STATE_T *thread, EGL_CONTEXT_T *context,
   EGL_SURFACE_T *draw, EGL_SURFACE_T *read)
{
   if ((khrn_mem_get_ref_count(context) > 1) && context->thread != thread) {
      // Fail - context is current to some other thread
      thread->error = EGL_BAD_ACCESS;
      return false;
   }

   enum { DRAW, READ, COUNT } i;
   EGL_SURFACE_T *surfaces[COUNT];

   surfaces[DRAW] = draw;
   surfaces[READ] = read;

   for (i = 0; i < COUNT; i++)
   {
      EGL_SURFACE_T *surface = surfaces[i];

      if ((khrn_mem_get_ref_count(surfaces[i]) > 1) && surface->thread != thread) {
         // Fail - draw surface is bound to context which is current to another thread
         thread->error = EGL_BAD_ACCESS;
         return false;
      }

      // causes a check that get_back_buffer() is OK for this surface
      uint32_t width, height;
      if (!egl_back_buffer_dims(surface, &width, &height)) {
         thread->error = EGL_BAD_NATIVE_WINDOW;
         return false;
      }

      if (!context_and_surface_are_compatible(context, surface)) {
         // Fail - draw surface is not compatible with context
         thread->error = EGL_BAD_MATCH;
         return false;
      }

      surface->width = width;
      surface->height = height;
   }

   egl_current_release_surfaces(state, thread);

   context->thread = thread;

   /* TODO: GLES supposedly doesn't support single-buffered rendering. Should we take this into account? */
   context->renderbuffer = egl_surface_get_render_buffer(draw);
   draw->thread = thread;
   read->thread = thread;

   KHRN_MEM_ASSIGN(thread->opengl.context, context);
   KHRN_MEM_ASSIGN(thread->opengl.draw, draw);
   KHRN_MEM_ASSIGN(thread->opengl.read, read);

   state->context_current_count++;

   egl_update_gl_buffers(&thread->opengl);

   return true;
}

static void flush_current_api(CLIENT_THREAD_STATE_T *thread)
{
   eglIntFlush_impl(thread->bound_api == EGL_OPENGL_ES_API);
}

EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface dr, EGLSurface rd, EGLContext ctx)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;
   EGL_SERVER_STATE_T *state = NULL; /* init to avoid warnings */

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   bool changed = true;
   /*
      check whether we are trying to release the current context
      Note that we can do this even if the display isn't initted.
   */

   if (dr == EGL_NO_SURFACE && rd == EGL_NO_SURFACE && ctx == EGL_NO_CONTEXT) {
      state = egl_get_process_state(thread, dpy, EGL_FALSE);

      if (state) {
         egl_current_release_surfaces(state, thread);
         thread->error = EGL_SUCCESS;
         result = EGL_TRUE;
      } else {
         result = EGL_FALSE;
      }
   } else  if (dr == EGL_NO_SURFACE || rd == EGL_NO_SURFACE || ctx == EGL_NO_CONTEXT) {
      thread->error = EGL_BAD_MATCH;
      result = EGL_FALSE;
   } else {
      /*
         get display
      */

      state = egl_get_process_state(thread, dpy, EGL_TRUE);

      if (!state)
         result = EGL_FALSE;
      else {
         /*
            get context
         */

         EGL_CONTEXT_T *context = egl_get_context(thread, state, ctx);

         if (context == NULL) {
            result = EGL_FALSE;
         }  else {

            /*
               get surfaces
            */

            EGL_SURFACE_T *draw = egl_get_surface(thread, state, dr);
            EGL_SURFACE_T *read = egl_get_surface(thread, state, rd);

            if (draw == NULL || read == NULL) {
               result = EGL_FALSE;
            } else if (thread->bound_api == EGL_OPENVG_API && dr != rd) {
               thread->error = EGL_BAD_MATCH;   //TODO: what error are we supposed to return here?
               result = EGL_FALSE;
            } else {
               EGL_CURRENT_T *current = &thread->opengl;

               /* Check if nothing is changing */
               changed = current->context != context || current->draw != draw || current->read != read;

               if (!changed || egl_current_set_surfaces(state, thread, context, draw, read))
               {
                  thread->error = EGL_SUCCESS;
                  result = EGL_TRUE;
               }
               else
               {
                  result = EGL_FALSE;
               }
            }
         }
      }
   }

   if (changed && result)
      flush_current_api(thread);

   CLIENT_UNLOCK();

   return result;
}

EGLAPI EGLContext EGLAPIENTRY eglGetCurrentContext(void)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLContext result;

   if (!egl_ensure_init_once())
      return EGL_NO_CONTEXT;

   CLIENT_LOCK();

   EGL_CURRENT_T *current = &thread->opengl;

   if (current->context == NULL)
      result = EGL_NO_CONTEXT;
   else {
      EGL_CONTEXT_T *context = current->context;
      result = context->name;
   }

   thread->error = EGL_SUCCESS;

   CLIENT_UNLOCK();

   return result;
}

EGLAPI EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLSurface result;

   if (!egl_ensure_init_once())
      return EGL_NO_SURFACE;

   CLIENT_LOCK();

   EGL_CURRENT_T *current = &thread->opengl;

   switch (readdraw) {
   case EGL_READ:
      result = current->read->name;
      thread->error = EGL_SUCCESS;
      break;
   case EGL_DRAW:
      result = current->draw->name;
      thread->error = EGL_SUCCESS;
      break;
   default:
      result = EGL_NO_SURFACE;
      thread->error = EGL_BAD_PARAMETER;
      break;
   }

   CLIENT_UNLOCK();

   return result;
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLDisplay result;

   if (!egl_ensure_init_once())
      return EGL_NO_DISPLAY;

   CLIENT_LOCK();

   EGL_CURRENT_T *current = &thread->opengl;

   if (current->context == NULL)
      result = EGL_NO_DISPLAY;
   else
      result = current->context->display;

   thread->error = EGL_SUCCESS;

   CLIENT_UNLOCK();

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      if (!value) {
         thread->error = EGL_BAD_PARAMETER;
         result = EGL_FALSE;
      } else {
         thread->error = EGL_SUCCESS;

         EGL_CONTEXT_T *context = egl_get_context(thread, state, ctx);

         if (context != NULL) {
            if (!egl_context_get_attrib(context, attribute, value))
               thread->error = EGL_BAD_ATTRIBUTE;
         }
         result = thread->error == EGL_SUCCESS;
      }
      CLIENT_UNLOCK();
   }
   else
      result = EGL_FALSE;

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitGL(void)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   //TODO: "If the surface associated with the calling thread's current context is no
   //longer valid, EGL_FALSE is returned and an EGL_BAD_CURRENT_SURFACE error is
   //generated".
   eglIntFinish_impl(true);

   thread->error = EGL_SUCCESS;
   result = EGL_TRUE;

   CLIENT_UNLOCK();

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   //TODO: "If the surface associated with the calling thread's current context is no
   //longer valid, EGL_FALSE is returned and an EGL_BAD_CURRENT_SURFACE error is
   //generated".

   if (engine == EGL_CORE_NATIVE_ENGINE) {
   //TODO: currently nothing we can do here
      thread->error = EGL_SUCCESS;
      result = EGL_TRUE;
   } else {
      thread->error = EGL_BAD_PARAMETER;
      result = EGL_FALSE;
   }

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surf)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   if (CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
   {
      thread->error = EGL_SUCCESS;

      EGL_SURFACE_T *surface = egl_get_surface(thread, state, surf);

      if (surface != NULL) {

#if !(EGL_KHR_lock_surface)
         /* Surface to be displayed must be bound to current context and API */
         /* This check is disabled if we have the EGL_KHR_lock_surface extension */
         if (thread->bound_api == EGL_OPENGL_ES_API && surface != thread->opengl.draw && surface != thread->opengl.read
          || thread->bound_api == EGL_OPENVG_API    && surface != thread->openvg.draw) {
            thread->error = EGL_BAD_SURFACE;
         } else
#endif
         {
            if (surface->type == WINDOW) {
               /* the egl spec says eglSwapBuffers is supposed to be a no-op for
                * single-buffered surfaces, but we pass it through as the
                * semantics are potentially useful:
                * - any ops outstanding on the surface are flushed
                * - the surface is resubmitted to the display once the
                *   outstanding ops complete (for displays which have their own
                *   memory, this is useful)
                * - the surface is resized to fit the backing window */

               /* TODO: raise EGL_BAD_ALLOC if we try to enlarge window and then run out of memory */

               egl_swapbuffers(surface);
            }
         }
      }

      result = (thread->error == EGL_SUCCESS);
      CLIENT_UNLOCK();
   }
   else
      result = EGL_FALSE;

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglCopyBuffers(EGLDisplay dpy, EGLSurface surf, EGLNativePixmapType target)
{
   CLIENT_THREAD_STATE_T *thread;
   EGL_SERVER_STATE_T *state;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   if (!CLIENT_LOCK_AND_GET_STATES(dpy, &thread, &state))
      return EGL_FALSE;

   EGLBoolean result;
   EGLint error = EGL_SUCCESS;

   thread->error = EGL_SUCCESS;

   EGL_SURFACE_T *surface = egl_get_surface(thread, state, surf);

   if ((thread->bound_api == EGL_OPENGL_ES_API && surface != thread->opengl.draw && surface != thread->opengl.read)) {
         /* Surface to be displayed must be bound to current context and API */
         /* TODO remove this restriction, as we'll need to for eglSwapBuffers? */
         error = EGL_BAD_SURFACE;
         goto end;
   }

   if (surface)
      error = egl_copybuffers(surface, target);

end:
   result = (error == EGL_SUCCESS);
   thread->error = error;

   CLIENT_UNLOCK();

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy,
   const EGLint *attrib_list, EGLConfig *configs,
   EGLint config_size, EGLint *num_config)
{
   if (!egl_ensure_init_once())
      return EGL_FALSE;

   return egl_choose_config(dpy, attrib_list, configs, config_size, num_config);
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy,
   EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
   if (!egl_ensure_init_once())
      return EGL_FALSE;

   /*
   * Technically I don't think you need to sort them for eglGetConfigs, but
   * since if you don't, the order is unspecified, it doesn't do any harm.
   */
   return egl_choose_config(dpy, NULL, configs, config_size, num_config);
}
