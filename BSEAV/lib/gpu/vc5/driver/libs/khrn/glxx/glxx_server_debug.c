/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "glxx_server_internal.h"

GL_API void GL_APIENTRY glInsertEventMarkerEXT(GLsizei length, const GLchar *marker)
{
   /* Do nothing.
      When gpumon_hook is enabled, it will trap this function and pass to
      GPUMonitor (other debuggers may be available).
      That's all that is needed
   */
   vcos_unused(length);
   vcos_unused(marker);
}

GL_API void GL_APIENTRY glPushGroupMarkerEXT(GLsizei length, const GLchar *marker)
{
   /* Do nothing.
      When gpumon_hook is enabled, it will trap this function and pass to
      GPUMonitor (other debuggers may be available).
      That's all that is needed
   */
   vcos_unused(length);
   vcos_unused(marker);
}

GL_API void GL_APIENTRY glPopGroupMarkerEXT(void)
{
   /* Do nothing.
      When gpumon_hook is enabled, it will trap this function and pass to
      GPUMonitor (other debuggers may be available).
      That's all that is needed
   */
}
