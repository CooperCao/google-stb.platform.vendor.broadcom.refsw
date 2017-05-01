/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "libs/util/demand.h" /* TODO Shouldn't be using demand in production code... */
#include "../common/khrn_process.h"
#include "../common/khrn_record.h"
#include "egl_thread.h"
#include "egl_display.h"
#include "egl_platform.h"
#include "egl_process.h"
#include "egl_surface.h"
#include "egl_context_base.h"
#include "egl_config.h"

LOG_DEFAULT_CAT("egl_thread")

static void thread_destroy(void* t);

// State associated with all of the EGL threads, i.e. with the whole process
static struct
{
   VCOS_ONCE_T       once;
   bool              once_ok;
   VCOS_MUTEX_T      lock;
   VCOS_TLS_KEY_T    tls_key;
   EGL_THREAD_T*     list;
} egl_thread;

static void init_once(void)
{
   egl_thread.once_ok = vcos_mutex_create(&egl_thread.lock, "egl_thread.lock") == VCOS_SUCCESS;
   egl_thread.once_ok &= vcos_tls_create(&egl_thread.tls_key, thread_destroy) == VCOS_SUCCESS;

   if (!egl_thread.once_ok)
   {
      log_error("Fatal: unable to init once egl_thread");
   }
}

static bool ensure_init_once(void)
{
   demand(vcos_once(&egl_thread.once, init_once) == VCOS_SUCCESS);
   return egl_thread.once_ok;
}

static EGLint make_current(EGL_THREAD_T *thread, egl_api_t api,
      EGL_SURFACE_T *draw, EGL_SURFACE_T *read, EGL_CONTEXT_T *context);

// Initialize the EGL_THREAD_T in this thread's local storage
static EGL_THREAD_T *thread_create(void)
{
   if (!ensure_init_once())
   {
      return NULL;
   }

   EGL_THREAD_T *thread = calloc(1, sizeof *thread);
   if (!thread) return NULL;

   // add a reference to egl_process, but don't actually initialise it
   egl_process_add_ref();

   if (vcos_tls_set(egl_thread.tls_key, thread) == VCOS_SUCCESS)
   {
      // all ok
      thread->error = EGL_SUCCESS;

      // add to list
      vcos_mutex_lock(&egl_thread.lock);
      thread->next = egl_thread.list;
      egl_thread.list = thread;
      vcos_mutex_unlock(&egl_thread.lock);

      return thread;
   }
   log_error("Fatal: unable to allocate thread-local-storage for EGL");

   egl_process_release();

   free(thread);
   return NULL;
}

static void thread_destroy(void* t)
{
   EGL_THREAD_T *thread = t;

   vcos_mutex_lock(&egl_thread.lock);

   for (egl_api_t api = 0; api < API_COUNT; api++)
   {
      make_current(thread, api, NULL, NULL, NULL);
   }

   // remove from linked list
   for (EGL_THREAD_T** prev = &egl_thread.list; *prev != NULL; prev = &(*prev)->next)
   {
      if (*prev == thread)
      {
         *prev = thread->next;
         break;
      }
   }

   vcos_tls_set(egl_thread.tls_key, NULL);

   vcos_mutex_unlock(&egl_thread.lock);

   // this is ref counted
   egl_process_release();

   free(t);
}

//! get the elg_thread object if it exists, otherwise return NULL
static inline EGL_THREAD_T *egl_thread_try_get(void)
{
   // ensure tls_key is initialised
   if (!ensure_init_once()) return NULL;

   return vcos_tls_get(egl_thread.tls_key);
}

EGL_THREAD_T *egl_thread_get(void)
{
   EGL_THREAD_T* thread = egl_thread_try_get();
   if (!thread)
   {
      thread = thread_create();
   }
   return thread;
}

egl_api_t egl_api_from_name(EGLenum api) {
   switch (api)
   {
   case EGL_OPENGL_ES_API: return API_OPENGL;
   case EGL_OPENVG_API:    return API_OPENVG;
   default:
      unreachable();
   }
}

EGLenum egl_name_from_api(egl_api_t api) {
   switch (api)
   {
   case API_OPENGL: return EGL_OPENGL_ES_API;
   case API_OPENVG: return EGL_OPENVG_API;
   default: unreachable();
   }
}

EGLAPI EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api)
{
   EGL_THREAD_T *thread;

   if (!egl_initialized(NULL, false))
      return EGL_FALSE;

   thread = egl_thread_get();

   switch (api)
   {
   case EGL_OPENGL_ES_API:
      thread->current_api = egl_api_from_name(api);
      break;
   default:
      thread->error = EGL_BAD_PARAMETER;
      return EGL_FALSE;
   }

   return EGL_TRUE;
}

EGLAPI EGLenum EGLAPIENTRY eglQueryAPI(void)
{
   EGL_THREAD_T *thread;

   if (!egl_initialized(NULL, false))
      return EGL_NONE;

   thread = egl_thread_get();
   return egl_name_from_api(thread->current_api);
}

EGLAPI EGLint EGLAPIENTRY eglGetError(void)
{
   /*
    * egl_initialized clears the last error, so read it first. This means we
    * do need to check for a NULL thread.
    */
   EGL_THREAD_T *thread = egl_thread_get();

   if (thread)
   {
      EGLint result = thread->error;
      thread->error = EGL_SUCCESS;
      return result;
   }
   else
      return EGL_NOT_INITIALIZED;
}

bool egl_thread_set_error(EGLint error)
{
   EGL_THREAD_T *thread = egl_thread_get();
   if (!thread) return false;

   thread->error = error;
   return true;
}

EGLAPI EGLContext EGLAPIENTRY eglGetCurrentContext(void)
{
   EGL_THREAD_T *thread;
   egl_api_t api;

   if (!egl_initialized(0, false))
      return EGL_NO_CONTEXT;

   thread = egl_thread_get();
   api = thread->current_api;
   assert(api >= 0 && api < API_COUNT);

   return egl_get_context_handle(thread->contexts[api]);
}

EGLAPI EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw)
{
   EGL_THREAD_T *thread;
   EGL_CONTEXT_T *context;
   EGL_SURFACE_T *surface = NULL;
   egl_api_t api;
   bool      readBuf = false;

   if (!egl_initialized(0, false))
      return EGL_NO_SURFACE;

   switch (readdraw)
   {
   case EGL_READ:
      readBuf = true;
      break;
   case EGL_DRAW:
      readBuf = false;
      break;
   default:
      egl_thread_set_error(EGL_BAD_PARAMETER);
      return EGL_NO_SURFACE;
   }

   thread = egl_thread_get();
   api = thread->current_api;
   assert(api >= 0 && api < API_COUNT);

   context = thread->contexts[api];
   if (!context)
      return EGL_NO_SURFACE;

   if (readBuf)
      surface = context->read;
   else
      surface = context->draw;

   return egl_get_surface_handle(surface);
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void)
{
   EGL_THREAD_T *thread;
   egl_api_t api;

   if (!egl_initialized(0, false))
      return EGL_NO_DISPLAY;

   thread = egl_thread_get();
   api = thread->current_api;
   assert(api >= 0 && api < API_COUNT);

   if (!thread->contexts[api])
      return EGL_NO_DISPLAY;

   return egl_platform_get_default_display();
}

static EGLint make_current(EGL_THREAD_T *thread, egl_api_t api,
      EGL_SURFACE_T *draw, EGL_SURFACE_T *read, EGL_CONTEXT_T *context)
{
   EGL_THREAD_T *bound_thread;
   EGL_CONTEXT_T *previous;

   enum { DRAW, READ, COUNT } i;
   EGL_SURFACE_T *surfaces[COUNT];

   assert(api >= 0 && api < API_COUNT);
   previous = thread->contexts[api];

   if (context == NULL)
   {
      if (draw == NULL && read == NULL)
      {
         if (previous)
         {
            egl_context_flush(previous);
            egl_context_detach(previous);
            thread->contexts[api] = NULL;
         }
         return EGL_SUCCESS;
      }
      return EGL_BAD_MATCH;
   }

   /* either both or none must be NULL*/
   if (draw == NULL || read == NULL)
   {
      if (draw != read) /* one of them is NULL - check the other one */
         return EGL_BAD_MATCH;
   }

   bound_thread = context->bound_thread;
   if (bound_thread && bound_thread != thread)
      return EGL_BAD_ACCESS;

   surfaces[DRAW] = draw;
   surfaces[READ] = read;

   for (i = 0; i < COUNT; i++)
   {
      if (surfaces[i])
      {
         EGL_THREAD_T *surface_thread;

         surface_thread = egl_surface_get_thread(surfaces[i]);
         if (surface_thread && surface_thread != thread)
            return EGL_BAD_ACCESS;

         if (egl_surface_get_back_buffer(surfaces[i]) == NULL)
            return EGL_BAD_NATIVE_WINDOW;

         if (!egl_config_context_surface_compatible(context, surfaces[i]))
            return EGL_BAD_MATCH;

         if (!egl_surface_resize(surfaces[i]))
            return EGL_BAD_ALLOC;
      }
   }

   if (previous)
   {
      egl_context_flush(previous);
      egl_context_detach(previous);
   }

   context->bound_thread = thread;
   thread->contexts[api] = context;

   egl_context_attach(context, surfaces[DRAW], surfaces[READ]);

   return EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy,
      EGLSurface dr, EGLSurface rd, EGLContext ctx)
{
   EGLint error = EGL_SUCCESS;
   EGL_THREAD_T *thread = egl_thread_get();
   EGL_SURFACE_T *draw = NULL;
   EGL_SURFACE_T *read = NULL;
   EGL_CONTEXT_T *context = NULL;

   if (!egl_is_valid_display(dpy))
   {
      error = EGL_BAD_DISPLAY;
      goto end;
   }

   if (dr != EGL_NO_SURFACE || rd != EGL_NO_SURFACE || ctx != EGL_NO_CONTEXT)
   {
      if (!egl_initialized(dpy, true))
      {
         error = EGL_NOT_INITIALIZED;
         goto end;
      }

      if (  (dr != EGL_NO_SURFACE && !(draw = egl_get_surface(dr)))
         || (rd != EGL_NO_SURFACE && !(read = egl_get_surface(rd)))
         )
      {
         error = EGL_BAD_SURFACE;
         goto end;
      }

      if (ctx != EGL_NO_CONTEXT && !(context = egl_get_context(ctx)))
      {
         error = EGL_BAD_CONTEXT;
         goto end;
      }
   }

   if (thread != NULL)
   {
      vcos_mutex_lock(&egl_thread.lock); // is this lock necessary?
      error = make_current(thread, thread->current_api, draw, read, context);
      vcos_mutex_unlock(&egl_thread.lock);
   }

end:
   if (thread != NULL)
      thread->error = error;
   return error == EGL_SUCCESS;
}

EGL_CONTEXT_T *egl_thread_get_context(void)
{
   EGL_THREAD_T *thread = egl_thread_get();
   EGL_CONTEXT_T *ret;

   egl_api_t api = thread->current_api;
   assert(api >= 0 && api < API_COUNT);
   ret = thread->contexts[api];

   assert(ret == NULL || ret->api == api);
   return ret && ret->valid ? ret : NULL;
}

EGLBoolean egl_release_thread(void)
{
   EGL_THREAD_T *thread = egl_thread_try_get();
   if (!thread) return EGL_FALSE;

   // destroy thread and clear pointer from TLS
   thread_destroy(thread);

   return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglReleaseThread(void)
{
   return egl_release_thread();
}
