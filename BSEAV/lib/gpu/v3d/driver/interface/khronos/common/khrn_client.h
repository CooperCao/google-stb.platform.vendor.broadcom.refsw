/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

typedef struct EGL_CURRENT EGL_CURRENT_T;
typedef struct CLIENT_THREAD_STATE CLIENT_THREAD_STATE_T;

#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/egl/egl_client_context.h"
#include "interface/khronos/egl/egl_client_surface.h"
#include <EGL/eglext.h>
#include "interface/khronos/common/khrn_client_pointermap.h"
#include "middleware/khronos/egl/egl_server.h"

struct EGL_CURRENT {
   EGL_CONTEXT_T *context;
   EGL_SURFACE_T *draw;
   EGL_SURFACE_T *read;
};

struct CLIENT_THREAD_STATE {
   /*
      error

      Invariant:
      (CLIENT_THREAD_STATE_ERROR)
      error is one of
         EGL_SUCCESS
         EGL_NOT_INITIALIZED
         EGL_BAD_ACCESS
         EGL_BAD_ALLOC
         EGL_BAD_ATTRIBUTE
         EGL_BAD_CONTEXT
         EGL_BAD_CONFIG
         EGL_BAD_CURRENT SURFACE
         EGL_BAD_DISPLAY
         EGL_BAD_SURFACE
         EGL_BAD_MATCH
         EGL_BAD_PARAMETER
         EGL_BAD_NATIVE PIXMAP
         EGL_BAD_NATIVE WINDOW
         EGL_CONTEXT_LOST
   */
   EGLint error;

   EGLenum bound_api;

   /*
      handles to current display, context and surfaces for each API

      Availability:

      Thread owns EGL lock
   */

   EGL_CURRENT_T opengl;

   bool events_acquired;
   bool perf_counters_acquired;
};

extern void client_thread_state_init(CLIENT_THREAD_STATE_T *state);
extern void client_thread_state_term(CLIENT_THREAD_STATE_T *state);

extern PLATFORM_TLS_T client_tls;

static inline CLIENT_THREAD_STATE_T *CLIENT_GET_THREAD_STATE(void)
{
   return (CLIENT_THREAD_STATE_T *)platform_tls_get(client_tls);
}

/*
   exposed bits of EGL
*/

EGL_SERVER_STATE_T *egl_get_process_state(CLIENT_THREAD_STATE_T *thread, EGLDisplay dpy, EGLBoolean check_inited);
EGL_CONTEXT_T *egl_get_context(CLIENT_THREAD_STATE_T *thread, EGL_SERVER_STATE_T *state, EGLContext ctx);
EGL_SURFACE_T *egl_get_surface(CLIENT_THREAD_STATE_T *thread, EGL_SERVER_STATE_T *state, EGLSurface surf);

/*
   client state
*/

extern bool egl_ensure_init_once(void);

extern void CLIENT_LOCK(void);
extern void CLIENT_UNLOCK(void);

/*
   bool CLIENT_LOCK_AND_GET_STATES(EGLDisplay dpy, CLIENT_THREAD_STATE_T **thread, CLIENT_PROCESS_STATE_T **process)

   Try to acquire EGL lock and get thread and process state.

   Implementation notes:

   TODO make sure this gets reviewed

   Preconditions:

   thread is a valid pointer to a thread*
   process is a valid pointer to a process*
   Mutex: >(MUTEX_EGL_LOCK)
   Is being called from a function which calls CLIENT_UNLOCK() if we return true

   Postconditions:

   The following conditions cause error to assume the specified value

      EGL_BAD_DISPLAY               An EGLDisplay argument does not name a valid EGLDisplay
      EGL_NOT_INITIALIZED           EGL is not initialized for the specified display.

   if more than one condition holds, the first error is generated.

   Either:
      Mutex: (MUTEX_EGL_LOCK)
      Thread owns EGL lock
      result is true
   Or:
      Nothing changes
      result is false
*/

static inline bool CLIENT_LOCK_AND_GET_STATES(EGLDisplay dpy, CLIENT_THREAD_STATE_T **thread, EGL_SERVER_STATE_T **state)
{
   *thread = CLIENT_GET_THREAD_STATE();
   CLIENT_LOCK();
   *state = egl_get_process_state(*thread, dpy, EGL_TRUE);
   if (*state != NULL)
      return true;
   else
   {
      CLIENT_UNLOCK();
      return false;
   }
}

/*
   process and thread attach/detach hooks
*/

extern bool client_process_attach(void);
extern bool client_thread_attach(PLATFORM_TLS_T tls);
/* The tls parameter will only be valid if the client_thread_detach originates from the thread destructor in pthreads */
extern void client_thread_detach(void * tls);
extern void client_process_detach(void);
