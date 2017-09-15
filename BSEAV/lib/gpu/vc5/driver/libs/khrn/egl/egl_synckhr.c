/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "vcos.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "egl_display.h"
#include "egl_thread.h"
#include "egl_sync.h"
#include "egl_platform.h"
#include "libs/platform/v3d_platform.h"
#include "../glxx/glxx_server.h"
#include "egl_context_gl.h"

/* in case of error, the function is responsible for setting an egl error */
static EGL_SYNC_T* egl_gl_synckhr_new(EGL_CONTEXT_T *context,
      const void *attrib_list, EGL_AttribType attrib_type)
{
   EGL_SYNC_T *egl_sync = NULL;
   GLXX_SERVER_STATE_T *state = NULL;
   EGL_GL_CONTEXT_T *gl_context;
   GLenum error = EGL_BAD_ALLOC;
   EGLint name;
   EGLAttribKHR value;

   /* check attrib list is either NULL or empty */
   if (egl_next_attrib(&attrib_list, attrib_type, &name, &value))
   {
      vcos_unused(name);
      vcos_unused(value);
      error = EGL_BAD_ATTRIBUTE;
      goto end;
   }

   assert(context->api == API_OPENGL);
   gl_context = (EGL_GL_CONTEXT_T *) context;

   if (!egl_context_gl_lock())
      goto end;
   state = egl_context_gl_server_state(gl_context);
   assert(state);
   egl_sync = egl_sync_create(EGL_SYNC_FENCE_KHR,
      EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR, state->fences.fence);
   egl_context_gl_unlock();

   if (!egl_sync)
      goto end;

   error = EGL_SUCCESS;
end:
   if (error != EGL_SUCCESS)
      egl_thread_set_error(error);
   return egl_sync;
}

static bool egl_cl_event_valid(void *cl_event)
{
   /* TODO: check event validity once cl_events are supported */
   return false;
}

static GLenum egl_cl_synckhr_parse_attrib_list(const void *attrib_list,
      EGL_AttribType attrib_type, void **cl_event)
{
   EGLint name;
   EGLAttribKHR value;
   GLenum result = EGL_BAD_ATTRIBUTE;
   bool finished = !attrib_list || !cl_event;

   while (!finished)
   {
      finished = !egl_next_attrib(&attrib_list, attrib_type, &name, &value);
      switch (name)
      {
         case EGL_CL_EVENT_HANDLE_KHR:
            *cl_event = (void *)value;
            if (egl_cl_event_valid(*cl_event))
                  result = EGL_SUCCESS;
            break;
         default:
            break;
      }
   }
   return result;
}

/* in case of error, the function is responsible for setting an egl error */
static EGL_SYNC_T* egl_cl_synckhr_new(EGL_CONTEXT_T *context,
      const void *attrib_list, EGL_AttribType attrib_type)
{
   EGL_SYNC_T *egl_sync = NULL;
   GLXX_SERVER_STATE_T *state = NULL;
   EGL_GL_CONTEXT_T *gl_context;
   GLenum error = EGL_SUCCESS;
   void *cl_event = NULL;

   error = egl_cl_synckhr_parse_attrib_list(attrib_list, attrib_type, &cl_event);
   if (error != EGL_SUCCESS)
      goto end;

   error = EGL_BAD_ALLOC; /* for now */

   assert(context->api == API_OPENGL);
   gl_context = (EGL_GL_CONTEXT_T *) context;

   if (!egl_context_gl_lock())
      goto end;
   state = egl_context_gl_server_state(gl_context);
   assert(state);
   egl_sync = egl_sync_create_from_cl_event(EGL_CL_EVENT_HANDLE_KHR,
         EGL_SYNC_CL_EVENT_COMPLETE_KHR, cl_event);
   egl_context_gl_unlock();

   if (!egl_sync)
      goto end;

   error = EGL_SUCCESS;
end:
   if (error != EGL_SUCCESS)
      egl_thread_set_error(error);
   return egl_sync;
}

static EGLint egl_gl_synckhr_wait(EGL_CONTEXT_T *context, EGL_SYNC_T *egl_sync)
{
   EGLint error = EGL_SUCCESS;
   EGL_GL_CONTEXT_T *gl_context;
   GLXX_SERVER_STATE_T *state = NULL;

   assert(context->api == API_OPENGL);

   gl_context = (EGL_GL_CONTEXT_T *) context;

   if (egl_context_gl_lock())
   {
      state = egl_context_gl_server_state(gl_context);
      assert(state);
      if (!glxx_server_state_add_fence_to_depend_on(state, egl_sync->fence))
         error = EGL_BAD_ALLOC;
      egl_context_gl_unlock();
   }
   else
      error = EGL_BAD_ALLOC;

   return error;
}

static EGLSyncKHR egl_create_sync_impl(EGLDisplay dpy, EGLenum type,
      const void *attrib_list, EGL_AttribType attrib_type)
{
   EGL_SYNC_T *egl_sync = NULL;
   EGLSyncKHR sync = EGL_NO_SYNC_KHR;
   EGL_CONTEXT_T *context;
   EGLint error;

   if (!egl_initialized(dpy, true))
      return sync;

   context = egl_thread_get_context();
   if (!context)
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   switch (type)
   {
      case EGL_SYNC_FENCE_KHR:
         egl_sync = egl_gl_synckhr_new(context, attrib_list, attrib_type);
         break;
      case EGL_CL_EVENT_HANDLE_KHR:
         egl_sync = egl_cl_synckhr_new(context, attrib_list, attrib_type);
         break;
      default:
         egl_sync = egl_platform_synckhr_new(context, type, attrib_list, attrib_type);
         break;
   }

   if (!egl_sync)
      return EGL_NO_SYNC_KHR;

   sync = egl_map_sync(egl_sync);
   if (!sync)
   {
      error = EGL_BAD_ALLOC;
      goto end;
   }
   error = EGL_SUCCESS;
end:
   if (error != EGL_SUCCESS)
      egl_sync_refdec(egl_sync);

   egl_thread_set_error(error);
   return sync;
}

EGLAPI EGLSync EGLAPIENTRY eglCreateSync (EGLDisplay dpy, EGLenum type,
      const EGLAttrib *attrib_list)
{
   return egl_create_sync_impl(dpy, type, attrib_list, attrib_EGLAttribKHR);
}

EGLAPI EGLSyncKHR EGLAPIENTRY eglCreateSyncKHR(EGLDisplay dpy, EGLenum type,
      const EGLint *attrib_list)
{
   return egl_create_sync_impl(dpy, type, attrib_list, attrib_EGLint);
}

EGLAPI EGLSyncKHR EGLAPIENTRY eglCreateSync64KHR(EGLDisplay dpy, EGLenum type,
      const EGLAttribKHR *attrib_list)
{
   return egl_create_sync_impl(dpy, type, attrib_list, attrib_EGLAttribKHR);
}

static int nanosec_to_trunc_ms(EGLTimeKHR timeout)
{
   GLuint64 timeout_ms = timeout / (1000 * 1000);

   // Round up
   if ((timeout_ms * 1000 * 1000) < timeout)
      ++timeout_ms;

   if (timeout_ms > INT_MAX)
      timeout_ms = INT_MAX ;

   return (int)timeout_ms;
}

static EGLint egl_client_waitsync_impl(EGLDisplay dpy,
      EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout)
{
   EGL_SYNC_T *egl_sync = NULL;
   EGLint error = EGL_SUCCESS;
   EGLint ret = EGL_FALSE;
   int timeout_ms = 0;
   int platform_fence;
   enum v3d_fence_status status;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   if (flags != 0 && flags != EGL_SYNC_FLUSH_COMMANDS_BIT_KHR )
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   egl_sync = egl_get_sync_refinc(sync);

   if (egl_sync == NULL)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   if (egl_sync_is_signaled(egl_sync))
   {
      ret = EGL_CONDITION_SATISFIED_KHR;
      goto end;
   }

   if (timeout != EGL_FOREVER_KHR)
   {
      timeout_ms = nanosec_to_trunc_ms(timeout);
      if (timeout_ms == 0)
      {
         egl_context_gl_lock();
         khrn_fence_flush(egl_sync->fence);
         egl_context_gl_unlock();
         ret = EGL_TIMEOUT_EXPIRED_KHR;
         goto end;
       }
   }

   /* we need a gl lock on all the operations involving a khrn_fence because
    * we are tempering with khrn_fmems */
   if (!egl_context_gl_lock())
      goto end;
   platform_fence = khrn_fence_get_platform_fence(egl_sync->fence,
      EGL_SYNC_SIGNALED_DEPS_STATE, /*force_create=*/false);
   egl_context_gl_unlock();

   if (timeout == EGL_FOREVER_KHR)
   {
      v3d_platform_fence_wait(platform_fence);
      status = V3D_FENCE_SIGNALED;
   }
   else
      status = v3d_platform_fence_wait_timeout(platform_fence, timeout_ms);

   v3d_platform_fence_close(platform_fence);

   if (status == V3D_FENCE_TIMEOUT)
   {
      ret = EGL_TIMEOUT_EXPIRED_KHR;
      goto end;
   }
   assert(status == V3D_FENCE_SIGNALED);
   ret = EGL_CONDITION_SATISFIED_KHR;
   egl_sync_set_signaled(egl_sync);
end:
   egl_sync_refdec(egl_sync);
   egl_thread_set_error(error);
   return ret;
}

EGLAPI EGLint EGLAPIENTRY eglClientWaitSync(EGLDisplay dpy, EGLSync sync,
      EGLint flags, EGLTime timeout)
{
   return egl_client_waitsync_impl(dpy, sync, flags, timeout);
}

EGLAPI EGLint EGLAPIENTRY eglClientWaitSyncKHR(EGLDisplay dpy,
      EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout)
{
   return egl_client_waitsync_impl(dpy, sync, flags, timeout);
}

static EGLBoolean egl_destroy_sync_impl(EGLDisplay dpy,
      EGLSyncKHR sync_id)
{
   EGL_SYNC_T *egl_sync = NULL;
   EGLint error = EGL_BAD_PARAMETER;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   egl_sync = egl_unmap_sync(sync_id);
   if (!egl_sync)
      goto end;
   egl_sync_refdec(egl_sync);

   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySync(EGLDisplay dpy, EGLSyncKHR sync)
{
   return egl_destroy_sync_impl(dpy, sync);
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySyncKHR(EGLDisplay dpy, EGLSyncKHR sync)
{
   return egl_destroy_sync_impl(dpy, sync);
}

static EGLBoolean egl_get_sync_attrib_impl(EGLDisplay dpy, EGLSyncKHR sync,
      EGLint attribute, EGLAttribKHR *value)
{
   EGL_SYNC_T *egl_sync = NULL;
   EGLint error;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   egl_sync = egl_get_sync_refinc(sync);

   if (egl_sync == NULL || value == NULL)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   switch(attribute)
   {
      case EGL_SYNC_TYPE_KHR:
         *value =  egl_sync->type;
         break;
      case EGL_SYNC_STATUS_KHR:
         *value = egl_sync_is_signaled(egl_sync) ? EGL_SIGNALED_KHR :
            EGL_UNSIGNALED_KHR;
         break;
      case EGL_SYNC_CONDITION_KHR:
         *value = egl_sync->condition;
         break;
      default:
         error = EGL_BAD_ATTRIBUTE;
         goto end;
   }
   error = EGL_SUCCESS;

end:
   egl_sync_refdec(egl_sync);
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetSyncAttrib(EGLDisplay dpy, EGLSync sync,
      EGLint attribute, EGLAttrib *value)
{
   return egl_get_sync_attrib_impl(dpy, sync, attribute, value);
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetSyncAttribKHR(EGLDisplay dpy, EGLSyncKHR sync,
      EGLint attribute, EGLint *value)
{
   EGLAttribKHR new_value = 0;
   EGLBoolean result = egl_get_sync_attrib_impl(dpy, sync, attribute,
         value ? &new_value : NULL); /* if value==NULL pass NULL in as well */
   if (result && value)
      *value = (EGLint)new_value;
   return result;
}

static EGLint egl_wait_sync_impl(EGLDisplay dpy,
      EGLSyncKHR sync, EGLint flags)
{
   EGL_SYNC_T *egl_sync = NULL;
   EGLint error = EGL_SUCCESS;
   EGL_CONTEXT_T *context;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   if (flags != 0)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   context = egl_thread_get_context();
   if (!context)
   {
      error = EGL_BAD_MATCH;
      goto end;
   }

   egl_sync = egl_get_sync_refinc(sync);

   if (egl_sync == NULL)
   {
      error = EGL_BAD_PARAMETER;
      goto end;
   }

   error = egl_gl_synckhr_wait(context, egl_sync);
end:
   egl_sync_refdec(egl_sync);
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitSync (EGLDisplay dpy, EGLSync sync,
      EGLint flags)
{
   return egl_wait_sync_impl(dpy, sync, flags);
}

EGLAPI EGLint EGLAPIENTRY eglWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync,
      EGLint flags)
{
   return egl_wait_sync_impl(dpy, sync, flags);
}
