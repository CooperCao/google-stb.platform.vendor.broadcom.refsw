/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_options.h"

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef ANDROID
#include <cutils/log.h>
#include <cutils/properties.h>
#endif

KHRN_OPTIONS_T khrn_options;

#ifdef WIN32
/* only used for shadertool */
KHAPI KHRN_OPTIONS_T *khrn_get_options(void)
{
   return &khrn_options;
}
#endif

/* For now, we will use environment variables for the options.
   We might, at some point, want to use ini files perhaps */
static bool read_bool_option(const char *name, bool cur)
{
   char *val = getenv(name);

   if (val == NULL)
      return cur;

   if (val[0] == 't' || val[0] == 'T' || val[0] == '1')
      return true;

   return false;
}

static void read_string_option(const char *name, char *res, size_t len)
{
   char *val = getenv(name);

   if (val != NULL)
   {
      strncpy(res, val, len);
      res[len - 1] = '\0';
   }
}

#ifdef ANDROID
static uint32_t read_uint32_option(const char *name, uint32_t cur)
{
   char val[PROPERTY_VALUE_MAX];
   char __default[PROPERTY_VALUE_MAX];

   snprintf(__default, PROPERTY_VALUE_MAX, "%d", cur);

   /* this cant fail, the default is returned if the key doesn't exist */
   property_get(name, val, __default);

   return atoi(val);
}
#endif

void khrn_init_options(void)
{
   khrn_options.gl_error_assist = false;
   khrn_options.force_dither_off = false;
   khrn_options.output_graphviz = false;
   khrn_options.glfinish_defer_off = false;
   khrn_options.old_glsl_sched = false;
   khrn_options.glsl_single_issue = false;
   khrn_options.vs_uniform_consts = false;
   khrn_options.vs_uniform_long_immed = true;
   khrn_options.fs_uniform_consts = false;
   khrn_options.fs_uniform_long_immed = true;
   khrn_options.glsl_debug_on = false;
   khrn_options.glsl_optimizations_on = true;
   khrn_options.disable_tweaks = false;
#if defined(ANDROID) || defined(WAYLAND)
   khrn_options.shadow_egl_images = true;
#else
   khrn_options.shadow_egl_images = false;
#endif
   khrn_options.glsl_sts_saving_on = false;
   strncpy(khrn_options.graphviz_folder, ".", (sizeof(khrn_options.graphviz_folder) - 1));
   strncpy(khrn_options.glsl_sts_save_dir, ".", (sizeof(khrn_options.glsl_sts_save_dir) - 1));

#ifndef DISABLE_OPTION_PARSING
   /* Now read the options */
   khrn_options.gl_error_assist        = read_bool_option(  "V3D_GL_ERROR_ASSIST",        khrn_options.gl_error_assist);
   khrn_options.force_dither_off       = read_bool_option(  "V3D_FORCE_DITHER_OFF",       khrn_options.force_dither_off);
   khrn_options.output_graphviz        = read_bool_option(  "V3D_OUTPUT_GRAPHVIZ",        khrn_options.output_graphviz);
   khrn_options.glfinish_defer_off     = read_bool_option(  "V3D_GLFINISH_DEFER_OFF",     khrn_options.glfinish_defer_off);
   khrn_options.old_glsl_sched          = read_bool_option(  "V3D_OLD_GLSL_SCHED",        khrn_options.old_glsl_sched);
   khrn_options.glsl_single_issue       = read_bool_option(  "V3D_GLSL_SINGLE_ISSUE",     khrn_options.glsl_single_issue);
   khrn_options.vs_uniform_consts       = read_bool_option(  "V3D_VS_UNIFORM_CONSTS",     khrn_options.vs_uniform_consts);
   khrn_options.vs_uniform_long_immed   = read_bool_option(  "V3D_VS_UNIFORM_LONG_IMMED", khrn_options.vs_uniform_long_immed);
   khrn_options.fs_uniform_consts       = read_bool_option(  "V3D_FS_UNIFORM_CONSTS",     khrn_options.fs_uniform_consts);
   khrn_options.fs_uniform_long_immed   = read_bool_option(  "V3D_FS_UNIFORM_LONG_IMMED", khrn_options.fs_uniform_long_immed);
   khrn_options.glsl_debug_on           = read_bool_option(  "V3D_GLSL_DEBUG_ON",         khrn_options.glsl_debug_on);
   khrn_options.glsl_optimizations_on   = read_bool_option(  "V3D_GLSL_OPTIMIZATIONS_ON", khrn_options.glsl_optimizations_on);
   khrn_options.disable_tweaks          = read_bool_option(  "V3D_DISABLE_TWEAKS",        khrn_options.disable_tweaks);
   khrn_options.shadow_egl_images       = read_bool_option(  "V3D_SHADOW_EGL_IMAGES",     khrn_options.shadow_egl_images);
   khrn_options.glsl_sts_saving_on      = read_bool_option(  "V3D_GLSL_STS_SAVING_ON",    khrn_options.glsl_sts_saving_on);
   read_string_option("V3D_GLSL_STS_SAVE_DIR",    khrn_options.glsl_sts_save_dir, MAX_OPTION_PATHLEN);
   read_string_option("V3D_GRAPHVIZ_FOLDER",      khrn_options.graphviz_folder, MAX_OPTION_PATHLEN);
#endif

#ifdef ANDROID
   khrn_options.gl_error_assist        = read_uint32_option("debug.gl.hw.gl_error_assist",         khrn_options.gl_error_assist);
   khrn_options.force_dither_off       = read_uint32_option("debug.gl.hw.force_dither_off",        khrn_options.force_dither_off);
   khrn_options.glfinish_defer_off     = read_uint32_option("debug.gl.hw.glfinish_defer_off",      khrn_options.glfinish_defer_off);
   khrn_options.output_graphviz        = read_uint32_option("debug.gl.hw.output_graphviz",         khrn_options.output_graphviz);
   khrn_options.gl_error_assist         = read_uint32_option("debug.gl.hw.gl_error_assist",         khrn_options.gl_error_assist);
   khrn_options.force_dither_off        = read_uint32_option("debug.gl.hw.force_dither_off",        khrn_options.force_dither_off);
   khrn_options.old_glsl_sched          = read_uint32_option("debug.gl.hw.old_glsl_sched",          khrn_options.old_glsl_sched);
   khrn_options.glsl_single_issue       = read_uint32_option("debug.gl.hw.glsl_single_issue",       khrn_options.glsl_single_issue);
   khrn_options.vs_uniform_consts       = read_uint32_option("debug.gl.hw.vs_uniform_consts",       khrn_options.vs_uniform_consts);
   khrn_options.vs_uniform_long_immed   = read_uint32_option("debug.gl.hw.vs_uniform_long_immed",   khrn_options.vs_uniform_long_immed);
   khrn_options.fs_uniform_consts       = read_uint32_option("debug.gl.hw.fs_uniform_consts",       khrn_options.fs_uniform_consts);
   khrn_options.fs_uniform_long_immed   = read_uint32_option("debug.gl.hw.fs_uniform_long_immed",   khrn_options.fs_uniform_long_immed);
   khrn_options.glsl_optimizations_on   = read_uint32_option("debug.gl.hw.glsl_optimizations_on",   khrn_options.glsl_optimizations_on);
   khrn_options.disable_tweaks          = read_uint32_option("debug.gl.hw.disable_tweaks",          khrn_options.disable_tweaks);
   khrn_options.shadow_egl_images       = read_uint32_option("debug.gl.hw.shadow_egl_images",       khrn_options.shadow_egl_images);
   khrn_options.glsl_sts_saving_on      = read_uint32_option("debug.gl.hw.glsl_sts_saving_on",      khrn_options.glsl_sts_saving_on);
   khrn_options.glsl_debug_on           = read_uint32_option("debug.gl.hw.glsl_debug_on",           khrn_options.glsl_debug_on);

   LOGD("==========================================================================");
   LOGD("debug.gl.hw.gl_error_assist = %d",  khrn_options.gl_error_assist);
   LOGD("debug.gl.hw.force_dither_off = %d", khrn_options.force_dither_off);
   LOGD("debug.gl.hw.glfinish_defer_off = %d", khrn_options.glfinish_defer_off);
   LOGD("debug.gl.hw.output_graphviz = %d",  khrn_options.output_graphviz);
   LOGD("debug.gl.hw.old_glsl_sched = %d",   khrn_options.old_glsl_sched);
   LOGD("debug.gl.hw.glsl_single_issue = %d", khrn_options.glsl_single_issue);
   LOGD("debug.gl.hw.vs_uniform_consts = %d", khrn_options.vs_uniform_consts);
   LOGD("debug.gl.hw.vs_uniform_long_immed = %d", khrn_options.vs_uniform_long_immed);
   LOGD("debug.gl.hw.fs_uniform_consts = %d", khrn_options.fs_uniform_consts);
   LOGD("debug.gl.hw.fs_uniform_long_immed = %d", khrn_options.fs_uniform_long_immed);
   LOGD("debug.gl.hw.glsl_debug_on = %d",  khrn_options.glsl_debug_on);
   LOGD("debug.gl.hw.glsl_optimizations_on = %d",  khrn_options.glsl_optimizations_on);
   LOGD("debug.gl.hw.disable_tweaks = %d", khrn_options.disable_tweaks);
   LOGD("debug.gl.hw.shadow_egl_images = %d", khrn_options.shadow_egl_images);
   LOGD("debug.gl.hw.glsl_sts_saving_on = %d",  khrn_options.glsl_sts_saving_on);
   LOGD("==========================================================================");
#endif
}

void khrn_error_assist(GLenum error, const char *func, unsigned int line)
{
   if (khrn_options.gl_error_assist && error != GL_NO_ERROR)
   {
#ifdef ANDROID
      switch (error)
      {
      case GL_INVALID_ENUM      : LOGD("V3D ERROR ASSIST : GL_INVALID_ENUM in %s (line %d)", func, line); break;
      case GL_INVALID_VALUE     : LOGD("V3D ERROR ASSIST : GL_INVALID_VALUE in %s (line %d)", func, line); break;
      case GL_INVALID_OPERATION : LOGD("V3D ERROR ASSIST : GL_INVALID_OPERATION in %s (line %d)", func, line); break;
      case GL_OUT_OF_MEMORY     : LOGD("V3D ERROR ASSIST : GL_OUT_OF_MEMORY in %s (line %d)", func, line); break;
      default                   : LOGD("V3D ERROR ASSIST : ERROR CODE %d in %s (line %d)", (int)error, func, line); break;
      }
#else
      switch (error)
      {
      case GL_INVALID_ENUM      : fprintf(stderr, "V3D ERROR ASSIST : GL_INVALID_ENUM in %s (line %d)\n", func, line); break;
      case GL_INVALID_VALUE     : fprintf(stderr, "V3D ERROR ASSIST : GL_INVALID_VALUE in %s (line %d)\n", func, line); break;
      case GL_INVALID_OPERATION : fprintf(stderr, "V3D ERROR ASSIST : GL_INVALID_OPERATION in %s (line %d)\n", func, line); break;
      case GL_OUT_OF_MEMORY     : fprintf(stderr, "V3D ERROR ASSIST : GL_OUT_OF_MEMORY in %s (line %d)\n", func, line); break;
      default                   : fprintf(stderr, "V3D ERROR ASSIST : ERROR CODE %d in %s (line %d)\n", (int)error, func, line); break;
      }
      fflush(stderr);
#endif
   }
}
