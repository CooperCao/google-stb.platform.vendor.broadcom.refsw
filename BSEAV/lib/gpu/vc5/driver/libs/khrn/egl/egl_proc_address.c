/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "libs/core/v3d/v3d_ver.h"
#include "../glxx/gl_public_api.h"
#include "../glxx/glxx_int_config.h"
#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>
#include "egl_platform.h"

#include <string.h>

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
   #include "egl_proc_address.inc"
   #undef CASE
   default: break;
   }

   // The hash keys are unique for each API function, but since arbitrary strings
   // can be given to getProcAddress we have to be sure it really matches.
   if (res != NULL)
      if (strcmp(matched, procname))
         res = NULL;

   // Allow platform to add API calls for platform-specific extensions
   // but disallow overriding anything.
   if (res == NULL)
      res = egl_platform_get_proc_address(procname);

   return res;
}
