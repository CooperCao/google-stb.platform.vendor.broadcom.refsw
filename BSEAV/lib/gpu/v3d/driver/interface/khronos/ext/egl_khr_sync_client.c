/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/ext/egl_khr_sync_client.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "middleware/khronos/glxx/glxx_hw.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/common/khrn_mem.h"

static void egl_server_sync_term(void *p)
{
   EGL_SYNC_T *sync = p;
   vcos_semaphore_delete(&sync->sem);
}

static EGL_SYNC_T *egl_sync_create(EGLenum type, EGLint condition, EGLint status)
{
   EGL_SERVER_STATE_T *state = EGL_GET_SERVER_STATE();

   EGL_SYNC_T *sync = KHRN_MEM_ALLOC_STRUCT(EGL_SYNC_T);                   // check
   if (sync == NULL)
      return NULL;

   if (vcos_semaphore_create(&sync->sem, "egl sync object", 0) != VCOS_SUCCESS) {
      KHRN_MEM_ASSIGN(sync, NULL);
      return NULL;
   }

   khrn_mem_set_term(sync, egl_server_sync_term);

   sync->type = type;
   sync->condition = condition;
   sync->status = status;

   if (type == EGL_SYNC_FENCE_KHR) {
      GLXX_SERVER_STATE_T *glstate = glxx_lock_server_state(OPENGL_ES_ANY);
      bool inserted = false;
      if (glstate) {
         inserted = glxx_hw_insert_sync(glstate, sync);
         glxx_unlock_server_state(OPENGL_ES_ANY);
      }
      if (!inserted)
         goto end;
   }

   return sync;

end:
   KHRN_MEM_ASSIGN(sync, NULL);
   return NULL;
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

static bool egl_sync_get_attrib(EGL_SYNC_T *sync, EGLint attrib, EGLint *value)
{
   bool result = true;
   switch (attrib) {
   case EGL_SYNC_TYPE_KHR:
      *value = sync->type;
      break;
   case EGL_SYNC_STATUS_KHR:
      *value = sync->status;
      break;
   case EGL_SYNC_CONDITION_KHR:
      *value = sync->condition;
      break;
   default:
      result = false;
      break;
   }
   return result;
}

EGLAPI EGLSyncKHR EGLAPIENTRY eglCreateSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLSyncKHR result = EGL_NO_SYNC_KHR;
   EGL_CURRENT_T *current;

   if (!egl_ensure_init_once())
      return EGL_NO_SYNC_KHR;

   current = &thread->opengl;

   if (current->context == NULL) {
      thread->error = EGL_BAD_MATCH;
      return EGL_NO_SYNC_KHR;
   }

   CLIENT_LOCK();

   EGL_SERVER_STATE_T *process = egl_get_process_state(thread, dpy, EGL_TRUE);

   if (!process) goto end;

   EGLint condition;
   EGLint status;
   if (egl_sync_check_attribs(attrib_list, type, &condition, &status)) {
      EGL_SYNC_T *sync = egl_sync_create(type, condition, status);

      if (sync != NULL) {
         if (khrn_map_insert(&process->syncs, process->next_sync, sync)) {
            thread->error = EGL_SUCCESS;
            result = (EGLSurface)(size_t)process->next_sync++;
         }
         else {
            thread->error = EGL_BAD_ALLOC;
            KHRN_MEM_ASSIGN(sync, NULL);
         }
      }
      else
         thread->error = EGL_BAD_ALLOC;
   }
   else
      thread->error = EGL_BAD_ATTRIBUTE;

end:
   CLIENT_UNLOCK();

   return result;
}

// TODO: should we make sure any syncs have come back before destroying the object?

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySyncKHR(EGLDisplay dpy, EGLSyncKHR _sync)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result = EGL_FALSE;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   EGL_SERVER_STATE_T *process = egl_get_process_state(thread, dpy, EGL_TRUE);
   if (!process) goto end;

   EGL_SYNC_T *sync = khrn_map_lookup(&process->syncs, (uint32_t)(uintptr_t)_sync);

   if (sync != NULL) {
      thread->error = EGL_SUCCESS;

      khrn_map_delete(&process->syncs, (uint32_t)(uintptr_t)_sync);

      KHRN_MEM_ASSIGN(sync, NULL);
   } else
      thread->error = EGL_BAD_PARAMETER;

   result = (thread->error == EGL_SUCCESS ? EGL_TRUE : EGL_FALSE);
end:

   CLIENT_UNLOCK();
   return result;
}

static int sync_wait_timeout(EGL_SYNC_T *sync, uint32_t timeout)
{
   VCOS_SEMAPHORE_T *sem = &sync->sem;

   int res = VCOS_EINVAL;
   if (sem) {
      if (timeout == 0xFFFFFFFF)
         res = vcos_semaphore_wait(sem);
      else
         res = vcos_semaphore_wait_timeout(sem, timeout);
   }

   return res;
}

EGLAPI EGLint EGLAPIENTRY eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR _sync, EGLint flags, EGLTimeKHR timeout)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLint result = 0;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   EGL_SERVER_STATE_T *process = egl_get_process_state(thread, dpy, EGL_TRUE);

   if (!process) goto end;

   EGL_SYNC_T *sync = khrn_map_lookup(&process->syncs, (uint32_t)(uintptr_t)_sync);

   if (sync == NULL) {
      thread->error = EGL_BAD_PARAMETER;
      goto end;
   }

   if (flags & ~EGL_SYNC_FLUSH_COMMANDS_BIT_KHR) {
      thread->error = EGL_BAD_PARAMETER;
      goto end;
   }

   EGLint value;
   egl_sync_get_attrib(sync, EGL_SYNC_STATUS_KHR, &value);

   /* Copied from VC5 - some apps don't set this bit and wait indefinitely */
   /* if (flags & EGL_SYNC_FLUSH_COMMANDS_BIT_KHR) */
   {
      /* needs to force a flush to the pipeline */
      eglIntFlush_impl(thread->bound_api == EGL_OPENGL_ES_API);
   }

   if (value == EGL_UNSIGNALED_KHR) {
      if (timeout != EGL_FOREVER_KHR) {
         EGLTimeKHR ms_timeout = timeout / 1000000;
         /* clamp the timeout to something reasonable - max 2147483 seconds */
         if (ms_timeout >> 32)
            ms_timeout = 0x7FFFFFFF;

         if (sync_wait_timeout(sync, (int)ms_timeout) == KHR_SUCCESS)
            result = EGL_CONDITION_SATISFIED_KHR;
         else
            result = EGL_TIMEOUT_EXPIRED_KHR;
      }
      else {
         /* wait forever */
         sync_wait_timeout(sync, 0xFFFFFFFF);
         result = EGL_CONDITION_SATISFIED_KHR;
      }
   }
   else
      result = EGL_CONDITION_SATISFIED_KHR;

end:
   CLIENT_UNLOCK();
   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetSyncAttribKHR(EGLDisplay dpy, EGLSyncKHR _sync, EGLint attribute, EGLint *value)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result = EGL_FALSE;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   EGL_SERVER_STATE_T *process = egl_get_process_state(thread, dpy, EGL_TRUE);

   if (!process) goto end;

   if (!value) {
      thread->error = EGL_BAD_PARAMETER;
      goto end;
   }

   EGL_SYNC_T *sync = khrn_map_lookup(&process->syncs, (uint32_t)(uintptr_t)_sync);

   if (sync != NULL) {
      if (egl_sync_get_attrib(sync, attribute, value)) {
         thread->error = EGL_SUCCESS;
         result = EGL_TRUE;
      } else
         thread->error = EGL_BAD_ATTRIBUTE;
   } else
      thread->error = EGL_BAD_PARAMETER;

end:
   CLIENT_UNLOCK();
   return result;
}
