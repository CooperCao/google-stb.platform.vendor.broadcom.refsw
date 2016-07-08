/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Debug extensions
=============================================================================*/

#include "vcos.h"
#include "glxx_server_internal.h"

GL_API void GL_APIENTRY glInsertEventMarkerEXT(GLsizei length, const GLchar *marker)
{
   /* Do nothing.
      When gpumon_hook is enabled, it will trap this function and pass to
      GPUMonitor (other debuggers may be available).
      That's all that is needed
   */
   UNUSED(length);
   UNUSED(marker);
}

GL_API void GL_APIENTRY glPushGroupMarkerEXT(GLsizei length, const GLchar *marker)
{
   /* Do nothing.
      When gpumon_hook is enabled, it will trap this function and pass to
      GPUMonitor (other debuggers may be available).
      That's all that is needed
   */
   UNUSED(length);
   UNUSED(marker);
}

GL_API void GL_APIENTRY glPopGroupMarkerEXT(void)
{
   /* Do nothing.
      When gpumon_hook is enabled, it will trap this function and pass to
      GPUMonitor (other debuggers may be available).
      That's all that is needed
   */
}
