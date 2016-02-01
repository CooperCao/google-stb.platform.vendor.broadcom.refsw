/*=============================================================================
Copyright (c) 2012 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Client-side implementation of the GL_EXT_debug_marker extension.
=============================================================================*/

#define GL_GLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_client_mangle.h"

#include "interface/khronos/common/khrn_int_common.h"

#include "interface/khronos/glxx/glxx_client.h"

#include "interface/khronos/glxx/glxx_int_impl.h"
#include "interface/khronos/glxx/gl20_int_impl.h"

#include "interface/khronos/include/GLES2/gl2.h"
#include "interface/khronos/include/GLES2/gl2ext.h"

GL_API void GL_APIENTRY glInsertEventMarkerEXT(GLsizei length, const GLchar *marker)
{
   /* Do nothing.
      When SpyHook is enabled, it will trap this function and pass to SpyTool.
      That's all that is needed
   */
   UNUSED(length);
   UNUSED(marker);
}

GL_API void GL_APIENTRY glPushGroupMarkerEXT(GLsizei length, const GLchar *marker)
{
   /* Do nothing.
      When SpyHook is enabled, it will trap this function and pass to SpyTool.
      That's all that is needed
   */
   UNUSED(length);
   UNUSED(marker);
}

GL_API void GL_APIENTRY glPopGroupMarkerEXT(void)
{
   /* Do nothing.
      When SpyHook is enabled, it will trap this function and pass to SpyTool.
      That's all that is needed
   */
}
