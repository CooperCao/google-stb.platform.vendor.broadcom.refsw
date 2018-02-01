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

#include "middleware/khronos/egl/egl_server.h"

#if EGL_BRCM_driver_monitor

EGLAPI EGLBoolean EGLAPIENTRY eglInitDriverMonitorBRCM(EGLDisplay dpy, EGLint hw_bank, EGLint l3c_bank)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   {
      EGL_SERVER_STATE_T *state = egl_get_process_state(thread, dpy, EGL_TRUE);

      if (state)
      {
         if (!state->driver_monitor_inited_)
            state->driver_monitor_inited_ = eglInitDriverMonitorBRCM_impl(hw_bank, l3c_bank);

         if (state->driver_monitor_inited_)
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

void egl_driver_monitor_term(EGL_SERVER_STATE_T *state)
{
   if (state->driver_monitor_inited_)
   {
      eglTermDriverMonitorBRCM_impl();

      state->driver_monitor_inited_ = false;
   }
}

EGLAPI void EGLAPIENTRY eglGetDriverMonitorXMLBRCM(EGLDisplay dpy, EGLint bufSize, EGLint *length, char *xmlStats)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

   if (!egl_ensure_init_once())
      return;

   CLIENT_LOCK();

   {
      EGL_SERVER_STATE_T *state = egl_get_process_state(thread, dpy, EGL_TRUE);

      if (state)
      {
         if (state->driver_monitor_inited_ && xmlStats != NULL)
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

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();

   {
      EGL_SERVER_STATE_T *state = egl_get_process_state(thread, dpy, EGL_TRUE);

      if (state)
      {
         egl_driver_monitor_term(state);

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