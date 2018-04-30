/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_client.h"

#if EGL_KHR_sync
#include "interface/khronos/ext/egl_khr_sync_client.h"
#endif

#include "middleware/khronos/egl/egl_server.h"

#include "interface/vcos/vcos.h"

void client_thread_state_init(CLIENT_THREAD_STATE_T *state)
{
   memset(state, 0, sizeof(CLIENT_THREAD_STATE_T));

   state->error = EGL_SUCCESS;

   state->bound_api = EGL_OPENGL_ES_API;

   state->opengl.context = NULL;
   state->opengl.draw = NULL;
   state->opengl.read = NULL;

   state->events_acquired = false;
   state->perf_counters_acquired = false;
}

extern void egl_current_release_surfaces(EGL_SERVER_STATE_T *state, CLIENT_THREAD_STATE_T *thread);

void client_thread_state_term(CLIENT_THREAD_STATE_T *thread)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   if (state)
      egl_current_release_surfaces(state, thread);
}

EGL_CONTEXT_T *egl_get_context(CLIENT_THREAD_STATE_T *thread, EGL_SERVER_STATE_T *state, EGLContext ctx)
{
   EGL_CONTEXT_T *context = khrn_map_lookup(&state->contexts, (uint32_t)(uintptr_t)ctx);

   if (context == NULL)
      thread->error = EGL_BAD_CONTEXT;

   return context;
}

EGL_SURFACE_T *egl_get_surface(CLIENT_THREAD_STATE_T *thread, EGL_SERVER_STATE_T *state, EGLSurface surf)
{
   EGL_SURFACE_T *surface = khrn_map_lookup(&state->surfaces, (uint32_t)(uintptr_t)surf);

   if (surface == NULL)
      thread->error = EGL_BAD_SURFACE;

   return surface;
}

PLATFORM_TLS_T client_tls;

bool client_process_attach(void)
{
   KHR_STATUS_T status = platform_tls_create(&client_tls, client_thread_detach);
   if (status != KHR_SUCCESS) {
      return false;
   }

#ifndef NDEBUG
   printf("**********************************************************\n"
          "*****          D E B U G   B U I L D\n"
          "*****\n"
          "***** You are running a debug build of the V3D driver.\n"
          "***** This will impact OpenGLES performance significantly.\n"
          "***** You must build in release mode for correct\n"
          "***** performance of the V3D driver.\n"
          "**********************************************************\n");
#endif

   return true;
}

bool client_thread_attach(PLATFORM_TLS_T tls)
{
   CLIENT_THREAD_STATE_T *state = (CLIENT_THREAD_STATE_T *)malloc(sizeof(CLIENT_THREAD_STATE_T));

   if (!state)
      return false;

   client_thread_state_init(state);

   ANDROID_LOGD("%s %d : TLS %p STATE %p\n", __FUNCTION__, __LINE__, (void *)tls, state);

   platform_tls_set(tls, state);

   return true;
}

void client_thread_detach(void * tls)
{
   CLIENT_THREAD_STATE_T *state;

   if (tls == NULL)
      state = CLIENT_GET_THREAD_STATE();
   else
      state = (CLIENT_THREAD_STATE_T *)tls;

   platform_tls_remove(client_tls);
   client_thread_state_term(state);
   free(state);
}

void client_process_detach(void)
{
   client_thread_detach(NULL);
   server_process_state_term();
   platform_tls_destroy(client_tls);
}
