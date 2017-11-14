/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/common/khrn_int_image.h"
#include "interface/khronos/egl/egl_int.h"
#include "middleware/khronos/common/khrn_mem.h"

extern EGL_SURFACE_ID_T eglIntCreateSurface_impl(
   uintptr_t win,
   uint32_t buffers,
   uint32_t width,
   uint32_t height,
   bool secure,
   KHRN_IMAGE_FORMAT_T colorformat,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   KHRN_IMAGE_FORMAT_T multisampleformat,
   uint32_t mipmap,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits,
   uint32_t type);

extern EGL_SURFACE_ID_T eglIntCreateWrappedSurface_impl(
   void *pixmap,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   KHRN_IMAGE_FORMAT_T multisample,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits);

// Create server states. To actually use these, call, eglIntMakeCurrent.
extern EGL_GL_CONTEXT_ID_T eglIntCreateGLES11_impl(EGL_GL_CONTEXT_ID_T share_id, EGL_CONTEXT_TYPE_T share_type);
extern EGL_GL_CONTEXT_ID_T eglIntCreateGLES20_impl(EGL_GL_CONTEXT_ID_T share, EGL_CONTEXT_TYPE_T share_type);
extern EGL_VG_CONTEXT_ID_T eglIntCreateVG_impl(EGL_VG_CONTEXT_ID_T share, EGL_CONTEXT_TYPE_T share_type);

// Disassociates surface or context objects from their handles. The objects
// themselves still exist as long as there is a reference to them. In
// particular, if you delete part of a triple buffer group, the remaining color
// buffers plus the ancillary buffers all survive.
// If, eglIntDestroySurface is called on a locked surface then that ID is
// guaranteed not to be reused until the surface is unlocked (otherwise a call
// to makevcimage or unlock might target the wrong surface)
extern int eglIntDestroySurface_impl(EGL_SURFACE_ID_T);
extern void eglIntDestroyGL_impl(EGL_GL_CONTEXT_ID_T);

// Selects the given process id for all operations. Most resource creation is
//  associated with the currently selected process id
// Selects the given context, draw and read surfaces for GL operations.
// Selects the given context and surface for VG operations.
// Any of the surfaces may be identical to each other.
// If the GL context or surfaces have changed then GL will be flushed. Similarly for VG.
// If any of the surfaces have been resized then the color and ancillary buffers
//  are freed and recreated in the new size.
extern void eglIntMakeCurrent_impl(uint32_t glversion, EGL_GL_CONTEXT_ID_T, EGL_SURFACE_ID_T, EGL_SURFACE_ID_T);

// Flushes one or both context, and waits for the flushes to complete before returning.
// Equivalent to:
// if (flushgl) glFinish())
// if (flushvg) vgFinish())
extern int eglIntFlushAndWait_impl(uint32_t flushgl, uint32_t flushvg);
extern void eglIntFlush_impl(uint32_t flushgl, uint32_t flushvg);

extern bool eglIntBackBufferDims_impl(EGL_SURFACE_ID_T s, uint32_t *width, uint32_t *height);

extern void eglIntSwapBuffers_impl(EGL_SURFACE_ID_T s);
extern void eglIntSelectMipmap_impl(EGL_SURFACE_ID_T s, int level);

extern int eglIntCopyBuffers_impl(EGL_SURFACE_ID_T s, void *pixmap);
extern void eglIntGetColorData_impl(EGL_SURFACE_ID_T s, KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height, int32_t stride, uint32_t y_offset, void *data);
extern void eglIntSetColorData_impl(EGL_SURFACE_ID_T s, KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height, int32_t stride, uint32_t y_offset, const void *data);

extern bool eglIntBindTexImage_impl(EGL_SURFACE_ID_T s);
extern void eglIntReleaseTexImage_impl(EGL_SURFACE_ID_T s);

extern void eglIntSwapInterval_impl(EGL_SURFACE_ID_T s, uint32_t swap_interval);

extern EGL_SYNC_ID_T eglIntCreateSync_impl(uint32_t type, int32_t condition, int32_t status);

extern void eglIntDestroySync_impl(EGL_SYNC_ID_T);
extern void eglSyncGetAttrib_impl(EGL_SYNC_ID_T s, int32_t attrib, int32_t *value);
extern int eglIntSyncWaitTimeout_impl(EGL_SYNC_ID_T s, uint32_t timeout);

#if EGL_KHR_image
extern int eglCreateImageKHR_impl(uint32_t glversion, EGL_CONTEXT_ID_T ctx, EGLenum target,
                                  EGLClientBuffer buffer, EGLint texture_level, EGLint *results);
extern EGLBoolean eglDestroyImageKHR_impl(EGLImageKHR image);
#endif

#if EGL_BRCM_driver_monitor
extern bool eglInitDriverMonitorBRCM_impl(EGLint hw_bank, EGLint l3c_bank);
extern void eglTermDriverMonitorBRCM_impl(void);
extern void eglGetDriverMonitorXMLBRCM_impl(EGLint bufSize, char *xmlStats);
#endif
