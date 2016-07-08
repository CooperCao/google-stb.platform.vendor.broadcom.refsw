/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "../glxx/gl_public_api.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>
#include <EGL/eglext_android.h>

#include <string.h>
#include "vcos.h"

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY
   eglGetProcAddress(const char *procname)
{
   __eglMustCastToProperFunctionPointerType res = (__eglMustCastToProperFunctionPointerType)NULL;
   if (!procname)
      return res;

   res = vcos_get_module_proc_address(procname);

   return res;
}
