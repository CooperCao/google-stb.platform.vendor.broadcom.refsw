/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
EGL configuration function declarations.
=============================================================================*/

#ifndef EGL_CLIENT_CONFIG_H
#define EGL_CLIENT_CONFIG_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include "egl_surface.h"
#include "libs/core/lfmt/lfmt.h"

#define EGL_CONFIG_MIN_SWAP_INTERVAL 0
#define EGL_CONFIG_MAX_SWAP_INTERVAL 0x7fffffff

#define EGL_CONFIG_MAX_WIDTH 4096
#define EGL_CONFIG_MAX_HEIGHT 4096

/*
 * EGLConfigs as used in EGL Apis are just castable to const EGL_CONFIG_T *
 */
typedef struct
{
   int         samples;
   bool        lockable;
   GFX_LFMT_T  color_api_fmt;
   GFX_LFMT_T  depth_stencil_api_fmt;
   GFX_LFMT_T  stencil_api_fmt;
   GFX_LFMT_T  mask_api_fmt;
   bool        invalid;
}
EGL_CONFIG_T;

extern bool egl_config_is_valid(const EGL_CONFIG_T *config);

/*
 * Return the value of attrib in config, or 0 if attrib is not a valid
 * attribute name. Also, if valid != NULL, sets *valid if attribute was valid.
 *
 * The uses of return value and pointer arg are swapped compared with
 * eglGetConfigAttrib because it's more useful this way around-- often you
 * already know the attrib is valid so can just pass NULL to valid.
 */
extern EGLint egl_config_get_attrib(const EGL_CONFIG_T *config,
      EGLint attrib, bool *valid);

extern bool egl_config_bindable(const EGL_CONFIG_T *config, EGLenum format);

extern uint32_t egl_config_get_api_support(const EGL_CONFIG_T *config);

/* bpps of all buffers match */
extern bool egl_config_bpps_match(const EGL_CONFIG_T *config0,
      const EGL_CONFIG_T *config1);

extern uint32_t egl_config_get_api_conformance(const EGL_CONFIG_T *config);

extern bool egl_config_context_surface_compatible(const EGL_CONTEXT_T *context,
      const EGL_SURFACE_T *surface);

/*
 * Convert config->color to the colour format to actually use for a surface
 * with the given colourspace, alphaformat.
 */
extern GFX_LFMT_T egl_config_colorformat(const EGL_CONFIG_T *config,
      egl_surface_colorspace_t colorspace,
      egl_surface_alphaformat_t alphaformat);

extern GFX_LFMT_T egl_config_color_api_fmt(const EGL_CONFIG_T *config);
extern GFX_LFMT_T egl_config_depth_stencil_api_fmt(const EGL_CONFIG_T *config);
extern GFX_LFMT_T egl_config_stencil_api_fmt(const EGL_CONFIG_T *config);

extern bool egl_can_render_format(GFX_LFMT_T lfmt);
extern bool egl_can_texture_from_format(GFX_LFMT_T lfmt);
extern bool egl_can_display_format(GFX_LFMT_T lfmt);

#endif
