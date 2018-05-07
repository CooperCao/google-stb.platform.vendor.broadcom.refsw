/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/begl_platform.h>

#include "interface/khronos/egl/egl_int.h"

#include "interface/khronos/common/khrn_client_platform.h"
#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/glxx/glxx_texture.h"

typedef struct
{
   EGLSurface name;

   /*
      type

      Invariants:

      (EGL_SURFACE_TYPE)
      type in {WINDOW, PBUFFER, PIXMAP}
   */
   EGL_SURFACE_TYPE_T type;

   /*
      config

      Invariants:

      (EGL_SURFACE_CONFIG)
      config is a valid EGLConfig
   */
   EGLConfig config;

   /*
      buffers

      Usually 1 or 3.

      Invariants:

      (EGL_SURFACE_BUFFERS)
      1 <= buffers <= EGL_MAX_BUFFERS
   */
   uint32_t buffers;

   /*
      width

      Invariants:

      (EGL_SURFACE_WIDTH)
      1 <= width <= EGL_CONFIG_MAX_WIDTH
   */
   uint32_t width;

   /*
      height

      Invariants:

      (EGL_SURFACE_HEIGHT)
      1 <= height <= EGL_CONFIG_MAX_HEIGHT
   */
   uint32_t height;

   struct CLIENT_THREAD_STATE *thread;    // If we are current, which the EGL client state for the thread are we associated with.

   /*
      swap_behavior

      Invariant:

      (EGL_SURFACE_SWAP_BEHAVIOUR)
      swap_behavior in {EGL_BUFFER_DESTROYED, EGL_BUFFER_PRESERVED}
   */
   EGLint swap_behavior;

   /*
      multisample_resolve

      Invariant:

      (EGL_SURFACE_MULTISAMPLE_RESOLVE)
      multisample_resolve == EGL_MULTISAMPLE_RESOLVE_DEFAULT
   */
   EGLint multisample_resolve;

   /* For WINDOW types only */

   /*
      win

      Validity:
      type == WINDOW
   */
   EGLNativeWindowType win;

   /*
      largest_pbuffer

      Validity:
      type == PBUFFER
   */
   bool largest_pbuffer;

   /*
      mipmap_texture

      Validity:
      type == PBUFFER
   */
   bool mipmap_texture;

   /*
      mipmap_level

      Validity:
      type == PBUFFER
   */
   uint32_t mipmap_level;

   /*
      texture_format

      Validity:
      type == PBUFFER

      Invariant:
      texture_format in {EGL_NO_TEXTURE, EGL_TEXTURE_RGB, EGL_TEXTURE_RGBA}
   */
   EGLenum texture_format;

   /*
      texture_target

      Validity:
      type == PBUFFER

      Invariant:
      texture_target in {EGL_NO_TEXTURE, EGL_TEXTURE_2D}
   */
   EGLenum texture_target;

   /* For PIXMAP types only */

   /*
      pixmap

      Validity:
      type == PIXMAP

      Invariant:
      pixmap is a valid client-side pixmap handle for pixmap P
   */
   EGLNativePixmapType pixmap;

   bool mipmap;
   uint32_t back_buffer_index;

   KHRN_IMAGE_T *color[EGL_MAX_BUFFERS];
   KHRN_IMAGE_T *depth;
   KHRN_IMAGE_T *ds_multi;
   KHRN_IMAGE_T *color_multi;

   uint8_t config_depth_bits;   // How many depth bits were requested in config. May not match actual buffer.
   uint8_t config_stencil_bits; // How many stencil bits were requested in config. May not match actual buffer.

   GLXX_TEXTURE_T *bound_texture;
   uint32_t swap_interval;

   /* Get the buffer to draw to */
   KHRN_IMAGE_T *(*get_back_buffer)(void *p);
   KHRN_IMAGE_T *active_image;

   void *native_window_state;
   KHRN_IMAGE_FORMAT_T colorformat;

} EGL_SURFACE_T;

extern EGLint egl_surface_check_attribs(
   EGL_SURFACE_TYPE_T type,
   const EGLint *attrib_list,
   int *width,
   int *height,
   bool *largest_pbuffer,
   EGLenum *texture_format,
   EGLenum *texture_target,
   bool *mipmap_texture,
   bool *secure
);
struct CLIENT_PROCESS_STATE;

extern EGL_SURFACE_T *egl_surface_create(
   EGLSurface name,
   EGL_SURFACE_TYPE_T type,
   bool secure,
   uint32_t buffers,
   uint32_t width,
   uint32_t height,
   EGLConfig config,
   EGLNativeWindowType win,
   uintptr_t serverwin,
   bool largest_pbuffer,
   bool mipmap_texture,
   EGLenum texture_format,
   EGLenum texture_target,
   EGLNativePixmapType pixmap,
   int *result);

extern EGLBoolean egl_surface_get_attrib(EGL_SURFACE_T *surface, EGLint attrib, EGLint *value);
extern EGLint egl_surface_set_attrib(EGL_SURFACE_T *surface, EGLint attrib, EGLint value);
extern EGLint egl_surface_get_render_buffer(EGL_SURFACE_T *surface);
