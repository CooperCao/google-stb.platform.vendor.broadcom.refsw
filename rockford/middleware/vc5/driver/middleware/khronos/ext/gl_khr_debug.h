/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Module   :  Implementation of KHR_debug extension
=============================================================================*/

#ifndef __GL_KHR_DEBUG_H__
#define __GL_KHR_DEBUG_H__

#include "interface/khronos/glxx/gl_public_api.h"
#include "middleware/khronos/ext/gl_khr_debug_msgs.h"
#include "middleware/khronos/glxx/glxx_server_state.h"

typedef struct GLXX_KHR_DEBUG_LOG_ENTRY_T
{
   GLenum                              source;
   GLenum                              type;
   GLenum                              severity;
   uint32_t                            id;
   char                                *message;
   int32_t                             message_len;
   struct GLXX_KHR_DEBUG_LOG_ENTRY_T   *next;
} GLXX_KHR_DEBUG_LOG_ENTRY_T;

typedef struct GLXX_KHR_DEBUG_LOG_T
{
   GLXX_KHR_DEBUG_LOG_ENTRY_T    *head;
   GLXX_KHR_DEBUG_LOG_ENTRY_T    *tail;
   uint32_t                      count;
} GLXX_KHR_DEBUG_LOG_T;

typedef struct GLXX_KHR_DEBUG_MESSAGE_CTRL_T
{
   GLenum   source;
   GLenum   type;
   GLenum   severity;
   GLsizei  count;
   GLuint   *ids;
   bool     enabled;
   struct GLXX_KHR_DEBUG_MESSAGE_CTRL_T   *next;   // Linked list
} GLXX_KHR_DEBUG_MESSAGE_CTRL_T;

// I made a design decision to make the push/pop stack as dynamic as possible.
// Since it will mostly be unused the majority of the time, I didn't want to have a 64
// entry stack of data lying around all the time. Each stack level is allocated only
// when needed.
typedef struct GLXX_KHR_DEBUG_STACK_DATA_T
{
   // Data needed by push & pop group
   GLenum      source;
   GLuint      id;
   char        *message;
   int32_t     message_len;

   // The current set of message controls
   GLXX_KHR_DEBUG_MESSAGE_CTRL_T *filters;
   GLXX_KHR_DEBUG_MESSAGE_CTRL_T *filters_tail;

   struct GLXX_KHR_DEBUG_STACK_DATA_T *prev;    // Previous stack entry

} GLXX_KHR_DEBUG_STACK_DATA_T;

typedef struct GLXX_KHR_DEBUG_STATE_T
{
   bool                          debug_output;
   bool                          debug_output_synchronous;

   GLDEBUGPROCKHR                callback;
   const void *                  user_param;

   GLXX_KHR_DEBUG_LOG_T          message_log;

   GLXX_KHR_DEBUG_STACK_DATA_T   group_stack_head;
   GLXX_KHR_DEBUG_STACK_DATA_T   *group_stack_top;
   uint32_t                      active_stack_level;

} GLXX_KHR_DEBUG_STATE_T;

extern bool glxx_init_khr_debug_state(GLXX_KHR_DEBUG_STATE_T *state);
extern void glxx_destroy_khr_debug_state(GLXX_KHR_DEBUG_STATE_T *state);

extern bool glxx_debug_enabled(GLXX_SERVER_STATE_T *state);
extern void glxx_debug_message(GLXX_SERVER_STATE_T *state, GLenum source, GLenum type, GLenum severity, GLuint id, const char *msg);
extern void glxx_debug_message_preset(GLXX_SERVER_STATE_T *state, GLenum source, GLenum type, GLenum severity, GLuint id);
extern void glxx_debug_message_maybe_async(GLenum source, GLenum type, GLenum severity, GLuint id, const char *msg);
extern void glxx_debug_message_preset_maybe_async(GLenum source, GLenum type, GLenum severity, GLuint id);

#endif /* __GL_KHR_DEBUG_H__ */