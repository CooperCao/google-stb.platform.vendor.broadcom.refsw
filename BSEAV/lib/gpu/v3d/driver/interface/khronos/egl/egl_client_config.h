/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_CLIENT_CONFIG_H
#define EGL_CLIENT_CONFIG_H

#include "interface/khronos/common/khrn_client_platform.h"

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/include/EGL/eglext_android.h"

#include "interface/khronos/common/khrn_int_image.h"

#include "interface/khronos/egl/egl_int_config.h"

extern int EGL_MAX_CONFIGS;

/*
   EGL_CONFIG_MIN_SWAP_INTERVAL

   Khronos config attrib name: EGL_MIN_SWAP_INTERVAL

   0 <= EGL_CONFIG_MIN_SWAP_INTERVAL <= 1
*/
#define EGL_CONFIG_MIN_SWAP_INTERVAL 0

/*
   EGL_CONFIG_MAX_SWAP_INTERVAL

   Khronos config attrib name: EGL_MAX_SWAP_INTERVAL

   1 <= EGL_CONFIG_MAX_SWAP_INTERVAL
   EGL_CONFIG_MIN_SWAP_INTERVAL <= EGL_CONFIG_MAX_SWAP_INTERVAL
*/
#define EGL_CONFIG_MAX_SWAP_INTERVAL 0x7fffffff
/*
   EGL_CONFIG_MAX_WIDTH

   Khronos config attrib name: EGL_MAX_PBUFFER_WIDTH

   EGL_CONFIG_MAX_WIDTH > 0
*/
#define EGL_CONFIG_MAX_WIDTH 2048
/*
   EGL_CONFIG_MAX_HEIGHT

   Khronos config attrib name: EGL_MAX_PBUFFER_HEIGHT

   EGL_CONFIG_MAX_HEIGHT > 0
*/
#define EGL_CONFIG_MAX_HEIGHT 2048

/*
   EGLConfig egl_config_from_id(int id)

   Converts between our internally-used index and EGLConfig (by adding 1).

   Preconditions:

   0 <= id < EGL_MAX_CONFIGS

   Postconditions:

   Return value is a valid EGLConfig
*/

static inline EGLConfig egl_config_from_id(int id)
{
   return (EGLConfig)(size_t)(id + 1);
}

/*
   int egl_config_to_id(EGLConfig config)

   Inverse of egl_config_from_id

   Preconditions:

   config is a valid EGLConfig

   Postconditions:

   0 <= result < EGL_MAX_CONFIGS

*/

static inline int egl_config_to_id(EGLConfig config)
{
   return (int)(size_t)config - 1;
}

extern void egl_config_sort(int *ids, bool use_red, bool use_green, bool use_blue, bool use_alpha);
extern bool egl_config_check_attribs(const EGLint *attrib_list);
extern bool egl_config_filter(int id, const EGLint *attrib_list);
extern bool egl_config_get_attrib(int id, EGLint attrib, EGLint *value);
extern void egl_config_install_configs(int type);

extern KHRN_IMAGE_FORMAT_T egl_config_get_color_format(int id);
extern KHRN_IMAGE_FORMAT_T egl_config_get_depth_format(int id);
extern KHRN_IMAGE_FORMAT_T egl_config_get_mask_format(int id);
extern KHRN_IMAGE_FORMAT_T egl_config_get_multisample_format(int id);
extern bool egl_config_get_multisample(int id);
extern bool egl_config_bindable(int id, EGLenum format);
extern bool egl_config_match_pixmap_info(int id, KHRN_IMAGE_WRAP_T *image);
extern uint32_t egl_config_get_api_support(int id);
extern bool egl_config_bpps_match(int id0, int id1); /* bpps of all buffers match */
extern uint32_t egl_config_get_api_conformance(int id);

#if EGL_KHR_lock_surface
extern KHRN_IMAGE_FORMAT_T egl_config_get_mapped_format(int id);
extern bool egl_config_is_lockable(int id);
#endif

#if EGL_ANDROID_framebuffer_target
extern bool egl_config_is_framebuffer_target(int id);
#endif

#if EGL_ANDROID_recordable
extern bool egl_config_is_recordable(int id);
#endif

#endif
