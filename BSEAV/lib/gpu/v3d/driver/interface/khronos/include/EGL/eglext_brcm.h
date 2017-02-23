/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Broadcom EGL extensions header

FILE DESCRIPTION
This is included at the end of our version of EGL/eglext.h. If you're using a
different EGL/eglext.h, you can include this after it
=============================================================================*/

#ifndef EGLEXT_BRCM_H
#define EGLEXT_BRCM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "eglplatform.h"
#include "eglext.h"

/* We want this */
#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#endif

#ifndef EGL_BRCM_driver_monitor
#define EGL_BRCM_driver_monitor 1
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglInitDriverMonitorBRCM(EGLDisplay display, EGLint hw_bank, EGLint l3c_bank);
EGLAPI void EGLAPIENTRY eglGetDriverMonitorXMLBRCM(EGLDisplay display, EGLint bufSize, EGLint *length, char *xmlStats);
EGLAPI EGLBoolean EGLAPIENTRY eglTermDriverMonitorBRCM(EGLDisplay display);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef void (EGLAPIENTRYP PFNEGLINITDRIVERMONITORBRCMPROC)(EGLDisplay display, EGLint hw_bank, EGLint l3c_bank);
typedef void (EGLAPIENTRYP PFNEGLGETDRIVERMONITORXMLBRCMPROC)(EGLDisplay display, EGLint bufSize, EGLint *length, char *xmlStats);
typedef void (EGLAPIENTRYP PFNEGLTERMDRIVERMONITORBRCMPROC)(EGLDisplay display);
#endif

#ifndef EGL_BRCM_flush
#define EGL_BRCM_flush 0
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI void EGLAPIENTRY eglFlushBRCM(void);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef void (EGLAPIENTRYP PFNEGLFLUSHBRCMPROC)(void);
#endif

#ifndef EGL_BRCM_image_wrap_bcg
#define EGL_BRCM_image_wrap_bcg 1
#define EGL_IMAGE_WRAP_BRCM_BCG 0x9993141

typedef struct {
   BEGL_BufferFormat format;

   uint16_t width;
   uint16_t height;

   int32_t stride; /* in bytes */

   void *storage;
   uint32_t offset;
} EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T;
#endif

#ifndef EGL_BRCM_image_update_control
#define EGL_BRCM_image_update_control 0
#define EGL_IMAGE_UPDATE_CONTROL_SET_MODE_BRCM           0x3260
#define EGL_IMAGE_UPDATE_CONTROL_CHANGED_REGION_BRCM     0x3261
#define EGL_IMAGE_UPDATE_CONTROL_SET_LOCK_STATE_BRCM     0x3262
#define EGL_IMAGE_UPDATE_CONTROL_ALWAYS_BRCM             0x3263
#define EGL_IMAGE_UPDATE_CONTROL_EXPLICIT_BRCM           0x3264
#define EGL_IMAGE_UPDATE_CONTROL_LOCK_BRCM               0x3265
#define EGL_IMAGE_UPDATE_CONTROL_UNLOCK_BRCM             0x3266

#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglImageUpdateParameterivBRCM(EGLDisplay dpy, EGLImageKHR image, EGLenum pname, const EGLint *params);
EGLAPI EGLBoolean EGLAPIENTRY eglImageUpdateParameteriBRCM(EGLDisplay dpy, EGLImageKHR image, EGLenum pname, EGLint param);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLIMAGEUPDATEPARAMETERIVBRCMPROC) (EGLDisplay dpy, EGLImageKHR image, EGLenum pname, const EGLint *params);
typedef EGLBoolean (EGLAPIENTRYP PFNEGLIMAGEUPDATEPARAMETERIBRCMPROC) (EGLDisplay dpy, EGLImageKHR image, EGLenum pname, EGLint param);
#endif

#ifdef __cplusplus
}
#endif

#endif
