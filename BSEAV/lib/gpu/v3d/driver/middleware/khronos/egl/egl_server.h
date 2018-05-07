/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/egl/egl_int.h"
#include "middleware/khronos/common/khrn_map.h"
#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/common/khrn_hw.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "interface/khronos/common/khrn_client_pointermap.h"

#include "vcfw/rtos/abstract/rtos_abstract_mem.h"
#include <stdint.h>

// There is a single global instance of this
typedef struct
{
   khrn_map contexts;
   khrn_map surfaces;
   khrn_map eglimages;
   khrn_map syncs;

   uint32_t next_surface;
   uint32_t next_eglimage;
   uint32_t next_context;
   uint32_t next_sync;

   /*
   * EGL display this state belongs to
   */
   EGLDisplay display;

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
   platform_inited

   It specifies whether BEGL display platform has been initialised.
   Initialisation happens in eglInitialise() but termination may be deferred
   past the eglTerminate() if any thread still holds a current context.
   Platform termination can happen in eglTerminate(), eglMakeCurrent(),
   eglReleaseThread() or in the thread-local storage destructor.
   */
   bool platform_inited;

} EGL_SERVER_STATE_T;

typedef struct
{
   int32_t type;
   int32_t condition;
   int32_t status;

   VCOS_SEMAPHORE_T sem;
} EGL_SERVER_SYNC_T;

extern EGL_SERVER_STATE_T egl_server_state;

/*
   EGL_SERVER_STATE_T *EGL_GET_SERVER_STATE()

   Returns pointer to EGL server state.

   Implementation notes:

   There is only one of these globally, and it does not need locking and unlocking.

   Preconditions:

   Valid EGL server state exists

   Postconditions:

   Return value is a valid pointer
*/

static inline EGL_SERVER_STATE_T *EGL_GET_SERVER_STATE(void)
{
   return &egl_server_state;
}

extern bool server_process_state_init(void);
extern void server_process_state_term(void);

#include "interface/khronos/egl/egl_int_impl.h"
