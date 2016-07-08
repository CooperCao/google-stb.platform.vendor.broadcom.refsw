/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Module   :  Implementation of KHR_debug extension
=============================================================================*/

#include "../glxx/gl_public_api.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_server_internal.h"
#include "../glxx/glxx_server_pipeline.h"
#include "../gl20/gl20_program.h"
#include "../gl20/gl20_shader.h"

// Does the given id match the list in the node?
// If the node has no list, it will also match.
static bool id_matches(GLXX_KHR_DEBUG_MESSAGE_CTRL_T *node, GLuint id)
{
   for (GLsizei i = 0; i < node->count; i++)
   {
      if (node->ids[i] == id)
         return true;
   }

   return node->count == 0;   // Empty id lists always match
}

// Should the given message be filtered out?
static bool filter_message_out(GLXX_KHR_DEBUG_STATE_T *dbg,
                               GLenum source, GLenum type, GLenum severity,
                               GLuint id)
{
   bool enabled = true;

   // Severity LOW messages are disabled by default, so handle that first
   if (severity == GL_DEBUG_SEVERITY_LOW_KHR)
      enabled = false;

   // Now look through each filter - in the order they were submitted and
   // adjust the current enable state as appropriate
   GLXX_KHR_DEBUG_MESSAGE_CTRL_T *node = dbg->group_stack_top->filters;
   while (node != NULL)
   {
      if ((node->source == GL_DONT_CARE || node->source == source) &&
          (node->type == GL_DONT_CARE || node->type == type) &&
          (node->severity == GL_DONT_CARE || node->severity == severity) &&
          (id_matches(node, id)))
      {
         // This node matches the passed parameters
         enabled = node->enabled;
      }

      node = node->next;
   }

   return !enabled;
}

// Append a new filter node to the list in the given stack level
static bool new_filter_node(GLXX_KHR_DEBUG_STACK_DATA_T *data,
                            GLenum source, GLenum type,
                            GLenum severity, GLsizei count,
                            const GLuint *ids, GLboolean enabled)
{
   // Add a new node to the filter list
   GLXX_KHR_DEBUG_MESSAGE_CTRL_T *new_node = calloc(1, sizeof(GLXX_KHR_DEBUG_MESSAGE_CTRL_T));
   if (new_node == NULL)
      return false;

   // Hook in the new node
   if (data->filters_tail)
      data->filters_tail->next = new_node;

   data->filters_tail = new_node;

   if (data->filters == NULL)
      data->filters = new_node;

   // Fill the basic data
   new_node->source = source;
   new_node->type = type;
   new_node->severity = severity;
   new_node->count = count;
   new_node->enabled = enabled;

   if (count > 0)
   {
      // Clone the id array
      new_node->ids = malloc(count * sizeof(GLuint));
      if (new_node->ids == NULL)
         return false;

      memcpy(new_node->ids, ids, count * sizeof(GLuint));
   }

   return true;
}

// Clone the filters list from src into dst
static bool clone_filters(GLXX_KHR_DEBUG_STACK_DATA_T *dst, GLXX_KHR_DEBUG_STACK_DATA_T *src)
{
   assert(dst->filters == NULL);
   assert(dst->filters_tail == NULL);

   if (src->filters == NULL)
      return true;

   GLXX_KHR_DEBUG_MESSAGE_CTRL_T *sNode = src->filters;
   while (sNode != NULL)
   {
      if (!new_filter_node(dst, sNode->source, sNode->type, sNode->severity,
                           sNode->count, sNode->ids, sNode->enabled))
         return false;

      sNode = sNode->next;
   }
   return true;
}

// All message logging goes via this function.
// This is where they are filtered out as required.
static void debug_message_internal(GLXX_KHR_DEBUG_STATE_T *dbg,
                                   GLenum source, GLenum type, GLenum severity,
                                   GLuint id, const char *msg, int32_t msg_len)
{
   if (!dbg->debug_output || msg == NULL)
      return;

   if (filter_message_out(dbg, source, type, severity, id))
      return;

   const char *the_msg = msg;
   int32_t     the_len = msg_len;
   char        clamped[GLXX_CONFIG_MAX_DEBUG_MESSAGE_LENGTH];

   // Clamp message length to GL_MAX_DEBUG_MESSAGE_LENGTH
   if (msg_len >= GLXX_CONFIG_MAX_DEBUG_MESSAGE_LENGTH)
   {
      memcpy(clamped, msg, GLXX_CONFIG_MAX_DEBUG_MESSAGE_LENGTH);
      clamped[GLXX_CONFIG_MAX_DEBUG_MESSAGE_LENGTH - 1] = '\0';

      the_msg = clamped;
      the_len = GLXX_CONFIG_MAX_DEBUG_MESSAGE_LENGTH - 1;
   }

   if (dbg->callback != NULL)
   {
      // Issue callback
      dbg->callback(source, type, id, severity, the_len, the_msg, dbg->user_param);
   }
   else
   {
      // Note: we will silently ignore allocation failures in this function.
      // If we were to return an OUT_OF_MEMORY error, we will end up being called again
      // to report the error and potentially never get out.

      // If the log is full, new messages are silently discarded
      if (dbg->message_log.count >= GLXX_CONFIG_MAX_DEBUG_LOGGED_MESSAGES)
         return;

      // Place in the message log
      GLXX_KHR_DEBUG_LOG_ENTRY_T *entry = malloc(sizeof(GLXX_KHR_DEBUG_LOG_ENTRY_T));
      if (entry == NULL)
         return;

      entry->next = NULL;
      entry->id = id;
      entry->source = source;
      entry->severity = severity;
      entry->type = type;
      entry->message_len = the_len;
      entry->message = malloc(sizeof(char) * (the_len + 1));
      if (entry->message == NULL)
      {
         free(entry);
         return;
      }
      memcpy(entry->message, the_msg, the_len + 1);

      if (dbg->message_log.head == NULL)
      {
         // First item in log
         dbg->message_log.head = entry;
         dbg->message_log.tail = entry;
      }
      else
      {
         dbg->message_log.tail->next = entry;
         dbg->message_log.tail       = entry;
      }

      dbg->message_log.count++;
   }
}

static bool validate_enums(GLenum source, GLenum type, GLenum severity)
{
   switch (source)
   {
   case GL_DONT_CARE:
   case GL_DEBUG_SOURCE_API_KHR:
   case GL_DEBUG_SOURCE_SHADER_COMPILER_KHR:
   case GL_DEBUG_SOURCE_WINDOW_SYSTEM_KHR:
   case GL_DEBUG_SOURCE_THIRD_PARTY_KHR:
   case GL_DEBUG_SOURCE_APPLICATION_KHR:
   case GL_DEBUG_SOURCE_OTHER_KHR:
      break;
   default:
      return false;
   }

   switch (type)
   {
   case GL_DONT_CARE:
   case GL_DEBUG_TYPE_ERROR_KHR:
   case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR:
   case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR:
   case GL_DEBUG_TYPE_PERFORMANCE_KHR:
   case GL_DEBUG_TYPE_PORTABILITY_KHR:
   case GL_DEBUG_TYPE_OTHER_KHR:
   case GL_DEBUG_TYPE_MARKER_KHR:
   case GL_DEBUG_TYPE_PUSH_GROUP_KHR:
   case GL_DEBUG_TYPE_POP_GROUP_KHR:
      break;
   default:
      return false;
   }

   switch (severity)
   {
   case GL_DONT_CARE:
   case GL_DEBUG_SEVERITY_HIGH_KHR:
   case GL_DEBUG_SEVERITY_MEDIUM_KHR:
   case GL_DEBUG_SEVERITY_LOW_KHR:
   case GL_DEBUG_SEVERITY_NOTIFICATION_KHR:
      break;
   default:
      return false;
   }

   return true;
}

static void clean_stack_entry(GLXX_KHR_DEBUG_STACK_DATA_T *entry)
{
   if (entry->message != NULL)
      free(entry->message);

   // Free up the filters list
   if (entry->filters != NULL)
   {
      GLXX_KHR_DEBUG_MESSAGE_CTRL_T *node = entry->filters;
      while (node != NULL)
      {
         GLXX_KHR_DEBUG_MESSAGE_CTRL_T *del = node;
         node = del->next;
         free(del->ids);
         free(del);
      }
   }
}

static void destroy_stack_entry(GLXX_KHR_DEBUG_STACK_DATA_T *entry)
{
   clean_stack_entry(entry);
   free(entry);
}

GL_APICALL void GL_APIENTRY glDebugMessageControlKHR(GLenum source, GLenum type,
                                                     GLenum severity, GLsizei count,
                                                     const GLuint *ids, GLboolean enabled)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   if (state == NULL)
      return;

   GLXX_KHR_DEBUG_STATE_T *dbg = &state->khr_debug;

   if (!validate_enums(source, type, severity))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (count < 0)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   if (count > 0)
   {
      if (source == GL_DONT_CARE || type == GL_DONT_CARE || severity != GL_DONT_CARE)
      {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         goto end;
      }
   }

   if (source == GL_DONT_CARE && type == GL_DONT_CARE && severity == GL_DONT_CARE && count == 0)
   {
      // This filter affects all messages - just remove all the filters and start afresh
      GLXX_KHR_DEBUG_MESSAGE_CTRL_T *node = dbg->group_stack_top->filters;
      while (node != NULL)
      {
         GLXX_KHR_DEBUG_MESSAGE_CTRL_T *del = node;
         node = del->next;
         free(del->ids);
         free(del);
      }

      dbg->group_stack_top->filters = NULL;
      dbg->group_stack_top->filters_tail = NULL;
   }

   // Add a new node to the filter list
   if (!new_filter_node(dbg->group_stack_top, source, type, severity, count, ids, enabled))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }

end:
   GLXX_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glDebugMessageInsertKHR(GLenum source, GLenum type, GLuint id,
                                                    GLenum severity, GLsizei length,
                                                    const GLchar *buf)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   if (state == NULL)
      return;

   // If the DEBUG_OUTPUT state is disabled calls to DebugMessageInsert are discarded and do
   // not generate an error
   if (!state->khr_debug.debug_output)
      goto end;

   if (!validate_enums(source, type, severity))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (source != GL_DEBUG_SOURCE_APPLICATION_KHR &&
       source != GL_DEBUG_SOURCE_THIRD_PARTY_KHR)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (length < 0)
      length = strlen(buf);

   if (length >= GLXX_CONFIG_MAX_DEBUG_MESSAGE_LENGTH)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   debug_message_internal(&state->khr_debug, source, type, severity, id, buf, length);

end:
   GLXX_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glDebugMessageCallbackKHR(GLDEBUGPROCKHR callback,
                                                      const void *userParam)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   if (state == NULL)
      return;

   state->khr_debug.callback   = callback;
   state->khr_debug.user_param = userParam;

   GLXX_UNLOCK_SERVER_STATE();
}

GL_APICALL GLuint GL_APIENTRY glGetDebugMessageLogKHR(GLuint count, GLsizei bufSize,
                                                      GLenum *sources, GLenum *types,
                                                      GLuint *ids, GLenum *severities,
                                                      GLsizei *lengths, GLchar *messageLog)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   GLuint               cnt = 0;

   if (state == NULL)
      return cnt;

   if (bufSize < 0 && messageLog != NULL)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GLsizei  remainingChars = bufSize;
   GLchar   *cptr = messageLog;

   GLXX_KHR_DEBUG_LOG_ENTRY_T *entry = state->khr_debug.message_log.head;
   while (entry != NULL && cnt < count)
   {
      // Will this string fit?
      if (cptr != NULL && remainingChars < entry->message_len + 1)
         break;

      if (cptr != NULL)
      {
         memcpy(cptr, entry->message, entry->message_len);
         cptr += entry->message_len;
         *cptr++ = '\0';
         remainingChars -= entry->message_len + 1;
      }

      if (sources != NULL)
         sources[cnt] = entry->source;

      if (types != NULL)
         types[cnt] = entry->type;

      if (ids != NULL)
         ids[cnt] = entry->id;

      if (severities != NULL)
         severities[cnt] = entry->severity;

      if (lengths != NULL)
         lengths[cnt] = entry->message_len + 1;

      // Delete this entry and set head ptr to next
      state->khr_debug.message_log.head = entry->next;
      free(entry->message);
      free(entry);

      entry = state->khr_debug.message_log.head;

      state->khr_debug.message_log.count--;
      cnt++;
   }

   if (state->khr_debug.message_log.head == NULL)
   {
      state->khr_debug.message_log.tail = NULL;
      assert(state->khr_debug.message_log.count == 0);
   }

end:
   GLXX_UNLOCK_SERVER_STATE();
   return cnt;
}

GL_APICALL void GL_APIENTRY glPushDebugGroupKHR(GLenum source, GLuint id, GLsizei length,
                                                const GLchar *message)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   if (state == NULL)
      return;

   GLXX_KHR_DEBUG_STACK_DATA_T *data = NULL;

   // Only these are allowed
   if (source != GL_DEBUG_SOURCE_APPLICATION_KHR &&
       source != GL_DEBUG_SOURCE_THIRD_PARTY_KHR)
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   // Strictly adhering to the spec would need this code, but the CTS checks for
   // a condition matching the spec of glDebugMessageInsert here, so we'll do that
   // instead as it also seems more correct.
   // if (length < 0 && strlen(message) >= GLXX_CONFIG_MAX_DEBUG_MESSAGE_LENGTH)
   // {
   //    glxx_server_state_set_error(state, GL_INVALID_VALUE);
   //    goto end;
   // }

   if (length < 0)
      length = strlen(message);

   if (length >= GLXX_CONFIG_MAX_DEBUG_MESSAGE_LENGTH)
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GLXX_KHR_DEBUG_STATE_T *dbg = &state->khr_debug;

   // Stack full?
   if (dbg->active_stack_level == GLXX_CONFIG_MAX_DEBUG_GROUP_STACK_DEPTH - 1)
   {
      glxx_server_state_set_error(state, GL_STACK_OVERFLOW_KHR);
      goto end;
   }

   // Make a new entry on the stack
   data = calloc(1, sizeof(GLXX_KHR_DEBUG_STACK_DATA_T));
   if (data == NULL)
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }

   dbg->active_stack_level++;

   data->prev = dbg->group_stack_top;
   dbg->group_stack_top = data;

   // Fill the data in this new stack level
   data->id = id;
   data->source = source;
   data->message_len = length;
   data->message = calloc(length, sizeof(GLchar));
   if (data->message == NULL)
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }

   strncpy(data->message, message, length);

   // Clone the current filters into the new stack level
   if (!clone_filters(data, data->prev))
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }

   // Fire the push message
   glxx_debug_message(state, data->source, GL_DEBUG_TYPE_PUSH_GROUP_KHR,
                      GL_DEBUG_SEVERITY_NOTIFICATION_KHR, data->id, data->message);

   GLXX_UNLOCK_SERVER_STATE();
   return;

end:
   if (data != NULL)
      destroy_stack_entry(data);

   GLXX_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glPopDebugGroupKHR(void)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   if (state == NULL)
      return;

   GLXX_KHR_DEBUG_STATE_T *dbg = &state->khr_debug;

   // Stack empty?
   if (dbg->active_stack_level == 0)
   {
      glxx_server_state_set_error(state, GL_STACK_UNDERFLOW_KHR);
      goto end;
   }

   GLXX_KHR_DEBUG_STACK_DATA_T  *data = dbg->group_stack_top;
   char                         *msg  = data->message;

   // Fire the message we saved during push
   assert(msg != NULL);
   glxx_debug_message(state, data->source, GL_DEBUG_TYPE_POP_GROUP_KHR,
                      GL_DEBUG_SEVERITY_NOTIFICATION_KHR, data->id, msg);

   dbg->active_stack_level--;

   assert(dbg->group_stack_top != &dbg->group_stack_head);
   assert(data->prev != NULL);

   dbg->group_stack_top = data->prev;

   destroy_stack_entry(data);

end:
   GLXX_UNLOCK_SERVER_STATE();
}

// No strndup on Windows
static char *local_strndup(const char *str, GLsizei length)
{
   char *d = malloc(sizeof(char) * (length + 1));
   if (d != NULL)
   {
      strncpy(d, str, length);
      d[length] = '\0';
   }
   return d;
}

static void copy_label(char **dst, const char *src, GLsizei length)
{
   free(*dst);
   *dst = NULL;

   if (src != NULL)
      *dst = local_strndup(src, length);
}

GL_API void GL_APIENTRY glObjectLabelKHR(GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();

   if (!state)
      return;

   if (label != NULL)
   {
      if (length < 0)
         length = strlen(label);

      if (length >= GLXX_CONFIG_MAX_LABEL_LENGTH)
         goto invalid_value;
   }

   switch (identifier)
   {
   case GL_BUFFER_KHR:
   {
      GLXX_BUFFER_T *buffer = glxx_shared_get_buffer(state->shared, name);

      if (buffer != NULL && buffer->enabled)
         copy_label(&buffer->debug_label, label, length);
      else
         goto invalid_value;
   }
   break;

   case GL_SHADER_KHR:
   {
      GL20_SHADER_T *shader = glxx_shared_get_pobject(state->shared, name);

      if (shader != NULL && gl20_is_shader(shader))
         copy_label(&shader->debug_label, label, length);
      else
         goto invalid_value;
   }
   break;

   case GL_PROGRAM_KHR:
   {
      GL20_PROGRAM_T *program = glxx_shared_get_pobject(state->shared, name);

      if (program != NULL && gl20_is_program(program))
         copy_label((char**)&program->debug_label, label, length);
      else
         goto invalid_value;
   }
   break;

   case GL_VERTEX_ARRAY_KHR:
   {
      GLXX_VAO_T *vao = glxx_get_vao(state, name);

      if (vao != NULL)
         copy_label(&vao->debug_label, label, length);
      else
         goto invalid_value;
   }
   break;

   case GL_QUERY_KHR:
   {
      GLXX_QUERY_T *query = glxx_get_query(state, name);

      if (query != NULL)
         copy_label(&query->debug_label, label, length);
      else
         goto invalid_value;
   }
   break;

   case GL_PROGRAM_PIPELINE_KHR:
   {
      GLXX_PIPELINE_T *pipeline = glxx_pipeline_get(state, name);

      if (pipeline != NULL)
         copy_label(&pipeline->debug_label, label, length);
      else
         goto invalid_value;
   }
   break;

   case GL_TRANSFORM_FEEDBACK:   // No KHR version of this
   {
      GLXX_TRANSFORM_FEEDBACK_T *tf = glxx_get_transform_feedback(state, name);

      if (tf != NULL)
         copy_label(&tf->debug_label, label, length);
      else
         goto invalid_value;
   }
   break;

   case GL_SAMPLER_KHR:
   {
      GLXX_TEXTURE_SAMPLER_STATE_T *sampler = glxx_shared_get_sampler(state->shared, name);

      if (sampler != NULL)
         copy_label(&sampler->debug_label, label, length);
      else
         goto invalid_value;
   }
   break;

   case GL_TEXTURE:              // No KHR version of this
   {
      GLXX_TEXTURE_T *texture = glxx_shared_get_texture(state->shared, name);

      if (texture != NULL)
         copy_label(&texture->debug_label, label, length);
      else
         goto invalid_value;
   }
   break;

   case GL_RENDERBUFFER:         // No KHR version of this
   {
      GLXX_RENDERBUFFER_T *renderbuffer = glxx_shared_get_renderbuffer(state->shared, name, false);

      if (renderbuffer != NULL)
         copy_label(&renderbuffer->debug_label, label, length);
      else
         goto invalid_value;
   }
   break;

   case GL_FRAMEBUFFER:          // No KHR version of this
   {
      GLXX_FRAMEBUFFER_T *framebuffer = glxx_server_state_get_framebuffer(state, name, false);

      if (framebuffer != NULL)
         copy_label(&framebuffer->debug_label, label, length);
      else
         goto invalid_value;
   }
   break;

   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   goto end;

invalid_value:
   glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GLXX_UNLOCK_SERVER_STATE();
}

static void copy_sanitised_outputs(GLsizei bufSize, GLsizei *length, GLchar *label, GLchar *inLab)
{
   GLsizei        inLen = 0;

   // Deal with not finding a label
   if (inLab == NULL)
      inLen = 0;
   else
      inLen = strlen(inLab);

   // If no label is requested, we must fill in the full return length
   if (label == NULL && length != NULL)
      *length = inLen;

   if (inLen >= bufSize)
      inLen = vcos_max(0, bufSize - 1);

   // If there is a label, fill the length we'll use
   if (label != NULL && length != NULL)
      *length = inLen;

   if (label != NULL && bufSize > 0)
   {
      if (inLen > 0)
         strncpy(label, inLab, inLen);

      label[inLen] = '\0';
   }
}

GL_APICALL void GL_APIENTRY glGetObjectLabelKHR(GLenum identifier, GLuint name, GLsizei bufSize,
                                                GLsizei *length, GLchar *label)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   GLchar              *lab = NULL;

   if (!state)
      return;

   switch (identifier)
   {
   case GL_BUFFER_KHR:
   {
      GLXX_BUFFER_T *buffer = glxx_shared_get_buffer(state->shared, name);
      if (buffer != NULL && buffer->enabled)
         lab = buffer->debug_label;
      else
         goto invalid_value;
   }
   break;

   case GL_SHADER_KHR:
   {
      GL20_SHADER_T *shader = glxx_shared_get_pobject(state->shared, name);
      if (shader != NULL)
         lab = shader->debug_label;
      else
         goto invalid_value;
   }
   break;

   case GL_PROGRAM_KHR:
   {
      GL20_PROGRAM_T *program = glxx_shared_get_pobject(state->shared, name);
      if (program != NULL)
         lab = program->debug_label;
      else
         goto invalid_value;
   }
   break;

   case GL_VERTEX_ARRAY_KHR:
   {
      GLXX_VAO_T *vao = glxx_get_vao(state, name);
      if (vao != NULL)
         lab = vao->debug_label;
      else
         goto invalid_value;
   }
   break;

   case GL_QUERY_KHR:
   {
      GLXX_QUERY_T *query = glxx_get_query(state, name);
      if (query != NULL)
         lab = query->debug_label;
      else
         goto invalid_value;
   }
   break;

   case GL_PROGRAM_PIPELINE_KHR:
   {
      GLXX_PIPELINE_T *pipeline = glxx_pipeline_get(state, name);

      if (pipeline != NULL)
         lab = pipeline->debug_label;
      else
         goto invalid_value;
   }
   break;

   case GL_TRANSFORM_FEEDBACK:   // No KHR version of this
   {
      GLXX_TRANSFORM_FEEDBACK_T *tf = glxx_get_transform_feedback(state, name);
      if (tf != NULL)
         lab = tf->debug_label;
      else
         goto invalid_value;
   }
   break;

   case GL_SAMPLER_KHR:
   {
      GLXX_TEXTURE_SAMPLER_STATE_T *sampler = glxx_shared_get_sampler(state->shared, name);
      if (sampler != NULL)
         lab = sampler->debug_label;
      else
         goto invalid_value;
   }
   break;

   case GL_TEXTURE:              // No KHR version of this
   {
      GLXX_TEXTURE_T *texture = glxx_shared_get_texture(state->shared, name);
      if (texture != NULL)
         lab = texture->debug_label;
      else
         goto invalid_value;
   }
   break;

   case GL_RENDERBUFFER:         // No KHR version of this
   {
      GLXX_RENDERBUFFER_T *renderbuffer = glxx_shared_get_renderbuffer(state->shared, name, false);
      if (renderbuffer != NULL)
         lab = renderbuffer->debug_label;
      else
         goto invalid_value;
   }
   break;

   case GL_FRAMEBUFFER:          // No KHR version of this
   {
      GLXX_FRAMEBUFFER_T *framebuffer = glxx_server_state_get_framebuffer(state, name, false);
      if (framebuffer != NULL)
         lab = framebuffer->debug_label;
      else
         goto invalid_value;
   }
   break;

   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   copy_sanitised_outputs(bufSize, length, label, lab);

   goto end;

invalid_value:
   glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GLXX_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glObjectPtrLabelKHR(const void *ptr, GLsizei length,
                                                const GLchar *label)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();

   if (!state)
      return;

   if (label != NULL)
   {
      if (length < 0)
         length = strlen(label);

      if (length >= GLXX_CONFIG_MAX_LABEL_LENGTH)
      {
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
         goto end;
      }
   }

   GLXX_FENCESYNC_T *fsync = glxx_shared_get_fencesync(state->shared, (GLsync)ptr);

   if (fsync != NULL)
      copy_label(&fsync->debug_label, label, length);
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GLXX_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glGetObjectPtrLabelKHR(const void *ptr, GLsizei bufSize,
                                                   GLsizei *length, GLchar *label)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   GLchar              *lab = NULL;

   if (!state)
      return;

   GLXX_FENCESYNC_T *fsync = glxx_shared_get_fencesync(state->shared, (GLsync)ptr);

   if (fsync != NULL)
      lab = fsync->debug_label;
   else
      goto invalid_value;

   copy_sanitised_outputs(bufSize, length, label, lab);

   goto end;

invalid_value:
   glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GLXX_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glGetPointervKHR(GLenum pname, void **params)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();

   if (state == NULL)
      return;

   switch (pname)
   {
   case GL_DEBUG_CALLBACK_FUNCTION_KHR:
      params[0] = state->khr_debug.callback;
      break;
   case GL_DEBUG_CALLBACK_USER_PARAM_KHR:
      params[0] = (GLvoid*)state->khr_debug.user_param;
      break;
   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

   GLXX_UNLOCK_SERVER_STATE();
}

bool glxx_init_khr_debug_state(GLXX_KHR_DEBUG_STATE_T *state)
{
   state->debug_output                    = false;
   state->debug_output_synchronous        = false;
   state->callback                        = NULL;
   state->user_param                      = NULL;

   state->message_log.count               = 0;
   state->message_log.head                = NULL;
   state->message_log.tail                = NULL;

   state->active_stack_level              = 0;

   state->group_stack_top                 = &state->group_stack_head;
   state->group_stack_head.prev           = NULL;
   state->group_stack_head.filters        = NULL;
   state->group_stack_head.filters_tail   = NULL;

   return true;
}

extern void glxx_destroy_khr_debug_state(GLXX_KHR_DEBUG_STATE_T *state)
{
   // Destroy any remaining push/pop stack entries
   GLXX_KHR_DEBUG_STACK_DATA_T *entry = state->group_stack_top;
   while (entry != NULL && entry != &state->group_stack_head)
   {
      GLXX_KHR_DEBUG_STACK_DATA_T *prev = entry->prev;
      destroy_stack_entry(entry);
      entry = prev;
   }
   clean_stack_entry(state->group_stack_top);

   // Destroy any remaining message log entries
   GLXX_KHR_DEBUG_LOG_ENTRY_T *log = state->message_log.head;
   while (log != NULL)
   {
      GLXX_KHR_DEBUG_LOG_ENTRY_T *next = log->next;
      free(log->message);
      free(log);
      log = next;
   }
}

// If you are inside an API call and have access to a GLXX_SERVER_STATE_T, call this
// function for synchronous message behavior.
void glxx_debug_message(GLXX_SERVER_STATE_T *state, GLenum source, GLenum type,
                        GLenum severity, GLuint id, const char *msg)
{
   if (msg != NULL)
      debug_message_internal(&state->khr_debug, source, type, severity, id, msg, strlen(msg));
}

// If you are inside an API call and have access to a GLXX_SERVER_STATE_T, call this
// function for synchronous message behavior. This variant looks up the message string
// in a table based on the id.
void glxx_debug_message_preset(GLXX_SERVER_STATE_T *state, GLenum source, GLenum type,
                               GLenum severity, GLuint id)
{
   const char *msg = glxx_lookup_preset_debug_message(id);
   if (msg != NULL)
      debug_message_internal(&state->khr_debug, source, type, severity, id, msg, strlen(msg));
}

extern bool glxx_debug_enabled(GLXX_SERVER_STATE_T *state)
{
   return state->khr_debug.debug_output;
}
