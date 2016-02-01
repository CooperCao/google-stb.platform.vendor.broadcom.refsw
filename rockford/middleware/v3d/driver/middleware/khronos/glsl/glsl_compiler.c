/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#include "middleware/khronos/glsl/glsl_common.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>

#include "middleware/khronos/glsl/glsl_fastmem.h"
#include "middleware/khronos/glsl/glsl_errors.h"
#include "middleware/khronos/glsl/glsl_compiler.h"
#include "middleware/khronos/glsl/glsl_map.h"
#include "middleware/khronos/glsl/glsl_globals.h"
#include "middleware/khronos/glsl/glsl_symbols.h"
#include "middleware/khronos/glsl/glsl_ast.h"
#include "middleware/khronos/glsl/glsl_dataflow.h"
#include "middleware/khronos/glsl/glsl_trace.h"
#include "middleware/khronos/glsl/glsl_stringbuilder.h"
#include "middleware/khronos/glsl/glsl_stack.h"
#include "middleware/khronos/glsl/glsl_intern.h"

#include "middleware/khronos/glsl/glsl_backend.h"

// Yacc and Lex declarations.
int yyparse(void* top_level_statement);

void glsl_init_preprocessor(void);
void glsl_init_lexer(void);
void glsl_init_parser(void);

void glsl_term_lexer(void);

char error_buffer[MAX_ERROR_LENGTH];

static void compile(ShaderFlavour flavour, int sourcec, const char** sourcev)
{
   Statement* ast;

   // Save input strings.
   g_ShaderSourceCount = sourcec;
   g_ShaderSourceValues = sourcev;

   // Save compiler parameters.
   g_ShaderFlavour = flavour;

   // Clear error.
   error_buffer[0] = '\0';

   // Parse input.
   TRACE(("Compiling as %s shader...\n", g_ShaderFlavour == SHADER_VERTEX ? "vertex" : "fragment"));
   glsl_init_preprocessor();
   glsl_init_lexer();
   glsl_init_parser();
   yyparse(&ast);
   glsl_term_lexer();
   // This is a bit nasty, but means that future ast constructions
   // will not appear to be from the last line.
   g_LineNumber = LINE_NUMBER_UNDEFINED;
   TRACE(("Input parsed successfully.\n\n"));

   // Find main function definition.
   {
      Statement* shader_main = NULL;
      StatementChainNode* decl;

      for (decl = ast->u.ast.decls->first; decl; decl = decl->next)
      {
         if (STATEMENT_FUNCTION_DEF == decl->statement->flavour)
         {
            Symbol* h = decl->statement->u.function_def.header;
            if (0 == strcmp(h->name, "main"))
            {
               // This is good enough as main cannot be overloaded.
               vcos_assert(NULL == h->u.function_instance.next_overload);
               shader_main = decl->statement;
               break;
            }
         }
      }

      if (!shader_main || !shader_main->u.function_def.body)
      {
         // Missing main function definition.
         glsl_compile_error(ERROR_LINKER, 9, ast->line_num, NULL);
         return;
      }

      shader_main->u.function_def.active = true;    // stop recursive call to main
   }

   // glsl_print_statement(stdout, ast, true, 3, false);

   // Construct dataflow graph.
   // Store in the scalar values fields of the global var instance symbols.
   glsl_calculate_dataflow(ast);
}

static GLenum get_gl_type(SymbolType* type)
{
   switch (type->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         return primitiveTypesToGLenums[type->u.primitive_type.index];

      case SYMBOL_ARRAY_TYPE:
         return get_gl_type(type->u.array_type.member_type);

      case SYMBOL_STRUCT_TYPE:
         // We should have broken these up.
         // Fall.
      default:
         UNREACHABLE();
         return GL_NONE;
   }
}

// Given the context of this uniform, and its type, reduce recursively,
// until we are left with only:
// - arrays of non-sampler primitives
// - singleton non-sampler primitives (but these are treated as arrays of length 1)
// - singleton samplers
// Check whether the uniform/sampler is actually used by inspecting dataflow dependencies, and if it is:
// - for non-sampler primitive arrays:
//   - read where to put this uniform in the vertex and/or fragment shaders (from row);
//   - record this uniform in program->uniforms;
//   - update the Dataflow nodes in the vertex and/or fragment shaders;
//   - increase program->num_uniforms and row appropriately.
// - for samplers:
//   - read where to put this sampler in the fragment shader (from frag_scalar_values[0]);
//   - record this sampler in program->fsamplers;
//   - increase program->num_fsamplers appropriately.

typedef struct
{
   slang_program* program;          // state of uniforms packed so far
   Dataflow** vert_scalar_values;   // the Dataflow nodes for this uniform in the vertex shader, or NULL if not in that shader
   Dataflow** frag_scalar_values;   // the Dataflow nodes for this uniform in the fragment shader, or NULL if not in that shader
   unsigned int row;                // the next slot available for packing non-sampler uniforms
   StringBuilder sb;                // the uniform's name as qualified so far
} pack_uniform_t;

/*
   if a uniform is assigned directly to one of the special vertex shader outputs (gl_Position or gl_PointSize) it may have no
   dependents but will still need to be packed so that it is available to move into the shaded vertex structure
*/

static INLINE bool requires_forced_packing(Dataflow *dataflow)
{
   int i;

   for (i = 0; i < 4; i++)
      if (dataflow == g_BuiltInVar__gl_Position->u.var_instance.scalar_values[i])
         return true;

   if (dataflow == g_BuiltInVar__gl_PointSize->u.var_instance.scalar_values[0])
      return true;

   return false;
}

static void pack_array(pack_uniform_t* context, GLenum type, unsigned int array_length, unsigned int scalar_count, bool is_array)
{
   unsigned int i, j = 0;
   char* name;
   bool active = false;

   vcos_assert(context->vert_scalar_values || context->frag_scalar_values);

   for (i = 0; i < array_length; i++)
      for (j = 0; j < scalar_count; j++)
      {
         if (context->vert_scalar_values)
         {
            vcos_assert(DATAFLOW_UNIFORM == context->vert_scalar_values[i * scalar_count + j]->flavour || DATAFLOW_CONST_SAMPLER == context->vert_scalar_values[i * scalar_count + j]->flavour);

            if (context->vert_scalar_values[i * scalar_count + j]->dependents.count > 0 ||
                context->vert_scalar_values[i * scalar_count + j]->iodependents.count > 0 ||
                requires_forced_packing(context->vert_scalar_values[i * scalar_count + j]))
            {
               active = true;
               break;
            }
         }
         if (context->frag_scalar_values)
         {
            vcos_assert(DATAFLOW_UNIFORM == context->frag_scalar_values[i * scalar_count + j]->flavour || DATAFLOW_CONST_SAMPLER == context->frag_scalar_values[i * scalar_count + j]->flavour);

            if (context->frag_scalar_values[i * scalar_count + j]->dependents.count > 0 || context->frag_scalar_values[i * scalar_count + j]->iodependents.count > 0)
            {
               active = true;
               break;
            }
         }
      }

   if (active)
   {
      name = glsl_sb_copy_out_fast(&context->sb);

      context->program->uniforms[context->program->num_uniforms].type = type;
      context->program->uniforms[context->program->num_uniforms].u.name = name;
      context->program->uniforms[context->program->num_uniforms].row = context->row;
      context->program->uniforms[context->program->num_uniforms].array_length = array_length;
      context->program->uniforms[context->program->num_uniforms].is_array = is_array;

      for (i = 0; i < array_length; i++) {
         if (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE || type == GL_SAMPLER_EXTERNAL_OES) {
            unsigned location;

            if (context->frag_scalar_values) {
               vcos_assert(context->frag_scalar_values[i]->flavour == DATAFLOW_CONST_SAMPLER);
               location = context->frag_scalar_values[i]->u.const_sampler.location;

               if (location != (unsigned int)SAMPLER_LOCATION_UNDEFINED) {
                  vcos_assert(location < GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS);

                  context->program->samplers[location].uniform = context->program->num_uniforms;
                  context->program->samplers[location].array_index = i;
                  context->program->samplers[location].in_vshader = false;

                  if (location + 1 > context->program->num_samplers)
                     context->program->num_samplers = location + 1;
               }
               context->frag_scalar_values[i]->u.const_sampler.name = name;
            }

            if (context->vert_scalar_values) {
               vcos_assert(context->vert_scalar_values[i]->flavour == DATAFLOW_CONST_SAMPLER);
               location = context->vert_scalar_values[i]->u.const_sampler.location;

               if (location != (unsigned int)SAMPLER_LOCATION_UNDEFINED) {
                  vcos_assert(location < GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS);

                  context->program->samplers[location].uniform = context->program->num_uniforms;
                  context->program->samplers[location].array_index = i;
                  context->program->samplers[location].in_vshader = true;

                  if (location + 1 > context->program->num_samplers)
                     context->program->num_samplers = location + 1;
               }
               context->vert_scalar_values[i]->u.const_sampler.name = name;
            }

         } else {
            for (j = 0; j < scalar_count; j++)
            {
               if (context->vert_scalar_values)
               {
                  vcos_assert(context->vert_scalar_values[i * scalar_count + j]->flavour == DATAFLOW_UNIFORM);

                  context->vert_scalar_values[i * scalar_count + j]->u.linkable_value.row = context->row + i * scalar_count + j;
                  context->vert_scalar_values[i * scalar_count + j]->u.linkable_value.name = name;
               }
               if (context->frag_scalar_values)
               {
                  vcos_assert(context->frag_scalar_values[i * scalar_count + j]->flavour == DATAFLOW_UNIFORM);

                  context->frag_scalar_values[i * scalar_count + j]->u.linkable_value.row = context->row + i * scalar_count + j;
                  context->frag_scalar_values[i * scalar_count + j]->u.linkable_value.name = name;
               }
            }
         }
      }

      context->row += array_length * scalar_count;
      context->program->num_uniforms++;
   }

   // Update scalar value pointers.
   if (context->vert_scalar_values) context->vert_scalar_values += array_length * scalar_count;
   if (context->frag_scalar_values) context->frag_scalar_values += array_length * scalar_count;
}

static void pack_uniform(pack_uniform_t* context, SymbolType* type)
{
   STACK_CHECK();

   switch (type->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         {
            pack_array(context, get_gl_type(type), 1, type->scalar_count, false);
         }
         return;

      case SYMBOL_STRUCT_TYPE:
         {
            int i;

            // Recurse for every member of the struct.
            for (i = 0; i < type->u.struct_type.member_count; i++)
            {
               size_t len_delta;

               // Record current length so we can back up later.
               len_delta = context->sb.len;

               // Append field selector.
               glsl_sb_append(
                  &context->sb,
                  1 + strlen(type->u.struct_type.member_names[i]),
                  ".%s",
                  type->u.struct_type.member_names[i]);

               // Recurse.
               pack_uniform(context, type->u.struct_type.member_types[i]);

               // Back up context->sb to remove field selector.
               len_delta = context->sb.len - len_delta;
               glsl_sb_back_up(&context->sb, len_delta);
            }
         }
         return;

      case SYMBOL_ARRAY_TYPE:
         {
            if (SYMBOL_PRIMITIVE_TYPE == type->u.array_type.member_type->flavour)
            {
               // Array of primitives.
               pack_array(context, get_gl_type(type), type->u.array_type.member_count, type->u.array_type.member_type->scalar_count, true);
               return;
            }
            else
            {
               // Array of non-primitives.
               int i;

               // Recurse for every member of the array.
               for (i = 0; i < type->u.array_type.member_count; i++)
               {
                  size_t len_delta;

                  // Record current length so we can back up later.
                  len_delta = context->sb.len;

                  // Append array subscript.
                  glsl_sb_append(
                     &context->sb,
                     2 + (sizeof(i) * 3), // 2 for the brackets and 3 digits for every byte
                     "[%d]",
                     i);

                  // Recurse.
                  pack_uniform(context, type->u.array_type.member_type);

                  // Back up context->sb to remove subscript.
                  len_delta = context->sb.len - len_delta;
                  glsl_sb_back_up(&context->sb, len_delta);
               }

               return;
            }
         }
         return;

      default:
         UNREACHABLE();
         return;
   }
}

/*
   constants and non-offset linkable values assigned to varyings can't be computed directly into the appropriate row of the VRF
*/

static INLINE bool requires_mov(Dataflow *dataflow)
{
   DataflowFlavour flavour = dataflow->flavour;

   switch (flavour)
   {
      case DATAFLOW_CONST_BOOL:
      case DATAFLOW_CONST_INT:
      case DATAFLOW_CONST_FLOAT:
      case DATAFLOW_CONST_SAMPLER:
      case DATAFLOW_UNIFORM:
      case DATAFLOW_ATTRIBUTE:
      case DATAFLOW_VARYING:
         return true;

      default:
         return false;
   }
}

static INLINE int get_binding(slang_program* program, const char *name)
{
   unsigned int i;

   for (i = 0; i < program->num_bindings; i++)
      if (!strcmp(program->bindings[i].name, name))
         return program->bindings[i].index;

   return -1;
}

static INLINE int get_attribute_blocks(SymbolType *type)
{
   vcos_assert(type->flavour == SYMBOL_PRIMITIVE_TYPE);

   switch (type->u.primitive_type.index) {
   case PRIM_FLOAT:
   case PRIM_VEC2:
   case PRIM_VEC3:
   case PRIM_VEC4:
      return 1;
   case PRIM_MAT2:
      return 2;
   case PRIM_MAT3:
      return 3;
   case PRIM_MAT4:
      return 4;
   default:
      UNREACHABLE();
      return 0;
   }
}

static INLINE int get_attribute_row_offset(SymbolType *type, int index)
{
   vcos_assert(type->flavour == SYMBOL_PRIMITIVE_TYPE);

   switch (type->u.primitive_type.index) {
   case PRIM_FLOAT:
   case PRIM_VEC2:
   case PRIM_VEC3:
   case PRIM_VEC4:
      return index;
   case PRIM_MAT2:
      return index % 2 + (index / 2) * 4;
   case PRIM_MAT3:
      return index % 3 + (index / 3) * 4;
   case PRIM_MAT4:
      return index % 4 + (index / 4) * 4;
   default:
      UNREACHABLE();
      return 0;
   }
}

static INLINE int get_attribute_mask(SymbolType *type)
{
   vcos_assert(type->flavour == SYMBOL_PRIMITIVE_TYPE);

   switch (type->u.primitive_type.index) {
   case PRIM_FLOAT:
      return 0x1;
   case PRIM_VEC2:
      return 0x3;
   case PRIM_VEC3:
      return 0x7;
   case PRIM_VEC4:
      return 0xf;
   case PRIM_MAT2:
      return 0x33;
   case PRIM_MAT3:
      return 0x777;
   case PRIM_MAT4:
      return 0xffff;
   default:
      UNREACHABLE();
      return 0;
   }
}

static void bind_attribute(slang_program *program, DataflowSource* v_attribute, int binding)
{
   int i;
   int mask;

   vcos_assert(binding >= 0 && binding <= 8);

   if (binding + get_attribute_blocks(v_attribute->source->type) > 8 || program->num_attributes == SLANG_MAX_NUM_ATTRIBUTES) {
      // attribute doesn't fit into the top half of the VRF in its chosen binding position
      // or we simply have too many attributes
      glsl_compile_error(ERROR_LINKER, 4, v_attribute->initial_scalar_values[0]->line_num, NULL);
   }

   // Success. Fill in an attribute entry
   program->attributes[program->num_attributes].u.name = v_attribute->source->name;
   program->attributes[program->num_attributes].row = binding * 4;
   program->attributes[program->num_attributes].type = get_gl_type(v_attribute->source->type);
   program->num_attributes++;

   // Fill in scalar value mask
   mask = get_attribute_mask(v_attribute->source->type) << binding * 4;

   g_AttributeScalarAliasMask |= mask & g_AttributeScalarValueMask;
   g_AttributeScalarValueMask |= mask;

   // For every scalar...
   for (i = 0; i < v_attribute->source->type->scalar_count; i++)
   {
      unsigned int scalar_row = binding * 4 + get_attribute_row_offset(v_attribute->source->type, i);

      // Write index to dataflow nodes.
      v_attribute->initial_scalar_values[i]->u.linkable_value.row = scalar_row;
      v_attribute->initial_scalar_values[i]->u.linkable_value.name = v_attribute->source->name;
      // Record the slot that this attribute will be accessible from in the vertex shader.
      v_attribute->initial_scalar_values[i]->slot = DATAFLOW_SLOT_VRF | scalar_row;
   }

   v_attribute->handled = true;
}

/* NenaMark 1 has a bug with uniform locations. If we pack the uniforms *
 * backwards then it just happens to work.                              */
#define PACK_UNIFORMS_BACKWARDS

#define PACK_FIRST true
#define PACK_SECOND false
static void pack_uniforms(ShaderFlavour flavour, pack_uniform_t *context, MapNode *head_node,
                          DataflowSources* other_shader_dataflow_sources, bool pack_first)
{
   MapNode *node;

#ifdef PACK_UNIFORMS_BACKWARDS
   if (head_node->next) pack_uniforms(flavour, context, head_node->next, other_shader_dataflow_sources, pack_first);
   node = head_node;

#else
   for (node = head_node; node; node = node->next)
#endif
   {
      size_t len;
      DataflowSource *uniform       = (DataflowSource *)node->v;
      DataflowSource *other_uniform = NULL;

      vcos_assert(uniform != NULL);

      // If this is the first shader of the program then check the other shader too
      if (pack_first) {
         other_uniform = (DataflowSource *)glsl_map_get(other_shader_dataflow_sources->uniforms,
                                                        uniform->source->name, true);
         // We shouldn't have seen any variables before if this is the first pack
         vcos_assert(!uniform->handled && (!other_uniform || !other_uniform->handled));
      }

      if (other_uniform)
      {
         // Check that the types match.
         if (!glsl_deep_match_nonfunction_types(uniform->source->type, other_uniform->source->type))
         {
            // Global variables must have the same type.
            glsl_compile_error(ERROR_LINKER, 1, uniform->initial_scalar_values[0]->line_num, NULL);
            return;
         }
      }

      // If it has been seen before, skip it.
      if (uniform->handled) goto next;

      // Check it will fit.
      if (context->row + uniform->source->type->scalar_count > SLANG_UNIFORMS_LIMIT_ROW)
      {
         // Too many uniforms.
         glsl_compile_error(ERROR_LINKER, 5, uniform->initial_scalar_values[0]->line_num, NULL);
         return;
      }

      // Update context.
      glsl_sb_reset(&context->sb);
      len = strlen(uniform->source->name);
      glsl_sb_append(&context->sb, len, "%s", uniform->source->name);
      if (flavour == SHADER_FRAGMENT) {
         context->vert_scalar_values = other_uniform ? other_uniform->initial_scalar_values : NULL;
         context->frag_scalar_values = uniform->initial_scalar_values;
      } else {
         context->vert_scalar_values = uniform->initial_scalar_values;
         context->frag_scalar_values = other_uniform ? other_uniform->initial_scalar_values : NULL;
      }

      // Pack.
      pack_uniform(context, uniform->source->type);

      // Set handled flag(s).
next:
      uniform->handled = true;
      if (other_uniform) other_uniform->handled = true;
   }
}

bool glsl_compile(ShaderFlavour flavour, int sourcec, const char** sourcev)
{
   //
   // Set long jump point in case of error.
   //

   if (setjmp(g_ErrorHandlerEnv) != 0)
   {
      // We must be jumping back from an error.
      glsl_term_lexer();
      return false;
   }

   //
   // Initialize string intern facility
   //

   glsl_init_intern(1024);

   //
   // Initialize stack for dataflow graph construction
   //

   glsl_init_dataflow_stack();

   glsl_init_samplers();

   //
   // Compile vertex shader.
   //

   glsl_init_texture_lookups();

   g_AssignedFragColor = false;
   g_AssignedFragData = false;

   compile(flavour, sourcec, sourcev);

   if (g_AssignedFragColor && g_AssignedFragData) {
      glsl_compile_error(ERROR_CUSTOM, 33, -1, NULL);
      return false;
   }

   return true;
}


bool glsl_compile_and_link(slang_program* program)
{
   DataflowSources* v_dataflow_sources;
   DataflowSources* f_dataflow_sources;

   Symbol* frag_value;

   Dataflow *submit_a;
   Dataflow *submit_b;
   Dataflow *submit_g;
   Dataflow *submit_r;

   MapNode* node;

   Dataflow *zzz;
   Dataflow *submit_discard;

   //
   // Set long jump point in case of error.
   //

   if (setjmp(g_ErrorHandlerEnv) != 0)
   {
      // We must be jumping back from an error.
      glsl_term_lexer();
      return false;
   }

   //
   // Initialize string intern facility
   //

   glsl_init_intern(1024);

   //
   // Initialize stack for dataflow graph construction
   //

   glsl_init_dataflow_stack();

   glsl_init_samplers();

   //
   // Compile vertex shader.
   //

   glsl_init_texture_lookups();

   compile(SHADER_VERTEX, program->vshader.sourcec, (const char **)program->vshader.sourcev);
   v_dataflow_sources = g_DataflowSources;

   //
   // Compile fragment shader.
   //

   glsl_init_texture_lookups();

   g_AssignedFragColor = false;
   g_AssignedFragData = false;

   compile(SHADER_FRAGMENT, program->fshader.sourcec, (const char **)program->fshader.sourcev);
   f_dataflow_sources = g_DataflowSources;

   if (g_AssignedFragColor && g_AssignedFragData) {
      glsl_compile_error(ERROR_CUSTOM, 33, -1, NULL);
      return false;
   }

   if (g_AssignedFragColor)
      frag_value = g_BuiltInVar__gl_FragColor;
   else
      frag_value = g_BuiltInVar__gl_FragData;

   /*
      We need to do this here. Otherwise if uniforms are connected directly to the
      output, they will be detected as having no dependents and hence not considered
      active.
   */
   submit_a = frag_value->u.var_instance.scalar_values[3];
   submit_b = frag_value->u.var_instance.scalar_values[2];
   submit_g = frag_value->u.var_instance.scalar_values[1];
   submit_r = frag_value->u.var_instance.scalar_values[0];
   submit_discard = g_BuiltInVar__discard->u.var_instance.scalar_values[0];

   zzz = glsl_dataflow_construct_const_int(0);
   glsl_dataflow_add_iodependent(g_BuiltInVar__gl_Position->u.var_instance.scalar_values[0], zzz);
   glsl_dataflow_add_iodependent(g_BuiltInVar__gl_Position->u.var_instance.scalar_values[1], zzz);
   glsl_dataflow_add_iodependent(g_BuiltInVar__gl_Position->u.var_instance.scalar_values[2], zzz);
   glsl_dataflow_add_iodependent(g_BuiltInVar__gl_Position->u.var_instance.scalar_values[3], zzz);
   glsl_dataflow_add_iodependent(submit_r, zzz);
   glsl_dataflow_add_iodependent(submit_g, zzz);
   glsl_dataflow_add_iodependent(submit_b, zzz);
   glsl_dataflow_add_iodependent(submit_a, zzz);

   //
   // Pack varyings.
   //
   // Note that varying variables can appear in the source as:
   // - float scalars/vectors/matrices
   // - arrays of these
   {
      unsigned int row = SLANG_VARYINGS_BASE_ROW;

      // Allocate array to point to the Dataflow nodes
      // that represent the values written to each varying by the end of the vertex shader.
      // As we don't know g_VaryingScalarValueCount yet, allocate the maximum space necessary.
      g_VaryingScalarValueCount = 0;
      g_VertexVaryingScalarValues = (Dataflow **)malloc_fast(sizeof(Dataflow*) * (SLANG_VARYINGS_LIMIT_ROW - SLANG_VARYINGS_BASE_ROW));

      // For every varying variable in the fragment shader...
      for (node = f_dataflow_sources->varyings->head; node; node = node->next)
      {
         int i;
         DataflowSource* f_varying = (DataflowSource *)node->v;
         DataflowSource* v_varying = NULL;

         // Check whether any scalar component of this varying is statically referred to in the fragment shader
         bool has_static_use = false;

         for (i = 0; i < f_varying->source->type->scalar_count; i++)
         {
            if (f_varying->initial_scalar_values[i]->dependents.count > 0 || f_varying->initial_scalar_values[i]->iodependents.count > 0) {
               has_static_use = true;
               break;
            }
         }

         if (!has_static_use)
            continue;

         // Find this variable in the vertex shader.
         v_varying = (DataflowSource *)glsl_map_get(v_dataflow_sources->varyings, f_varying->source->name, true);
         if (!v_varying)
         {
            // Fragment shader uses a varying that has not been declared in the vertex shader.
            glsl_compile_error(ERROR_LINKER, 7, f_varying->initial_scalar_values[0]->line_num, NULL);
            return false;
         }

         // Assert that neither has been handled before.
         vcos_assert(!v_varying->handled && !f_varying->handled);

         // Check that the types match.
         if (!glsl_deep_match_nonfunction_types(f_varying->source->type, v_varying->source->type))
         {
            // Global variables must have the same type.
            glsl_compile_error(ERROR_LINKER, 1, f_varying->initial_scalar_values[0]->line_num, NULL);
            return false;
         }

         // Check it will fit.
         if (row + f_varying->source->type->scalar_count > SLANG_VARYINGS_LIMIT_ROW)
         {
            // Too many varyings.
            glsl_compile_error(ERROR_LINKER, 6, f_varying->initial_scalar_values[0]->line_num, NULL);
            return false;
         }

         // Success. We'll need to include this varying in the program.

         // For every scalar...
         for (i = 0; i < f_varying->source->type->scalar_count; i++)
         {
            unsigned int scalar_row = row + i;
            Dataflow *node;

            // Write index to dataflow nodes.
            v_varying->initial_scalar_values[i]->u.linkable_value.row = scalar_row;
            v_varying->initial_scalar_values[i]->u.linkable_value.name = v_varying->source->name;
            f_varying->initial_scalar_values[i]->u.linkable_value.row = scalar_row;
            f_varying->initial_scalar_values[i]->u.linkable_value.name = f_varying->source->name;

            // Record the slot that this varying will be accessible from in the fragment shader.
            // and that it must be written to in the vertex shader

            vcos_assert(f_varying->initial_scalar_values[i]->slot == -1);
            vcos_assert(v_varying->initial_scalar_values[i]->slot == -1);

            f_varying->initial_scalar_values[i]->slot = DATAFLOW_SLOT_SPM | scalar_row;
            v_varying->initial_scalar_values[i]->slot = DATAFLOW_SLOT_VRF | scalar_row;

            // Save pointer to the dataflow node that holds the last value written to the varying in the vertex shader
            // making a duplicate if necessary.

            node = v_varying->source->u.var_instance.scalar_values[i];

            if (requires_mov(node) || node->slot != -1)
               node = glsl_dataflow_construct_unary_op(DATAFLOW_MOV, node);

            node->slot = DATAFLOW_SLOT_VRF | scalar_row;

            g_VertexVaryingScalarValues[g_VaryingScalarValueCount++] = node;
         }

         // Set handled flags.
         v_varying->handled = true;
         f_varying->handled = true;

         row += i;
      }

      program->num_scalar_varyings = row - SLANG_VARYINGS_BASE_ROW;
   }

   //
   // Pack attributes.
   //
   // Note that attribute variables can appear in the source as:
   // - float scalars/vectors/matrices
   //
   // note must do this after packing varyings as only the construction of
   // the mov node tells us that a uniform used to initialise a varying is live
   {
      program->num_attributes = 0;

      g_AttributeScalarValueMask = 0;
      g_AttributeScalarAliasMask = 0;

      // Check whether any scalar component of each attribute is statically referred to in the vertex shader
      for (node = v_dataflow_sources->attributes->head; node; node = node->next)
      {
         int i;
         DataflowSource* v_attribute = (DataflowSource *)node->v;

         for (i = 0; i < v_attribute->source->type->scalar_count; i++)
         {
            if (v_attribute->initial_scalar_values[i]->dependents.count > 0 || v_attribute->initial_scalar_values[i]->iodependents.count > 0 || requires_forced_packing(v_attribute->initial_scalar_values[i])) {
               v_attribute->active = true;
               break;
            }
         }
      }

      // First allocate the attributes which possess explicit bindings
      for (node = v_dataflow_sources->attributes->head; node; node = node->next)
      {
         int binding;
         DataflowSource* v_attribute = (DataflowSource *)node->v;

         if (!v_attribute->active)
            continue;

         binding = get_binding(program, v_attribute->source->name);

         if (binding >= 0)
            bind_attribute(program, v_attribute, binding);
      }

      // Then allocate the remaining active attributes.
      // First reverse the map so we allocate in source order
      // which helps dodgy apps (e.g. see Pool Master Pro SW-8297)
      glsl_map_reverse(v_dataflow_sources->attributes);
      for (node = v_dataflow_sources->attributes->head; node; node = node->next)
      {
         int binding;
         DataflowSource* v_attribute = (DataflowSource *)node->v;

         if (!v_attribute->active || v_attribute->handled)
            continue;

         for (binding = 0; binding < 8; binding++)
            if (!(g_AttributeScalarValueMask & get_attribute_mask(v_attribute->source->type) << binding * 4))
               break;

         bind_attribute(program, v_attribute, binding);
      }

      program->live_attributes = g_AttributeScalarValueMask;
   }

   //
   // Pack uniforms and samplers.
   //
   // Note that uniform variables can appear in the source as:
   // - float/int/bool scalars/vectors/matrices
   // - samplers
   // - arrays of these
   // - structs of these
   //
   // note must do this after packing varyings as only the construction of
   // the mov node tells us that a uniform used to initialise a varying is live
   {
#ifndef NDEBUG
      int i = 0;
#endif
      pack_uniform_t context;
      program->num_uniforms = 0;
      program->num_samplers = 0;

#ifndef NDEBUG
      for (i = 0; i < GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS; i++)
         program->samplers[i].uniform = -1;
#endif

      // Set up context.
      context.program = program;
      context.row = SLANG_UNIFORMS_BASE_ROW;
      context.sb.buf = NULL; // this ensures that glsl_sb_reset() allocates a new buffer the first time round

      /* Try to pack the uniforms for the two shaders. PACK_FIRST indicates
       * that the other program hasn't been packed yet, so variables in both
       * shaders should be considered. By PACK_SECOND all such entries are done
       */
      pack_uniforms(SHADER_FRAGMENT, &context, f_dataflow_sources->uniforms->head, v_dataflow_sources, PACK_FIRST);
      pack_uniforms(SHADER_VERTEX, &context, v_dataflow_sources->uniforms->head, NULL, PACK_SECOND);

      program->num_scalar_uniforms = context.row - SLANG_UNIFORMS_BASE_ROW;
   }

#ifdef BUILD_FOR_DAVES_TEST
#define COMPILER_LISTATTRIBUTEANDUNIFORMLOCATIONSONSTDOUT
#endif
#ifdef COMPILER_LISTATTRIBUTEANDUNIFORMLOCATIONSONSTDOUT
   {
      int a;
     for(a=0;a<program->num_attributes;a++)
     {
        struct slang_attribute_s *attribute = &program->attributes[a];
       printf("; Attribute: %d %s %d\n", attribute->type, attribute->name, attribute->row);
     }
   }
   {
      int u;
     for(u=0;u<program->num_uniforms;u++)
     {
        struct slang_uniform_s *uniform = &program->uniforms[u];
       printf("; Uniform: %d %s %d %d\n", uniform->type, uniform->name, uniform->row, uniform->array_length);
     }
   }
#endif

   return glsl_backend_create_shaders(
      program,
      g_BuiltInVar__gl_Position->u.var_instance.scalar_values[0],
      g_BuiltInVar__gl_Position->u.var_instance.scalar_values[1],
      g_BuiltInVar__gl_Position->u.var_instance.scalar_values[2],
      g_BuiltInVar__gl_Position->u.var_instance.scalar_values[3],
      g_BuiltInVar__gl_PointSize->u.var_instance.scalar_values[0],
      g_VertexVaryingScalarValues,
      g_VaryingScalarValueCount,
      submit_r,
      submit_g,
      submit_b,
      submit_a,
      submit_discard);
}

void yyerror(char *s)
{
   if (0 == strcmp(s, "syntax error"))
   {
      // Catch syntax errors and redirect them.
      glsl_compile_error(ERROR_LEXER_PARSER, 1, g_LineNumber, NULL);
   }
   else
   {
      glsl_compile_error(ERROR_UNKNOWN, 0, g_LineNumber, s);
   }
}

/*
   glsl_compiler_exit

   Exits compilation, presumably due to an error.

   Implementation notes:

   Uses longjmp

   Preconditions:
   Compilation is in progress

   Postconditions:
   Does not return normally

   Invariants preserved:
   -
*/

void glsl_compiler_exit(bool bad_tree)
{
   longjmp(g_ErrorHandlerEnv, bad_tree ? GLSL_EXIT_WITH_BAD_TREE : GLSL_EXIT);
}
