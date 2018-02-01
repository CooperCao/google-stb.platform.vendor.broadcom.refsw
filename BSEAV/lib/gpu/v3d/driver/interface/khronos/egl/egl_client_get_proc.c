/******************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
******************************************************************************/
#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/GLES/glext.h"
#include "interface/khronos/include/GLES2/gl2.h"
#include "interface/khronos/include/GLES2/gl2ext.h"

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_options.h"

#include "interface/khronos/egl/egl_client_surface.h"
#include "interface/khronos/egl/egl_client_context.h"
#include "interface/khronos/egl/egl_client_config.h"

#include "interface/khronos/common/khrn_client.h"

#include "interface/khronos/egl/egl_int_impl.h"

#ifdef KHRONOS_EGL_PLATFORM_OPENWFC
#include "interface/khronos/wf/wfc_client_stream.h"
#endif

#if defined(RPC_DIRECT_MULTI)
#include "middleware/khronos/egl/egl_server.h"
#endif

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static uint32_t hash_string(const char *s)
{
   uint32_t hash = 0;
   for (; *s; ++s)
      hash = (65599 * hash) + *s;
   return hash ^ (hash >> 16);
}

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY
   eglGetProcAddress(const char *procname)
{
   __eglMustCastToProperFunctionPointerType res = (__eglMustCastToProperFunctionPointerType)NULL;
   if (!procname)
      return res;

   const char *matched = NULL;
   switch (hash_string(procname))
   {
#define CASE(NAME, HASH) \
      case HASH: matched = #NAME; res = (__eglMustCastToProperFunctionPointerType)NAME; break;
#include "interface/khronos/egl/egl_proc_address.inc"
#undef CASE
   default: break;
   }

   // The hash keys are unique for each API function, but since arbitrary strings
   // can be given to getProcAddress we have to be sure it really matches.
   if (res != NULL)
      if (strcmp(matched, procname))
         res = NULL;

   if (res == NULL)
   {
#if EGL_KHR_lock_surface
      if (!strcmp(procname, "eglLockSurfaceKHR"))
         return (__eglMustCastToProperFunctionPointerType)eglLockSurfaceKHR;
      if (!strcmp(procname, "eglUnlockSurfaceKHR"))
         return (__eglMustCastToProperFunctionPointerType)eglUnlockSurfaceKHR;
#endif

#if EGL_BRCM_driver_monitor
      if (!strcmp(procname, "eglInitDriverMonitorBRCM"))
         return (__eglMustCastToProperFunctionPointerType)eglInitDriverMonitorBRCM;
      if (!strcmp(procname, "eglGetDriverMonitorXMLBRCM"))
         return (__eglMustCastToProperFunctionPointerType)eglGetDriverMonitorXMLBRCM;
      if (!strcmp(procname, "eglTermDriverMonitorBRCM"))
         return (__eglMustCastToProperFunctionPointerType)eglTermDriverMonitorBRCM;
#endif

#ifdef WAYLAND
      if (!strcmp(procname, "eglBindWaylandDisplayWL"))
         return (__eglMustCastToProperFunctionPointerType)eglBindWaylandDisplayWL;
      if (!strcmp(procname, "eglUnbindWaylandDisplayWL"))
         return (__eglMustCastToProperFunctionPointerType)eglUnbindWaylandDisplayWL;
      if (!strcmp(procname, "eglQueryWaylandBufferWL"))
         return (__eglMustCastToProperFunctionPointerType)eglQueryWaylandBufferWL;
#endif
   }

   return res;
}

#ifdef __cplusplus
}
#endif
