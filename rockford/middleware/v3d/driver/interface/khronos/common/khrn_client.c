/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of EGL displays.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#include "interface/khronos/common/khrn_client_check_types.h"
#include "interface/khronos/common/khrn_client.h"

#include "interface/khronos/common/khrn_int_misc_impl.h"

#if EGL_KHR_sync
#include "interface/khronos/ext/egl_khr_sync_client.h"
#endif

#if EGL_BRCM_driver_monitor
#include "interface/khronos/ext/egl_brcm_driver_monitor_client.h"
#endif

#ifdef KHRONOS_CLIENT_LOGGING
#include <stdio.h>
extern FILE *xxx_vclog;
#endif

#ifdef BCG_MULTI_THREADED
static VCOS_MUTEX_T api_mutex;
#endif

/*
   void client_process_state_init(CLIENT_PROCESS_STATE_T *state)

   Implementation notes:

   Does nothing if already initialised.

   Preconditions:

   process is a valid pointer
   Thread owns EGL lock
   client_process_state_term must be called at some point after calling this function if we return true.

   Postconditions:

   Either:
   - an error occurred and false is returned, or
   - process->inited is true

   Invariants preserved:

   All invariants on CLIENT_PROCESS_STATE_T
   (CLIENT_PROCESS_STATE_INITED_SANITY)

   Invariants used:

   -
*/

bool client_process_state_init(CLIENT_PROCESS_STATE_T *process)
{
   if (!process->inited) {

      if (!khrn_pointer_map_init(&process->contexts, 64))
         return false;

      if (!khrn_pointer_map_init(&process->surfaces, 64))
      {
         khrn_pointer_map_term(&process->contexts);
         return false;
      }
#if EGL_KHR_sync
      if (!khrn_pointer_map_init(&process->syncs, 64))
      {
         khrn_pointer_map_term(&process->contexts);
         khrn_pointer_map_term(&process->surfaces);
         return false;
      }
#endif
      process->next_surface = 1;
      process->next_context = 1;
#if EGL_KHR_sync
      process->next_sync = 0x80000001;
#endif

      process->inited = true;
   }

   return true;
}

#include "interface/khronos/common/khrn_client_cr.c"

/*
   void client_process_state_term(CLIENT_PROCESS_STATE_T *process)

   Implementation notes:

   Does nothing if already terminated.

   Preconditions:

   process is a valid pointer
   Thread owns EGL lock

   Postconditions:

   process->inited is false.

   Invariants preserved:

   (EGL_CONTEXT_IS_DESTROYED)
   (CLIENT_PROCESS_STATE_INITED_SANITY)

   Invariants used:

   -
*/

void client_process_state_term(CLIENT_PROCESS_STATE_T *process)
{
   if (process->inited) {
      khrn_pointer_map_iterate(&process->contexts, callback_destroy_context, NULL);
      khrn_pointer_map_term(&process->contexts);

      khrn_pointer_map_iterate(&process->surfaces, callback_destroy_surface, NULL);
      khrn_pointer_map_term(&process->surfaces);

#if EGL_KHR_sync
      khrn_pointer_map_iterate(&process->syncs, callback_destroy_sync, NULL);
      khrn_pointer_map_term(&process->syncs);
#endif

#if EGL_BRCM_driver_monitor
      egl_driver_monitor_term(process);
#endif

      process->inited = false;
   }
}

CLIENT_PROCESS_STATE_T client_process_state;

void client_thread_state_init(CLIENT_THREAD_STATE_T *state)
{
   memset(state, 0, sizeof(CLIENT_PROCESS_STATE_T));

   state->error = EGL_SUCCESS;

   state->bound_api = EGL_OPENGL_ES_API;

   state->opengl.context = 0;
   state->opengl.draw = 0;
   state->opengl.read = 0;

#ifndef NO_OPENVG
   state->openvg.context = 0;
   state->openvg.draw = 0;
   state->openvg.read = 0;
#endif /* NO_OPENVG */

   state->high_priority = false;

   state->merge_pos = 0;
   state->merge_end = 0;

   state->has_rendered_to_pixmap = false;
   state->is_current_pixmap = false;

   state->cached_gl_error = GL_NO_ERROR;
   state->cached_egl_error = EGL_SUCCESS;
}

extern void egl_current_release_surfaces(CLIENT_PROCESS_STATE_T *process, EGL_CURRENT_T *current);

void client_thread_state_term(CLIENT_THREAD_STATE_T *thread)
{
   CLIENT_PROCESS_STATE_T *process = CLIENT_GET_PROCESS_STATE();

   if (process) {
      egl_current_release_surfaces(process, &thread->opengl);
#ifndef NO_OPENVG
      egl_current_release_surfaces(process, &thread->openvg);
#endif /* NO_OPENVG */
   }

   // TODO: termination
   platform_term_rpc( thread );
}

EGL_CONTEXT_T *client_egl_get_context(CLIENT_THREAD_STATE_T *thread, CLIENT_PROCESS_STATE_T *process, EGLContext ctx)
{
   EGL_CONTEXT_T *context = (EGL_CONTEXT_T *)khrn_pointer_map_lookup(&process->contexts, (uint32_t)(size_t)ctx);

   vcos_assert(!context || !context->is_destroyed);

   if (!context)
      thread->error = EGL_BAD_CONTEXT;

   return context;
}

EGL_SURFACE_T *client_egl_get_surface(CLIENT_THREAD_STATE_T *thread, CLIENT_PROCESS_STATE_T *process, EGLSurface surf)
{
   EGL_SURFACE_T *surface = (EGL_SURFACE_T *)khrn_pointer_map_lookup(&process->surfaces, (uint32_t)(size_t)surf);

   vcos_assert (!surface || !surface->is_destroyed);

   if (!surface)
      thread->error = EGL_BAD_SURFACE;

#if EGL_KHR_lock_surface
   if (surface && surface->is_locked) {
      thread->error = EGL_BAD_ACCESS;
      surface = NULL;
   }
#endif

   return surface;
}

/*
   We don't actually insist that the surface is locked. But unlike client_egl_get_surface, we don't throw an
   error if it isn't.
*/
EGL_SURFACE_T *client_egl_get_locked_surface(CLIENT_THREAD_STATE_T *thread, CLIENT_PROCESS_STATE_T *process, EGLSurface surf)
{
   EGL_SURFACE_T *surface = (EGL_SURFACE_T *)khrn_pointer_map_lookup(&process->surfaces, (uint32_t)(size_t)surf);

   vcos_assert (!surface || !surface->is_destroyed);

   if (!surface)
      thread->error = EGL_BAD_SURFACE;

   return surface;
}

static uint32_t convert_gltype(EGL_CONTEXT_TYPE_T type)
{
   switch (type) {
   case OPENGL_ES_11: return EGL_SERVER_GL11;
   case OPENGL_ES_20: return EGL_SERVER_GL20;
   default:           UNREACHABLE(); return 0;
   }
}

void client_send_make_current(CLIENT_THREAD_STATE_T *thread)
{
   uint64_t pid                  = khronos_platform_get_process_id();
   uint32_t gltype               = thread->opengl.context ? convert_gltype(thread->opengl.context->type) : 0;
   EGL_GL_CONTEXT_ID_T servergl  = thread->opengl.context ? thread->opengl.context->servercontext : EGL_SERVER_NO_GL_CONTEXT;
   EGL_SURFACE_ID_T servergldraw = thread->opengl.draw    ? thread->opengl.draw->serverbuffer     : EGL_SERVER_NO_SURFACE;
   EGL_SURFACE_ID_T serverglread = thread->opengl.read    ? thread->opengl.read->serverbuffer     : EGL_SERVER_NO_SURFACE;
#ifndef NO_OPENVG
   EGL_VG_CONTEXT_ID_T servervg  = thread->openvg.context ? thread->openvg.context->servercontext : EGL_SERVER_NO_VG_CONTEXT;
   EGL_SURFACE_ID_T servervgsurf = thread->openvg.draw    ? thread->openvg.draw->serverbuffer     : EGL_SERVER_NO_SURFACE;
#endif /* NO_OPENVG */

   /*
      if the size of this call in the merge buffer changes,
      CLIENT_MAKE_CURRENT_SIZE in khrn_client.h should be updated
   */


   if (!thread->opengl.context || !thread->opengl.draw)
   {
      KHRONOS_CLIENT_LOG("Send null make current %x %x\n",
                 (unsigned int)(char *)thread->opengl.context, (unsigned int)(char *)thread->opengl.draw);
   }
   else
   {
      KHRONOS_CLIENT_LOG("Send make current %d[%d %s%s] %d[%d %d%s]\n",
            (int)thread->opengl.context->name,
            thread->opengl.context->servercontext,
            thread->opengl.context->is_current ? " C" : "",
            thread->opengl.context->is_destroyed ? " D" : "",
            (int)thread->opengl.draw->name,
            thread->opengl.draw->serverbuffer,
            thread->opengl.draw->context_binding_count,
            thread->opengl.draw->is_destroyed ? " D" : "");
   }

   eglIntMakeCurrent_impl((uint32_t)pid,
                          (uint32_t)(pid >> 32),
                          gltype,
                          servergl,
                          servergldraw,
                          serverglread
#ifndef NO_OPENVG
                          ,
                          servervg,
                          servervgsurf
#endif /* NO_OPENVG */
                          );
}

PLATFORM_TLS_T client_tls;
PLATFORM_MUTEX_T client_mutex;

bool client_process_attach(void)
{
   KHR_STATUS_T            status;
   CLIENT_PROCESS_STATE_T *process;

   status = platform_tls_create(&client_tls, NULL);
   if (status != KHR_SUCCESS) {
      return false;
   }

#ifdef BCG_MULTI_THREADED
   /* Create the API locking mutex */
   if (vcos_mutex_create(&api_mutex, "threaded_api_mutex") != VCOS_SUCCESS) {
      platform_tls_destroy(client_tls);
      return false;
   }
#endif

   status = platform_mutex_create(&client_mutex);

   if (status != KHR_SUCCESS) {
#ifdef BCG_MULTI_THREADED
      vcos_mutex_delete(&api_mutex);
#endif
      platform_tls_destroy(client_tls);
      return false;
   }

   process = CLIENT_GET_PROCESS_STATE();
   if (process)
   {
      if (!client_process_state_init(process))
         assert(0);
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
   CLIENT_THREAD_STATE_T *state = (CLIENT_THREAD_STATE_T *)khrn_platform_malloc(sizeof(CLIENT_THREAD_STATE_T), "CLIENT_THREAD_STATE_T");

   if (!state)
      return false;

   client_thread_state_init(state);

   ANDROID_LOGD("%s %d : TLS %p STATE %p\n", __FUNCTION__, __LINE__, (void *)tls, state);

   platform_tls_set(tls, state);

   client_send_make_current(state);

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
   khrn_platform_free(state);
   platform_maybe_free_process();
}

void client_process_detach(void)
{
   CLIENT_PROCESS_STATE_T *process = CLIENT_GET_PROCESS_STATE();

   client_process_state_term(process);

   client_thread_detach(NULL);
   platform_tls_destroy(client_tls);
   platform_mutex_destroy(&client_mutex);
#ifdef BCG_MULTI_THREADED
   vcos_mutex_delete(&api_mutex);
#endif
}
#ifdef BCG_MULTI_THREADED

/* #define TRACK_LINUX_SYSTEM_MEM */

#ifdef TRACK_LINUX_SYSTEM_MEM
static int threshold = 100;
#endif

CLIENT_THREAD_STATE_T *client_lock_api(void)
{
   CLIENT_PROCESS_STATE_T *process = CLIENT_GET_PROCESS_STATE();
   CLIENT_THREAD_STATE_T  *thread  = NULL;

   // Lock the API call mutex to prevent another thread from preempting while we process this call
   vcos_mutex_lock(&api_mutex);

#ifdef TRACK_LINUX_SYSTEM_MEM
   if (threshold-- == 0)
   {
      char info[4096];
      int tm, fm, ma;
      FILE *fp;
      fp = fopen("/proc/meminfo", "r");
      fread(info, sizeof(info), 1, fp);
      sscanf(info, "MemTotal: %d kB\nMemFree: %d kB\nMemAvailable: %d kB\n", &tm, &fm, &ma);
      fclose(fp);
      printf("TotalMemory %d kB, FreeMemory %d kB, Available %d kB\n", tm, fm, ma);
      threshold = 100;
   }
#endif

   // We can only use the thread state once the process has been initialized
   if (process->inited)
   {
      thread = CLIENT_GET_THREAD_STATE();

      // Ensure the correct thread context is current for the API call
      if (thread != NULL && thread->bound_api != 0)
         client_send_make_current(thread);
   }

   return thread;
}

void client_unlock_api(void)
{
   // Unlock the API mutex so other threads can have a go
   vcos_mutex_unlock(&api_mutex);
}

void client_check_lock(void)
{
   assert(vcos_mutex_is_locked(&api_mutex));
}

#else
void client_lock_api(void) {}
void client_unlock_api(void) {}
#endif
