/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_common.h"
#include <GLES/gl.h>
#include <limits.h>

#define MAX_OPTION_PATHLEN 1024

typedef struct {
   bool     gl_error_assist;           /* Outputs useful info when the error occurs */
   bool     force_dither_off;          /* Ensure dithering is always off */
   bool     output_graphviz;           /* Output dataflow graph during shader compilation */
   bool     glfinish_defer_off;        /* Dont deferr glFinish until an observable point */
   bool     old_glsl_sched;            /* Use the old glsl code scheduler */
   bool     glsl_single_issue;         /* Force one ALU only per instruction */
   bool     vs_uniform_consts;         /* Use uniforms for all consts in vertex shaders */
   bool     vs_uniform_long_immed;     /* Use uniforms for long immediates in vertex shaders */
   bool     fs_uniform_consts;         /* Use uniforms for all consts in fragment shaders */
   bool     fs_uniform_long_immed;     /* Use uniforms for long immediates in fragment shaders */
   bool     glsl_debug_on;             /* Output debug & graphs during shader compilation */
   bool     glsl_optimizations_on;     /* Enable optimizations */
   bool     disable_tweaks;            /* Disable driver tweaks */
   bool     shadow_egl_images;         /* If enabled, the M2MC is used to take a scratch copy at draw time */
   bool     glsl_sts_saving_on;        /* Save STS files for every shader program */
   char     glsl_sts_save_dir[MAX_OPTION_PATHLEN]; /* Folder for sts saves */
   char     graphviz_folder[MAX_OPTION_PATHLEN];   /* Folder for graphviz output */
} KHRN_OPTIONS_T;

extern KHRN_OPTIONS_T khrn_options;

extern void khrn_init_options(void);
extern void khrn_error_assist(GLenum error, const char *func, unsigned int line);

#ifdef WIN32

/* used for shadertool */
KHAPI KHRN_OPTIONS_T *khrn_get_options(void);

#undef __func__
#define __func__ __FUNCTION__
#endif
