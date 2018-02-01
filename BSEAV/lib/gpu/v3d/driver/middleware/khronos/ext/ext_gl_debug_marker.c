/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#define GL_GLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/include/GLES/gl.h"

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
