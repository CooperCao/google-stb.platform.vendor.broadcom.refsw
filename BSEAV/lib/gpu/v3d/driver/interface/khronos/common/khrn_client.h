/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
client side API
=============================================================================*/
#ifndef KHRN_CLIENT_H
#define KHRN_CLIENT_H

typedef struct CLIENT_PROCESS_STATE CLIENT_PROCESS_STATE_T;
typedef struct CLIENT_THREAD_STATE CLIENT_THREAD_STATE_T;

#include "interface/khronos/common/khrn_client_platform.h"
#include "interface/khronos/egl/egl_client_context.h"
#include "interface/khronos/egl/egl_client_surface.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/common/khrn_client_pointermap.h"

/*
   per-thread state

   - EGL error
   - EGL bound API
   - EGL context and surfaces for each API

   - RPC merge buffer
*/

#define MERGE_BUFFER_SIZE  4080

typedef struct {
   EGL_CONTEXT_T *context;
   EGL_SURFACE_T *draw;
   EGL_SURFACE_T *read;
} EGL_CURRENT_T;

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
#ifndef NO_OPENVG
   EGL_CURRENT_T openvg;
#endif /* NO_OPENVG */

   /*
      rpc stuff
   */

   bool high_priority;

   uint8_t merge_buffer[MERGE_BUFFER_SIZE];

   uint32_t merge_pos;
   uint32_t merge_end;

   /* Has a pixmap surface been made current at all since the last glFlush or glFinish call?
      glFlush & glFinish are no-ops unless we are currently rendering to a pixmap or have done since the last calls.
    */
   bool has_rendered_to_pixmap;
   bool is_current_pixmap;

   /* Per-thread error caches used by the API interposer (for GPU monitor) */
   uint32_t cached_egl_error;
   uint32_t cached_gl_error;
};

extern void client_thread_state_init(CLIENT_THREAD_STATE_T *state);
extern void client_thread_state_term(CLIENT_THREAD_STATE_T *state);

extern PLATFORM_TLS_T client_tls;

/*
   CLIENT_GET_THREAD_STATE

   Implementation notes:

   TODO: make sure this gets code-reviewed

   Preconditions:

   -

   Postconditions:

   Result is a valid pointer to a thread-local CLIENT_THREAD_STATE_T structure.

   Invariants preserved:

   -

   Invariants used:

   -
*/

static INLINE CLIENT_THREAD_STATE_T *CLIENT_GET_THREAD_STATE(void)
{
   return (CLIENT_THREAD_STATE_T *)platform_tls_get(client_tls);
}

/*
   per-process state

   - EGL initialization stage
   - EGL contexts and surfaces
   - EGL counters
*/

struct CLIENT_PROCESS_STATE {

   /*
   number of current contexts across all threads in this process. this is valid
   even if !inited
   */
   uint32_t context_current_count;

   /*
   inited

   Specifies whether the structure has been initialised and all of the other members are valid.
   inited is true between eglInitialise/eglTerminate. threads can still have
   things current when !inited

   Invariants:
   (CLIENT_PROCESS_STATE_INITED_SANITY)
   Only client_process_state_init/client_process_state_term modify this value
   */
   bool inited;

   /*
   contexts

   A map from context id (EGLContext) to EGL_CONTEXT_T
   TODO: use pointers as key rather than integers

   Validity: inited is true

   Invariants:
   (CLIENT_PROCESS_STATE_CONTEXTS)
   If id is a key in contexts:
      contexts[id].name == id
      contexts[id].is_destroyed == false
   */
   KHRN_POINTER_MAP_T contexts;

   /*
   surfaces

   A map from context id (EGLContext) to EGL_SURFACE_T

   Validity: inited is true

   Invariants:
   (CLIENT_PROCESS_STATE_SURFACES)
   If id is a key in surfaces:
      surfaces[id].name == id
      surfaces[id].is_destroyed == false
   */
   KHRN_POINTER_MAP_T surfaces;
#if EGL_KHR_sync

   /*
   syncs

   Validity: inited is true
   */
   KHRN_POINTER_MAP_T syncs;
#endif

   /*
      next_surface

      Implementation notes:
      TODO: these could theoretically overflow

      Validity: inited is true

      Invariant:
      (CLIENT_PROCESS_STATE_NEXT_SURFACE)
      next_surface is greater than every key in surfaces
      next_surface >= 1
   */
   uint32_t next_surface;
   /*
      next_context

      Validity: inited is true
   */
   uint32_t next_context;
#if EGL_KHR_sync
   /*
      next_sync

      Validity: inited is true
   */
   uint32_t next_sync;
#endif

#if EGL_BRCM_driver_monitor
   /*
      driver_monitor_inited

      Validity: inited is true
   */
   bool driver_monitor_inited;
#endif
};

extern bool client_process_state_init(CLIENT_PROCESS_STATE_T *process);
extern void client_process_state_term(CLIENT_PROCESS_STATE_T *process);

extern CLIENT_PROCESS_STATE_T client_process_state;

/*
   CLIENT_GET_PROCESS_STATE()

   Returns the process-global CLIENT_PROCESS_STATE_T object.
*/
static INLINE CLIENT_PROCESS_STATE_T *CLIENT_GET_PROCESS_STATE(void)
{
   return &client_process_state;
}

/*
   exposed bits of EGL
*/

CLIENT_PROCESS_STATE_T *client_egl_get_process_state(CLIENT_THREAD_STATE_T *thread, EGLDisplay dpy, EGLBoolean check_inited);
EGL_CONTEXT_T *client_egl_get_context(CLIENT_THREAD_STATE_T *thread, CLIENT_PROCESS_STATE_T *process, EGLContext ctx);
EGL_SURFACE_T *client_egl_get_surface(CLIENT_THREAD_STATE_T *thread, CLIENT_PROCESS_STATE_T *process, EGLSurface surf);
EGL_SURFACE_T *client_egl_get_locked_surface(CLIENT_THREAD_STATE_T *thread, CLIENT_PROCESS_STATE_T *process, EGLSurface surf);

/*
   client state
*/

#define CLIENT_MAKE_CURRENT_SIZE 36 /* RPC_CALL8 */
extern void client_send_make_current(CLIENT_THREAD_STATE_T *thread);

/*
   big giant lock
*/

extern PLATFORM_MUTEX_T client_mutex;

/*
   CLIENT_LOCK()

   Acquires EGL lock.

   Implementation notes:

   TODO make sure this gets reviewed

   Preconditions:

   TODO: check mutex hierarchy methodology
   Mutex: >(MUTEX_EGL_LOCK)
   Is being called from a function which _always_ subsequently calls CLIENT_UNLOCK()

   Postconditions:

   Mutex: (MUTEX_EGL_LOCK)
   Thread owns EGL lock
*/

static INLINE void CLIENT_LOCK(void)
{
      platform_client_lock();
}

/*
   CLIENT_UNLOCK()

   Releases EGL lock.

   Implementation notes:

   TODO make sure this gets reviewed

   Preconditions:

   Mutex: (MUTEX_EGL_LOCK)
   Thread owns EGL lock
   Is being called from a function which has _always_ previously called CLIENT_LOCK()

   Postconditions:
   Mutex: >(MUTEX_EGL_LOCK)
*/

static INLINE void CLIENT_UNLOCK(void)
{
    platform_client_release();
}

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

static INLINE bool CLIENT_LOCK_AND_GET_STATES(EGLDisplay dpy, CLIENT_THREAD_STATE_T **thread, CLIENT_PROCESS_STATE_T **process)
{
   *thread = CLIENT_GET_THREAD_STATE();
   CLIENT_LOCK();
   *process = client_egl_get_process_state(*thread, dpy, EGL_TRUE);
   if (*process != NULL)
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

#ifdef BCG_MULTI_THREADED
extern CLIENT_THREAD_STATE_T *client_lock_api(void);
extern void client_unlock_api(void);
#endif

#endif
