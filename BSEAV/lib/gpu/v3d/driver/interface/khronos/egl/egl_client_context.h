/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
EGL context structure declaration.
=============================================================================*/

#ifndef EGL_CLIENT_CONTEXT_H
#define EGL_CLIENT_CONTEXT_H

#include "interface/khronos/egl/egl_int.h"

typedef struct {
   EGLContext name;
   EGLDisplay display;
   EGLConfig config;

   EGL_CONTEXT_TYPE_T type;

   EGLint renderbuffer;    //EGL_NONE, EGL_BACK_BUFFER or EGL_SINGLE_BUFFER

   void *state;                                 // GLXX_CLIENT_STATE_T or VG_CLIENT_STATE_T
   EGL_CONTEXT_ID_T servercontext;

   struct CLIENT_THREAD_STATE *thread;          // If we are current, which the client state for the thread are we associated with.

   /*
      is_current

      Invariant:

      (EGL_CONTEXT_IS_CURRENT)
      Iff true, the context is current to some thread.
   */
   bool is_current;
   /*
      is_destroyed

      Invariant:

      (EGL_CONTEXT_IS_DESTROYED)
      Iff true, is not a member of the CLIENT_PROCESS_STATE_T.contexts
   */
   bool is_destroyed;

   /* context is marked secure */
   bool secure;
} EGL_CONTEXT_T;

extern EGLBoolean egl_context_check_attribs(const EGLint *attrib_list, EGLint max_version,
   EGLint *version, bool *secure);

extern EGL_CONTEXT_T *egl_context_create(EGL_CONTEXT_T *share_context, EGLContext name,
   EGLDisplay display, EGLConfig config, EGL_CONTEXT_TYPE_T type, bool secure);

extern void egl_context_term(EGL_CONTEXT_T *context);

extern EGLBoolean egl_context_get_attrib(EGL_CONTEXT_T *context, EGLint attrib, EGLint *value);
extern void egl_context_maybe_free(EGL_CONTEXT_T *context);

#endif
