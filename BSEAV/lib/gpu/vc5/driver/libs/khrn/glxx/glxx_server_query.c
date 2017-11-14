/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "../common/khrn_int_common.h"
#include "glxx_query.h"
#include "glxx_query.h"
#include "glxx_server.h"
#include "libs/util/profile/profile.h"

GL_API void GL_APIENTRY glGenQueries(GLsizei n, GLuint *ids)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   GLsizei i;
   uint32_t start_name;
   GLXX_QUERY_T *query;
   GLenum error = GL_NO_ERROR;
   bool ok;

   if (!state)
      return;

   start_name = state->queries.next_name;

   if (n < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!ids)
      goto end;

   i = 0;
   while (i < n)
   {
      error = GL_OUT_OF_MEMORY;
      query = glxx_query_create(state->queries.next_name);
      if (!query)
         goto end;

      ok = khrn_map_insert(&state->queries.objects, state->queries.next_name, query);

      /* if insert succeded,the map has a reference to the object; either way, release it */
      KHRN_MEM_ASSIGN(query, NULL);
      if (!ok)
         goto end;

      error = GL_NO_ERROR;
      ids[i] = state->queries.next_name;
      state->queries.next_name++;
      i++;
   }

end:
   if (error != GL_NO_ERROR)
   {
      /* delete all the newly created queries and set error */
      uint32_t name;
      for (name = start_name; name < state->queries.next_name; name++)
         khrn_map_delete(&state->queries.objects, name);

      state->queries.next_name = start_name;
      glxx_server_state_set_error(state, error);
   }

   glxx_unlock_server_state();
}

GL_API GLboolean GL_APIENTRY glIsQuery(GLuint id)
{
   GLboolean result = GL_FALSE;
   GLXX_SERVER_STATE_T *state;
   GLXX_QUERY_T *query;

   state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return result;

   query = khrn_map_lookup(&state->queries.objects, id);
   if (query && query->target != GL_NONE)
      result = GL_TRUE;

   glxx_unlock_server_state();
   return result;
}

GL_API void GL_APIENTRY glDeleteQueries(GLsizei n, const GLuint *ids)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   GLenum error = GL_NO_ERROR;
   GLsizei i;

   if (!state)
      return;

   if (n < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   for (i = 0; i < n; i++)
      khrn_map_delete(&state->queries.objects, ids[i]);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}

GLXX_QUERY_T *glxx_get_query(GLXX_SERVER_STATE_T *state, GLuint id)
{
   return khrn_map_lookup(&state->queries.objects, id);
}

static bool is_query_target(GLenum target)
{
   bool result;

   switch(target)
   {
      case GL_ANY_SAMPLES_PASSED:
      case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
      case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
         result = true;
         break;
      case GL_PRIMITIVES_GENERATED:
#if V3D_VER_AT_LEAST(4,0,2,0)
         /* this is supported in es31 with extension geometry_shader */
         return (V3D_VER_AT_LEAST(3,3,0,0) ? true : false);
#else
         return false;
#endif
      default:
         result = false;
   }
   return result;
}


/* returns true if query is already an active query for any target */
static bool is_query_active(const GLXX_SERVER_STATE_T *state,
      const GLXX_QUERY_T *query)
{
   const struct glxx_queries *queries = &state->queries;
   unsigned i;

   assert(query != NULL);

   for (i = 0; i < GLXX_Q_COUNT; i++)
   {
      const struct glxx_queries_of_type *qot = &queries->queries[i];
      if (qot->active == query)
         return true;
   }

   return false;
}

static void set_active_query(GLXX_SERVER_STATE_T *state,
      GLXX_QUERY_T *query)
{
   struct glxx_queries_of_type *queries;

   assert(query->target != GL_NONE);

   queries = &state->queries.queries[query->type];
   assert(queries->active == NULL);
   KHRN_MEM_ASSIGN(queries->active, query);
}

static void remove_active_query(GLXX_SERVER_STATE_T *state,
      enum glxx_query_type type)
{
   struct glxx_queries_of_type *queries;

   queries = &state->queries.queries[type];
   KHRN_MEM_ASSIGN(queries->active, NULL);

}

static void query_record_current(GLXX_SERVER_STATE_T *state,
      GLXX_QUERY_T *query)
{
   struct glxx_queries_of_type *queries;

   assert(query->target != GL_NONE);

   queries = &state->queries.queries[query->type];
   query->wait_point = khrn_timeline_get_current(&queries->timeline);
}

static GLXX_QUERY_T* get_active_query(const GLXX_SERVER_STATE_T *state,
      enum glxx_query_target target)
{
   const struct glxx_queries_of_type *queries;
   enum glxx_query_type type;

   assert(target != GL_NONE);
   type = glxx_query_target_to_type(target);
   queries = &state->queries.queries[type];

   if(queries->active != NULL &&
         queries->active->target == target)
      return queries->active;

   return NULL;
}

/* returns true if the active query for target is non zero; for targets
 * ANY_SAMPLES_PASSED and ANY_SAMPLES_PASSED_CONSERVATIVE, return true if the
 * active query for either targets is non zero */
static bool is_active_query_non_zero(const GLXX_SERVER_STATE_T *state,
      enum glxx_query_target target)
{
   const struct glxx_queries_of_type *queries;
   enum glxx_query_type type;

   assert(target != GL_NONE);
   type = glxx_query_target_to_type(target);
   queries = &state->queries.queries[type];

   return queries->active != NULL;
}

GL_API void GL_APIENTRY glBeginQuery(GLenum target, GLuint id)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   GLXX_QUERY_T *query;
   GLenum error = GL_NO_ERROR;

   if (!state)
      return;

   if (!is_query_target(target))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   query = khrn_map_lookup(&state->queries.objects, id);
   if (!query)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   /* we would never find id=0 in the map */
   assert(id != 0);

   if (is_active_query_non_zero(state, target))
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (query->target != GL_NONE && is_query_active(state, query))
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   if (!glxx_query_begin_new_instance(query, target))
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   set_active_query(state, query);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glEndQuery(GLenum target)
{
   GLXX_SERVER_STATE_T  *state = glxx_lock_server_state(OPENGL_ES_3X);
   GLXX_QUERY_T *active_query;
   GLenum error = GL_NO_ERROR;

   if (!state)
      return;

   if (!is_query_target(target))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   active_query = get_active_query(state, target);
   if (active_query == NULL)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   query_record_current(state, active_query);

   remove_active_query(state, active_query->type);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   GLenum error = GL_NO_ERROR;

   if (!state)
   {
      state = glxx_lock_server_state_unchanged_even_if_reset(OPENGL_ES_3X);
      if (!state)
         return;

      /* Error should have been set by the first glxx_lock_server_state */
      assert(state->error == GL_CONTEXT_LOST);
      if (pname == GL_QUERY_RESULT_AVAILABLE)
         *params = GL_TRUE;

      goto end;
   }

   if (pname != GL_QUERY_RESULT_AVAILABLE &&
       pname != GL_QUERY_RESULT)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   GLXX_QUERY_T *query = khrn_map_lookup(&state->queries.objects, id);
   if (!query || query->target == GL_NONE ||
       is_query_active(state, query))
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   struct glxx_queries_of_type *qot = &state->queries.queries[query->type];
   if (pname == GL_QUERY_RESULT_AVAILABLE)
   {
      if (khrn_timeline_check(&qot->timeline, query->wait_point,
               V3D_SCHED_DEPS_COMPLETED))
         *params = GL_TRUE;
      else
         *params = GL_FALSE;
   }
   else
   {
      assert(pname == GL_QUERY_RESULT);
      khrn_timeline_wait(&qot->timeline, query->wait_point,
            V3D_SCHED_DEPS_FINALISED);
      *params = glxx_query_get_result(query);
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glGetQueryiv(GLenum target, GLenum pname, GLint* params)
{
   GLXX_SERVER_STATE_T  *state = glxx_lock_server_state(OPENGL_ES_3X);
   GLXX_QUERY_T *active_query;
   GLenum error = GL_NO_ERROR;

   if (!state)
      return;

   if (pname != GL_CURRENT_QUERY || !is_query_target(target))
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   active_query = get_active_query(state, target);
   if (active_query == NULL)
      *params = 0;
   else
      *params = active_query->name;

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   glxx_unlock_server_state();
}

static bool occlusion_query_install(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs)
{
   struct glxx_queries *queries = &state->queries;
   bool ok = true;

   struct glxx_queries_of_type *qot = &queries->queries[GLXX_Q_OCCLUSION];
   if (qot->active)
   {
      ok = khrn_timeline_record(&qot->timeline, (khrn_render_state*) rs);
      if (!ok)
         return false;
   }

   //enable or disable
   ok = glxx_occlusion_query_record(rs, qot->active);
   return ok;
}

#if V3D_VER_AT_LEAST(4,0,2,0)
static bool prim_counts_queries_install(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs)
{
   //install prim_gen and prim_written queries
   enum glxx_query_type type = GLXX_Q_PRIM_GEN;
   for( unsigned i = 0; i < 2; i++)
   {
      struct glxx_queries_of_type *qot = &state->queries.queries[type];
      if (qot->active)
      {
         if (!khrn_timeline_record(&qot->timeline, (khrn_render_state*) rs))
            return false;
      }
      type = GLXX_Q_PRIM_WRITTEN;
   }
   return glxx_prim_counts_query_record(rs, state->queries.queries[GLXX_Q_PRIM_GEN].active,
      state->queries.queries[GLXX_Q_PRIM_WRITTEN].active);
}
#endif

bool glxx_server_queries_install(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs)
{
   if (!occlusion_query_install(state, rs))
      return false;

#if V3D_VER_AT_LEAST(4,0,2,0)
   if (!prim_counts_queries_install(state, rs))
      return false;
#endif
   return true;
}

bool glxx_server_has_active_query_type(enum glxx_query_type type,
      const GLXX_SERVER_STATE_T *state, const GLXX_HW_RENDER_STATE_T *rs)
{
   const struct glxx_queries         *queries = &state->queries;
   const struct glxx_queries_of_type *qot = &queries->queries[type];
   return qot->active;
}
