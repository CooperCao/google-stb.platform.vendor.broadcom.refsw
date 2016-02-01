/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>

#include "glsl_common.h"
#include "glsl_fastmem.h"
#include "glsl_errors.h"
#include "glsl_compiler.h"
#include "glsl_ast_print.h"
#include "glsl_map.h"
#include "glsl_globals.h"
#include "glsl_symbols.h"
#include "glsl_dataflow.h"
#include "glsl_shader_interfaces.h"
#include "glsl_source.h"
#include "glsl_stringbuilder.h"
#include "glsl_intern.h"
#include "glsl_uniform_layout.h"
#include "glsl_ir_program.h"

#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"

#include "glsl_compiled_shader.h"

static GLenum get_gl_type(const SymbolType* type)
{
   switch (type->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         return primitiveTypesToGLenums[type->u.primitive_type.index];

      case SYMBOL_ARRAY_TYPE:
         return get_gl_type(type->u.array_type.member_type);

      // Everything else should be broken up
      default:
         UNREACHABLE();
         return GL_NONE;
   }
}

/* Information used for packing default uniforms */
typedef struct {
   GLSL_PROGRAM_T *program;       /* Used to fill in sampler info and things */

   int binding;                   /* Ignored except atomics and samplers, -1 if not bound */

   int max_offset;                /* The largest offset so far assigned */
} pack_uniform_t;

typedef struct {
   GLSL_BLOCK_T *records;
   unsigned    n_records;

   GLSL_LAYOUT_BINDING_T *bindings;
   unsigned n_bindings;

   unsigned int num_vs_blocks;
   unsigned int num_fs_blocks;
   unsigned int num_cs_blocks;
} pack_blocks_t;

static int pack_sampler_array(pack_uniform_t *ctx, const int **ids, int **maps, int n_stages, unsigned int array_length, const SymbolType *type, unsigned unif_loc, bool in_vshader)
{
   for (unsigned i = 0; i < array_length; i++) {
      /* We're extra agressive about trimming unused samplers, so check each one here */
      bool s_used[SHADER_FLAVOUR_COUNT] = { false, };
      bool any_used = false;
      for (int j=0; j<n_stages; j++) {
         s_used[j] = (ids[j][i] != -1);
         any_used = any_used || s_used[j];
      }

      if (!any_used) continue;

      const unsigned location = ctx->program->num_samplers++;

      for (int j=0; j<n_stages; j++)
         if (s_used[j]) maps[j][ids[j][i]] = location;

      if (location >= GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS) {
         /* Too many combined samplers */
         glsl_compile_error(ERROR_LINKER, 10, -1, "Too many samplers, max %d", GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS);
         return -1;
      }

      ctx->program->samplers[location].location    = unif_loc + i;
      ctx->program->samplers[location].type        = get_gl_type(type);
      ctx->program->samplers[location].is_32bit    = false; /* XXX: Overridden later */
      ctx->program->samplers[location].in_vshader  = in_vshader;

      if (ctx->binding >= 0) {
         GLSL_LAYOUT_BINDING_T *b = &ctx->program->sampler_binding[ctx->program->num_sampler_bindings++];
         b->id = unif_loc + i;
         b->binding = ctx->binding + i;
      }
   }
   return unif_loc + array_length - 1;
}

static int update_uniform_link_map(const int **ids, int **maps, int n_stages, unsigned int array_length, const SymbolType *type, unsigned int location_base)
{
   /* TODO: This is a bit of a hack. We should really use the layout here too */
   unsigned int el_scalar_count = type->scalar_count;
   unsigned int padded_el_scalar_count;

   /* Matrices are treated as arrays of column vectors */
   if (primitiveTypeFlags[type->u.primitive_type.index] & PRIM_MATRIX_TYPE) {
      int n_cols     = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 0);
      int col_length = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 1);

      array_length   *= n_cols;
      el_scalar_count = col_length;
   }

   if (el_scalar_count == 3) padded_el_scalar_count = 4;
   else                      padded_el_scalar_count = el_scalar_count;

   for (int stage=0; stage<n_stages; stage++) {
      const int *id = ids[stage];
      int *map = maps[stage];

      for (unsigned i = 0; i < array_length; i++) {
         for (unsigned j=0; j<el_scalar_count; j++) {
            if (id[i*el_scalar_count + j] != -1)
               map[id[i*el_scalar_count + j]] = location_base + i*padded_el_scalar_count + j;
         }
      }
   }
   return location_base + (array_length-1)*padded_el_scalar_count + (el_scalar_count - 1);
}

struct if_var_t {
   const char *name;
   const SymbolType *type;
   const MEMBER_LAYOUT_T *layout;
   unsigned array_length;
   unsigned top_level_size;
   unsigned top_level_stride;
};

static void record_var(GLSL_BLOCK_MEMBER_T *unif, const struct if_var_t *var, int offset,
                       const bool *used, bool default_block, int atomic_idx)
{
   unif->type          = get_gl_type(var->type);
   unif->name          = strdup(var->name);
   unif->offset        = offset;
   unif->array_length  = var->array_length;

   unif->top_level_size   = var->top_level_size;
   unif->top_level_stride = var->top_level_stride;

   unif->atomic_idx    = atomic_idx;

   unif->used_in_vs    = used[SHADER_VERTEX];
   unif->used_in_fs    = used[SHADER_FRAGMENT];
   unif->used_in_cs    = used[SHADER_COMPUTE];

   if(!unif->name) glsl_compile_error(ERROR_LINKER, 2, -1, NULL);

   unif->column_major  = true;

   bool backed_by_buffer = (atomic_idx != -1) || !default_block;
   /* For reasons unknown this isn't recorded for default uniforms, and even *
    * the default values are different.                                      */
   if (!backed_by_buffer) {
      unif->array_stride  = -1;
      unif->matrix_stride = -1;
   } else {
      unif->array_stride  =  0;
      unif->matrix_stride =  0;

      const MEMBER_LAYOUT_T *layout = var->layout;
      if (layout->flavour == MEMBER_ARRAY) {
         unif->array_stride = layout->u.array_layout.stride;
         /* Hacky way of recycling the code below */
         layout = &layout->u.array_layout.member_layouts[0];
      }

      bool is_matrix = !!(primitiveTypeFlags[var->type->u.primitive_type.index] & PRIM_MATRIX_TYPE);
      if (is_matrix) {
         unif->matrix_stride = layout->u.matrix_layout.stride;
         unif->column_major  = layout->u.matrix_layout.column_major;
      }
   }
}

/* If this is an array then 'type' tells us the type of array members */
static void pack_var_default(pack_uniform_t *context, int offset, const int **ids, int **maps, int n_stages, const bool *used, const struct if_var_t *var) {
   /* Now update the dataflow and, if required, the programs sampler info */
   if (glsl_prim_is_prim_sampler_type(var->type)) {
      context->max_offset = pack_sampler_array(context, ids, maps, n_stages,
                                               var->array_length, var->type,
                                               offset, used[SHADER_VERTEX]);
   } else {
      context->max_offset = update_uniform_link_map(ids, maps, n_stages, var->array_length, var->type, offset);
   }

   for (int i=0; i<n_stages; i++) {
      ids[i] += var->array_length * var->type->scalar_count;
   }
}

static void enumerate(struct if_var_t *var, const char *name, const SymbolType *type,
                      unsigned int array_length, const MEMBER_LAYOUT_T *layout)
{
   var->name = name;
   var->type = type;
   var->layout = layout;
   var->array_length = array_length;
   /* Fill in defaults. This is written at the top-level */
   var->top_level_size = 1;
   var->top_level_stride = 0;
}

static int enumerate_members(struct if_var_t *vars, const char *name, const MEMBER_LAYOUT_T *layout)
{
   const SymbolType *type = layout->type;

   switch (type->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         enumerate(vars, name, type, 1, layout);
         return 1;

      case SYMBOL_STRUCT_TYPE:
      {
         int n = 0;
         for (unsigned i = 0; i < type->u.struct_type.member_count; i++)
         {
            const char *member_name = asprintf_fast("%s.%s", name, type->u.struct_type.member[i].name);
            n += enumerate_members(&vars[n], member_name, &layout->u.struct_layout.member_layouts[i]);
         }
         return n;
      }

      case SYMBOL_ARRAY_TYPE:
      {
         unsigned int array_length = type->u.array_type.member_count;
         SymbolType  *member_type  = type->u.array_type.member_type;

         if(member_type->flavour==SYMBOL_PRIMITIVE_TYPE) {
            /* Since we pack the whole array, pass layout, not member_layout */
            enumerate(vars, asprintf_fast("%s[0]", name), member_type, array_length, layout);
            return 1;
         } else {
            int n = 0;
            for (unsigned i = 0; i < array_length; i++)
               n += enumerate_members(&vars[n], asprintf_fast("%s[%d]", name, i), &layout->u.array_layout.member_layouts[i]);
            return n;
         }
      }

      case SYMBOL_BLOCK_TYPE:
      default:
         UNREACHABLE();
         return 0;
   }
}

static int enumerate_block_members(struct if_var_t *vars, const char *name, const MEMBER_LAYOUT_T *layout, bool ssbo)
{
   const SymbolType *type = layout->type;
   assert(type->flavour == SYMBOL_BLOCK_TYPE);

   int n = 0;
   for (unsigned i = 0; i < type->u.block_type.member_count; i++)
   {
      MEMBER_LAYOUT_T *member_layout = &layout->u.block_layout.member_layouts[i];
      const char *member_name = type->u.block_type.member[i].name;
      if(type->u.block_type.has_named_instance)
         member_name = asprintf_fast("%s.%s", name, member_name);

      if (ssbo && member_layout->type->flavour == SYMBOL_ARRAY_TYPE &&
          member_layout->type->u.array_type.member_type->flavour != SYMBOL_PRIMITIVE_TYPE)
      {
         int c = enumerate_members(&vars[n], asprintf_fast("%s[0]", member_name),
                                   &member_layout->u.array_layout.member_layouts[0]);
         for (int j=0; j<c; j++) {
            vars[n+j].top_level_size   = member_layout->type->u.array_type.member_count;
            vars[n+j].top_level_stride = member_layout->u.array_layout.stride;
         }
         n += c;
      } else
         n += enumerate_members(&vars[n], member_name, member_layout);
   }
   return n;
}

static unsigned max_buffer_size(StorageQualifier sq) {
   assert(sq == STORAGE_UNIFORM || sq == STORAGE_BUFFER);
   if (sq == STORAGE_UNIFORM) return GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE;
   else                       return GLXX_CONFIG_MAX_SHADER_STORAGE_BLOCK_SIZE;
}

static unsigned max_combined_block_types(StorageQualifier sq) {
   assert(sq == STORAGE_UNIFORM || sq == STORAGE_BUFFER);
   if (sq == STORAGE_UNIFORM) return GLXX_CONFIG_MAX_COMBINED_UNIFORM_BLOCKS;
   else                       return GLXX_CONFIG_MAX_COMBINED_SSBOS;
}

/* TODO: The used information is terrible. It should be per-member-of-an-instance,
 *       but now it's per-array-of-instances */
static void pack_block(pack_blocks_t *ctx, const Symbol *b, const int **ids, int **maps, int n_stages, const bool *used)
{
   bool is_array = (b->type->flavour == SYMBOL_ARRAY_TYPE);
   int array_length = is_array ? b->type->u.array_type.member_count : 1;
   SymbolType *type = is_array ? b->type->u.array_type.member_type  : b->type;

   assert(type->flavour==SYMBOL_BLOCK_TYPE);
   bool active = type->u.block_type.lq->unif_bits & (LAYOUT_SHARED | LAYOUT_STD140 | LAYOUT_STD430);
   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) active = active || used[i];

   if(!active) return;

   if (ctx->n_records == max_combined_block_types(b->u.interface_block.sq))
      glsl_compile_error(ERROR_LINKER, 5, -1, "Too many %s blocks, max %d",
                         glsl_storage_qual_string(b->u.interface_block.sq),
                         max_combined_block_types(b->u.interface_block.sq));

   GLSL_BLOCK_T    *block  = &(ctx->records[ctx->n_records]);
   MEMBER_LAYOUT_T *layout = malloc_fast(sizeof(MEMBER_LAYOUT_T));

   calculate_block_layout(layout, type);

   if (layout->u.block_layout.size > max_buffer_size(b->u.interface_block.sq))
      glsl_compile_error(ERROR_LINKER, 5, -1, "%s block %s too large: %d",
                         glsl_storage_qual_string(b->u.interface_block.sq),
                         b->name, layout->u.block_layout.size);

   struct if_var_t *vars = glsl_safemem_malloc(deep_member_count(layout) * sizeof(struct if_var_t));
   int n_members = enumerate_block_members(vars, b->name, layout, (b->u.interface_block.sq == STORAGE_BUFFER));

   if (ctx->n_records == 0)
      block->index = 0;
   else
      block->index = ctx->records[ctx->n_records-1].index +
                     ctx->records[ctx->n_records-1].array_length;
   block->array_length = array_length;
   block->is_array     = is_array;
   block->num_members  = n_members;
   block->members      = malloc(sizeof(*block->members) * block->num_members);
   block->size         = layout->u.block_layout.size;
   block->name         = strdup(b->name);
   block->used_in_vs   = used[SHADER_VERTEX];
   block->used_in_fs   = used[SHADER_FRAGMENT];
   block->used_in_cs   = used[SHADER_COMPUTE];

   if( !block->name || (!block->members && block->num_members > 0)) {
      glsl_compile_error(ERROR_LINKER, 2, -1, NULL);
   }

   ctx->n_records++;

   for (unsigned int i=0; i<block->num_members; i++)
      record_var(&block->members[i], &vars[i], vars[i].layout->offset,
                 used, /*default_block=*/false, /*no atomic_idx */-1);

   glsl_safemem_free(vars);

   if (block->used_in_vs) ctx->num_vs_blocks += array_length;
   if (block->used_in_fs) ctx->num_fs_blocks += array_length;
   if (block->used_in_cs) ctx->num_cs_blocks += array_length;

   for (int stage=0; stage < n_stages; stage++) {
      int *map = maps[stage];
      const int *id = ids[stage];

      for (int i=0; i<array_length; i++) {
         if (id[i] != -1) map[id[i]] = block->index + i;
      }
   }

   for (int i=0; i<array_length; i++) {
      if (b->u.interface_block.layout_bind_specified) {
         GLSL_LAYOUT_BINDING_T *binding = &ctx->bindings[ctx->n_bindings++];
         binding->id = block->index + i;
         binding->binding = b->u.interface_block.layout_binding + i;
      }
   }
}

static void validate_uniforms_match(ShaderInterface *f_uniforms, ShaderInterface *v_uniforms, int link_version)
{
   /* We only need to validate uniforms if they are declared in both shaders.
    * Loop over those declared in the fshader and skip if they're not also
    * declared in the vshader.                                             */
   for (int i=0; i<f_uniforms->n_vars; i++) {
      InterfaceVar *f_unif = &f_uniforms->var[i];
      InterfaceVar *v_unif = interface_var_find(v_uniforms, f_unif->symbol->name);
      bool match;

      if (v_unif == NULL) continue;

      assert(f_unif->symbol->flavour == SYMBOL_VAR_INSTANCE ||
             f_unif->symbol->flavour == SYMBOL_INTERFACE_BLOCK);

      /* Interface blocks are not required to match for precision */
      bool match_prec = f_unif->symbol->flavour == SYMBOL_VAR_INSTANCE;
#ifndef GLSL_STRICT
      /* Some broken but important shaders use mismatched precisions. Allow it */
      match_prec = match_prec && (link_version >= GLSL_SHADER_VERSION(3, 0, 1));
#endif

      match = glsl_deep_match_nonfunction_types(f_unif->symbol->type, v_unif->symbol->type, match_prec);
      if (match_prec) {
         match = match && f_unif->symbol->u.var_instance.prec_qual ==
                          v_unif->symbol->u.var_instance.prec_qual;
      }

      if (!match) {
         // Global variables must have the same type.
         glsl_compile_error(ERROR_LINKER, 1, -1, "%s", f_unif->symbol->name);
      }

      bool f_bound, v_bound;
      int f_binding, v_binding;
      if (f_unif->symbol->flavour == SYMBOL_VAR_INSTANCE) {
         f_bound = f_unif->symbol->u.var_instance.layout_bind_specified;
         v_bound = v_unif->symbol->u.var_instance.layout_bind_specified;
         f_binding = f_unif->symbol->u.var_instance.layout_binding;
         v_binding = v_unif->symbol->u.var_instance.layout_binding;
      } else {
         f_bound = f_unif->symbol->u.interface_block.layout_bind_specified;
         v_bound = v_unif->symbol->u.interface_block.layout_bind_specified;
         f_binding = f_unif->symbol->u.interface_block.layout_binding;
         v_binding = v_unif->symbol->u.interface_block.layout_binding;
      }

      if (f_bound != v_bound)
         glsl_compile_error(ERROR_LINKER, 1, -1, "%s: binding qualification must match", f_unif->symbol->name);

      if (f_bound && f_binding != v_binding)
         glsl_compile_error(ERROR_LINKER, 1, -1, "%s: binding mismatch (%d and %d)", f_unif->symbol->name,
                                                                                     v_binding, f_binding);

   }
}

static int record_atomic(GLSL_PROGRAM_T *program, const Symbol *counters, const bool *used) {
   for (unsigned i=0; i<program->num_atomic_buffers; i++) {
      if (program->atomic_buffers[i].binding == counters->u.var_instance.layout_binding) {
         /* This buffer is already recorded, so update and return that */
         GLSL_ATOMIC_BUF_T *buf = &program->atomic_buffers[i];
         buf->used_in_vs = buf->used_in_vs || used[SHADER_VERTEX];
         buf->used_in_fs = buf->used_in_fs || used[SHADER_FRAGMENT];
         buf->used_in_cs = buf->used_in_cs || used[SHADER_COMPUTE];
         buf->size = MAX(buf->size, counters->u.var_instance.offset + 4*counters->type->scalar_count);
         return i;
      }
   }

   int idx = program->num_atomic_buffers++;
   if (program->num_atomic_buffers > GLXX_CONFIG_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS) {
      glsl_compile_error(ERROR_LINKER, 11, -1, "Too many atomic counter buffers, max %d",
                                               GLXX_CONFIG_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS);
      return -1;
   }

   GLSL_ATOMIC_BUF_T *buf = &program->atomic_buffers[idx];
   buf->binding = counters->u.var_instance.layout_binding;
   buf->used_in_vs = used[SHADER_VERTEX];
   buf->used_in_fs = used[SHADER_FRAGMENT];
   buf->used_in_cs = used[SHADER_COMPUTE];
   buf->size = counters->u.var_instance.offset + 4*counters->type->scalar_count;
   return idx;
}

static void pack_shader_uniforms(GLSL_PROGRAM_T *program,
                                 pack_blocks_t *ubo_ctx,
                                 ShaderFlavour *stages,
                                 ShaderInterface **uniforms,
                                 int **stage_maps,
                                 int n_stages)
{
   pack_uniform_t context;
   context.program     = program;
   context.max_offset  = -1;

   for (int i=0; i<n_stages; i++) {
      for (int j=0; j<uniforms[i]->n_vars; j++) {
         bool used[SHADER_FLAVOUR_COUNT] = { false, };
         int var_n_stages = 0;
         const int *ids [SHADER_FLAVOUR_COUNT] = { NULL, };
               int *maps[SHADER_FLAVOUR_COUNT] = { NULL, };

         InterfaceVar *uniform = &uniforms[i]->var[j];
         bool already_done = false;
         for (int k=0; k<i; k++) {
            if (interface_var_find(uniforms[k], uniform->symbol->name)) already_done = true;
         }
         if (already_done) continue;

         /* We know this will match for k=i. Maybe a little wasteful */
         for (int k=i; k<n_stages; k++) {
            InterfaceVar *u = interface_var_find(uniforms[k], uniform->symbol->name);
            if (u) {
               used[stages[k]]  = u->active;
               ids [var_n_stages]   = u->ids;
               maps[var_n_stages++] = stage_maps[k];
            }
         }

         // Pack.
         if (uniform->symbol->flavour == SYMBOL_VAR_INSTANCE) {
            bool any_stage_active = false;
            for (int k=0; k<SHADER_FLAVOUR_COUNT; k++)
               if (used[k]) any_stage_active = true;

            if (any_stage_active) {
               MEMBER_LAYOUT_T *layout = malloc_fast(sizeof(MEMBER_LAYOUT_T));
               calculate_non_block_layout(layout, uniform->symbol->type);
               int num_members = deep_member_count(layout);
               struct if_var_t *vars = glsl_safemem_malloc(num_members * sizeof(struct if_var_t));

               if (uniform->symbol->u.var_instance.layout_bind_specified)
                  context.binding = uniform->symbol->u.var_instance.layout_binding;
               else
                  context.binding = -1;

               int base_offset = round_up(context.max_offset + 1, layout->base_alignment/4);
               int n = enumerate_members(vars, uniform->symbol->name, layout);
               if (program->default_uniforms.num_members + n > GL20_CONFIG_MAX_UNIFORM_VECTORS)
                  glsl_compile_error(ERROR_LINKER, 5, -1, NULL);

               int atomic_idx = -1;
               if (glsl_type_contains(uniform->symbol->type, PRIM_ATOMIC_TYPE)) {
                  atomic_idx = record_atomic(program, uniform->symbol, used);
                  base_offset = uniform->symbol->u.var_instance.offset;
               }

               for (int j=0; j<n; j++) {
                  int offset;
                  if (atomic_idx == -1) offset = base_offset + vars[j].layout->offset / 4;
                  else offset = base_offset + j * 4*vars[j].type->scalar_count;
                  // Pack uniform into default block
                  int index = program->default_uniforms.num_members++;
                  GLSL_BLOCK_MEMBER_T *unif = &program->default_uniforms.members[index];
                  record_var(unif, &vars[j], offset, used, /*default_block=*/true, atomic_idx);

                  pack_var_default(&context, offset, ids, maps, var_n_stages, used, &vars[j]);
                  if (context.binding != -1) context.binding += vars[j].array_length;
               }
               glsl_safemem_free(vars);
            }
         } else {
            pack_block(ubo_ctx, uniform->symbol, ids, maps, var_n_stages, used);
         }
      }
   }
}

void pack_uniforms(GLSL_PROGRAM_T *program, ShaderInterface *v_uniforms, ShaderInterface *f_uniforms, ShaderInterface *c_uniforms, int *v_map, int *f_map, int *c_map)
{
#ifndef NDEBUG
   for (int i = 0; i < GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS; i++) {
      program->samplers[i].location = -1;
   }
#endif

   // Set up context.
   pack_blocks_t  ubo_ctx;
   ubo_ctx.records   = program->uniform_blocks;
   ubo_ctx.n_records = 0;
   ubo_ctx.bindings = program->ubo_binding;
   ubo_ctx.n_bindings = 0;
   ubo_ctx.num_vs_blocks = 0;
   ubo_ctx.num_fs_blocks = 0;
   ubo_ctx.num_cs_blocks = 0;

   ShaderFlavour    stages  [SHADER_FLAVOUR_COUNT];
   ShaderInterface *uniforms[SHADER_FLAVOUR_COUNT];
   int *stage_maps[SHADER_FLAVOUR_COUNT];
   int n_stages = 0;
   if (f_uniforms) {
      stages[n_stages] = SHADER_FRAGMENT;
      stage_maps[n_stages] = f_map;
      uniforms[n_stages++] = f_uniforms;
   }
   if (v_uniforms) {
      stages[n_stages] = SHADER_VERTEX;
      stage_maps[n_stages] = v_map;
      uniforms[n_stages++] = v_uniforms;
   }
   if (c_uniforms) {
      stages[n_stages] = SHADER_COMPUTE;
      stage_maps[n_stages] = c_map;
      uniforms[n_stages++] = c_uniforms;
   }

   /* Try to pack the uniforms for the two shaders. If both != NULL then *
    * refer to the same variable. Matching has already been validated    */
   pack_shader_uniforms(program, &ubo_ctx, stages, uniforms, stage_maps, n_stages);

   bool atomic_slot_used[GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS][GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_SIZE/4] = { { false,} , };
   for (unsigned i=0; i<program->default_uniforms.num_members; i++) {
      GLSL_BLOCK_MEMBER_T *unif = &program->default_uniforms.members[i];
      if (unif->atomic_idx != -1) {
         if (atomic_slot_used[unif->atomic_idx][unif->offset/4])
            glsl_compile_error(ERROR_LINKER, 5, -1, "Overlapping atomic counters");
         atomic_slot_used[unif->atomic_idx][unif->offset/4] = true;
      }
   }

   if (ubo_ctx.num_vs_blocks > GLXX_CONFIG_MAX_VERTEX_UNIFORM_BLOCKS)
      glsl_compile_error(ERROR_LINKER, 5, -1, "Too many vertex uniform blocks");
   if (ubo_ctx.num_fs_blocks > GLXX_CONFIG_MAX_FRAGMENT_UNIFORM_BLOCKS)
      glsl_compile_error(ERROR_LINKER, 5, -1, "Too many fragment uniform blocks");
   if (ubo_ctx.num_cs_blocks > GLXX_CONFIG_MAX_COMPUTE_UNIFORM_BLOCKS)
      glsl_compile_error(ERROR_LINKER, 5, -1, "Too many compute uniform blocks");

   program->num_uniform_blocks = ubo_ctx.n_records;
   program->num_ubo_bindings = ubo_ctx.n_bindings;
}

static void pack_shader_buffers(ShaderFlavour flavour, GLSL_PROGRAM_T *program, ShaderInterface *buffers, int *map)
{
   pack_blocks_t ctx;
   ctx.records = program->buffer_blocks;
   ctx.n_records = 0;
   ctx.bindings = program->ssbo_binding;
   ctx.n_bindings = 0;
   ctx.num_vs_blocks = 0;
   ctx.num_fs_blocks = 0;
   ctx.num_cs_blocks = 0;

   for (int i=0; i<buffers->n_vars; i++) {
      InterfaceVar *buffer = &buffers->var[i];
      assert(buffer->symbol->flavour == SYMBOL_INTERFACE_BLOCK);

      bool used[SHADER_FLAVOUR_COUNT] = { false, };
      used[flavour] = true;

      // Pack.
      const int *ids = buffer->ids;
      pack_block(&ctx, buffer->symbol, &ids, &map, 1, used);
   }

   if (ctx.num_fs_blocks > GLXX_CONFIG_MAX_FRAGMENT_SSBOS)
      glsl_compile_error(ERROR_LINKER, 5, -1, "Too many fragment buffer blocks");
   if (ctx.num_cs_blocks > GLXX_CONFIG_MAX_COMPUTE_SSBOS)
      glsl_compile_error(ERROR_LINKER, 5, -1, "Too many compute buffer blocks");

   program->num_buffer_blocks = ctx.n_records;
   program->num_ssbo_bindings = ctx.n_bindings;
}

/******** Attribute packing functions *********/

static inline int get_binding(const GLSL_PROGRAM_SOURCE_T *source, const Symbol *symbol)
{
   const char *name = symbol->name;

   assert(symbol->flavour==SYMBOL_VAR_INSTANCE);

   if(symbol->u.var_instance.layout_loc_specified) {
      return symbol->u.var_instance.layout_location;
   }

   for (unsigned i = 0; i < source->num_bindings; i++)
      if (!strcmp(source->bindings[i].name, name))
         return source->bindings[i].index;

   return -1;
}

/* An attribute takes up 1 block per "vector", this means that matrices
 * take up one block per column. mat3x2 is 3 columns of vec2s
 */
static inline int get_attribute_blocks(const SymbolType *type)
{
   assert(type->flavour == SYMBOL_PRIMITIVE_TYPE);

   if (!(primitiveTypeFlags[type->u.primitive_type.index] & PRIM_MATRIX_TYPE))
      return 1;
   else
      return glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 0);
}

static inline int get_attribute_row_offset(const SymbolType *type, int index)
{
   assert(type->flavour == SYMBOL_PRIMITIVE_TYPE);

   /* Non-matrices have only 1 column */
   if (!(primitiveTypeFlags[type->u.primitive_type.index] & PRIM_MATRIX_TYPE))
      return index;
   else {
      /* Column major ordering with each column padded to take a whole vec4 slot */
      /* col_length = n_rows */
      int col_length = glsl_prim_matrix_type_subscript_dimensions(type->u.primitive_type.index, 1);
      int col_no = index / col_length;
      int col_idx = index % col_length;
      return col_no * 4 + col_idx;
   }
}

/* The used masks are only 32 bits, so we need this */
static_assrt(GLXX_CONFIG_MAX_VERTEX_ATTRIBS <= 32);

static inline int get_attrib_mask(const SymbolType *type) {
   /* If an attribute takes n blocks, set the low n-bits of mask */
   return (1 << get_attribute_blocks(type)) - 1;
}

static void validate_attribute_loc(const Symbol *attribute, int binding, uint32_t *used_mask, int link_version)
{
   uint32_t attrib_mask;

   assert(binding >= 0 && binding < GLXX_CONFIG_MAX_VERTEX_ATTRIBS);

   if (binding + get_attribute_blocks(attribute->type) > GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
   {
      // Too many attribs or explicit binding too high for matrix
      glsl_compile_error(ERROR_LINKER, 3, -1, "%s", attribute->name);
   }

   // Fill in scalar value mask
   attrib_mask = get_attrib_mask(attribute->type) << binding;
   if (link_version == GLSL_SHADER_VERSION(3, 0, 1)) {
      /* For #version 300 es attribute aliasing is an error */
      if ((*used_mask & attrib_mask) != 0)
         glsl_compile_error(ERROR_LINKER, 3, -1, "Aliasing: %s", attribute->name);
   }
   (*used_mask) |= attrib_mask;
}

static void bind_attribute(GLSL_PROGRAM_T *program, InterfaceVar *attribute, int binding, int *id_ptr)
{
   assert(binding >= -1 && binding < GLXX_CONFIG_MAX_VERTEX_ATTRIBS);
   assert(program->num_attributes < SLANG_MAX_NUM_ATTRIBUTES);

   // Success. Fill in an attribute entry
   const int index = program->num_attributes++;
   program->attributes[index].index = binding;
   program->attributes[index].name = strdup(attribute->symbol->name);
   program->attributes[index].type = get_gl_type(attribute->symbol->type);
   program->attributes[index].is_array = false;
   program->attributes[index].array_size = 1;
   if(!program->attributes[index].name) {
      glsl_compile_error(ERROR_LINKER, 2, -1, NULL);
   }

   // For every scalar...
   for (unsigned i = 0; i < attribute->symbol->type->scalar_count; i++)
   {
      unsigned int scalar_row = binding * 4 + get_attribute_row_offset(attribute->symbol->type, i);

      // Write index to dataflow nodes.
      if (attribute->ids[i] != -1)
         id_ptr[attribute->ids[i]] = scalar_row;
   }
}

static bool is_hidden_builtin(const Symbol *s)
{
   if (!strncmp(s->name, "$$", 2)) return true;
   return false;
}

static bool is_builtin(const Symbol *s) {
   if (!strncmp(s->name, "gl_", 3)) return true;
   if (!strncmp(s->name, "$$",  2)) return true;
   return false;
}

static void bind_attributes(GLSL_PROGRAM_T *program, const GLSL_PROGRAM_SOURCE_T *source, ShaderInterface *attributes, int *id_ptr, int link_version)
{
   uint32_t attribs_used_mask = 0;

   struct locs {
      InterfaceVar *var;
      bool handled;
      int loc;
   };

   struct locs *attr_locs = glsl_safemem_malloc(attributes->n_vars * sizeof(struct locs));
   for (int i=0; i<attributes->n_vars; i++) {
      attr_locs[i].var = &attributes->var[i];
      attr_locs[i].handled = false;
      attr_locs[i].loc = 0;
   }

   /* Allocate the attributes which possess explicit bindings */
   for (int i=0; i<attributes->n_vars; i++)
   {
      int binding;
      InterfaceVar *attribute = &attributes->var[i];

      if (!attribute->active || is_builtin(attribute->symbol)) continue;

      binding = get_binding(source, attribute->symbol);

      if (binding >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
         glsl_compile_error(ERROR_LINKER, 3, -1, "Invalid explicit binding %s: %d",
                                                 attribute->symbol->name, binding);

      if (binding >= 0) {
         validate_attribute_loc(attribute->symbol, binding, &attribs_used_mask, link_version);
         attr_locs[i].handled = true;
         attr_locs[i].loc = binding;
      }
   }

   /* Then allocate the remaining active attributes. */
   for (int i=0; i<attributes->n_vars; i++)
   {
      int binding;
      InterfaceVar *attribute = &attributes->var[i];

      if (!attribute->active || is_builtin(attribute->symbol) || attr_locs[i].handled) continue;

      for (binding = 0; binding < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; binding++) {
         if ( !(attribs_used_mask & get_attrib_mask(attribute->symbol->type) << binding) )
            break;
      }

      if (binding == GLXX_CONFIG_MAX_VERTEX_ATTRIBS)
         glsl_compile_error(ERROR_LINKER, 3, -1, "%s", attribute->symbol->name);

      validate_attribute_loc(attribute->symbol, binding, &attribs_used_mask, link_version);
      attr_locs[i].handled = true;
      attr_locs[i].loc = binding;
   }

   program->num_attributes = 0;
   for (int i=0; i<attributes->n_vars; i++) {
      bool builtin = is_builtin(attr_locs[i].var->symbol);
      if (attr_locs[i].var->active && !is_hidden_builtin(attr_locs[i].var->symbol)) {
         assert(builtin || attr_locs[i].handled);
         bind_attribute(program, attr_locs[i].var, builtin ? -1 : attr_locs[i].loc, id_ptr);
      }
   }

   /* TODO: This is a bit of a hack */
   program->ir->live_attr_set = attribs_used_mask;

   glsl_safemem_free(attr_locs);
}

/******* ins/outs packing ********/

typedef struct {
   VARYING_INFO_T *varying_info;
   unsigned        num_scalar_varyings;
   int             scalar_varyings[GL20_CONFIG_MAX_VARYING_SCALARS];
   int            *f_id_ptr;
} pack_varying_t;

typedef struct {
   GLSL_VARY_MAP_T   *tf_vary_map;
   unsigned int       num_tf_captures;
   GLSL_TF_CAPTURE_T *tf_capture;
} pack_tf_t;

/* Record a varying as output from a vertex program. If the varying is also in
 * the fragment program, f_in points to it. If not (because of transform
 * feedback) then f_in is null
 */
static void pack_in_out(pack_varying_t *context, InterfaceVar *f_in, InterfaceVar *v_out)
{
   if (context->num_scalar_varyings + v_out->symbol->type->scalar_count > GL20_CONFIG_MAX_VARYING_SCALARS)
   {
      // Too many varyings.
      glsl_compile_error(ERROR_LINKER, 4, -1, "%s", v_out->symbol->name);
      return;
   }

   // For every scalar...
   for (unsigned i = 0; i < v_out->symbol->type->scalar_count; i++)
   {
      VARYING_INFO_T *vary_info = context->varying_info;
      unsigned int scalar_row = context->num_scalar_varyings;

      // Record this varying in the output arrays
      context->scalar_varyings[context->num_scalar_varyings] = v_out->ids[i];

      // Write index to dataflow nodes.
      if (f_in != NULL && f_in->ids[i] != -1) {
         context->f_id_ptr[f_in->ids[i]] = scalar_row;

         // record whether varying is centroid or flat based on type qual
         switch (f_in->symbol->u.var_instance.type_qual) {
            case TYPE_QUAL_CENTROID:
               vary_info[context->num_scalar_varyings].centroid = true; break;
            case TYPE_QUAL_FLAT:
               vary_info[context->num_scalar_varyings].flat = true; break;
            default:
               /* No action */ break;
         }
      }

      context->num_scalar_varyings++;
   }
}

/* Outputs must have matching smooth/flat, but need not have matching centroid */
#define is_flat(tq)  ((tq) == TYPE_QUAL_FLAT)
static bool glsl_match_type_quals(const Symbol *out, const Symbol *in)
{
   assert(out->flavour == SYMBOL_VAR_INSTANCE && in->flavour == SYMBOL_VAR_INSTANCE);

   if (is_flat(out->u.var_instance.type_qual) != is_flat(in->u.var_instance.type_qual))
       return false;

   return true;
}

static void validate_ins_outs_match(ShaderInterface *f_ins, ShaderInterface *v_outs) {
   /* Variables that are not referenced in the fragment shader are always fine
    * so validation need only loop over fragment shader declarations */

   for (int i=0; i<f_ins->n_vars; i++) {
      InterfaceVar *f_in = &f_ins->var[i];
      InterfaceVar *v_out = interface_var_find(v_outs, f_in->symbol->name);

      /* If the variable is not declared in the vshader... */
      if (v_out == NULL) {
         /* ... then it must be unused in the fragment shader... */
         if (f_in->active)    /* TODO: This should be checking static_use */
            if (strncmp(f_in->symbol->name, "__brcm_", 7) != 0 && strncmp(f_in->symbol->name, "gl_", 3) != 0)
               // Fragment shader uses a varying that has not been declared in the vertex shader.
               glsl_compile_error(ERROR_LINKER, 7, -1, "%s", f_in->symbol->name);

         continue;
      }

      /* ... If it is declared then typed and qualifiers must match. */
      if (!glsl_deep_match_nonfunction_types(f_in->symbol->type, v_out->symbol->type, false))
      {
         // Global variables must have the same type.
         glsl_compile_error(ERROR_LINKER, 1, -1, "%s", f_in->symbol->name);
         return;
      }

      if (!glsl_match_type_quals(v_out->symbol, f_in->symbol))
      {
         /* Global variables must have the correct type qualifiers */
         glsl_compile_error(ERROR_LINKER, 1, -1, "%s", f_in->symbol->name);
         return;
      }
   }
}

static void validate_tf_varyings(const GLSL_PROGRAM_SOURCE_T *source, ShaderInterface *v_outs)
{
   for (unsigned i=0; i<source->num_tf_varyings; i++) {
      InterfaceVar *v_out = interface_var_find(v_outs, source->tf_varyings[i]);

      if (v_out == NULL)
         glsl_compile_error(ERROR_LINKER, 9, LINE_NUMBER_UNDEFINED, "%s not a vertex output", source->tf_varyings[i]);

      if (v_out->symbol->type->flavour == SYMBOL_STRUCT_TYPE)
         glsl_compile_error(ERROR_LINKER, 9, LINE_NUMBER_UNDEFINED, "%s: struct types not valid", source->tf_varyings[i]);

      for (unsigned j=i+1; j<source->num_tf_varyings; j++) {
         if (strcmp(source->tf_varyings[i], source->tf_varyings[j]) == 0)
            glsl_compile_error(ERROR_LINKER, 9, LINE_NUMBER_UNDEFINED, "%s specified twice", source->tf_varyings[i]);
      }
   }
}

static bool tf_requires_point_size(const GLSL_PROGRAM_SOURCE_T *src) {
   for (unsigned i=0; i<src->num_tf_varyings; i++) {
      if (strcmp(src->tf_varyings[i], "gl_PointSize") == 0) return true;
   }
   return false;
}

static bool is_a_tf_varying(const GLSL_PROGRAM_SOURCE_T *source, const char *name) {
   for (unsigned i=0; i<source->num_tf_varyings; i++) {
      if (strcmp(source->tf_varyings[i], name) == 0) return true;
   }
   return false;
}

static void pack_ins_outs(pack_varying_t *context, const GLSL_PROGRAM_SOURCE_T *source, ShaderInterface *f_ins, ShaderInterface *v_outs)
{
   for (int i=0; i<v_outs->n_vars; i++) {
      InterfaceVar *v_out = &v_outs->var[i];
      InterfaceVar *f_in  = interface_var_find(f_ins, v_out->symbol->name);

      if (is_builtin(v_out->symbol)) {
         /* Skip builtins unless this is point size and TF requires point size */
         if (strcmp(v_out->symbol->name, "gl_PointSize") != 0 || !tf_requires_point_size(source))
            continue;
      }

      /* Remove inactive f_ins from consideration */
      if (f_in != NULL && !f_in->active) f_in = NULL;

      if (f_in == NULL && !is_a_tf_varying(source, v_out->symbol->name)) continue;

      pack_in_out(context, f_in, v_out);
   }
}

static void pack_tf(pack_tf_t *tf_ctx, const pack_varying_t *vary_ctx, const GLSL_PROGRAM_SOURCE_T *source, ShaderInterface *v_outs)
{
   /* Go through the vertex outputs extracting TF information */
   for (unsigned i=0; i<source->num_tf_varyings; i++) {
      InterfaceVar *v_out = interface_var_find(v_outs, source->tf_varyings[i]);
      int idx;

      idx = tf_ctx->num_tf_captures++;
      tf_ctx->tf_capture[idx].name = strdup(source->tf_varyings[i]);
      tf_ctx->tf_capture[idx].type = get_gl_type(v_out->symbol->type);
      if (v_out->symbol->type->flavour == SYMBOL_ARRAY_TYPE)
         tf_ctx->tf_capture[idx].array_length = v_out->symbol->type->u.array_type.member_count;
      else
         tf_ctx->tf_capture[idx].array_length = 1;

      if (tf_ctx->tf_capture[idx].name == NULL)
         glsl_compile_error(ERROR_LINKER, 2, -1, NULL);

      if (!strcmp(source->tf_varyings[i], "gl_Position")) continue;

      /* Create the TF varying map */
      for (unsigned j=0; j<v_out->symbol->type->scalar_count; j++) {
         int varying_id = -1;
         unsigned int i;
         GLSL_VARY_MAP_T *tf_vary_map = tf_ctx->tf_vary_map;
         /* It's possible that this loop finds a previous output of the same
          * thing, which is ok, but the varying map may contain repeats
          */
         /* TODO: I think this breaks for TF output of uninitialised variables */
         for (i=0; i<vary_ctx->num_scalar_varyings; i++) {
            if (vary_ctx->scalar_varyings[i] == v_out->ids[j]) {
               varying_id = i;
               break;
            }
         }
         assert(varying_id >= 0);

         tf_vary_map->entries[tf_vary_map->n++] = varying_id;
      }
   }
}

static void validate_fragment_location(const Symbol *s, int loc, uint32_t *used)
{
   int array_size;
   if (s->type->flavour == SYMBOL_ARRAY_TYPE) {
      array_size = s->type->u.array_type.member_count;
   } else {
      array_size = 1;
   }

   int usage_mask = ((1 << array_size) - 1) << loc;

   if (loc + array_size > GLXX_MAX_RENDER_TARGETS)
      glsl_compile_error(ERROR_LINKER, 6, -1, "Fragment output layout qualifier too large");

   if ((*used & usage_mask) != 0)
      glsl_compile_error(ERROR_LINKER, 6, -1, "Conflicting fragment output locations");

   *used |= usage_mask;
}

static void pack_fragment_output_info(const InterfaceVar *out_src, int loc,
                                      GLSL_PROGRAM_T *program)
{
   int  array_size;
   bool is_array;

   if (out_src->symbol->type->flavour == SYMBOL_ARRAY_TYPE) {
      array_size = out_src->symbol->type->u.array_type.member_count;
      is_array = true;
   } else {
      array_size = 1;
      is_array = false;
   }

   int index = program->num_frag_outputs++;
   assert(index < GLXX_MAX_RENDER_TARGETS + 1);
   program->frag_out[index].name       = strdup(out_src->symbol->name);
   program->frag_out[index].index      = loc;
   program->frag_out[index].is_array   = is_array;
   program->frag_out[index].array_size = array_size;
   program->frag_out[index].type = get_gl_type(out_src->symbol->type);
   if (!program->frag_out[index].name)
      glsl_compile_error(ERROR_LINKER, 2, -1, NULL);
}

static void pack_fragment_output_nodes(const InterfaceVar *out_src, int loc,
                                       int *out_nodes)
{
   int array_size, member_count;

   if (out_src->symbol->type->flavour == SYMBOL_ARRAY_TYPE) {
      array_size = out_src->symbol->type->u.array_type.member_count;
      member_count = out_src->symbol->type->u.array_type.member_type->scalar_count;
   } else {
      array_size = 1;
      member_count = out_src->symbol->type->scalar_count;
   }

   for (int i = 0; i < array_size; i++) {
      for (int j = 0; j < member_count; j++)
         out_nodes[DF_FNODE_R(loc + i) + j] = out_src->ids[i*member_count + j];
   }
}

void fill_vs_out_nodes(const pack_varying_t *context, ShaderInterface *v_outs, int *out_nodes)
{
   InterfaceVar *position   = interface_var_find(v_outs, "gl_Position");
   InterfaceVar *point_size = interface_var_find(v_outs, "gl_PointSize");

   out_nodes[DF_VNODE_X] = position->ids[0];
   out_nodes[DF_VNODE_Y] = position->ids[1];
   out_nodes[DF_VNODE_Z] = position->ids[2];
   out_nodes[DF_VNODE_W] = position->ids[3];
   out_nodes[DF_VNODE_POINT_SIZE] = point_size->ids[0];

   for (unsigned i = 0; i < context->num_scalar_varyings; i++) {
      out_nodes[DF_VNODE_VARY(i)] = context->scalar_varyings[i];
   }
}

static void validate_f_out(const ShaderInterface *f_outs) {
   int n = 0;
   int n_non_loc_qualed = 0;
   uint32_t loc_used = 0;
   for (int i=0; i<f_outs->n_vars; i++) {
      const Symbol *s = f_outs->var[i].symbol;
      if (is_builtin(s)) continue;

      bool loc_qual = s->u.var_instance.layout_loc_specified;
      int loc = 0;   /* Defaults to 0 if none specified */

      if (loc_qual) loc = s->u.var_instance.layout_location;

      n++;
      if (!loc_qual) n_non_loc_qualed++;

      validate_fragment_location(s, loc, &loc_used);
   }

   if (n > 1 && n_non_loc_qualed != 0)
      glsl_compile_error(ERROR_LINKER, 6, -1, "Mix of location qualified and unqualified outputs");
}

/* Extract sampler information that is needed by the driver */
static void extract_sampler_info(GLSL_PROGRAM_T *program, IRShader *sh, int *unif_map) {
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      for (int j=0; j<sh->blocks[i].num_dataflow; j++) {
         Dataflow *d = &sh->blocks[i].dataflow[j];

         if (d->flavour == DATAFLOW_CONST_SAMPLER) {
            int sampler_loc = unif_map[d->u.linkable_value.row];
            program->samplers[sampler_loc].is_32bit = d->u.const_sampler.is_32bit;
         }
      }
   }
}

/* TODO: Having 'num_outs' like this is a bit weird */
static LinkMap *allocate_link_map(CompiledShader *sh, int num_outs) {
   return glsl_link_map_alloc(interface_max_id(&sh->in) + 1, num_outs,
                              interface_max_id(&sh->uniform) + 1, interface_max_id(&sh->buffer) + 1);
}

GLSL_PROGRAM_T *glsl_link_compute_program(CompiledShader *csh) {
   GLSL_PROGRAM_T  *program = glsl_program_create();
   if(!program) return NULL;

   IR_PROGRAM_T *ir = program->ir;
   ir->flink_map = allocate_link_map(csh, 0);
   if (!ir->flink_map) goto exit_with_error;

   glsl_fastmem_init();

   // Set long jump point in case of error.
   if (setjmp(g_ErrorHandlerEnv) != 0)
   {
      // We must be jumping back from an error.
      goto exit_with_error;
   }

   /* Pack uniforms and samplers. */
   pack_uniforms(program, NULL, NULL, &csh->uniform, NULL, NULL, ir->flink_map->uniforms);
   pack_shader_buffers(SHADER_COMPUTE, program, &csh->buffer, ir->flink_map->buffers);

   ir->fshader = glsl_ir_shader_from_blocks(csh->blocks,  csh->num_cfg_blocks,
                                            csh->outputs, csh->num_outputs);
   if (ir->fshader == NULL)
      goto exit_with_error;

   extract_sampler_info(program, ir->fshader, ir->flink_map->uniforms);

   glsl_fastmem_term();

#ifndef NDEBUG
   glsl_safemem_verify();
#endif

   /* This must be last because there won't be space for anything else in
      the program when it returns */
   glsl_program_shrink(program);

   return program;

 exit_with_error:
   glsl_program_free(program);
   glsl_safemem_cleanup();
   return NULL;
}

GLSL_PROGRAM_T *glsl_link_program(CompiledShader *vsh, CompiledShader *fsh, const GLSL_PROGRAM_SOURCE_T *source)
{
   GLSL_PROGRAM_T  *program = glsl_program_create();
   if(!program) return NULL;

   glsl_fastmem_init();

   // Set long jump point in case of error.
   if (setjmp(g_ErrorHandlerEnv) != 0)
   {
      // We must be jumping back from an error.
      goto exit_with_error;
   }

   if (vsh->version != fsh->version)
      glsl_compile_error(ERROR_LINKER, 8, -1, NULL);

   /* Validate that the interface rules have been followed */
   if (fsh->version >= GLSL_SHADER_VERSION(3, 0, 1))
      validate_f_out(&fsh->out);

   validate_ins_outs_match(&fsh->in, &vsh->out);
   validate_uniforms_match(&fsh->uniform, &vsh->uniform, fsh->version);
   validate_tf_varyings(source, &vsh->out);

   /* The intention is that language rules get checked above, then we process
    * data below safe in the knowledge that we'll succeed (barring out of memory).
    * TODO: This isn't yet true and there are errors hidden in various places */
   IR_PROGRAM_T *ir = program->ir;

   ir->vlink_map = allocate_link_map(vsh, DF_BLOB_VERTEX_COUNT);
   ir->flink_map = allocate_link_map(fsh, DF_BLOB_FRAGMENT_COUNT);
   if (ir->vlink_map == NULL || ir->flink_map == NULL)
      goto exit_with_error;

   /* The out interface differs from the others because it has a fixed layout,
    * meaning we must write -1 where there are no variables. Others only have
    * link map entries where variables actually exist                         */
   for (int i=0; i<DF_BLOB_VERTEX_COUNT; i++) ir->vlink_map->outs[i] = -1;
   for (int i=0; i<DF_BLOB_FRAGMENT_COUNT; i++) ir->flink_map->outs[i] = -1;

   for (int i=0; i<fsh->out.n_vars; i++) {
      InterfaceVar *out_src = &fsh->out.var[i];
      int loc = 0;
      bool builtin = is_builtin(out_src->symbol);
      bool frag_color = (!strcmp(out_src->symbol->name, "gl_FragColor") && out_src->static_use);
      bool frag_data  = (!strcmp(out_src->symbol->name, "gl_FragData")  && out_src->static_use);

      if (is_hidden_builtin(out_src->symbol))
         continue;   // Not visible, so don't pack

      if (out_src->symbol->u.var_instance.layout_loc_specified)
         loc = out_src->symbol->u.var_instance.layout_location;

      if (!builtin || out_src->static_use)
         pack_fragment_output_info(out_src, builtin ? -1 : loc, program);

      /* This is a bit of a hacky way to allow FragColor/Data to use the same code */
      if (!builtin || frag_color || frag_data)
         pack_fragment_output_nodes(out_src, loc, ir->flink_map->outs);
   }

   InterfaceVar *discard = interface_var_find(&fsh->out, "$$discard");
   ir->flink_map->outs[DF_FNODE_DISCARD] = discard->ids[0];

   InterfaceVar *frag_depth = interface_var_find(&fsh->out, "gl_FragDepth");
   if (frag_depth != NULL && frag_depth->static_use)
      ir->flink_map->outs[DF_FNODE_DEPTH] = frag_depth->ids[0];

   /* Pack varyings. */
   pack_varying_t varying_context;
   varying_context.varying_info        = ir->varying;
   varying_context.num_scalar_varyings = 0;
   varying_context.f_id_ptr            = ir->flink_map->ins;
   pack_ins_outs(&varying_context, source, &fsh->in, &vsh->out);

   pack_tf_t tf_context;
   tf_context.tf_vary_map     = &ir->tf_vary_map;
   tf_context.num_tf_captures = 0;
   tf_context.tf_capture      = program->tf_capture;
   pack_tf(&tf_context, &varying_context, source, &vsh->out);
   program->num_tf_captures = tf_context.num_tf_captures;

   bind_attributes(program, source, &vsh->in, ir->vlink_map->ins, vsh->version);

   /* Pack uniforms and samplers. */
   pack_uniforms(program, &vsh->uniform, &fsh->uniform, NULL, ir->vlink_map->uniforms, ir->flink_map->uniforms, NULL);

   /* Pack buffer variables. Should check limits as well */
   if (vsh->buffer.n_vars > 0)
      glsl_compile_error(ERROR_LINKER, 5, -1, "Buffer variables are not allowed in the vertex shader");
   pack_shader_buffers(SHADER_FRAGMENT, program, &fsh->buffer, ir->flink_map->buffers);

   /* Fill in output nodes from vertex shader */
   fill_vs_out_nodes(&varying_context, &vsh->out, ir->vlink_map->outs);

   /* Set up the IR */
   ir->vshader = glsl_ir_shader_from_blocks(vsh->blocks,  vsh->num_cfg_blocks,
                                            vsh->outputs, vsh->num_outputs);
   ir->fshader = glsl_ir_shader_from_blocks(fsh->blocks,  fsh->num_cfg_blocks,
                                            fsh->outputs, fsh->num_outputs);
   if (ir->vshader == NULL || ir->fshader == NULL)
      goto exit_with_error;

   extract_sampler_info(program, ir->vshader, ir->vlink_map->uniforms);
   extract_sampler_info(program, ir->fshader, ir->flink_map->uniforms);

   glsl_fastmem_term();

#ifndef NDEBUG
   glsl_safemem_verify();
#endif

   /* This must be last because there won't be space for anything else in
      the program when it returns */
   glsl_program_shrink(program);

   return program;

 exit_with_error:
   glsl_program_free(program);
   glsl_safemem_cleanup();
   return NULL;
}
