/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __GLXX_DEBUG_H__
#define __GLXX_DEBUG_H__

#include "gl_public_api.h"
#include "glxx_server_state.h"

typedef struct GLXX_KHR_DEBUG_LOG_ENTRY_T
{
   GLenum                              source;
   GLenum                              type;
   GLenum                              severity;
   uint32_t                            id;
   char                               *message;
   int32_t                             message_len;
   struct GLXX_KHR_DEBUG_LOG_ENTRY_T  *next;
} GLXX_KHR_DEBUG_LOG_ENTRY_T;

typedef struct GLXX_KHR_DEBUG_LOG_T
{
   GLXX_KHR_DEBUG_LOG_ENTRY_T   *head;
   GLXX_KHR_DEBUG_LOG_ENTRY_T   *tail;
   uint32_t                      count;
} GLXX_KHR_DEBUG_LOG_T;

typedef struct GLXX_KHR_DEBUG_MESSAGE_CTRL_T
{
   GLenum   source;
   GLenum   type;
   GLenum   severity;
   GLsizei  count;
   GLuint  *ids;
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
   char       *message;

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
   const void                   *user_param;

   GLXX_KHR_DEBUG_LOG_T          message_log;

   GLXX_KHR_DEBUG_STACK_DATA_T   group_stack_head;
   GLXX_KHR_DEBUG_STACK_DATA_T  *group_stack_top;
   uint32_t                      active_stack_level;
} GLXX_KHR_DEBUG_STATE_T;

extern bool glxx_init_khr_debug_state(GLXX_KHR_DEBUG_STATE_T *state);
extern void glxx_destroy_khr_debug_state(GLXX_KHR_DEBUG_STATE_T *state);

extern void glxx_debug_message(GLXX_KHR_DEBUG_STATE_T *state, GLenum source, GLenum type, GLenum severity,
                               unsigned id, const char *msg, size_t msg_len);
extern void glxx_debug_log_error(GLXX_KHR_DEBUG_STATE_T *state, GLenum error, const char *func, const char *file, int line);

#endif /* __GLXX_DEBUG_H__ */
