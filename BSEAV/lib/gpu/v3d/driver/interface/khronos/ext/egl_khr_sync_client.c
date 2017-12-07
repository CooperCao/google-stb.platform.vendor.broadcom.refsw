/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_client_mangle.h"

#include "interface/khronos/ext/egl_khr_sync_client.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/common/khrn_int_misc_impl.h"

static EGL_SYNC_T *egl_sync_create(EGLenum type, EGLint condition, EGLint status)
{
   EGL_SYNC_T *sync = (EGL_SYNC_T *)malloc(sizeof(EGL_SYNC_T));

   if (!sync)
      return 0;

   sync->type = type;

   sync->serversync = eglIntCreateSync_impl(type, condition, status);
   if (!sync->serversync) {
      free(sync);
      return 0;
   }

   return sync;
}

void egl_sync_term(EGL_SYNC_T *sync)
{
   eglIntDestroySync_impl(sync->serversync);
}

static EGLBoolean egl_sync_check_attribs(const EGLint *attrib_list, EGLenum type, EGLint *condition, EGLint *status)
{
   EGLBoolean res = EGL_TRUE;
   *condition = EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR;
   *status = EGL_UNSIGNALED_KHR;

   if (type == EGL_SYNC_FENCE_KHR) {
      if (attrib_list) {
         while (1) {
            int name = *attrib_list++;
            if (name == EGL_NONE)
               break;
            else {
               /* int value = * */attrib_list++; /* at present no name/value pairs are handled */

               /*
                  EGL_SYNC_TYPE_KHR      EGL_SYNC_FENCE_KHR
                  EGL_SYNC_STATUS_KHR    EGL_UNSIGNALED_KHR
                  EGL_SYNC_CONDITION_KHR EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR
               */

               switch (name) {
               default:
                  return EGL_FALSE;
               }
            }
         }
      }
   }
   else
      res = EGL_FALSE;

   return res;
}

static EGLBoolean egl_sync_get_attrib(EGL_SYNC_T *sync, EGLint attrib, EGLint *value)
{
   switch (attrib) {
   case EGL_SYNC_TYPE_KHR:
      *value = sync->type;
      return EGL_TRUE;
   case EGL_SYNC_STATUS_KHR:
   case EGL_SYNC_CONDITION_KHR:
      eglSyncGetAttrib_impl(sync->serversync, attrib, value);
      return EGL_TRUE;
   default:
      return EGL_FALSE;
   }
}

EGLAPI EGLSyncKHR EGLAPIENTRY eglCreateSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLSyncKHR result = EGL_NO_SYNC_KHR;
   EGL_CURRENT_T *current;

   current = &thread->opengl;

   if (!current->context)
      thread->error = EGL_BAD_MATCH;
   else {
      CLIENT_LOCK();

      {
         CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

         EGLint condition;
         EGLint status;

         if (process)
         {
            if (egl_sync_check_attribs(attrib_list, type, &condition, &status)) {
               EGL_SYNC_T *sync = egl_sync_create(type, condition, status);

               if (sync) {
                  if (khrn_pointer_map_insert(&process->syncs, process->next_sync, sync)) {
                     thread->error = EGL_SUCCESS;
                     result = (EGLSurface)(size_t)process->next_sync++;
                  }
                  else {
                     thread->error = EGL_BAD_ALLOC;
                     egl_sync_term(sync);
                     free(sync);
                  }
               }
               else
                  thread->error = EGL_BAD_ALLOC;
            }
            else
               thread->error = EGL_BAD_ATTRIBUTE;
         }
      }

      CLIENT_UNLOCK();
   }

   return result;
}

// TODO: should we make sure any syncs have come back before destroying the object?

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySyncKHR(EGLDisplay dpy, EGLSyncKHR _sync)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   CLIENT_LOCK();

   {
      CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (process) {
         EGL_SYNC_T *sync = (EGL_SYNC_T *)khrn_pointer_map_lookup(&process->syncs, (uint32_t)(size_t)_sync);

         if (sync) {
            thread->error = EGL_SUCCESS;

            khrn_pointer_map_delete(&process->syncs, (uint32_t)(uintptr_t)_sync);

            egl_sync_term(sync);
            free(sync);
         } else
            thread->error = EGL_BAD_PARAMETER;

         result = (thread->error == EGL_SUCCESS ? EGL_TRUE : EGL_FALSE);
      } else
         result = EGL_FALSE;
   }

   CLIENT_UNLOCK();

   return result;
}

EGLAPI EGLint EGLAPIENTRY eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR _sync, EGLint flags, EGLTimeKHR timeout)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   CLIENT_LOCK();

   {
      CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (process) {
         EGL_SYNC_T *sync = (EGL_SYNC_T *)khrn_pointer_map_lookup(&process->syncs, (uint32_t)(size_t)_sync);

         if (sync && !(flags & ~EGL_SYNC_FLUSH_COMMANDS_BIT_KHR)) {
            EGLint res;
            EGLint value;
            egl_sync_get_attrib(sync, EGL_SYNC_STATUS_KHR, &value);

            if (flags & EGL_SYNC_FLUSH_COMMANDS_BIT_KHR) {
               /* needs to force a flush to the pipeline */
               khrn_misc_rpc_flush_impl();

               (void)eglIntFlush_impl(thread->bound_api == EGL_OPENGL_ES_API,
                                       thread->bound_api == EGL_OPENVG_API);
            }

            if (value == EGL_UNSIGNALED_KHR) {
               if (timeout != EGL_FOREVER_KHR) {
                  EGLTimeKHR ms_timeout = timeout / 1000000;
                  /* clamp the timeout to something reasonable - max 2147483 seconds */
                  if (ms_timeout >> 32)
                     ms_timeout = 0x7FFFFFFF;
                  if (eglIntSyncWaitTimeout_impl(sync->serversync, (int)ms_timeout) == KHR_SUCCESS)
                     res = EGL_CONDITION_SATISFIED_KHR;
                  else
                     res = EGL_TIMEOUT_EXPIRED_KHR;
               } else {
                  /* wait forever */
                  eglIntSyncWaitTimeout_impl(sync->serversync, 0xFFFFFFFF);
                  res = EGL_CONDITION_SATISFIED_KHR;
               }
            }
            else
               res = EGL_CONDITION_SATISFIED_KHR;

            CLIENT_UNLOCK();

            return res;
         } else
            thread->error = EGL_BAD_PARAMETER;
      }
   }

   CLIENT_UNLOCK();

   return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetSyncAttribKHR(EGLDisplay dpy, EGLSyncKHR _sync, EGLint attribute, EGLint *value)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result = EGL_FALSE;

   CLIENT_LOCK();

   {
      CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (process) {
         if (value) {
            EGL_SYNC_T *sync = (EGL_SYNC_T *)khrn_pointer_map_lookup(&process->syncs, (uint32_t)(size_t)_sync);

            if (sync) {
               if (egl_sync_get_attrib(sync, attribute, value)) {
                  thread->error = EGL_SUCCESS;
                  result = EGL_TRUE;
               } else
                  thread->error = EGL_BAD_ATTRIBUTE;
            } else
               thread->error = EGL_BAD_PARAMETER;
         }
         else
            thread->error = EGL_BAD_PARAMETER;
      }
   }

   CLIENT_UNLOCK();

   return result;
}
