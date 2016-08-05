/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include <EGL/egl.h>
#include "vcos.h"
#include "egl_platform.h"
#include "egl_thread.h"
#include "egl_display.h"
#include "egl_surface.h"
#include "egl_surface_base.h"
#include "egl_context_base.h"
#include "egl_config.h"
#include "egl_context_gl.h"

EGLAPI EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy,
      EGLConfig config, EGLContext share_ctx, const EGLint *attrib_list)
{
   EGLContext ret = EGL_NO_CONTEXT;
   EGL_CONTEXT_T *context;
   EGLint error;

   if (!egl_initialized(dpy, true))
      return EGL_NO_CONTEXT;

   error = egl_context_gl_create((EGL_GL_CONTEXT_T**)&context, config, share_ctx, attrib_list);
   if (error != EGL_SUCCESS)
      goto end;

   if (share_ctx && !egl_get_context(share_ctx))
   {
      error = EGL_BAD_CONTEXT;
      goto end;
   }

   ret = egl_map_context(context);
   if (ret == EGL_NO_CONTEXT)
   {
      error = EGL_BAD_ALLOC;
      goto end;
   }

   error = EGL_SUCCESS;
end:
   if (error != EGL_SUCCESS)
   {
      egl_unmap_context(ret);
      egl_context_try_delete(context);

      context = NULL;
      ret = EGL_NO_CONTEXT;
   }
   egl_thread_set_error(error);
   return ret;
}

EGLAPI EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy,
      EGLContext ctx, EGLint attribute, EGLint *value)
{
   EGLint error = EGL_BAD_ATTRIBUTE;
   const EGL_CONTEXT_T *context;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   context = egl_get_context(ctx);
   if (!context)
   {
      error = EGL_BAD_CONTEXT;
      goto end;
   }

   switch (attribute)
   {
   case EGL_CONFIG_ID:
      *value = egl_config_get_attrib(context->config, EGL_CONFIG_ID, NULL);
      break;

   case EGL_CONTEXT_CLIENT_TYPE:
      *value = egl_name_from_api(context->api);
      break;

   case EGL_CONTEXT_CLIENT_VERSION:
      *value = egl_context_client_version(context);
      break;

   case EGL_RENDER_BUFFER:
      if (context->draw == NULL)
         *value = EGL_NONE;
      else
      {
         EGLAttribKHR attrib;
         if (egl_surface_base_get_attrib(context->draw, EGL_RENDER_BUFFER, &attrib))
            *value = (EGLint)attrib;
      }
      break;

   default:
      goto end;
   }

   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
   EGL_CONTEXT_T *context;
   EGLint error = EGL_BAD_CONTEXT;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   context = egl_get_context(ctx);
   if (!context) goto end;

   egl_unmap_context(ctx);
   egl_context_try_delete(context);

   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitClient(void)
{
   EGL_CONTEXT_T *context;
   EGLint error = EGL_BAD_CURRENT_SURFACE;

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   context = egl_thread_get_context();
   if (!context) return EGL_TRUE;

   if (!context->draw) goto end;

   egl_context_wait(context);

   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitGL(void)
{
   EGL_THREAD_T *thread;
   egl_api_t current_api;
   EGL_CONTEXT_T *context;
   EGLint error = EGL_BAD_CURRENT_SURFACE;

   if (!egl_initialized(0, false))
      return EGL_FALSE;

   thread = egl_thread_get();
   current_api = thread->current_api;
   thread->current_api = API_OPENGL;
   context = egl_thread_get_context();

   if (!context)
   {
      thread->current_api = current_api;
      return EGL_TRUE;
   }

   if (!context->draw) goto end;

   egl_context_wait(context);

   error = EGL_SUCCESS;
end:
   thread->current_api = current_api;
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine)
{
   return egl_platform_wait_native(engine);
}

void egl_context_attach(EGL_CONTEXT_T *context,
      EGL_SURFACE_T *draw, EGL_SURFACE_T *read)
{
   if (context->fns->attach == NULL)
      egl_context_base_attach(context, draw, read);
   else
      context->fns->attach(context, draw, read);
}

void egl_context_reattach(EGL_CONTEXT_T *context)
{
   egl_context_attach(context, context->draw, context->read);
}

void egl_context_detach(EGL_CONTEXT_T *context)
{
   if (context->fns->detach == NULL)
      egl_context_base_detach(context);
   else
      context->fns->detach(context);
}

void egl_context_flush(EGL_CONTEXT_T *context)
{
   context->fns->flush(context, false);
}

bool egl_context_copy_image(EGL_CONTEXT_T *context,
      KHRN_IMAGE_T *dst, KHRN_IMAGE_T *src)
{
   return context->fns->copy_image(context, dst, src);
}

void egl_context_invalidate_draw(EGL_CONTEXT_T *context,
   bool color, bool color_ms, bool other_aux)
{
   context->fns->invalidate_draw(context, color, color_ms, other_aux);
}

bool egl_context_try_delete(EGL_CONTEXT_T *context)
{
   if (!context || egl_get_context_handle(context) != EGL_NO_CONTEXT)
      return false;

   if (context->fns->invalidate)
      context->fns->invalidate(context);

   if (context->bound_thread)
      return false;

   free(context);
   return true;
}

int egl_context_client_version(const EGL_CONTEXT_T *context)
{
   return context->fns->client_version(context);
}

void egl_context_wait(EGL_CONTEXT_T *context)
{
   context->fns->flush(context, true);
}
