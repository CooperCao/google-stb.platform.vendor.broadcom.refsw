/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  EGL Broadcom flush extension

FILE DESCRIPTION
Implementation of EGL_BRCM_flush.
=============================================================================*/

#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/common/khrn_int_misc_impl.h"

#if EGL_BRCM_flush

/*
   any commands issued by the current thread before calling eglFlushBRCM() are
   guaranteed to complete before any commands issued by *any* thread after
   eglFlushBRCM() returns
*/

EGLAPI void EGLAPIENTRY eglFlushBRCM(void)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   RPC_FLUSH(thread);
}

#endif
