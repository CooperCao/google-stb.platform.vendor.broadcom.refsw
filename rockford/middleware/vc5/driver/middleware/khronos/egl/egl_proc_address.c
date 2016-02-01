/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "interface/khronos/glxx/gl_public_api.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/include/EGL/eglext_brcm.h"
#include "interface/khronos/include/EGL/eglext_android.h"

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
