/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
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
#include "glsl_mem_layout.h"
#include "glsl_ir_program.h"

#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"

#include "glsl_compiled_shader.h"
#include "glsl_dataflow_image.h"

#include "libs/util/gfx_util/gfx_util.h"

#include "../glxx/glxx_int_config.h"

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
         unreachable();
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

   unsigned int num_blocks[SHADER_FLAVOUR_COUNT];
} pack_blocks_t;

static int pack_sampler_array(pack_uniform_t *ctx, const int **ids, int **maps, int n_stages, unsigned int array_length, const SymbolType *type, unsigned unif_loc, bool in_binning)
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

      if (location >= GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS) {
         /* Too many combined samplers */
         glsl_compile_error(ERROR_LINKER, 10, -1, "Too many samplers, max %d", GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
         return -1;
      }

      ctx->program->samplers[location].location    = unif_loc + i;
      ctx->program->samplers[location].type        = get_gl_type(type);
      ctx->program->samplers[location].is_32bit    = false; /* XXX: Overridden later */
      ctx->program->samplers[location].in_binning  = in_binning;

      if (ctx->binding >= 0) {
         GLSL_LAYOUT_BINDING_T *b = &ctx->program->sampler_binding[ctx->program->num_sampler_bindings++];
         b->id = unif_loc + i;
         b->binding = ctx->binding + i;
      }
   }
   return unif_loc + array_length - 1;
}

static int pack_image_array(pack_uniform_t *ctx, const int **ids, int **maps, int n_stages, unsigned int array_length,
      const SymbolType *type, unsigned unif_loc, bool in_binning)
{
   for (unsigned i = 0; i < array_length; i++) {
      /* We're extra agressive about trimming unused images, so check each one here */
      bool s_used[SHADER_FLAVOUR_COUNT] = { false, };
      bool any_used = false;
      for (int j=0; j<n_stages; j++) {
         s_used[j] = (ids[j][i] != -1);
         any_used = any_used || s_used[j];
      }

      if (!any_used) continue;

      const unsigned location = ctx->program->num_images++;

      for (int j=0; j<n_stages; j++)
         if (s_used[j]) maps[j][ids[j][i]] = location;

      if (location >= GLXX_CONFIG_MAX_IMAGE_UNITS) {
         glsl_compile_error(ERROR_LINKER, 10, -1, "Too many images, max %d", GLXX_CONFIG_MAX_IMAGE_UNITS);
         return -1;
      }

      ctx->program->images[location].location    = unif_loc + i;
      ctx->program->images[location].type        = get_gl_type(type);
      ctx->program->images[location].is_32bit    = false; /* XXX: Overridden later */
      ctx->program->images[location].in_binning  = in_binning;
      ctx->program->images[location].internalformat   = GL_NONE; /* Overridden later */

      if (ctx->binding >= 0) {
         GLSL_LAYOUT_BINDING_T *b = &ctx->program->image_binding[ctx->program->num_image_bindings++];
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
   const MemLayout *layout;
   unsigned offset;
   unsigned array_length;
   unsigned top_level_size;
   unsigned top_level_stride;
   TypeQualifier type_qual;
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
   unif->used_in_tcs   = used[SHADER_TESS_CONTROL];
   unif->used_in_tes   = used[SHADER_TESS_EVALUATION];
   unif->used_in_gs    = used[SHADER_GEOMETRY];
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

      const MemLayout *layout = var->layout;
      if (layout->flavour == LAYOUT_ARRAY) {
         unif->array_stride = layout->u.array_layout.stride;
         /* Hacky way of recycling the code below */
         layout = layout->u.array_layout.member_layout;
      }

      bool is_matrix = !!(primitiveTypeFlags[var->type->u.primitive_type.index] & PRIM_MATRIX_TYPE);
      if (is_matrix) {
         unif->matrix_stride = layout->u.prim_matrix_layout.stride;
         unif->column_major  = !layout->u.prim_matrix_layout.row_major;
      }
   }
}

static bool used_in_binning(const bool *used) {
   return used[SHADER_VERTEX]          || used[SHADER_TESS_CONTROL] ||
          used[SHADER_TESS_EVALUATION] || used[SHADER_GEOMETRY];
}

/* If this is an array then 'type' tells us the type of array members */
static void pack_var_default(pack_uniform_t *context, int offset, const int **ids, int **maps, int n_stages, const bool *used, const struct if_var_t *var) {
   /* Now update the dataflow and, if required, the programs sampler info */
   if (glsl_prim_is_prim_sampler_type(var->type)) {
      context->max_offset = pack_sampler_array(context, ids, maps, n_stages,
                                               var->array_length, var->type,
                                               offset, used_in_binning(used));
   }
   else if (glsl_prim_is_prim_image_type(var->type)) {
      context->max_offset = pack_image_array(context, ids, maps, n_stages,
                                             var->array_length, var->type,
                                             offset, used_in_binning(used));
   }
   else {
      context->max_offset = update_uniform_link_map(ids, maps, n_stages, var->array_length, var->type, offset);
   }

   for (int i=0; i<n_stages; i++) {
      ids[i] += var->array_length * var->type->scalar_count;
   }
}

static void enumerate(struct if_var_t *var, const char *name, const SymbolType *type,
                      unsigned int array_length, const MemLayout *layout, unsigned offset)
{
   var->name = name;
   var->type = type;
   var->layout = layout;
   var->array_length = array_length;
   var->offset = offset;
   /* Fill in defaults. This is written at the top-level */
   var->top_level_size = 1;
   var->top_level_stride = 0;
}

static int enumerate_members(struct if_var_t *vars, const char *name, const SymbolType *type, const MemLayout *layout, unsigned offset)
{
   switch (type->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         enumerate(vars, name, type, 1, layout, offset);
         return 1;

      case SYMBOL_STRUCT_TYPE:
      {
         int n = 0;
         for (unsigned i = 0; i < type->u.struct_type.member_count; i++)
         {
            const char *member_name = asprintf_fast("%s.%s", name, type->u.struct_type.member[i].name);
            int o = layout->u.struct_layout.member_offsets[i];
            n += enumerate_members(&vars[n], member_name, type->u.struct_type.member[i].type, &layout->u.struct_layout.member_layouts[i], offset + o);
         }
         return n;
      }

      case SYMBOL_ARRAY_TYPE:
      {
         unsigned int array_length = type->u.array_type.member_count;
         SymbolType  *member_type  = type->u.array_type.member_type;

         if(member_type->flavour==SYMBOL_PRIMITIVE_TYPE) {
            /* Since we pack the whole array, pass layout, not member_layout */
            enumerate(vars, asprintf_fast("%s[0]", name), member_type, array_length, layout, offset);
            return 1;
         } else {
            int n = 0;
            for (unsigned i = 0; i < array_length; i++)
               n += enumerate_members(&vars[n], asprintf_fast("%s[%d]", name, i), member_type, layout->u.array_layout.member_layout, offset + i * layout->u.array_layout.stride);
            return n;
         }
      }

      case SYMBOL_BLOCK_TYPE:
      default:
         unreachable();
         return 0;
   }
}

static int enumerate_block_members(struct if_var_t *vars, const char *name, const SymbolType *type, const MemLayout *layout, bool ssbo)
{
   assert(type->flavour == SYMBOL_BLOCK_TYPE);

   int n = 0;
   for (unsigned i = 0; i < type->u.block_type.member_count; i++)
   {
      MemLayout *member_layout = &layout->u.struct_layout.member_layouts[i];
      const SymbolType *member_type = type->u.block_type.member[i].type;
      const char *member_name = type->u.block_type.member[i].name;
      if(type->u.block_type.has_named_instance)
         member_name = asprintf_fast("%s.%s", name, member_name);

      if (ssbo && member_type->flavour == SYMBOL_ARRAY_TYPE &&
          member_type->u.array_type.member_type->flavour != SYMBOL_PRIMITIVE_TYPE)
      {
         int c = enumerate_members(&vars[n], asprintf_fast("%s[0]", member_name), member_type->u.array_type.member_type,
                                   member_layout->u.array_layout.member_layout, layout->u.struct_layout.member_offsets[i]);
         for (int j=0; j<c; j++) {
            vars[n+j].top_level_size   = member_type->u.array_type.member_count;
            vars[n+j].top_level_stride = member_layout->u.array_layout.stride;
         }
         n += c;
      } else
         n += enumerate_members(&vars[n], member_name, member_type, member_layout, layout->u.struct_layout.member_offsets[i]);
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
   else                       return GLXX_CONFIG_MAX_COMBINED_STORAGE_BLOCKS;
}

static void enumerate_in_out_var(struct if_var_t *var, const char *name,
                                 const SymbolType *type, PrecisionQualifier prec,
                                 TypeQualifier qual)
{
   var->name = name;
   var->type = type;
   /* TODO: What a terrible hack: */
   var->array_length = prec;
   /* Not used: */
   var->layout = NULL;
   var->top_level_size = 0;
   var->top_level_stride = 0;
   var->type_qual = qual;
}

static int enumerate_ins_outs(struct if_var_t *vars, const char *name, const SymbolType *type,
                              PrecisionQualifier prec, TypeQualifier type_qual)
{
   switch (type->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         enumerate_in_out_var(vars, name, type, prec, type_qual);
         return 1;

      case SYMBOL_STRUCT_TYPE:
      {
         int n = 0;
         for (unsigned i = 0; i < type->u.struct_type.member_count; i++)
         {
            const char *member_name = asprintf_fast("%s.%s", name, type->u.struct_type.member[i].name);
            n += enumerate_ins_outs(&vars[n], member_name, type->u.struct_type.member[i].type, type->u.struct_type.member[i].prec, type_qual);
         }
         return n;
      }

      case SYMBOL_ARRAY_TYPE:
      {
         unsigned int array_length = type->u.array_type.member_count;
         SymbolType  *member_type  = type->u.array_type.member_type;

         if(member_type->flavour==SYMBOL_PRIMITIVE_TYPE) {
            enumerate_in_out_var(vars, name, type, prec, type_qual);
            return 1;
         } else {
            int n = 0;
            for (unsigned i = 0; i < array_length; i++)
               n += enumerate_ins_outs(&vars[n], asprintf_fast("%s[%d]", name, i), member_type, prec, type_qual);
            return n;
         }
      }

      case SYMBOL_BLOCK_TYPE:
      default:
         unreachable();
         return 0;
   }
}

static int deep_member_count(const SymbolType *type) {
   if (type->flavour == SYMBOL_PRIMITIVE_TYPE) return 1;
   else if (type->flavour == SYMBOL_ARRAY_TYPE) {
      if (type->u.array_type.member_type->flavour == SYMBOL_PRIMITIVE_TYPE) return 1;
      else {
         unsigned n = type->u.array_type.member_count > 0 ? type->u.array_type.member_count : 1;
         int count = 0;
         for (unsigned i=0; i<n; i++)
            count += deep_member_count(type->u.array_type.member_type);
         return count;
      }
   } else {
      assert(type->flavour == SYMBOL_STRUCT_TYPE || type->flavour == SYMBOL_BLOCK_TYPE);
      int count = 0;
      for (unsigned i=0; i<type->u.struct_type.member_count; i++)
         count += deep_member_count(type->u.struct_type.member[i].type);
      return count;
   }
}

/* TODO: The used information is terrible. It should be per-member-of-an-instance,
 *       but now it's per-array-of-instances */
static void pack_block(pack_blocks_t *ctx, const Symbol *b, const int **ids, int **maps, int n_stages, const bool *used)
{
   SymbolType *t = b->u.interface_block.block_data_type;
   bool is_array = (t->flavour == SYMBOL_ARRAY_TYPE);
   int array_length = is_array ? t->u.array_type.member_count : 1;
   SymbolType *type = is_array ? t->u.array_type.member_type  : t;

   assert(type->flavour==SYMBOL_BLOCK_TYPE);
   bool active = type->u.block_type.lq->unif_bits & (LAYOUT_SHARED | LAYOUT_STD140 | LAYOUT_STD430);
   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) active = active || used[i];

   if(!active) return;

   if (ctx->n_records == max_combined_block_types(b->u.interface_block.sq))
      glsl_compile_error(ERROR_LINKER, 5, -1, "Too many %s blocks, max %d",
                         glsl_storage_qual_string(b->u.interface_block.sq),
                         max_combined_block_types(b->u.interface_block.sq));

   GLSL_BLOCK_T *block  = &(ctx->records[ctx->n_records]);
   MemLayout    *layout = malloc_fast(sizeof(MemLayout));

   glsl_mem_calculate_block_layout(layout, type);

   if (layout->u.struct_layout.size > max_buffer_size(b->u.interface_block.sq))
      glsl_compile_error(ERROR_LINKER, 5, -1, "%s block %s too large: %d",
                         glsl_storage_qual_string(b->u.interface_block.sq),
                         b->name, layout->u.struct_layout.size);

   struct if_var_t *vars = glsl_safemem_malloc(deep_member_count(type) * sizeof(struct if_var_t));
   int n_members = enumerate_block_members(vars, b->name, type, layout, (b->u.interface_block.sq == STORAGE_BUFFER));

   if (ctx->n_records == 0)
      block->index = 0;
   else
      block->index = ctx->records[ctx->n_records-1].index +
                     ctx->records[ctx->n_records-1].array_length;
   block->array_length = array_length;
   block->is_array     = is_array;
   block->num_members  = n_members;
   block->members      = malloc(sizeof(*block->members) * block->num_members);
   block->size         = layout->u.struct_layout.size;
   block->name         = strdup(b->name);
   block->used_in_vs   = used[SHADER_VERTEX];
   block->used_in_tcs  = used[SHADER_TESS_CONTROL];
   block->used_in_tes  = used[SHADER_TESS_EVALUATION];
   block->used_in_gs   = used[SHADER_GEOMETRY];
   block->used_in_fs   = used[SHADER_FRAGMENT];
   block->used_in_cs   = used[SHADER_COMPUTE];

   if( !block->name || (!block->members && block->num_members > 0)) {
      glsl_compile_error(ERROR_LINKER, 2, -1, NULL);
   }

   ctx->n_records++;

   for (unsigned int i=0; i<block->num_members; i++)
      record_var(&block->members[i], &vars[i], vars[i].offset,
                 used, /*default_block=*/false, /*no atomic_idx */-1);

   glsl_safemem_free(vars);

   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++)
      if (used[i]) ctx->num_blocks[i] += array_length;

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

static void validate_uniforms_match(ShaderInterface **uniforms, int n_stages, int link_version)
{
   /* For each uniform check that any use in subsequent stages matches. *
    * Since the last stage has no subsequent stages, skip it.           */
   for (int i=0; i<n_stages-1; i++) {
      for (int j=0; j<uniforms[i]->n_vars; j++) {
         InterfaceVar *uniform = &uniforms[i]->var[j];

         assert(uniform->symbol->flavour == SYMBOL_VAR_INSTANCE ||
                uniform->symbol->flavour == SYMBOL_INTERFACE_BLOCK);

         /* Check that the first subsequent declaration matches this one */
         for (int k=i+1; k<n_stages; k++) {
            InterfaceVar *u = interface_var_find(uniforms[k], uniform->symbol->name);
            if (u == NULL) continue;

            const Symbol *u1 = uniform->symbol;
            const Symbol *u2 = u->symbol;

            bool match;
            if (u1->flavour == SYMBOL_VAR_INSTANCE) {
               bool match_prec = true;
#ifndef GLSL_STRICT
               /* Some broken but important shaders use mismatched precisions. Allow it */
               match_prec = link_version >= GLSL_SHADER_VERSION(3, 0, 1);
#endif
               match = glsl_deep_match_nonfunction_types(u1->type, u2->type, match_prec);
               if (match_prec) {
                  match = match && u1->u.var_instance.prec_qual ==
                                   u2->u.var_instance.prec_qual;
               }
            } else
               match = glsl_deep_match_nonfunction_types(u1->u.interface_block.block_data_type,
                                                         u2->u.interface_block.block_data_type, false);


            // Global variables must have the same type.
            if (!match)
               glsl_compile_error(ERROR_LINKER, 1, -1, "%s", u1->name);

            bool bound1, bound2;
            int binding1, binding2;
            if (u1->flavour == SYMBOL_VAR_INSTANCE) {
               bound1 = u1->u.var_instance.layout_bind_specified;
               bound2 = u2->u.var_instance.layout_bind_specified;
               binding1 = u1->u.var_instance.layout_binding;
               binding2 = u2->u.var_instance.layout_binding;
            } else {
               bound1 = u1->u.interface_block.layout_bind_specified;
               bound2 = u2->u.interface_block.layout_bind_specified;
               binding1 = u1->u.interface_block.layout_binding;
               binding2 = u2->u.interface_block.layout_binding;
            }

            if (bound1 != bound2)
               glsl_compile_error(ERROR_LINKER, 1, -1, "%s: binding qualification must match", u1->name);

            if (bound1 && binding1 != binding2)
               glsl_compile_error(ERROR_LINKER, 1, -1, "%s: binding mismatch (%d and %d)", u1->name,
                                                                                           binding1, binding2);

            /* We only check the first subsequent declaration */
            break;
         }
      }
   }
}

static int record_atomic(GLSL_PROGRAM_T *program, const Symbol *counters, const bool *used) {
   for (unsigned i=0; i<program->num_atomic_buffers; i++) {
      if (program->atomic_buffers[i].binding == counters->u.var_instance.layout_binding) {
         /* This buffer is already recorded, so update and return that */
         GLSL_ATOMIC_BUF_T *buf = &program->atomic_buffers[i];
         buf->used_in_vs  = buf->used_in_vs  || used[SHADER_VERTEX];
         buf->used_in_tcs = buf->used_in_tcs || used[SHADER_TESS_CONTROL];
         buf->used_in_tes = buf->used_in_tes || used[SHADER_TESS_EVALUATION];
         buf->used_in_gs  = buf->used_in_gs  || used[SHADER_GEOMETRY];
         buf->used_in_fs  = buf->used_in_fs  || used[SHADER_FRAGMENT];
         buf->used_in_cs  = buf->used_in_cs  || used[SHADER_COMPUTE];
         buf->size = gfx_umax(buf->size, counters->u.var_instance.offset + 4*counters->type->scalar_count);
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
   buf->used_in_vs  = used[SHADER_VERTEX];
   buf->used_in_tcs = used[SHADER_TESS_CONTROL];
   buf->used_in_tes = used[SHADER_TESS_EVALUATION];
   buf->used_in_gs  = used[SHADER_GEOMETRY];
   buf->used_in_fs  = used[SHADER_FRAGMENT];
   buf->used_in_cs  = used[SHADER_COMPUTE];
   buf->size = counters->u.var_instance.offset + 4*counters->type->scalar_count;
   return idx;
}

static bool location_valid(bool *loc_used, unsigned start_loc, unsigned n_locs) {
   for (unsigned i=start_loc; i < start_loc + n_locs; i++) {
      if (loc_used[i]) return false;
   }
   return true;
}

static void pack_shader_uniforms(GLSL_PROGRAM_T *program,
                                 pack_uniform_t *context,
                                 pack_blocks_t *ubo_ctx,
                                 ShaderFlavour *stages,
                                 ShaderInterface **uniforms,
                                 int **stage_maps,
                                 int n_stages,
                                 int pass_no,
                                 bool *loc_used)
{
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

         bool has_location = (uniform->symbol->flavour == SYMBOL_VAR_INSTANCE &&
                              uniform->symbol->u.var_instance.layout_loc_specified);
         unsigned location = uniform->symbol->u.var_instance.layout_location;

         /* We know this will match for k=i. Maybe a little wasteful */
         for (int k=i; k<n_stages; k++) {
            InterfaceVar *u = interface_var_find(uniforms[k], uniform->symbol->name);
            if (u) {
               used[stages[k]]  = u->active;
               ids [var_n_stages]   = u->ids;
               maps[var_n_stages++] = stage_maps[k];
               if (u->symbol->u.var_instance.layout_loc_specified) {
                  has_location = true;
                  location = u->symbol->u.var_instance.layout_location;
               }
            }
         }

         if (pass_no == 0 && !has_location) continue;
         if (pass_no != 0 && has_location) continue;

         // Pack.
         if (uniform->symbol->flavour == SYMBOL_VAR_INSTANCE) {
            bool any_stage_active = false;
            for (int k=0; k<SHADER_FLAVOUR_COUNT; k++)
               if (used[k]) any_stage_active = true;

            int num_members = deep_member_count(uniform->symbol->type);
            struct if_var_t *vars = glsl_safemem_malloc(num_members * sizeof(struct if_var_t));

            MemLayout *layout = malloc_fast(sizeof(MemLayout));
            glsl_mem_calculate_non_block_layout(layout, uniform->symbol->type);
            enumerate_members(vars, uniform->symbol->name, uniform->symbol->type, layout, 0);

            unsigned locations_required = 0;
            for (int i=0; i<num_members; i++) locations_required += vars[i].array_length;

            unsigned base_loc;
            if (has_location) {
               base_loc = location;
               if (base_loc + locations_required > GLXX_CONFIG_MAX_UNIFORM_LOCATIONS ||
                   !location_valid(loc_used, base_loc, locations_required))
               {
                  glsl_compile_error(ERROR_LINKER, 5, -1, "Invalid location for %s", uniform->symbol->name);
               }
            } else {
               for (base_loc=0; base_loc<=GLXX_CONFIG_MAX_UNIFORM_LOCATIONS - locations_required; base_loc++) {
                  if (location_valid(loc_used, base_loc, locations_required)) break;
               }
               if (base_loc > GLXX_CONFIG_MAX_UNIFORM_LOCATIONS - locations_required)
                  glsl_compile_error(ERROR_LINKER, 5, -1, "No implicit location available for %s", uniform->symbol->name);
            }
            for (unsigned i=base_loc; i<base_loc + locations_required; i++) loc_used[i] = true;

            if (any_stage_active) {
               if (uniform->symbol->u.var_instance.layout_bind_specified)
                  context->binding = uniform->symbol->u.var_instance.layout_binding;
               else
                  context->binding = -1;

               int base_offset = gfx_uround_up_p2(context->max_offset + 1, layout->base_alignment/4);
               if (program->default_uniforms.num_members + num_members > GLXX_CONFIG_MAX_UNIFORM_VECTORS)
                  glsl_compile_error(ERROR_LINKER, 5, -1, NULL);

               int atomic_idx = -1;
               if (glsl_type_contains(uniform->symbol->type, PRIM_ATOMIC_TYPE)) {
                  atomic_idx = record_atomic(program, uniform->symbol, used);
                  base_offset = uniform->symbol->u.var_instance.offset;
               }

               unsigned loc = base_loc;
               for (int j=0; j<num_members; j++) {
                  int offset;
                  int real_offset;
                  if (atomic_idx == -1) {
                     offset = loc;
                     real_offset = base_offset + vars[j].offset / 4;
                     program->uniform_offsets[loc] = real_offset;
                     program->num_uniform_offsets = gfx_umax(program->num_uniform_offsets, loc+1);
                  } else {
                     offset = base_offset + j * 4*vars[j].type->scalar_count;
                     real_offset = offset;
                  }

                  // Pack uniform into default block
                  int index = program->default_uniforms.num_members++;
                  GLSL_BLOCK_MEMBER_T *unif = &program->default_uniforms.members[index];
                  record_var(unif, &vars[j], offset, used, /*default_block=*/true, atomic_idx);

                  pack_var_default(context, real_offset, ids, maps, var_n_stages, used, &vars[j]);
                  if (context->binding != -1) context->binding += vars[j].array_length;
                  loc += vars[j].array_length;
               }
            }
            glsl_safemem_free(vars);
         } else {
            pack_block(ubo_ctx, uniform->symbol, ids, maps, var_n_stages, used);
         }
      }
   }
}

void pack_uniforms(GLSL_PROGRAM_T *program, ShaderFlavour *stages, ShaderInterface **uniforms, int **stage_maps, int n_stages)
{
#ifndef NDEBUG
   for (int i = 0; i < GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS; i++) {
      program->samplers[i].location = -1;
   }
#endif

   // Set up context.
   pack_blocks_t  ubo_ctx = { .records   = program->uniform_blocks,
                              .n_records = 0,
                              .bindings = program->ubo_binding,
                              .n_bindings = 0, };

   pack_uniform_t context;
   context.program     = program;
   context.max_offset  = -1;

   bool loc_used[GLXX_CONFIG_MAX_UNIFORM_LOCATIONS] = { false, };
   /* Try to pack the uniforms for the shaders. If both != NULL then  *
    * refer to the same variable. Matching has already been validated */
   pack_shader_uniforms(program, &context, &ubo_ctx, stages, uniforms, stage_maps, n_stages, 0, loc_used);
   pack_shader_uniforms(program, &context, &ubo_ctx, stages, uniforms, stage_maps, n_stages, 1, loc_used);

   bool atomic_slot_used[GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS][GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_SIZE/4] = { { false,} , };
   for (unsigned i=0; i<program->default_uniforms.num_members; i++) {
      GLSL_BLOCK_MEMBER_T *unif = &program->default_uniforms.members[i];
      if (unif->atomic_idx != -1) {
         if (atomic_slot_used[unif->atomic_idx][unif->offset/4])
            glsl_compile_error(ERROR_LINKER, 5, -1, "Overlapping atomic counters");
         atomic_slot_used[unif->atomic_idx][unif->offset/4] = true;
      }
   }

   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      if (ubo_ctx.num_blocks[i] > GLXX_CONFIG_MAX_SHADER_UNIFORM_BLOCKS)
         glsl_compile_error(ERROR_LINKER, 5, -1, "Too many uniform blocks");
   }

   program->num_uniform_blocks = ubo_ctx.n_records;
   program->num_ubo_bindings = ubo_ctx.n_bindings;
}

static void pack_shader_buffers(ShaderFlavour flavour, GLSL_PROGRAM_T *program, ShaderInterface *buffers, int *map)
{
   pack_blocks_t ctx = { .records = program->buffer_blocks,
                         .n_records = 0,
                         .bindings = program->ssbo_binding,
                         .n_bindings = 0, };

   for (int i=0; i<buffers->n_vars; i++) {
      InterfaceVar *buffer = &buffers->var[i];
      assert(buffer->symbol->flavour == SYMBOL_INTERFACE_BLOCK);

      bool used[SHADER_FLAVOUR_COUNT] = { false, };
      used[flavour] = true;

      // Pack.
      const int *ids = buffer->ids;
      pack_block(&ctx, buffer->symbol, &ids, &map, 1, used);
   }

   if (ctx.num_blocks[SHADER_FRAGMENT] > GLXX_CONFIG_MAX_SHADER_STORAGE_BLOCKS)
      glsl_compile_error(ERROR_LINKER, 5, -1, "Too many fragment buffer blocks");
   if (ctx.num_blocks[SHADER_COMPUTE] > GLXX_CONFIG_MAX_SHADER_STORAGE_BLOCKS)
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

static void record_inout(unsigned *count, GLSL_INOUT_T *info, const struct if_var_t *v, int index, const bool *used) {
   int  array_size;
   bool is_array;

   if (v->type->flavour == SYMBOL_ARRAY_TYPE) {
      array_size = v->type->u.array_type.member_count;
      is_array = true;
   } else {
      array_size = 1;
      is_array = false;
   }

   const unsigned i = (*count)++;
   info[i].index        = index;
   info[i].name         = strdup(v->name);
   info[i].type         = get_gl_type(v->type);
   info[i].is_array     = is_array;
   info[i].array_size   = array_size;
   info[i].used_in_vs   = used[SHADER_VERTEX];
   info[i].used_in_tcs  = used[SHADER_TESS_CONTROL];
   info[i].used_in_tes  = used[SHADER_TESS_EVALUATION];
   info[i].used_in_gs   = used[SHADER_GEOMETRY];
   info[i].used_in_fs   = used[SHADER_FRAGMENT];
   info[i].used_in_cs   = used[SHADER_COMPUTE];
   info[i].precision    = v->array_length;           /* TODO: What a terrible hack... */
   info[i].flat         = v->type_qual == TYPE_QUAL_FLAT;
   info[i].centroid     = v->type_qual == TYPE_QUAL_CENTROID;
   info[i].is_per_patch = v->type_qual == TYPE_QUAL_PATCH;
   if(!info[i].name)
      glsl_compile_error(ERROR_LINKER, 2, -1, NULL);
}

static void record_input(GLSL_PROGRAM_T *program, const InterfaceVar *attribute, int index) {
   bool used[SHADER_FLAVOUR_COUNT] = { true, false, false, false, false, false };
   struct if_var_t v;
   enumerate_in_out_var(&v, attribute->symbol->name, attribute->symbol->type,
                            attribute->symbol->u.var_instance.prec_qual, attribute->symbol->u.var_instance.type_qual);
   record_inout(&program->num_inputs, program->inputs, &v, index, used);
}

static void record_output(GLSL_PROGRAM_T *program, const InterfaceVar *out, int index) {
   bool used[SHADER_FLAVOUR_COUNT] = { false, false, false, false, true, false };
   struct if_var_t v;
   enumerate_in_out_var(&v, out->symbol->name, out->symbol->type,
                            out->symbol->u.var_instance.prec_qual, out->symbol->u.var_instance.type_qual);
   record_inout(&program->num_outputs, program->outputs, &v, index, used);
}

static void bind_attribute(InterfaceVar *attribute, int binding, int *id_ptr)
{
   assert(binding >= -1 && binding < GLXX_CONFIG_MAX_VERTEX_ATTRIBS);

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
   /* TODO: Name __brcm_LineCoord better so that we don't have to do this */
   if (!strncmp(s->name, "__brcm", 6)) return true;
   return false;
}

static bool is_builtin(const Symbol *s) {
   if (!strncmp(s->name, "gl_", 3)) return true;
   if (!strncmp(s->name, "$$",  2)) return true;
   /* TODO: Name __brcm_LineCoord better so that we don't have to do this */
   if (!strncmp(s->name, "__brcm", 6)) return true;
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

   program->num_inputs = 0;
   for (int i=0; i<attributes->n_vars; i++) {
      bool builtin = is_builtin(attr_locs[i].var->symbol);
      if ( (builtin && attr_locs[i].var->static_use && !is_hidden_builtin(attr_locs[i].var->symbol)) ||
           (!builtin && attr_locs[i].var->active)     )
      {
         assert(builtin || attr_locs[i].handled);
         int loc = builtin ? -1 : attr_locs[i].loc;
         record_input(program, attr_locs[i].var, loc);
         bind_attribute(attr_locs[i].var, loc, id_ptr);
      }
   }

   /* TODO: This is a bit of a hack */
   program->ir->live_attr_set = attribs_used_mask;

   glsl_safemem_free(attr_locs);
}

/******* ins/outs packing ********/

typedef struct {
   VARYING_INFO_T *varying_info;
   bool            any_varying_per_sample;
   unsigned        num_scalar_varyings;
   int             scalar_varyings[GLXX_CONFIG_MAX_VARYING_SCALARS];
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
static void pack_in_out(pack_varying_t *context, const InterfaceVar *f_in, const InterfaceVar *v_out)
{
   if (context->num_scalar_varyings + v_out->symbol->type->scalar_count > GLXX_CONFIG_MAX_VARYING_SCALARS)
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

      if (f_in != NULL && f_in->ids[i] != -1) {
         // Update the fragment shader link map
         context->f_id_ptr[f_in->ids[i]] = scalar_row;

         // record whether varying is centroid or flat based on type qual
         switch (f_in->symbol->u.var_instance.type_qual) {
            case TYPE_QUAL_CENTROID:
               vary_info[context->num_scalar_varyings].centroid = true; break;
            case TYPE_QUAL_FLAT:
               vary_info[context->num_scalar_varyings].flat = true; break;
            case TYPE_QUAL_SAMPLE:
               context->any_varying_per_sample = true; break;
            default:
               /* No action */ break;
         }
      }

      context->num_scalar_varyings++;
   }
}

static void copy_out(pack_varying_t *context, const InterfaceVar *v_out)
{
   if (context->num_scalar_varyings + v_out->symbol->type->scalar_count > GLXX_CONFIG_MAX_VARYING_SCALARS)
   {
      // Too many varyings.
      glsl_compile_error(ERROR_LINKER, 4, -1, "%s", v_out->symbol->name);
      return;
   }

   // For every scalar...
   for (unsigned i = 0; i < v_out->symbol->type->scalar_count; i++)
   {
      unsigned int scalar_row = context->num_scalar_varyings;

      // Record this varying in the output arrays
      context->scalar_varyings[scalar_row] = v_out->ids[i];
      context->num_scalar_varyings++;
   }
}

static void copy_in(pack_varying_t *context, const InterfaceVar *f_in)
{
   if (context->num_scalar_varyings + f_in->symbol->type->scalar_count > GLXX_CONFIG_MAX_VARYING_SCALARS)
   {
      // Too many varyings.
      glsl_compile_error(ERROR_LINKER, 4, -1, "%s", f_in->symbol->name);
      return;
   }

   // For every scalar...
   for (unsigned i = 0; i < f_in->symbol->type->scalar_count; i++)
   {
      unsigned int scalar_row = context->num_scalar_varyings;

      if (f_in->ids[i] != -1) {
         // Update the fragment shader link map
         context->f_id_ptr[f_in->ids[i]] = scalar_row;

         // record whether varying is centroid or flat based on type qual
         switch (f_in->symbol->u.var_instance.type_qual) {
            case TYPE_QUAL_CENTROID:
               context->varying_info[context->num_scalar_varyings].centroid = true; break;
            case TYPE_QUAL_FLAT:
               context->varying_info[context->num_scalar_varyings].flat = true; break;
            default:
               /* No action */ break;
         }
      }

      context->num_scalar_varyings++;
   }
}

/* Outputs must have matching smooth/flat, but need not have matching centroid */
static bool type_quals_compatible(TypeQualifier a, TypeQualifier b) {
   if ( (a == TYPE_QUAL_FLAT)  != (b == TYPE_QUAL_FLAT)  ) return false;
   if ( (a == TYPE_QUAL_PATCH) != (b == TYPE_QUAL_PATCH) ) return false;
   return true;
}

static const InterfaceVar *interface_var_find_by_location(const ShaderInterface *iface, unsigned location) {
   for (int i=0; i<iface->n_vars; i++) {
      const InterfaceVar *v = &iface->var[i];
      if (v->symbol->u.var_instance.layout_loc_specified &&
          v->symbol->u.var_instance.layout_location == location)
      {
         return v;
      }
   }
   return NULL;
}

static void validate_outs_ins_match(ShaderInterface *outs, ShaderInterface *ins, bool arrayed_input) {
   /* Variables that are not referenced in the consuming shader are always fine
    * so validation need only loop over consuming shader declarations */

   for (int i=0; i<ins->n_vars; i++) {
      const InterfaceVar *in = &ins->var[i];
      const InterfaceVar *out;

      /* The intended behaviour for some-with-locations linking is in Bugzilla 13613.
       * If one has location and the other does not then linking will fail, so it doesn't
       * matter whether it is linked by location or name. We use location.  */
      if (in->symbol->u.var_instance.layout_loc_specified)
         out = interface_var_find_by_location(outs, in->symbol->u.var_instance.layout_location);
      else
         out = interface_var_find(outs, in->symbol->name);

      /* If the variable is not declared in the producing shader... */
      if (out == NULL) {
         /* ... then it must be unused in the consuming shader (or a builtin). */
         if (in->static_use && !is_builtin(in->symbol))
            glsl_compile_error(ERROR_LINKER, 7, -1, "%s", in->symbol->name);

         continue;
      }

      SymbolType *input_type = in->symbol->type;
      if (arrayed_input) {
         if (input_type->flavour != SYMBOL_ARRAY_TYPE) {
            glsl_compile_error(ERROR_LINKER, 1, -1, "arrayed interface requires array type for %s", in->symbol->name);
            return;
         }
         input_type = input_type->u.array_type.member_type;
      }

      /* ... If it is declared then type and qualifiers must match. */
      if (!glsl_deep_match_nonfunction_types(input_type, out->symbol->type, false)) {
         // Global variables must have the same type.
         glsl_compile_error(ERROR_LINKER, 1, -1, "%s", in->symbol->name);
         return;
      }

      /* Global variables must have the correct type qualifiers */
      if (!type_quals_compatible(out->symbol->u.var_instance.type_qual, in->symbol->u.var_instance.type_qual))
         glsl_compile_error(ERROR_LINKER, 1, -1, "%s", in->symbol->name);
   }
}

struct if_var_t *find_tf_resource(struct if_var_t *resources, int n_resources, const char *name) {
   for (int i=0; i<n_resources; i++)
      if (strcmp(resources[i].name, name) == 0) return &resources[i];

   return NULL;
}

static void validate_tf_varyings(const GLSL_PROGRAM_SOURCE_T *source, ShaderInterface *outs)
{
   if (source->num_tf_varyings == 0) return;

   /* Enumerate the resources present in the interface */
   int n = 0;
   for (int i=0; i<outs->n_vars; i++) n += deep_member_count(outs->var[i].symbol->type);
   struct if_var_t *resources = glsl_safemem_malloc(n * sizeof(struct if_var_t));
   int used = 0;
   for (int i=0; i<outs->n_vars; i++) {
      const Symbol *s = outs->var[i].symbol;
      used += enumerate_ins_outs(&resources[used], s->name, s->type, s->u.var_instance.prec_qual, s->u.var_instance.type_qual);
   }

   for (unsigned i=0; i<source->num_tf_varyings; i++) {
      struct if_var_t *v = find_tf_resource(resources, n, source->tf_varyings[i]);

      if (v == NULL) {
         /* Attempt to find a version with some array suffix chopped off */
         const char *name = source->tf_varyings[i];
         const char *rbr = strrchr(name, ']');
         const char *lbr = strrchr(name, '[');
         if (!rbr || !lbr)
            glsl_compile_error(ERROR_LINKER, 9, LINE_NUMBER_UNDEFINED, "%s not a vertex output", source->tf_varyings[i]);

         char *endptr;
         int offset = strtol(lbr + 1, &endptr, 10);
         if (endptr != rbr)
            glsl_compile_error(ERROR_LINKER, 9, LINE_NUMBER_UNDEFINED, "Malformed array suffix %s", lbr);

         /* XXX It's important that no errors are thrown while the memory is held. Otherwise we need safemem alternatives */
         char *reduced_name = strdup(name);
         if (reduced_name != NULL) {
            reduced_name[lbr-name] = '\0';

            v = find_tf_resource(resources, n, reduced_name);
         }
         free(reduced_name);

         if (v == NULL)
            glsl_compile_error(ERROR_LINKER, 9, LINE_NUMBER_UNDEFINED, "%s not a vertex output", source->tf_varyings[i]);

         if (v->type->flavour != SYMBOL_ARRAY_TYPE || offset < 0 || (unsigned)offset > v->type->u.array_type.member_count)
            glsl_compile_error(ERROR_LINKER, 9, LINE_NUMBER_UNDEFINED, "Array suffix out of bounds %d", offset);
      }

      for (unsigned j=i+1; j<source->num_tf_varyings; j++) {
         if (strcmp(source->tf_varyings[i], source->tf_varyings[j]) == 0)
            glsl_compile_error(ERROR_LINKER, 9, LINE_NUMBER_UNDEFINED, "%s specified twice", source->tf_varyings[i]);
      }
   }
   glsl_safemem_free(resources);
}

static bool tf_requires_point_size(const GLSL_PROGRAM_SOURCE_T *src) {
   for (unsigned i=0; i<src->num_tf_varyings; i++) {
      if (strcmp(src->tf_varyings[i], "gl_PointSize") == 0) return true;
   }
   return false;
}

static bool is_a_tf_varying(const GLSL_PROGRAM_SOURCE_T *src, const char *name) {
   for (unsigned i=0; i<src->num_tf_varyings; i++) {
      int name_len = strlen(name);
      if (strncmp(src->tf_varyings[i], name, name_len) == 0) {
         if (name[name_len] == '\0' || name[name_len] == '.' || name[name_len] == '[')
            return true;
      }
   }
   return false;
}

static void pack_ins_outs(pack_varying_t *context, const GLSL_PROGRAM_SOURCE_T *source, ShaderInterface *f_ins, ShaderInterface *v_outs)
{
   for (int i=0; i<v_outs->n_vars; i++) {
      const InterfaceVar *v_out = &v_outs->var[i];
      const InterfaceVar *f_in;

      if (v_out->symbol->u.var_instance.layout_loc_specified)
         f_in = interface_var_find_by_location(f_ins, v_out->symbol->u.var_instance.layout_location);
      else
         f_in = interface_var_find(f_ins, v_out->symbol->name);

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

/* Fill in output variables from vertex shader */
static void copy_outs(pack_varying_t *context,const GLSL_PROGRAM_SOURCE_T *source, ShaderInterface *outs)
{
   for (int i = 0; i < outs->n_vars; i++) {
      InterfaceVar *out = &outs->var[i];

      if (is_builtin(out->symbol))
         /* Skip builtins unless this is point size and TF requires point size */
         if (strcmp(out->symbol->name, "gl_PointSize") != 0 || !tf_requires_point_size(source))
            continue;

      copy_out(context, out);
   }
}

/* Fill in input variables from fragment shader */
static void copy_ins(pack_varying_t *context, const GLSL_PROGRAM_SOURCE_T *source, ShaderInterface *ins)
{
   for (int i=0; i<ins->n_vars; i++) {
      InterfaceVar *in = &ins->var[i];

      if (is_builtin(in->symbol))
         continue;

      copy_in(context, in);
   }
}

static void pack_tf(pack_tf_t *tf_ctx, const pack_varying_t *vary_ctx, const GLSL_PROGRAM_SOURCE_T *source, ShaderInterface *outs)
{
   if (source->num_tf_varyings == 0) return;

   /* Enumerate the resources present in the interface */
   int n = 0;
   for (int i=0; i<outs->n_vars; i++) n += deep_member_count(outs->var[i].symbol->type);
   struct if_var_t *resources  = glsl_safemem_malloc(n * sizeof(struct if_var_t));
   int             *var_starts = glsl_safemem_malloc(outs->n_vars * sizeof(int));
   int used = 0;
   for (int i=0; i<outs->n_vars; i++) {
      var_starts[i] = used;
      const Symbol *s = outs->var[i].symbol;
      used += enumerate_ins_outs(&resources[used], s->name, s->type, s->u.var_instance.prec_qual, s->u.var_instance.type_qual);
   }

   /* Go through the vertex outputs extracting TF information */
   for (unsigned i=0; i<source->num_tf_varyings; i++) {
      struct if_var_t *v = find_tf_resource(resources, n, source->tf_varyings[i]);

      bool whole_array = true;
      int offset = 0;
      if (v == NULL) {
         /* Attempt to find a version with some array suffix chopped off */
         const char *name = source->tf_varyings[i];
         const char *lbr = strrchr(name, '[');

         /* XXX It's important that no errors are thrown while the memory is held. Otherwise we need a safemem_strndup */
         char *reduced_name = strdup(name);
         if (reduced_name != NULL) {
            reduced_name[lbr-name] = '\0';

            v = find_tf_resource(resources, n, reduced_name);
         }
         free(reduced_name);

         offset = strtol(lbr + 1, NULL, 10);
         whole_array = false;
      }
      int res_num = v - resources;
      int v_num;
      for (v_num = 0; v_num < outs->n_vars-1 && var_starts[v_num + 1] <= res_num; v_num++);

      InterfaceVar *out = &outs->var[v_num];

      int idx = tf_ctx->num_tf_captures++;
      tf_ctx->tf_capture[idx].name = strdup(source->tf_varyings[i]);
      tf_ctx->tf_capture[idx].type = get_gl_type(v->type);
      tf_ctx->tf_capture[idx].array_length = (whole_array && v->type->flavour == SYMBOL_ARRAY_TYPE) ? v->type->u.array_type.member_count : 1;

      if (tf_ctx->tf_capture[idx].name == NULL)
         glsl_compile_error(ERROR_LINKER, 2, -1, NULL);

      if (!strcmp(source->tf_varyings[i], "gl_Position")) continue;

      /* Create the TF varying map */
      int *ids = out->ids;
      for (int i=var_starts[v_num]; i<res_num; i++) ids += resources[i].type->scalar_count;
      int output_count = v->type->scalar_count;
      if (!whole_array) {
         output_count = v->type->u.array_type.member_type->scalar_count;
         ids += v->type->u.array_type.member_type->scalar_count * offset;
      }

      for (int j=0; j<output_count; j++) {
         int varying_id = -1;
         GLSL_VARY_MAP_T *tf_vary_map = tf_ctx->tf_vary_map;
         /* It's possible that this loop finds a previous output of the same
          * thing, which is ok, but the varying map may contain repeats
          */
         for (unsigned i=0; i<vary_ctx->num_scalar_varyings; i++) {
            if (vary_ctx->scalar_varyings[i] == ids[j]) {
               varying_id = i;
               break;
            }
         }
         assert(varying_id >= 0);

         tf_vary_map->entries[tf_vary_map->n++] = varying_id;
      }
   }
   glsl_safemem_free(resources);
   glsl_safemem_free(var_starts);
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
      glsl_compile_error(ERROR_LINKER, 4, -1, "Fragment output layout qualifier too large");

   if ((*used & usage_mask) != 0)
      glsl_compile_error(ERROR_LINKER, 4, -1, "Conflicting fragment output locations");

   *used |= usage_mask;
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

static void fill_last_vtx_out_nodes(const pack_varying_t *context, ShaderInterface *outs, int *out_nodes)
{
   InterfaceVar *position   = interface_var_find(outs, "gl_Position");
   InterfaceVar *point_size = interface_var_find(outs, "gl_PointSize");

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
      glsl_compile_error(ERROR_LINKER, 4, -1, "Mix of location qualified and unqualified fragment outputs");
}

/* Extract sampler information that is needed by the driver */
static void extract_sampler_info(GLSL_PROGRAM_T *program, IRShader *sh, int *unif_map) {
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      for (int j=0; j<sh->blocks[i].num_dataflow; j++) {
         Dataflow *d = &sh->blocks[i].dataflow[j];

         if (d->flavour == DATAFLOW_CONST_SAMPLER) {
            int sampler_loc = unif_map[d->u.linkable_value.row];

            bool is_image = (d->type == DF_FIMAGE || d->type == DF_IIMAGE || d->type == DF_UIMAGE);
            if (is_image)
            {
               program->images[sampler_loc].is_32bit = d->u.const_sampler.is_32bit;
               assert(d->u.const_sampler.format_valid);
               program->images[sampler_loc].internalformat = glsl_fmt_qualifier_to_gl_enum(d->u.const_sampler.format);
            }
            else
               program->samplers[sampler_loc].is_32bit = d->u.const_sampler.is_32bit;
         }
      }
   }
}

static void record_inout_interface(unsigned *count, GLSL_INOUT_T *info, const ShaderInterface *iface, const bool *used) {
   for (int i=0; i<iface->n_vars; i++) {
      if (iface->var[i].static_use) {
         const Symbol *s = iface->var[i].symbol;

         int n = deep_member_count(s->type);
         struct if_var_t *v = glsl_safemem_malloc(n * sizeof(struct if_var_t));
         int loc = -1;
         if (s->u.var_instance.layout_loc_specified)
            loc = s->u.var_instance.layout_location;

         enumerate_ins_outs(v, s->name, s->type, s->u.var_instance.prec_qual, s->u.var_instance.type_qual);

         for (int i=0; i<n; i++) {
            record_inout(count, info, &v[i], loc, used);
            if (loc != -1) {
               int c = s->type->flavour == SYMBOL_ARRAY_TYPE ? s->type->u.array_type.member_count : 1;
               loc += c;
            }
         }
         glsl_safemem_free(v);
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
   ir->stage[SHADER_FRAGMENT].link_map = allocate_link_map(csh, 0);
   if (!ir->stage[SHADER_FRAGMENT].link_map) goto exit_with_error;

   for (int i=0; i<ir->stage[SHADER_FRAGMENT].link_map->num_ins; i++)
      ir->stage[SHADER_FRAGMENT].link_map->ins[i] = i;

   glsl_fastmem_init();
   glsl_compile_error_reset();

   for (int i=0; i<3; i++) program->wg_size[i] = csh->wg_size[i];
   program->shared_block_size = csh->shared_block_size;

   // Set long jump point in case of error.
   if (setjmp(g_ErrorHandlerEnv) != 0) {
      // We must be jumping back from an error.
      goto exit_with_error;
   }

   /* Pack uniforms and samplers. */
   ShaderFlavour f = SHADER_COMPUTE;
   ShaderInterface *u = &csh->uniform;
   pack_uniforms(program, &f, &u, &ir->stage[SHADER_FRAGMENT].link_map->uniforms, 1);
   pack_shader_buffers(SHADER_COMPUTE, program, &csh->buffer, ir->stage[SHADER_FRAGMENT].link_map->buffers);

   ir->stage[SHADER_FRAGMENT].ir = glsl_ir_shader_from_blocks(csh->blocks,  csh->num_cfg_blocks,
                                                              csh->outputs, csh->num_outputs);
   if (ir->stage[SHADER_FRAGMENT].ir == NULL)
      goto exit_with_error;

   ir->early_fragment_tests = true;

   const static bool used[SHADER_FLAVOUR_COUNT] = { false, false, false, false, false, true };
   record_inout_interface(&program->num_inputs, program->inputs, &csh->in, used);
   extract_sampler_info(program, ir->stage[SHADER_FRAGMENT].ir, ir->stage[SHADER_FRAGMENT].link_map->uniforms);

   glsl_fastmem_term();

#ifndef NDEBUG
   glsl_safemem_verify();
#endif

   /* This must be last because there won't be space for anything else in
      the program when it returns */
   glsl_program_shrink(program);

   /* All varyings are flat. */
   for (unsigned i = 0; i != countof(ir->varying); ++i)
   {
      ir->varying[i].flat = true;
   }

   return program;

 exit_with_error:
   glsl_program_free(program);
   glsl_safemem_cleanup();
   return NULL;
}

static bool inputs_arrayed(ShaderFlavour in) {
   switch (in) {
      case SHADER_TESS_CONTROL:     return true;
      case SHADER_TESS_EVALUATION:  return false;
      case SHADER_GEOMETRY:         return true;
      case SHADER_FRAGMENT:         return false;
      default: unreachable();       return false;
   }
}

static bool stages_valid(CompiledShader **sh, bool separable) {
   bool tess_valid = ( sh[SHADER_TESS_CONTROL] &&  sh[SHADER_TESS_EVALUATION]) ||
                     (!sh[SHADER_TESS_CONTROL] && !sh[SHADER_TESS_EVALUATION]   );
   bool vf_valid = sh[SHADER_VERTEX] && sh[SHADER_FRAGMENT];
   bool any_stage_valid = sh[SHADER_VERTEX]       || sh[SHADER_FRAGMENT] ||
                          sh[SHADER_TESS_CONTROL] || sh[SHADER_GEOMETRY];
   return any_stage_valid && tess_valid && (separable || vf_valid);
}

GLSL_PROGRAM_T *glsl_link_program(CompiledShader **sh, const GLSL_PROGRAM_SOURCE_T *source, bool separable)
{
   GLSL_PROGRAM_T  *program = glsl_program_create();
   if(!program) return NULL;

   glsl_fastmem_init();
   glsl_compile_error_reset();

   assert(sh[SHADER_COMPUTE] == NULL); /* Should be sent to link_compute_program */

   // Set long jump point in case of error.
   if (setjmp(g_ErrorHandlerEnv) != 0) {
      // We must be jumping back from an error.
      goto exit_with_error;
   }

   if (!stages_valid(sh, separable))
      glsl_compile_error(ERROR_LINKER, 6, -1, NULL);

   int link_version = 0;
   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      if (sh[i] != NULL) {
         if (link_version == 0) link_version = sh[i]->version;
         if (link_version != sh[i]->version)
            glsl_compile_error(ERROR_LINKER, 8, -1, NULL);
      }
   }

   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      int next = -1;
      if (sh[i] == NULL) continue;
      for (int j=i+1; j<SHADER_FLAVOUR_COUNT; j++) {
         if (sh[j] != NULL) {
            next = j;
            break;
         }
      }
      if (next != -1) {
         bool arrayed = inputs_arrayed(next);
         validate_outs_ins_match(&sh[i]->out, &sh[next]->in, arrayed);
      }
   }

   /* Validate that the interface rules have been followed */
   if (sh[SHADER_FRAGMENT] && link_version >= GLSL_SHADER_VERSION(3, 0, 1))
      validate_f_out(&sh[SHADER_FRAGMENT]->out);

   ShaderInterface *uniform_ifaces[SHADER_FLAVOUR_COUNT];
   int n_active_stages = 0;
   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      if (sh[i] == NULL) continue;

      uniform_ifaces[n_active_stages] = &sh[i]->uniform;
      n_active_stages++;
   }
   validate_uniforms_match(uniform_ifaces, n_active_stages, link_version);

   CompiledShader *last_vtx_stage = NULL;
   for (int i=SHADER_FRAGMENT-1; i>=0; i--) {
      if (sh[i] != NULL) {
         last_vtx_stage = sh[i];
         break;
      }
   }

   if (last_vtx_stage != NULL)
      validate_tf_varyings(source, &last_vtx_stage->out);

   for (int i=0; i<SHADER_FRAGMENT; i++) {
      if (sh[i] && sh[i]->buffer.n_vars > 0)
         glsl_compile_error(ERROR_LINKER, 5, -1, "Buffer variables are not allowed in the vertex pipeline");
   }

   IR_PROGRAM_T *ir = program->ir;

   /* The intention is that language rules get checked above, then we process
    * data below safe in the knowledge that we'll succeed (barring out of memory).
    * TODO: This isn't yet true and there are errors hidden in various places */
   if (sh[SHADER_TESS_CONTROL] != NULL) {
      ir->tess_vertices = sh[SHADER_TESS_CONTROL]->tess_vertices;
   }
   if (sh[SHADER_TESS_EVALUATION] != NULL) {
      CompiledShader *te = sh[SHADER_TESS_EVALUATION];
      ir->tess_mode       = te->tess_mode;
      ir->tess_spacing    = te->tess_spacing;
      ir->tess_cw         = te->tess_cw;
      ir->tess_point_mode = te->tess_point_mode;
   }

   if (sh[SHADER_GEOMETRY] != NULL) {
      CompiledShader *gs = sh[SHADER_GEOMETRY];
      ir->gs_out = gs->gs_out;
      ir->gs_n_invocations = gs->gs_n_invocations;
      ir->gs_max_vertices  = gs->gs_max_vertices;
   }

   static const int num_outs[SHADER_FLAVOUR_COUNT] = { DF_BLOB_VERTEX_COUNT, DF_BLOB_VERTEX_COUNT,
                                                       DF_BLOB_VERTEX_COUNT, DF_BLOB_VERTEX_COUNT,
                                                       DF_BLOB_FRAGMENT_COUNT, };
   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      if (sh[i] != NULL) {
         ir->stage[i].link_map = allocate_link_map(sh[i], num_outs[i]);
         if (ir->stage[i].link_map == NULL) goto exit_with_error;

         /* The out interface differs from the others because it has a fixed layout,
          * meaning we must write -1 where there are no variables. Others only have
          * link map entries where variables actually exist                         */
         for (int j=0; j<num_outs[i]; j++) ir->stage[i].link_map->outs[j] = -1;
      }
   }

   CompiledShader *fsh = sh[SHADER_FRAGMENT];
   if (fsh) {
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
            record_output(program, out_src, builtin ? -1 : loc);

         /* This is a bit of a hacky way to allow FragColor/Data to use the same code */
         if (!builtin || frag_color || frag_data)
            pack_fragment_output_nodes(out_src, loc, ir->stage[SHADER_FRAGMENT].link_map->outs);
      }

      InterfaceVar *discard = interface_var_find(&fsh->out, "$$discard");
      ir->stage[SHADER_FRAGMENT].link_map->outs[DF_FNODE_DISCARD] = discard->ids[0];

      InterfaceVar *frag_depth = interface_var_find(&fsh->out, "gl_FragDepth");
      if (frag_depth != NULL && frag_depth->static_use)
         ir->stage[SHADER_FRAGMENT].link_map->outs[DF_FNODE_DEPTH] = frag_depth->ids[0];

      InterfaceVar *sample_mask = interface_var_find(&fsh->out, "gl_SampleMask");
      if (sample_mask != NULL && sample_mask->static_use)
         ir->stage[SHADER_FRAGMENT].link_map->outs[DF_FNODE_SAMPLE_MASK] = sample_mask->ids[0];

      ir->early_fragment_tests = fsh->early_fragment_tests;
   }

   /* Pack varyings. */
   pack_varying_t varying_context = { .varying_info = ir->varying,
                                      .any_varying_per_sample = false,
                                      .num_scalar_varyings = 0,
                                      .f_id_ptr = fsh ? ir->stage[SHADER_FRAGMENT].link_map->ins : NULL };

   if (fsh && varying_context.any_varying_per_sample)
      ir->varyings_per_sample = true;

   /* TODO: The SSO handling here is broken for tessellation. Also, there is
    * minimal code reuse which makes this look particularly terrible */
   if (last_vtx_stage && fsh) {
      pack_ins_outs(&varying_context, source, &fsh->in, &last_vtx_stage->out);
   } else if (last_vtx_stage) {
      copy_outs(&varying_context, source, &last_vtx_stage->out);
   } else if (fsh) {
      copy_ins(&varying_context, source, &fsh->in);
   }

   CompiledShader *first = NULL; /* Initialised only to silence warning */
   for (int i=0; i <= SHADER_FRAGMENT; i++) {
      if (sh[i]) { first = sh[i]; break; }
   }
   CompiledShader *last = fsh ? fsh : last_vtx_stage;

   if (first->flavour != SHADER_VERTEX) {
      bool used[SHADER_FLAVOUR_COUNT] = { false, false, false, false, false, false };
      used[first->flavour] = true;
      record_inout_interface(&program->num_inputs, program->inputs, &first->in, used);
   } else if (last->flavour != SHADER_FRAGMENT) {
      bool used[SHADER_FLAVOUR_COUNT] = { false, false, false, false, false, false};
      used[last->flavour] = true;
      record_inout_interface(&program->num_outputs, program->outputs, &last->out, used);
   }

   if (last_vtx_stage) {
      pack_tf_t tf_context;
      tf_context.tf_vary_map     = &ir->tf_vary_map;
      tf_context.num_tf_captures = 0;
      tf_context.tf_capture      = program->tf_capture;
      pack_tf(&tf_context, &varying_context, source, &last_vtx_stage->out);
      program->num_tf_captures = tf_context.num_tf_captures;
   }

   if (sh[SHADER_VERTEX]) {
      bind_attributes(program, source, &sh[SHADER_VERTEX]->in, ir->stage[SHADER_VERTEX].link_map->ins, link_version);
   }

   /* Fill in output nodes from vertex shader */
   if (last_vtx_stage)
      fill_last_vtx_out_nodes(&varying_context, &last_vtx_stage->out, ir->stage[last_vtx_stage->flavour].link_map->outs);

   /* Pack uniforms and samplers. */
   ShaderFlavour stages[SHADER_FLAVOUR_COUNT];
   ShaderInterface *uniforms[SHADER_FLAVOUR_COUNT];
   int *maps[SHADER_FLAVOUR_COUNT];
   n_active_stages = 0;
   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      if (sh[i] != NULL) {
         stages[n_active_stages] = i;
         uniforms[n_active_stages] = &sh[i]->uniform;
         maps[n_active_stages] = ir->stage[i].link_map->uniforms;
         n_active_stages++;
      }
   }
   pack_uniforms(program, stages, uniforms, maps, n_active_stages);

   /* Pack buffer variables. Only valid in the fragment shader */
   if (fsh)
      pack_shader_buffers(SHADER_FRAGMENT, program, &fsh->buffer, ir->stage[SHADER_FRAGMENT].link_map->buffers);

   /* Set up the IR */
   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      if (sh[i] != NULL) {
         ir->stage[i].ir = glsl_ir_shader_from_blocks(sh[i]->blocks,  sh[i]->num_cfg_blocks,
                                                      sh[i]->outputs, sh[i]->num_outputs);
         if (ir->stage[i].ir == NULL)
            goto exit_with_error;

         extract_sampler_info(program, ir->stage[i].ir, ir->stage[i].link_map->uniforms);
      }
   }

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
