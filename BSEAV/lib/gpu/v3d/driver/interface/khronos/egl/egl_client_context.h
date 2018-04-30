/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <EGL/egl.h>
#include "interface/khronos/egl/egl_int.h"
#include "vcfw/rtos/abstract/rtos_abstract_mem.h"

typedef struct {
   EGLContext name;
   EGLDisplay display;
   EGLConfig config;

   EGL_CONTEXT_TYPE_T type;

   EGLint renderbuffer;    //EGL_NONE, EGL_BACK_BUFFER or EGL_SINGLE_BUFFER

   void *state;  // GLXX_SERVER_STATE_T

   struct CLIENT_THREAD_STATE *thread;          // If we are current, which the client state for the thread are we associated with.
} EGL_CONTEXT_T;

extern EGLint egl_context_check_attribs(const EGLint *attrib_list, EGLint max_version,
   EGLint *version, bool *secure);

extern EGL_CONTEXT_T *egl_context_create(EGL_CONTEXT_T *share_context, EGLContext name,
   EGLDisplay display, EGLConfig config, EGL_CONTEXT_TYPE_T type, bool secure);

extern EGLBoolean egl_context_get_attrib(EGL_CONTEXT_T *context, EGLint attrib, EGLint *value);