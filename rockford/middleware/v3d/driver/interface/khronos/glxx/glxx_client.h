/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Declaration of client-side GL state structures and include of Khronos GL header files.
=============================================================================*/

#ifndef GLXX_CLIENT_H
#define GLXX_CLIENT_H

#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/common/khrn_client_cache.h"

#include "interface/khronos/egl/egl_client_context.h"

#include "interface/khronos/glxx/glxx_int_attrib.h"
#include "interface/khronos/glxx/glxx_int_config.h"

/*
   Called just before a rendering command (i.e. anything which could modify
   the draw surface) is executed
 */
typedef void (*GL_RENDER_CALLBACK_T)(void);

/*
   Called just after rendering has been compeleted (i.e. flush or finish).
   wait should be true for finish-like behaviour, false for flush-like
   behaviour
*/
typedef void (*GL_FLUSH_CALLBACK_T)(bool wait);

/*
   GL 1.1 and 2.0 client state structure
*/

typedef struct {

   GLenum error;

   /*
      Open GL version

      Invariants:

      OPENGL_ES_11 or OPENGL_ES_20
   */

   unsigned int type;

   /*
      alignments

      used to work out how much data to send for glTexImage2D()
   */

   struct {
      GLint pack;
      GLint unpack;
   } alignment;

   struct {
      GLuint array;
      GLuint element_array;
   } bound_buffer;

   GLXX_ATTRIB_T attrib[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];

   KHRN_CACHE_T cache;

   //gl 1.1 specific
   struct {
      GLenum client;
      GLenum server;
   } active_texture;

   //gl 2.0 specific
   bool default_framebuffer;   //render_callback only called if we're rendering to default framebuffer
} GLXX_CLIENT_STATE_T;

extern int gl11_client_state_init(GLXX_CLIENT_STATE_T *state);
extern int gl20_client_state_init(GLXX_CLIENT_STATE_T *state);

extern void glxx_client_state_free(GLXX_CLIENT_STATE_T *state);

#define GLXX_GET_CLIENT_STATE(thread) glxx_get_client_state(thread)

static INLINE GLXX_CLIENT_STATE_T *glxx_get_client_state(CLIENT_THREAD_STATE_T *thread)
{
   EGL_CONTEXT_T *context = thread->opengl.context;
   GLXX_CLIENT_STATE_T * state;
   vcos_assert( context != NULL );
   vcos_assert(context->type == OPENGL_ES_11 || context->type == OPENGL_ES_20);
   state = (GLXX_CLIENT_STATE_T *)context->state;
   vcos_assert(context->type == state->type);
   return state;
}

#define IS_OPENGLES_11(thread)       is_opengles_11(thread)
#define IS_OPENGLES_20(thread)       is_opengles_20(thread)
#define IS_OPENGLES_11_OR_20(thread) is_opengles_11_or_20(thread)

static INLINE bool is_opengles_11(CLIENT_THREAD_STATE_T *thread)
{
   EGL_CONTEXT_T *context = thread->opengl.context;
   return context && context->type == OPENGL_ES_11;
}

static INLINE bool is_opengles_20(CLIENT_THREAD_STATE_T *thread)
{
   EGL_CONTEXT_T *context = thread->opengl.context;
   return context && context->type == OPENGL_ES_20;
}

static INLINE bool is_opengles_11_or_20(CLIENT_THREAD_STATE_T *thread)
{
   // Worthwhile to avoid the double TLS lookup of "IS_OPENGLES_11() || IS_OPENGLES_20()"
   // And on top of that, at present we only need to assert that it's 11 or 20 - can't be anything
   // else at the moment.
   EGL_CONTEXT_T *context = thread->opengl.context;
   if (context) {
      vcos_assert(context->type == OPENGL_ES_11 || context->type == OPENGL_ES_20);
   }
   return context != NULL;
}

#endif
