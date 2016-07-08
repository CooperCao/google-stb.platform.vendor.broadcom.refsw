/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
EGL client-side function declarations. Dispatches EGL calls via RPC or direct call.
New "client-side EGL" version
=============================================================================*/

#include "interface/khronos/common/khrn_client_unmangle.h"
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

#if defined(WIN32) || defined(__mips__)
#include "interface/khronos/common/khrn_int_misc_impl.h"
#endif

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

/* Mangle eglGetProcAddress */
#include "interface/khronos/common/khrn_client_mangle.h"

EGLAPI void EGLAPIENTRY (* eglGetProcAddress(const char *procname))(void)
{
/* Don't mangle the rest */
#include "interface/khronos/common/khrn_client_unmangle.h"
#include "interface/khronos/include/EGL/eglext.h"

   /* TODO: any other functions we need to return here?    */
   if(!procname) return (void(*)(void)) NULL;

#if EGL_KHR_image
   if (!strcmp(procname, "eglCreateImageKHR"))
      return (void(*)(void))eglCreateImageKHR;
   if (!strcmp(procname, "eglDestroyImageKHR"))
      return (void(*)(void))eglDestroyImageKHR;
#endif
#ifdef GL_EXT_discard_framebuffer
   if (!strcmp(procname, "glDiscardFramebufferEXT"))
      return (void(*)(void))glDiscardFramebufferEXT;
#endif
#ifdef GL_EXT_debug_marker
   if (!strcmp(procname, "glInsertEventMarkerEXT"))
      return (void(*)(void))glInsertEventMarkerEXT;
   if (!strcmp(procname, "glPushGroupMarkerEXT"))
      return (void(*)(void))glPushGroupMarkerEXT;
   if (!strcmp(procname, "glPopGroupMarkerEXT"))
      return (void(*)(void))glPopGroupMarkerEXT;
#endif
#if GL_OES_point_size_array
   if (!strcmp(procname, "glPointSizePointerOES"))
      return (void(*)(void))glPointSizePointerOES;
#endif
#if GL_OES_EGL_image
   if (!strcmp(procname, "glEGLImageTargetTexture2DOES"))
      return (void(*)(void))glEGLImageTargetTexture2DOES;
   if (!strcmp(procname, "glEGLImageTargetRenderbufferStorageOES"))
      return (void(*)(void))glEGLImageTargetRenderbufferStorageOES;
#endif
#ifndef NO_OPENVG
#if VG_KHR_EGL_image
   if (!strcmp(procname, "vgCreateEGLImageTargetKHR"))
      return (void(*)(void))vgCreateEGLImageTargetKHR;
#endif
#endif /* NO_OPENVG */
#if EGL_KHR_lock_surface
   if (!strcmp(procname, "eglLockSurfaceKHR"))
      return (void(*)(void))eglLockSurfaceKHR;
   if (!strcmp(procname, "eglUnlockSurfaceKHR"))
      return (void(*)(void))eglUnlockSurfaceKHR;
#endif
#if EGL_KHR_fence_sync
   if (!strcmp(procname, "eglCreateSyncKHR"))
      return (void(*)(void))eglCreateSyncKHR;
   if (!strcmp(procname, "eglDestroySyncKHR"))
      return (void(*)(void))eglDestroySyncKHR;
   if (!strcmp(procname, "eglClientWaitSyncKHR"))
      return (void(*)(void))eglClientWaitSyncKHR;
   if (!strcmp(procname, "eglGetSyncAttribKHR"))
      return (void(*)(void))eglGetSyncAttribKHR;
#endif
#if EGL_BRCM_perf_monitor
   if (!strcmp(procname, "eglInitPerfMonitorBRCM"))
      return (void(*)(void))eglInitPerfMonitorBRCM;
   if (!strcmp(procname, "eglTermPerfMonitorBRCM"))
      return (void(*)(void))eglTermPerfMonitorBRCM;
#endif
#if EGL_BRCM_driver_monitor
   if (!strcmp(procname, "eglInitDriverMonitorBRCM"))
      return (void(*)(void))eglInitDriverMonitorBRCM;
   if (!strcmp(procname, "eglGetDriverMonitorXMLBRCM"))
      return (void(*)(void))eglGetDriverMonitorXMLBRCM;
   if (!strcmp(procname, "eglTermDriverMonitorBRCM"))
      return (void(*)(void))eglTermDriverMonitorBRCM;
#endif
#ifdef EXPORT_DESTROY_BY_PID
   if (!strcmp(procname, "eglDestroyByPidBRCM"))
      return (void(*)(void))eglDestroyByPidBRCM;
#endif
#if GL_OES_draw_texture
   if (!strcmp(procname, "glDrawTexsOES"))
      return (void(*)(void))glDrawTexsOES;
   if (!strcmp(procname, "glDrawTexiOES"))
      return (void(*)(void))glDrawTexiOES;
   if (!strcmp(procname, "glDrawTexxOES"))
      return (void(*)(void))glDrawTexxOES;
   if (!strcmp(procname, "glDrawTexsvOES"))
      return (void(*)(void))glDrawTexsvOES;
   if (!strcmp(procname, "glDrawTexivOES"))
      return (void(*)(void))glDrawTexivOES;
   if (!strcmp(procname, "glDrawTexxvOES"))
      return (void(*)(void))glDrawTexxvOES;
   if (!strcmp(procname, "glDrawTexfOES"))
      return (void(*)(void))glDrawTexfOES;
   if (!strcmp(procname, "glDrawTexfvOES"))
      return (void(*)(void))glDrawTexfvOES;
#endif

#if GL_OES_query_matrix
   if (!strcmp(procname, "glQueryMatrixxOES"))
      return (void(*)(void))glQueryMatrixxOES;
#endif

#if GL_OES_framebuffer_object
   if (!strcmp(procname, "glIsRenderbufferOES"))
      return (void(*)(void))glIsRenderbufferOES;
   if (!strcmp(procname, "glBindRenderbufferOES"))
      return (void(*)(void))glBindRenderbufferOES;
   if (!strcmp(procname, "glDeleteRenderbuffersOES"))
      return (void(*)(void))glDeleteRenderbuffersOES;
   if (!strcmp(procname, "glGenRenderbuffersOES"))
      return (void(*)(void))glGenRenderbuffersOES;
   if (!strcmp(procname, "glRenderbufferStorageOES"))
      return (void(*)(void))glRenderbufferStorageOES;
   if (!strcmp(procname, "glGetRenderbufferParameterivOES"))
      return (void(*)(void))glGetRenderbufferParameterivOES;
   if (!strcmp(procname, "glIsFramebufferOES"))
      return (void(*)(void))glIsFramebufferOES;
   if (!strcmp(procname, "glBindFramebufferOES"))
      return (void(*)(void))glBindFramebufferOES;
   if (!strcmp(procname, "glDeleteFramebuffersOES"))
      return (void(*)(void))glDeleteFramebuffersOES;
   if (!strcmp(procname, "glGenFramebuffersOES"))
      return (void(*)(void))glGenFramebuffersOES;
   if (!strcmp(procname, "glCheckFramebufferStatusOES"))
      return (void(*)(void))glCheckFramebufferStatusOES;
   if (!strcmp(procname, "glFramebufferRenderbufferOES"))
      return (void(*)(void))glFramebufferRenderbufferOES;
   if (!strcmp(procname, "glFramebufferTexture2DOES"))
      return (void(*)(void))glFramebufferTexture2DOES;
   if (!strcmp(procname, "glGetFramebufferAttachmentParameterivOES"))
      return (void(*)(void))glGetFramebufferAttachmentParameterivOES;
   if (!strcmp(procname, "glGenerateMipmapOES"))
      return (void(*)(void))glGenerateMipmapOES;
#endif

#if EGL_BRCM_image_update_control
   if (!strcmp(procname, "eglImageUpdateParameterivBRCM"))
      return (void(*)(void))eglImageUpdateParameterivBRCM;
   if (!strcmp(procname, "eglImageUpdateParameteriBRCM"))
      return (void(*)(void))eglImageUpdateParameteriBRCM;
#endif

#if GL_EXT_multisampled_render_to_texture
   if (!strcmp(procname, "glRenderbufferStorageMultisampleEXT"))
         return (void(*)(void))glRenderbufferStorageMultisampleEXT;
   if (!strcmp(procname, "glFramebufferTexture2DMultisampleEXT"))
            return (void(*)(void))glFramebufferTexture2DMultisampleEXT;
#endif



   return (void(*)(void)) NULL;
}

#ifdef __cplusplus
}
#endif
