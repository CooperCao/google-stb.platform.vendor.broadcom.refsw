/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef EGL_IMAGE_NATIVE_BUFFER_ABSTRACT_H
#define EGL_IMAGE_NATIVE_BUFFER_ABSTRACT_H

/* This function is used in EGL_PLATFORM_FNS_T so it needs the attrib_list
 * arguments, even if the're unused.
 */
EGL_IMAGE_T *egl_image_native_buffer_abstract_new(
   EGL_CONTEXT_T *context, EGLenum target, EGLClientBuffer buffer,
   const void *attrib_list, EGL_AttribType attrib_type);

#endif
