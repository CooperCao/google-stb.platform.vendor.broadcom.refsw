/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Client-side function for notifying an OpenMAX IL component that an
EGLImage is available for rendering into.

=============================================================================*/

#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/ext/egl_openmaxil_client.h"

#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/common/khrn_client_rpc.h"
#include <assert.h>

int eglIntOpenMAXILDoneMarker (void* component_handle, EGLImageKHR egl_image)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   int res;

   res = RPC_INT_RES(RPC_CALL2_RES(eglIntOpenMAXILDoneMarker_impl,
        thread, EGLINTOPENMAXILDONEMARKER_ID, RPC_EGLID(component_handle), RPC_EGLID(egl_image)));

   return res;
}
