/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Client-side implementation of the EGL_BRCM_image_update_control extension.
=============================================================================*/

#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_client_mangle.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/egl/egl_int_impl.h"

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/include/EGL/eglext_brcm.h"
#include "interface/khronos/include/GLES/gl.h"

#ifdef KHRONOS_CLIENT_LOGGING
#include <stdio.h>
extern FILE *xxx_vclog;
#endif

static EGLBoolean ProcessCall(EGLDisplay dpy, EGLImageKHR image, EGLenum pname, const EGLint *params)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean res = EGL_FALSE;

   CLIENT_LOCK();

   {
      CLIENT_PROCESS_STATE_T *process = client_egl_get_process_state(thread, dpy, EGL_TRUE);

      if (process)
      {
         EGLint buf[4];
         buf[0] = buf[1] = buf[2] = buf[3] = 0;

         switch (pname)
         {
         case EGL_IMAGE_UPDATE_CONTROL_SET_MODE_BRCM :
            if (params[0] != EGL_IMAGE_UPDATE_CONTROL_ALWAYS_BRCM && params[0] != EGL_IMAGE_UPDATE_CONTROL_EXPLICIT_BRCM)
            {
               thread->error = EGL_BAD_PARAMETER;
               goto error;
            }
            buf[0] = params[0];
            break;
         case EGL_IMAGE_UPDATE_CONTROL_CHANGED_REGION_BRCM :
            if (params[0] < 0 || params[1] < 0 || params[2] < 0 || params[3] < 0)
            {
               thread->error = EGL_BAD_PARAMETER;
               goto error;
            }
            memcpy(buf, params, 4 * sizeof(EGLint));
            break;
         case EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM :
            if (params[0] != EGL_IMAGE_UPDATE_CONTROL_LOCK_BRCM && params[0] != EGL_IMAGE_UPDATE_CONTROL_UNLOCK_BRCM)
            {
               thread->error = EGL_BAD_PARAMETER;
               goto error;
            }
            buf[0] = params[0];
            if (params[0] == EGL_IMAGE_UPDATE_CONTROL_LOCK_BRCM)
            {
               // Lock is a special case as it needs a 4 stage calling process.
               // Phase 1 & 3 are called with an API lock in place
               // Phase 2 & 4 are called without an API lock

               bool phase2ret;
               /* Retained in this function, and released in lockPhase4 */
               MEM_HANDLE_T heglimage = eglImageUpdateParameterBRCM_lockPhase1(image);

               client_unlock_api();

               phase2ret = eglImageUpdateParameterBRCM_lockPhase2(heglimage);

               client_lock_api();

               eglImageUpdateParameterBRCM_lockPhase3(heglimage, phase2ret);

               client_unlock_api();

               eglImageUpdateParameterBRCM_lockPhase4(heglimage, phase2ret);

               // We'll be unlocked on exit, so lock again
               client_lock_api();
               res = EGL_TRUE;
               thread->error = EGL_SUCCESS;

               goto done;
            }
            break;
         default :
            thread->error = EGL_BAD_PARAMETER;
            goto error;
            break;
         }

         res = eglImageUpdateParameterBRCM_impl(image, pname, buf);

         if (res == EGL_FALSE)
            thread->error = EGL_BAD_PARAMETER;
         else
            thread->error = EGL_SUCCESS;
      }
   }

error:
done:
   CLIENT_UNLOCK();

   return res;
}

EGLAPI EGLBoolean EGLAPIENTRY eglImageUpdateParameterivBRCM(EGLDisplay dpy, EGLImageKHR image, EGLenum pname, const EGLint *params)
{
   KHRONOS_CLIENT_LOG("eglImageUpdateParameterivBRCM image=%d.\n", (int)image);
   return ProcessCall(dpy, image, pname, params);
}

EGLAPI EGLBoolean EGLAPIENTRY eglImageUpdateParameteriBRCM(EGLDisplay dpy, EGLImageKHR image, EGLenum pname, EGLint param)
{
   KHRONOS_CLIENT_LOG("eglImageUpdateParameteriBRCM image=%d.\n", (int)image);
   return ProcessCall(dpy, image, pname, &param);
}
