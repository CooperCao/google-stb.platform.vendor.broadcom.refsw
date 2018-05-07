/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdbool.h>
#include <GLES/gl.h>

typedef struct
{
   bool     tweaks_disabled;
   bool     es2;
   bool     depthmask;
   GLenum   depthfunc;
   bool     blend_enabled;
   bool     source_matches;
}
TWEAK_STATE_T;

void glxx_tweaker_init(TWEAK_STATE_T *ts, bool es2);
bool glxx_tweaker_update(TWEAK_STATE_T *ts);

void glxx_tweaker_setshadersource(TWEAK_STATE_T *ts, GLsizei count, const char *const*string, const GLint *length);
void glxx_tweaker_setdepthmask(TWEAK_STATE_T *ts, bool m);
void glxx_tweaker_setdepthfunc(TWEAK_STATE_T *ts, GLenum func);
void glxx_tweaker_setblendenabled(TWEAK_STATE_T *ts, bool enabled);
