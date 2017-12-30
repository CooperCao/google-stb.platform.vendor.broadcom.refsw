/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/egl/egl_client_config.h"
#include "interface/khronos/ext/egl_brcm_driver_monitor_client.h"
#include "interface/khronos/egl/egl_int_impl.h"

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"

#if EGL_BRCM_driver_monitor

EGLAPI EGLBoolean EGLAPIENTRY eglInitDriverMonitorBRCM(EGLDisplay dpy, EGLint hw_bank, EGLint l3c_bank)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   CLIENT_LOCK();

   {
      CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (process)
      {
         if (!process->driver_monitor_inited)
            process->driver_monitor_inited = eglInitDriverMonitorBRCM_impl(hw_bank, l3c_bank);

         if (process->driver_monitor_inited)
         {
            thread->error = EGL_SUCCESS;
            result = EGL_TRUE;
         }
         else
         {
            thread->error = EGL_BAD_ALLOC;
            result = EGL_FALSE;
         }
      }
      else
         result = EGL_FALSE;
   }

   CLIENT_UNLOCK();

   return result;
}

void egl_driver_monitor_term(CLIENT_PROCESS_STATE_T *process)
{
   if (process->driver_monitor_inited)
   {
      eglTermDriverMonitorBRCM_impl();

      process->driver_monitor_inited = false;
   }
}

EGLAPI void EGLAPIENTRY eglGetDriverMonitorXMLBRCM(EGLDisplay dpy, EGLint bufSize, EGLint *length, char *xmlStats)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   CLIENT_LOCK();

   {
      CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (process)
      {
         if (process->driver_monitor_inited && xmlStats != NULL)
         {
            eglGetDriverMonitorXMLBRCM_impl(bufSize, xmlStats);

            if (length != NULL)
               *length = strlen(xmlStats);
         }
      }
   }

   CLIENT_UNLOCK();
}

EGLAPI EGLBoolean EGLAPIENTRY eglTermDriverMonitorBRCM(EGLDisplay dpy)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   CLIENT_LOCK();

   {
      CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (process)
      {
         egl_driver_monitor_term(process);

         thread->error = EGL_SUCCESS;
         result = EGL_TRUE;
      }
      else
         result = EGL_FALSE;
   }

   CLIENT_UNLOCK();

   return result;
}

#endif
