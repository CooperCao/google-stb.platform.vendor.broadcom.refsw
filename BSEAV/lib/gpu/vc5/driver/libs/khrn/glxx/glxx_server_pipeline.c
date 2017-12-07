/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "libs/util/log/log.h"
#include "../common/khrn_int_common.h"
#include "glxx_server.h"
#include "glxx_shared.h"
#include "glxx_server_internal.h"
#include "glxx_server_pipeline.h"
#include "../gl20/gl20_program.h"
#include "../gl20/gl20_shader.h"
#include "../glsl/glsl_compiler.h"

/*****************************************************************************/
/* Program functions                                                         */
/*****************************************************************************/
static void program_release(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_T *prog)
{
   if (prog != NULL)
   {
      gl20_program_release(prog);
      gl20_server_try_delete_program(state->shared, prog);
   }
}

static void program_assign(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_T **lhs, GL20_PROGRAM_T *rhs)
{
   program_release(state, *lhs);

   if (rhs != NULL)
      gl20_program_acquire(rhs);
   *lhs = rhs;
}

/*****************************************************************************/
/* Pipeline stages functions                                                 */
/*****************************************************************************/
static void stage_assign(GLXX_SERVER_STATE_T *state, PIPELINE_STAGE *stage, GL20_PROGRAM_T *program)
{
   program_assign(state, &stage->program, program);
}

#if V3D_VER_AT_LEAST(3,3,0,0)

static void init_pipeline_stages(PIPELINE_STAGE *stage)
{
   for (int i = 0; i < STAGE_COUNT; ++i)
      stage[i].program = NULL;
}

#endif

static void free_pipeline_stages(GLXX_SERVER_STATE_T *state, PIPELINE_STAGE *stage)
{
   for (int i = 0; i < STAGE_COUNT; ++i)
      stage_assign(state, &stage[i], NULL);
}

/*****************************************************************************/

static void pipeline_object_release_programs(GLXX_SERVER_STATE_T *state, GLXX_PIPELINE_T *pipeline_object)
{
   free_pipeline_stages(state, pipeline_object->stage);
}

bool glxx_pipeline_state_initialise(GLXX_SERVER_STATE_T *state)
{
   struct glxx_pipelines *pipelines = &state->pipelines;

   pipelines->next = 1;

   if (!khrn_map_init(&pipelines->objects, 256))
      return false;

   return true;
}

static void release_pipeline_object_callback(khrn_map *map, uint32_t key, void *pobject, void *state)
{
   pipeline_object_release_programs(state, (GLXX_PIPELINE_T *)pobject);
}

void glxx_pipeline_state_term(GLXX_SERVER_STATE_T *state)
{
   struct glxx_pipelines *pipelines = &state->pipelines;

   KHRN_MEM_ASSIGN(pipelines->bound, NULL);

   khrn_map_iterate(&pipelines->objects, release_pipeline_object_callback, state);

   khrn_map_term(&pipelines->objects);
}

GLuint glxx_pipeline_get_binding(const GLXX_SERVER_STATE_T *state)
{
   return state->pipelines.bound != NULL ? state->pipelines.bound->name : 0;
}

GLXX_PIPELINE_T *glxx_pipeline_get(const GLXX_SERVER_STATE_T *state, GLuint pipeline)
{
   return khrn_map_lookup(&state->pipelines.objects, pipeline);
}

GLuint glxx_pipeline_get_program_name(const GLXX_SERVER_STATE_T *state, STAGE_T stage)
{
   if (state->pipelines.bound == NULL)
      return 0;

   if (state->pipelines.bound->stage[stage].program == NULL)
      return 0;

   return state->pipelines.bound->stage[stage].program->name;
}

GL20_PROGRAM_T *glxx_pipeline_get_active_program(const GLXX_SERVER_STATE_T *state)
{
   if (state->pipelines.bound == NULL)
      return NULL;

   if (state->pipelines.bound->active_program == 0)
      return NULL;

   return glxx_shared_get_pobject(state->shared, state->pipelines.bound->active_program);
}

#if V3D_VER_AT_LEAST(3,3,0,0)

/* Is the pipeline in the map? */
static GLboolean is_pipeline(GLXX_SERVER_STATE_T *state, GLuint pipeline)
{
   GLXX_PIPELINE_T   *object = glxx_pipeline_get(state, pipeline);

   return object != NULL && object->initialised;
}

/* Lookup pipeline in state and initialize if necessary */
static GLXX_PIPELINE_T *get_pipeline(GLXX_SERVER_STATE_T *state, GLuint pipeline)
{
   GLXX_PIPELINE_T *object = khrn_map_lookup(&state->pipelines.objects, pipeline);

   if (object != NULL && object->initialised == GL_FALSE)
   {
      object->name              = pipeline;
      object->initialised       = GL_TRUE;
      object->active_program    = 0;
      object->validation_status = GL_FALSE;

      init_pipeline_stages(object->stage);

      object->info_log          = NULL;
      object->debug_label       = NULL;

      gl20_program_common_init(&object->common);

      object->common.linked_glsl_program = glsl_program_create();
   }

   return object;
}

static void bind_pipeline(GLXX_SERVER_STATE_T *state, GLXX_PIPELINE_T *po)
{
   KHRN_MEM_ASSIGN(state->pipelines.bound, po);
}

static void pipeline_object_free_mem(GLXX_PIPELINE_T *pipeline_object)
{
   free(pipeline_object->info_log);
   pipeline_object->info_log = NULL;

   free(pipeline_object->debug_label);
   pipeline_object->debug_label = NULL;

   gl20_program_common_term(&pipeline_object->common);
}

static void pipeline_object_term(void *v, size_t size)
{
   GLXX_PIPELINE_T *pipeline_object = (GLXX_PIPELINE_T *)v;

   pipeline_object_free_mem(pipeline_object);
}

static void pipeline_object_init(GLXX_PIPELINE_T *pipeline_object)
{
   memset(pipeline_object, 0, sizeof(GLXX_PIPELINE_T));

   pipeline_object->initialised = GL_FALSE;

   khrn_mem_set_term(pipeline_object, pipeline_object_term);
}

static GLXX_PIPELINE_T *pipeline_object_create()
{
   GLXX_PIPELINE_T *pipeline_object = KHRN_MEM_ALLOC_STRUCT(GLXX_PIPELINE_T);

   pipeline_object_init(pipeline_object);

   return pipeline_object;
}

static void pipeline_object_destroy(GLXX_SERVER_STATE_T *state, GLXX_PIPELINE_T *pipeline_object)
{
   if (pipeline_object == NULL)
      return;

   pipeline_object_release_programs(state, pipeline_object);
}

GL_APICALL void GL_APIENTRY glGenProgramPipelines(GLsizei n, GLuint *names)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);

   uint32_t   start_name;
   GLenum     error = GL_NO_ERROR;

   if (state == NULL)
      return;

   start_name = state->pipelines.next;

   if (n < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (names == NULL)
      goto end;

   for (GLsizei i = 0; i < n; i++)
   {
      bool ok = false;

      GLXX_PIPELINE_T *pipeline_object = pipeline_object_create();

      if (pipeline_object != NULL)
      {
         ok = khrn_map_insert(&state->pipelines.objects, state->pipelines.next, pipeline_object);
         khrn_mem_release(pipeline_object);
      }

      if (!ok)
      {
         error = GL_OUT_OF_MEMORY;
         goto end;
      }

      names[i] = state->pipelines.next;
      state->pipelines.next++;
   }

end:
   if (error != GL_NO_ERROR)
   {
      /* delete all the newly created pipelines and set error */
      for (uint32_t name = start_name; name < state->pipelines.next; name++)
         khrn_map_delete(&state->pipelines.objects, name);

      state->pipelines.next = start_name;
      glxx_server_state_set_error(state, error);
   }

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program)
{
   GLenum               error  = GL_NO_ERROR;
   GLXX_PIPELINE_T     *object = NULL;
   GLXX_SERVER_STATE_T *state  = glxx_lock_server_state(OPENGL_ES_3X);

   if (state == NULL)
      return;

   if (state->transform_feedback.in_use)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   /* Get the pipeline (create one if it hasn't been bound) */
   object = get_pipeline(state, pipeline);

   if (object == NULL)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   const uint32_t valid_stages =
         GL_VERTEX_SHADER_BIT
      |  GL_FRAGMENT_SHADER_BIT
      |  GL_COMPUTE_SHADER_BIT
      |  (GLXX_HAS_TNG ? GL_TESS_CONTROL_SHADER_BIT : 0u)
      |  (GLXX_HAS_TNG ? GL_TESS_EVALUATION_SHADER_BIT : 0u)
      |  (GLXX_HAS_TNG ? GL_GEOMETRY_SHADER_BIT : 0u);
   if (stages != GL_ALL_SHADER_BITS && (stages & ~valid_stages) != 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   GL20_PROGRAM_T *program_object = NULL;

   if (program != 0)
   {
      // Get the program object -- sets error if it is not a program, or doesn't exist
      program_object = gl20_get_program(state, program);

      if (program_object == NULL)
         goto end;

      if (!program_object->common.separable || !program_object->common.linked_glsl_program)
      {
         error = GL_INVALID_OPERATION;
         goto end;
      }

      IR_PROGRAM_T *p = program_object->common.linked_glsl_program->ir;
      uint32_t p_stages = 0;
      if (p->stage[SHADER_VERTEX].ir)           p_stages |= GL_VERTEX_SHADER_BIT;
#if GLXX_HAS_TNG
      if (p->stage[SHADER_TESS_CONTROL].ir)     p_stages |= GL_TESS_CONTROL_SHADER_BIT;
      if (p->stage[SHADER_TESS_EVALUATION].ir)  p_stages |= GL_TESS_EVALUATION_SHADER_BIT;
      if (p->stage[SHADER_GEOMETRY].ir)         p_stages |= GL_GEOMETRY_SHADER_BIT;
#endif
      if (p->stage[SHADER_FRAGMENT].ir)         p_stages |= (GL_FRAGMENT_SHADER_BIT | GL_COMPUTE_SHADER_BIT);

      stages &= p_stages;

      if (stages == 0) program_object = NULL;
   }

   // program_object can be NULL here if program was 0
   if (stages & GL_VERTEX_SHADER_BIT)
      stage_assign(state, &object->stage[GRAPHICS_STAGE_VERTEX], program_object);

#if GLXX_HAS_TNG
   if (stages & GL_TESS_CONTROL_SHADER_BIT)
      stage_assign(state, &object->stage[GRAPHICS_STAGE_TESS_CONTROL], program_object);
   if (stages & GL_TESS_EVALUATION_SHADER_BIT)
      stage_assign(state, &object->stage[GRAPHICS_STAGE_TESS_EVALUATION], program_object);
   if (stages & GL_GEOMETRY_SHADER_BIT)
      stage_assign(state, &object->stage[GRAPHICS_STAGE_GEOMETRY], program_object);
#endif

   if (stages & GL_FRAGMENT_SHADER_BIT)
      stage_assign(state, &object->stage[GRAPHICS_STAGE_FRAGMENT], program_object);

   if (stages & GL_COMPUTE_SHADER_BIT)
      stage_assign(state, &object->stage[COMPUTE_STAGE_COMPUTE], program_object);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}


GL_APICALL void GL_APIENTRY glActiveShaderProgram(GLuint pipeline, GLuint program)
{
   GLenum               error           = GL_NO_ERROR;
   GLXX_PIPELINE_T     *pipeline_object = NULL;
   GLXX_SERVER_STATE_T *state           = glxx_lock_server_state(OPENGL_ES_3X);

   if (state == NULL)
      return;

   // Get the pipeline and initialise
   pipeline_object = get_pipeline(state, pipeline);

   if (pipeline_object == NULL)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   GL20_PROGRAM_T *program_object = NULL;

   if (program != 0)
   {
      program_object = gl20_get_program(state, program);

      // Error is set by gl20_get_program
      if (program_object == NULL)
         goto end;
   }

   // program_object will be NULL if program == 0
   pipeline_object->active_program = program_object != NULL ? program_object->name : 0;

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glBindProgramPipeline(GLuint pipeline)
{
   GLenum               error  = GL_NO_ERROR;
   GLXX_PIPELINE_T      *po    = NULL;
   GLXX_SERVER_STATE_T  *state = glxx_lock_server_state(OPENGL_ES_3X);

   if (state == NULL)
      return;

   po = pipeline == 0 ? NULL : get_pipeline(state, pipeline);

   if (po == NULL && pipeline != 0)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   bind_pipeline(state, po);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glDeleteProgramPipelines(GLsizei n, const GLuint *pipelines)
{
   GLenum                error   = GL_NO_ERROR;
   GLXX_SERVER_STATE_T  *state   = glxx_lock_server_state(OPENGL_ES_3X);

   if (state == NULL)
      return;

   if (n < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (pipelines == NULL)
      return;

   for (int i = 0; i < n; ++i)
   {
      GLuint name = pipelines[i];

      // Zeroes are ignored
      if (name != 0)
      {
         GLXX_PIPELINE_T *pipeline_object = khrn_map_lookup(&state->pipelines.objects, name);

         // Unused names are ignored
         if (pipeline_object != NULL)
         {
            if (state->pipelines.bound == pipeline_object)
               KHRN_MEM_ASSIGN(state->pipelines.bound, NULL);

            pipeline_object_destroy(state, pipeline_object);

            khrn_map_delete(&state->pipelines.objects, name);
         }
      }
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

GL_APICALL GLboolean GL_APIENTRY glIsProgramPipeline(GLuint pipeline)
{
   GLboolean            isPipeline = GL_FALSE;
   GLXX_SERVER_STATE_T *state      = glxx_lock_server_state(OPENGL_ES_3X);

   if (state == NULL)
      return false;

   if (pipeline != 0)
      isPipeline = is_pipeline(state, pipeline);

   glxx_unlock_server_state();

   return isPipeline;
}

GL_APICALL void GL_APIENTRY glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint *params)
{
   GLXX_SERVER_STATE_T *state  = glxx_lock_server_state(OPENGL_ES_3X);
   if (state == NULL)
      return;

   /* Get the pipeline (create one if necessary) */
   GLXX_PIPELINE_T *object = get_pipeline(state, pipeline);

   if (object == NULL)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   switch (pname)
   {
   case GL_ACTIVE_PROGRAM:
      *params = object->active_program;
      break;

   case GL_VALIDATE_STATUS:
      *params = object->validation_status ? 1 : 0;
      break;

   case GL_INFO_LOG_LENGTH:
      *params = object->info_log != NULL ? strlen(object->info_log) + 1 : 0;
      break;

   case GL_VERTEX_SHADER:
      *params = object->stage[GRAPHICS_STAGE_VERTEX].program != NULL ? object->stage[GRAPHICS_STAGE_VERTEX].program->name : 0;
      break;

#if GLXX_HAS_TNG
   case GL_TESS_CONTROL_SHADER:
      *params = object->stage[GRAPHICS_STAGE_TESS_CONTROL].program != NULL ? object->stage[GRAPHICS_STAGE_TESS_CONTROL].program->name : 0;
      break;
   case GL_TESS_EVALUATION_SHADER:
      *params = object->stage[GRAPHICS_STAGE_TESS_EVALUATION].program != NULL ? object->stage[GRAPHICS_STAGE_TESS_EVALUATION].program->name : 0;
      break;
   case GL_GEOMETRY_SHADER:
      *params = object->stage[GRAPHICS_STAGE_GEOMETRY].program != NULL ? object->stage[GRAPHICS_STAGE_GEOMETRY].program->name : 0;
      break;
#endif

   case GL_FRAGMENT_SHADER:
      *params = object->stage[GRAPHICS_STAGE_FRAGMENT].program != NULL ? object->stage[GRAPHICS_STAGE_FRAGMENT].program->name : 0;
     break;

   case GL_COMPUTE_SHADER:
      *params = object->stage[COMPUTE_STAGE_COMPUTE].program != NULL ? object->stage[COMPUTE_STAGE_COMPUTE].program->name : 0;
      break;

   default:
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      break;
   }

end:
   glxx_unlock_server_state();
}

#endif

static GLSL_INOUT_T *find_var_index(GLSL_INOUT_T *iface, unsigned count, int index) {
   for (unsigned i=0; i<count; i++) {
      if (iface[i].index == index) return &iface[i];
   }
   return NULL;
}

static GLSL_INOUT_T *find_var_name(GLSL_INOUT_T *iface, unsigned count, const char *name) {
   for (unsigned i=0; i<count; i++) {
      if (!strcmp(iface[i].name, name)) return &iface[i];
   }
   return NULL;
}

static GLSL_INOUT_T *find_var(GLSL_INOUT_T *list, unsigned int count, GLSL_INOUT_T *var)
{
   if (var->index != -1)
      return find_var_index(list, count, var->index);
   else
      return find_var_name (list, count, var->name);

   return NULL;
}

/* Is this sufficient? */
static bool is_builtin(const char *name)
{
   return !strncmp(name, "gl_", 3);
}

static void combine_glsl_programs(GLSL_PROGRAM_T *result, const GLSL_PROGRAM_T *vprog, const GLSL_PROGRAM_T *fprog,
                                  IR_PROGRAM_T *ir, unsigned int uniform_offset)
{
   glsl_ir_program_free(result->ir);
   free(result->samplers);
   free(result->images);

   unsigned num_samplers = vprog->num_samplers + fprog->num_samplers;

   result->num_samplers = num_samplers;
   result->samplers     = malloc(sizeof(GLSL_SAMPLER_T) * num_samplers);
   if (result->samplers != NULL) {
      memcpy(result->samplers,                       vprog->samplers, vprog->num_samplers * sizeof(GLSL_SAMPLER_T));
      memcpy(result->samplers + vprog->num_samplers, fprog->samplers, fprog->num_samplers * sizeof(GLSL_SAMPLER_T));
   }

   for (unsigned i = vprog->num_samplers; i < num_samplers; ++i)
      result->samplers[i].location += uniform_offset;

   unsigned num_images = vprog->num_images + fprog->num_images;

   result->num_images = num_images;
   result->images     = malloc(sizeof(GLSL_IMAGE_T) * num_images);
   if (result->images != NULL) {
      memcpy(result->images,                     vprog->images, vprog->num_images * sizeof(GLSL_IMAGE_T));
      memcpy(result->images + vprog->num_images, fprog->images, fprog->num_images * sizeof(GLSL_IMAGE_T));
   }

   for (unsigned i = vprog->num_images; i < num_images; ++i)
      result->images[i].sampler.location += uniform_offset;

   result->ir = ir;
}

static bool copy_int_array(int **res, int *res_size, int *in, int in_size)
{
   unsigned int   bytes = sizeof(int) * in_size;

   *res_size = 0;
   *res      = (int *)malloc(bytes);

   if (*res == NULL)
      return false;

   *res_size = in_size;
   memcpy(*res, in, bytes);
   return true;
}

/* Move to ir_shader implementation? */
static LinkMap *copy_link_map(LinkMap *map)
{
   LinkMap *res = malloc(sizeof(LinkMap));

   if (res == NULL)
      return NULL;

   bool copied = copy_int_array(&res->ins,      &res->num_ins,      map->ins,      map->num_ins)      &&
                 copy_int_array(&res->outs,     &res->num_outs,     map->outs,     map->num_outs)     &&
                 copy_int_array(&res->uniforms, &res->num_uniforms, map->uniforms, map->num_uniforms) &&
                 copy_int_array(&res->buffers,  &res->num_buffers,  map->buffers,  map->num_buffers);

   if (!copied)
   {
      free(res->ins);
      free(res->outs);
      free(res->uniforms);
      free(res->buffers);
      free(res);
      res = NULL;
   }

   return res;
}

/* Move to ir_shader implementation? */
static IRShader *copy_ir_shader(IRShader *shader)
{
   return glsl_ir_shader_from_blocks(shader->blocks, shader->num_cfg_blocks, shader->outputs, shader->num_outputs);
}

/* TODO -- this will need generalising for 3.2 */
void adjust_fragment_maps(IR_PROGRAM_T *ir, GLSL_PROGRAM_T *glsl_vprog, GLSL_PROGRAM_T *glsl_fprog)
{
   /* If the programs are the same, the work will already have been done.
      Moreover, the ins and outs are for the whole program so we don't even
      have the information on the vtx-frag interface */
   if (glsl_vprog == glsl_fprog)
      return;

   unsigned out_index = 0;
   unsigned out_index_map[GFX_MAX(GLXX_CONFIG_MAX_VERTEX_ATTRIBS*4, GLXX_CONFIG_MAX_VARYING_SCALARS)];

   for (unsigned i = 0; i < glsl_vprog->num_outputs; ++i) {
      GLSL_INOUT_T *out = &glsl_vprog->outputs[i];

      if (is_builtin(out->name))
         continue;

      out_index_map[i] = out_index;
      out_index += glxx_get_element_count(out->type);
   }

   unsigned        in_entry = 0;
   LinkMap        *link_map = ir->stage[SHADER_FRAGMENT].link_map;
   VARYING_INFO_T *varying  = ir->varying;

   for (unsigned i = 0; i < glsl_fprog->num_inputs; ++i) {
      GLSL_INOUT_T   *in  = &glsl_fprog->inputs[i];
      GLSL_INOUT_T   *out = find_var(glsl_vprog->outputs, glsl_vprog->num_outputs, in);

      /* Builtins will not appear in the outs */
      if (out == NULL)
         continue;

      unsigned out_offset = out_index_map[out - glsl_vprog->outputs];

      for (unsigned j = 0; j < glxx_get_element_count(in->type); ++j) {
         if (in_entry >= (unsigned)link_map->num_ins) break;

         varying[out_offset].centroid      = in->centroid;
         varying[out_offset].noperspective = in->noperspective;
         varying[out_offset].flat          = in->flat;

         link_map->ins[in_entry++]         = out_offset++;
      }
   }
}

static const struct {
   STAGE_T stage;
   ShaderFlavour flavour;
} gfx[] = { { GRAPHICS_STAGE_VERTEX,          SHADER_VERTEX          },
#if GLXX_HAS_TNG
            { GRAPHICS_STAGE_TESS_CONTROL,    SHADER_TESS_CONTROL    },
            { GRAPHICS_STAGE_TESS_EVALUATION, SHADER_TESS_EVALUATION },
            { GRAPHICS_STAGE_GEOMETRY,        SHADER_GEOMETRY        },
#endif
            { GRAPHICS_STAGE_FRAGMENT,        SHADER_FRAGMENT        } };

static IR_PROGRAM_T *combine_ir_programs(GLSL_PROGRAM_T **progs)
{
   IR_PROGRAM_T *result = glsl_ir_program_create();

   if (result == NULL)
      return NULL;

   for (int i=0; i<countof(gfx); i++) {
      GLSL_PROGRAM_T *gp = progs[gfx[i].stage];
      if (!gp) continue;

      IR_PROGRAM_T *p = gp->ir;
      result->stage[gfx[i].flavour].ir       = copy_ir_shader(p->stage[gfx[i].flavour].ir);
      result->stage[gfx[i].flavour].link_map = copy_link_map(p->stage[gfx[i].flavour].link_map);
   }

   result->max_known_layers = 1;
#if GLXX_HAS_TNG
   if (progs[GRAPHICS_STAGE_GEOMETRY]) {
      IR_PROGRAM_T* gs_ir = progs[GRAPHICS_STAGE_GEOMETRY]->ir;
      result->max_known_layers = gs_ir->max_known_layers;
      result->gs_in            = gs_ir->gs_in;
      result->gs_out           = gs_ir->gs_out;
      result->gs_n_invocations = gs_ir->gs_n_invocations;
      result->gs_max_vertices  = gs_ir->gs_max_vertices;
   }
#endif

   memcpy(result->varying, progs[GRAPHICS_STAGE_FRAGMENT]->ir->varying, V3D_MAX_VARYING_COMPONENTS * sizeof(VARYING_INFO_T));

   adjust_fragment_maps(result, progs[GRAPHICS_STAGE_VERTEX], progs[GRAPHICS_STAGE_FRAGMENT]);

   result->live_attr_set = progs[GRAPHICS_STAGE_VERTEX]->ir->live_attr_set;
   result->tf_vary_map   = progs[GRAPHICS_STAGE_VERTEX]->ir->tf_vary_map;

   return result;
}

static bool combine_gl20_program_common(GL20_PROGRAM_COMMON_T *result, GL20_PROGRAM_COMMON_T **prog)
{
   unsigned* ssbo_binding_point = NULL;
   gl20_program_dynamic_array* ssbo_dynamic_arrays = NULL;
   uint32_t* uniform_data = NULL;

   size_t ssbo_binding_point_size = GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS * sizeof(unsigned);
   ssbo_binding_point = malloc(ssbo_binding_point_size);
   if (!ssbo_binding_point)
      goto error;
   memcpy(ssbo_binding_point, prog[GRAPHICS_STAGE_FRAGMENT]->ssbo_binding_point, ssbo_binding_point_size);

   size_t ssbo_dynamic_arrays_size = prog[GRAPHICS_STAGE_FRAGMENT]->num_ssbo_dynamic_arrays * sizeof(gl20_program_dynamic_array);
   ssbo_dynamic_arrays = malloc(ssbo_dynamic_arrays_size);
   if (!ssbo_dynamic_arrays)
      goto error;
   memcpy(ssbo_dynamic_arrays, prog[GRAPHICS_STAGE_FRAGMENT]->ssbo_dynamic_arrays, ssbo_dynamic_arrays_size);

   unsigned num_uniforms = 0;
   for (STAGE_T i = GRAPHICS_STAGE_VERTEX; i < GRAPHICS_STAGE_COUNT; i++) {
      if (prog[i])
         num_uniforms += prog[i]->num_scalar_uniforms;
   }
   uniform_data = malloc(num_uniforms * sizeof(uint32_t));
   if (!uniform_data)
      goto error;

   unsigned offset = 0;
   for (STAGE_T i = GRAPHICS_STAGE_VERTEX; i < GRAPHICS_STAGE_COUNT; i++) {
      if (prog[i]) {
         memcpy(uniform_data + offset, prog[i]->uniform_data, prog[i]->num_scalar_uniforms*sizeof(uint32_t));
         offset += prog[i]->num_scalar_uniforms;
      }
   }

   free(result->ubo_binding_point);
   free(result->ssbo_binding_point);
   free(result->uniform_data);
   free(result->ssbo_dynamic_arrays);
   result->ubo_binding_point        = NULL;
   result->ssbo_binding_point       = ssbo_binding_point;
   result->ssbo_dynamic_arrays      = ssbo_dynamic_arrays;
   result->uniform_data             = uniform_data;
   result->num_scalar_uniforms      = num_uniforms;
   result->num_ssbo_dynamic_arrays  = prog[GRAPHICS_STAGE_FRAGMENT]->num_ssbo_dynamic_arrays;
   return true;

error:
   free(ssbo_binding_point);
   free(ssbo_dynamic_arrays);
   free(uniform_data);
   return false;
}

static void calculate_ordinals(int *ordinal_map, GLSL_INOUT_T *inouts, unsigned num)
{
   char     *base_name = NULL;
   unsigned  base_len  = 0;
   int       ordinal   = 0;

   for (unsigned i = 0; i < num; ++i) {
      char *pos = strchr(inouts[i].name, '.');

      /* Not a structure? */
      if (pos == NULL) {
         ordinal_map[i] = -1;
         base_name = NULL;
         base_len  = 0;
         continue;
      }

      /* First field of a new structure? */
      if (base_name == NULL || strncmp(base_name, inouts[i].name, base_len)) {
         base_name = inouts[i].name;
         base_len  = pos - inouts[i].name;
         ordinal   = 0;
      }

      /* Add ordinal into map */
      ordinal_map[i] = ordinal;
      ordinal++;
   }
}

static bool validate_in_out_interface(GLSL_INOUT_T *outs, unsigned num_outs, GLSL_INOUT_T *ins, unsigned num_ins) {
   int  in_ordinal[GFX_MAX(GLXX_CONFIG_MAX_VERTEX_ATTRIBS*4, GLXX_CONFIG_MAX_VARYING_SCALARS)];
   int out_ordinal[GFX_MAX(GLXX_CONFIG_MAX_VERTEX_ATTRIBS*4, GLXX_CONFIG_MAX_VARYING_SCALARS)];

   calculate_ordinals(in_ordinal, ins, num_ins);
   calculate_ordinals(out_ordinal, outs, num_outs);

   unsigned in_count  = 0;

   for (unsigned i=0; i<num_ins; i++) {
      GLSL_INOUT_T *in = &ins[i];
      if (is_builtin(in->name))
         continue;

      GLSL_INOUT_T *out = find_var(outs, num_outs, in);

      if (!out) return false;

      if (in->type      != out->type )     return false;
      if (in->precision != out->precision) return false;
      if (in->flat      != out->flat)      return false;
      if (in->is_array  != out->is_array)  return false;
      if (in->is_array)
         if (in->array_size != out->array_size) return false;
      if (in_ordinal[i] != -1)
      {
         unsigned out_ix = out - outs;
         if (in_ordinal[i] != out_ordinal[out_ix])
            return false;
      }
      if (in->struct_path != NULL && out->struct_path != NULL)
         if (strcmp(in->struct_path, out->struct_path) != 0)
            return false;

      in_count++;
   }

   unsigned out_count = 0;

   for (unsigned i=0; i<num_outs; i++) {
      GLSL_INOUT_T *out = &outs[i];
      if (is_builtin(out->name))
         continue;
      out_count++;
   }

   return in_count == out_count;
}

bool glxx_pipeline_validate(const GLXX_PIPELINE_T *pipeline)
{
   // Any stage that is attached must have a separable program bound
   for (STAGE_T i=0; i < STAGE_COUNT; i++) {
      GL20_PROGRAM_T *p = pipeline->stage[i].program;
      if (p && !p->separable) return false;
   }

   bool haveGraphics = false;
   for (int i=0; i<countof(gfx); i++) {
      if (pipeline->stage[gfx[i].stage].program != NULL) haveGraphics = true;
   }

   bool haveCompute = (pipeline->stage[COMPUTE_STAGE_COMPUTE].program != NULL);

   if (!haveCompute && !haveGraphics)
      return false;

   if (haveGraphics) {
#if GLXX_HAS_TNG
      // If there is a TES or a TCS then both must be present
      bool tcs = (pipeline->stage[GRAPHICS_STAGE_TESS_CONTROL].program != NULL);
      bool tes = (pipeline->stage[GRAPHICS_STAGE_TESS_EVALUATION].program != NULL);
      if (tcs != tes)
         return false;

      // If tessellation or geometry are active then a VS is required
      bool gs = (pipeline->stage[GRAPHICS_STAGE_GEOMETRY].program != NULL);
      bool vs = (pipeline->stage[GRAPHICS_STAGE_VERTEX].program != NULL);
      if ( (tcs || gs) && !vs )
         return false;
#endif

      // Otherwise, not having a stage present isn't an error

      // If a program is active for two stages no other program can be active between
      for (int i=0; i<countof(gfx); i++) {
         GL20_PROGRAM_T *p = pipeline->stage[gfx[i].stage].program;
         if (p == NULL) continue;

         // Find the last stage for which this program is active
         int last = i;
         for (int j = i+1; j < countof(gfx); j++) {
            if (pipeline->stage[gfx[j].stage].program == p)
               last = j;
         }

         /* We know that j < countof(gfx) but gcc gives a warning without the extra check */
         for (int j = i+1; j <= last && j < countof(gfx); j++) {
            GL20_PROGRAM_T *stage_p = pipeline->stage[gfx[j].stage].program;
            if (stage_p != NULL && stage_p != p)
               return false;
         }
         // Skip on to the last stage of this program
         i = last;
      }

      // If an active program contains a stage for which it is not active then fail
      for (int i=0; i<countof(gfx); i++) {
         GL20_PROGRAM_T *p = pipeline->stage[gfx[i].stage].program;
         if (p == NULL) continue;

         GLSL_PROGRAM_T *gp = p->common.linked_glsl_program;
         for (int j=0; j<countof(gfx); j++) {
            if (gp->ir->stage[gfx[j].flavour].ir != NULL && pipeline->stage[gfx[j].stage].program != p)
               return false;
         }
      }

      for (STAGE_T i = GRAPHICS_STAGE_VERTEX; i < GRAPHICS_STAGE_COUNT; i++ ) {
         if (!pipeline->stage[i].program) continue;

         STAGE_T next;
         for (next = i+1; next < GRAPHICS_STAGE_COUNT; next++) {
            if (pipeline->stage[next].program) break;
         }
         if (next == GRAPHICS_STAGE_COUNT) break;

         // If a stage is present then it must be linked correctly, so these are non-NULL
         GLSL_PROGRAM_T *a = pipeline->stage[i].program->common.linked_glsl_program;
         GLSL_PROGRAM_T *b = pipeline->stage[next].program->common.linked_glsl_program;

         // TODO: Need a better way of only running this function on inter-program interfaces
         // TODO: Not sure if this works properly with arrayed interfaces
         if (a != b) {
            if (!validate_in_out_interface(a->outputs, a->num_outputs, b->inputs,  b->num_inputs))
               return false;
         }
      }
   }

   return true;
}

bool glxx_pipeline_create_graphics_common(GLXX_PIPELINE_T *pipeline)
{
   GL20_PROGRAM_T *gl20_vprog = pipeline->stage[GRAPHICS_STAGE_VERTEX].program;
   GL20_PROGRAM_T *gl20_fprog = pipeline->stage[GRAPHICS_STAGE_FRAGMENT].program;

   /* Need a minimum of vertex and fragment programs */
   if (gl20_vprog == NULL || gl20_fprog == NULL)
      return false;

   GL20_PROGRAM_COMMON_T *gl20_progs[GRAPHICS_STAGE_COUNT];
   for (STAGE_T i = GRAPHICS_STAGE_VERTEX; i < GRAPHICS_STAGE_COUNT; i++)
      gl20_progs[i] = pipeline->stage[i].program ? &pipeline->stage[i].program->common : NULL;

   if (!combine_gl20_program_common(&pipeline->common, gl20_progs))
      return false;

   GLSL_PROGRAM_T *glsl_progs[GRAPHICS_STAGE_COUNT];
   for (STAGE_T i = GRAPHICS_STAGE_VERTEX; i < GRAPHICS_STAGE_COUNT; i++)
      glsl_progs[i] = gl20_progs[i] ? gl20_progs[i]->linked_glsl_program : NULL;

   IR_PROGRAM_T *ir = combine_ir_programs(glsl_progs);

   if (ir == NULL)
      return false;

   int *frag_uniforms = ir->stage[SHADER_FRAGMENT].link_map->uniforms;

   for (unsigned i = 0; i < glsl_progs[GRAPHICS_STAGE_FRAGMENT]->num_samplers; ++i) {
      int location = glsl_progs[GRAPHICS_STAGE_FRAGMENT]->samplers[i].location;

      frag_uniforms[location] -= gl20_progs[GRAPHICS_STAGE_VERTEX]->num_scalar_uniforms - glsl_progs[GRAPHICS_STAGE_VERTEX]->num_samplers;
   }

   for (int i=0; i<ir->stage[SHADER_FRAGMENT].link_map->num_uniforms; i++)
      frag_uniforms[i] += gl20_progs[GRAPHICS_STAGE_VERTEX]->num_scalar_uniforms;

   combine_glsl_programs(pipeline->common.linked_glsl_program, glsl_progs[GRAPHICS_STAGE_VERTEX], glsl_progs[GRAPHICS_STAGE_FRAGMENT], ir, gl20_progs[GRAPHICS_STAGE_VERTEX]->num_scalar_uniforms);

   pipeline->common_is_compute = false;

   return true;
}

bool glxx_pipeline_create_compute_common(GLXX_PIPELINE_T *p) {
   if (!p->stage[COMPUTE_STAGE_COMPUTE].program) return false;
   p->common_is_compute = true;
   return true;
}

#if V3D_VER_AT_LEAST(3,3,0,0)

GL_APICALL void GL_APIENTRY glValidateProgramPipeline(GLuint pipeline)
{
   /* Check its a real pipeline and then validate the shader combinations */
   GLenum               error  = GL_NO_ERROR;
   GLXX_SERVER_STATE_T *state  = glxx_lock_server_state(OPENGL_ES_3X);
   GLXX_PIPELINE_T     *object = NULL;

   if (state == NULL)
      return;

   /* Get the pipeline (create it if necessary) */
   object = get_pipeline(state, pipeline);

   if (object == NULL)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   object->validation_status = glxx_pipeline_validate(object);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

/*
   A null-terminating version of strncpy. Copies a string from src
   to dst with a maximum length of len, and forcibly null-terminates
   the result. Returns the number of characters written, not
   including the null terminator, or -1 either dst is NULL or length
   is less than 1 (giving us no space to even write the terminator).

   TODO -- this is duplicated -- move to utility library
*/

static size_t strzncpy(char *dst, const char *src, size_t len)
{
   if (dst && len > 0) {
      strncpy(dst, src, len);

      dst[len - 1] = '\0';

      return strlen(dst);
   } else
      return -1;
}

GL_APICALL void GL_APIENTRY glGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
   GLenum               error  = GL_NO_ERROR;
   GLXX_SERVER_STATE_T *state  = glxx_lock_server_state(OPENGL_ES_3X);
   GLXX_PIPELINE_T     *object = NULL;

   if (state == NULL)
      return;

   object = get_pipeline(state, pipeline);

   if (object == NULL || bufSize < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   size_t chars = 0;
   if (object->info_log != NULL)
   {
      chars = strzncpy(infoLog, object->info_log, bufSize);
   }
   else
   {
      if (bufSize > 0)
         *infoLog = 0;
   }

   if (length)
      *length = gfx_smax(0, (GLsizei)chars);

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

#endif
