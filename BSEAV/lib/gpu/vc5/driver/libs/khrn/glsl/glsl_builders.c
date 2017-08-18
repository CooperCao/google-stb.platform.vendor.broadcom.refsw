/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "glsl_common.h"
#include "glsl_builders.h"
#include "glsl_errors.h"
#include "glsl_globals.h"
#include "glsl_fastmem.h"
#include "glsl_ast.h"
#include "glsl_ast_print.h"
#include "glsl_intern.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"
#include "glsl_mem_layout.h"
#include "glsl_precision.h"
#include "glsl_symbol_table.h"
#include "glsl_stringbuilder.h"
#include "glsl_quals.h"
#include "glsl_extensions.h"

#include "../glxx/glxx_int_config.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

void glsl_error_if_type_incomplete(SymbolType *type) {
   while (type->flavour == SYMBOL_ARRAY_TYPE) {
      if(type->u.array_type.member_count==0)
         glsl_compile_error(ERROR_CUSTOM, 24, g_LineNumber, NULL);
      type = type->u.array_type.member_type;
   }
}

static void glsl_check_namespace_nonfunction(SymbolTable *table, const char *name)
{
   Symbol *sym = glsl_symbol_table_lookup_in_current_scope(table, name);
   if (sym)
      glsl_compile_error(ERROR_SEMANTIC, 13, g_LineNumber, "%s", name);
}

uint32_t glsl_layout_combine_block_bits(uint32_t a, uint32_t b) {
   uint32_t packing_mask = LAYOUT_PACKED | LAYOUT_SHARED | LAYOUT_STD140 | LAYOUT_STD430;
   uint32_t matrix_order_mask = LAYOUT_ROW_MAJOR | LAYOUT_COLUMN_MAJOR;

   if(b & matrix_order_mask) {
      a &= ~matrix_order_mask;
      a |= (b & matrix_order_mask);
   }

   if(b & packing_mask) {
      a &= ~packing_mask;
      a |= (b & packing_mask);
   }
   return a;
}

LayoutQualifier *glsl_create_mixed_lq(const LayoutQualifier *lq_local, const LayoutQualifier *lq_global)
{
   LayoutQualifier *res = malloc_fast(sizeof(LayoutQualifier));
   memcpy(res, lq_global, sizeof(LayoutQualifier));

   if(!lq_local) return res;

   if(lq_local->qualified & LOC_QUALED) {
      res->qualified |= LOC_QUALED;
      res->location = lq_local->location;
   }

   if(lq_local->qualified & BINDING_QUALED) {
      res->qualified |= BINDING_QUALED;
      res->binding = lq_local->binding;
   }

   if(lq_local->qualified & OFFSET_QUALED) {
      res->qualified |= OFFSET_QUALED;
      res->offset = lq_local->offset;
   }

   if(lq_local->qualified & UNIF_QUALED) {
      res->qualified |= UNIF_QUALED;
      res->unif_bits = glsl_layout_combine_block_bits(res->unif_bits, lq_local->unif_bits);
   }

   return res;
}

void glsl_insert_function_definition(Statement *statement)
{
   assert(statement->flavour == STATEMENT_FUNCTION_DEF);
   Symbol *header_sym = statement->u.function_def.header;

   if (header_sym->u.function_instance.function_def)
      glsl_compile_error(ERROR_SEMANTIC, 13, g_LineNumber, "function %s already defined", header_sym->name);

   header_sym->u.function_instance.function_def = statement;
}

void glsl_commit_struct_type(SymbolTable *table, SymbolType *type)
{
   Symbol *symbol = malloc_fast(sizeof(Symbol));
   glsl_symbol_construct_type(symbol, type);
   glsl_check_namespace_nonfunction(table, type->name);
   glsl_symbol_table_insert(table, symbol);
}

static void check_singleton_is_instantiable(const char *name, const SymbolType *type)
{
   // Cannot instantiate void type.
   if (type == &primitiveTypes[PRIM_VOID])
      glsl_compile_error(ERROR_CUSTOM, 18, g_LineNumber, NULL);

   // Cannot instantiate symbol whose name begins "gl_"
   if (!strncmp(name, "gl_", 3))
      glsl_compile_error(ERROR_CUSTOM, 14, g_LineNumber, NULL);
}

static SymbolType *make_array_type_internal(SymbolType *member, unsigned size) {
   SymbolType *type                = malloc_fast(sizeof(SymbolType));
   type->flavour                   = SYMBOL_ARRAY_TYPE;
   type->scalar_count              = size * member->scalar_count;
   type->u.array_type.member_type  = member;
   type->u.array_type.member_count = size;

   if (size != 0)
      type->name = asprintf_fast("%s[%d]", member->name, type->u.array_type.member_count);
   else
      type->name = asprintf_fast("%s[]", member->name);

   return type;
}

void glsl_complete_array_type(SymbolType *type, int member_count)
{
   assert(type->flavour==SYMBOL_ARRAY_TYPE);

   SymbolType *member_type = type->u.array_type.member_type;
   // Array size must be greater than zero.
   if (member_count <= 0)
      glsl_compile_error(ERROR_SEMANTIC, 11, g_LineNumber, NULL);

   assert(type->u.array_type.member_count == 0 ||
          type->u.array_type.member_count == (unsigned)member_count);

   // Re-write the type fields that we've updated
   type->name = glsl_intern(asprintf_fast("%s[%d]", member_type->name, member_count), false);
   type->u.array_type.member_count = member_count;
   type->scalar_count = member_count*member_type->scalar_count;
}

/* build preliminary (perhaps incomplete) type */
static SymbolType *make_array_type(SymbolType *member_type, Expr *size)
{
   if (member_type == &primitiveTypes[PRIM_VOID])
      glsl_compile_error(ERROR_CUSTOM, 18, g_LineNumber, NULL);

   // In version 100 members cannot be arrays: multi-dimensional array not allowed
   if (g_ShaderVersion < GLSL_SHADER_VERSION(3, 10, 1) && member_type->flavour == SYMBOL_ARRAY_TYPE)
      glsl_compile_error(ERROR_CUSTOM, 23, g_LineNumber, NULL);

   /* You can make arrays without specifying a size, but if there is one, check it */
   if (size != NULL) {
      if (size->type != &primitiveTypes[PRIM_INT] && size->type != &primitiveTypes[PRIM_UINT])
         glsl_compile_error(ERROR_SEMANTIC, 10, g_LineNumber, "found type %s", size->type->name);
      if (!size->compile_time_value)
         glsl_compile_error(ERROR_SEMANTIC, 10, g_LineNumber, NULL);
      if (size->type == &primitiveTypes[PRIM_INT] && (int)size->compile_time_value[0] <= 0)
         glsl_compile_error(ERROR_SEMANTIC, 11, g_LineNumber, "%d", (int)size->compile_time_value[0]);
      if (size->type == &primitiveTypes[PRIM_UINT] && size->compile_time_value[0] == 0)
         glsl_compile_error(ERROR_SEMANTIC, 11, g_LineNumber, "%u", size->compile_time_value[0]);
   }

   return make_array_type_internal(member_type, size ? size->compile_time_value[0] : 0);
}

/* build preliminary (perhaps incomplete) type */
SymbolType *glsl_build_array_type(SymbolType *member_type, ExprChain *array_specifier) {
   SymbolType *ret = member_type;
   for (ExprChainNode *n = array_specifier->last; n != NULL; n = n->prev) {
      ret = make_array_type(ret, n->expr);
   }
   return ret;
}

static bool is_array_of_blocks(const SymbolType *type) {
   while (type->flavour == SYMBOL_ARRAY_TYPE) type = type->u.array_type.member_type;
   return type->flavour == SYMBOL_BLOCK_TYPE;
}

static const char *extract_block_name(const SymbolType *type) {
   while (type->flavour == SYMBOL_ARRAY_TYPE) type = type->u.array_type.member_type;
   return type->name;
}

static bool storage_valid_for_block(StorageQualifier sq) {
   if (sq == STORAGE_UNIFORM) return g_ShaderVersion >= GLSL_SHADER_VERSION(3,  0, 1);
   if (sq == STORAGE_BUFFER)  return g_ShaderVersion >= GLSL_SHADER_VERSION(3, 10, 1);
   if (sq == STORAGE_IN || sq == STORAGE_OUT) return g_ShaderVersion >= GLSL_SHADER_VERSION(3, 20, 1) ||
                                                     glsl_ext_status(GLSL_EXT_IO_BLOCKS) != GLSL_DISABLED;
   return false;
}

static int max_block_binding(StorageQualifier sq) {
   assert(sq == STORAGE_UNIFORM || sq == STORAGE_BUFFER);
   if (sq == STORAGE_UNIFORM) return GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS - 1;
   else                       return GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS - 1;
}

static void complete_array_type_from_defaults(SymbolType *type, StorageQualifier sq, DeclDefaultState *dflt) {
   if (sq != STORAGE_IN && sq != STORAGE_OUT) return;

   unsigned d = (sq == STORAGE_IN) ? dflt->input_size : dflt->output_size;
   if (type->u.array_type.member_count == 0 && d > 0)
      glsl_complete_array_type(type, d);
}

Symbol *glsl_commit_block_type(SymbolTable *table, DeclDefaultState *dflt, SymbolType *type, Qualifiers *quals)
{
   if(!storage_valid_for_block(quals->sq))
      glsl_compile_error(ERROR_CUSTOM, 4, g_LineNumber, "interface %s does not support blocks",
                                                        glsl_storage_qual_string(quals->sq));

   if (quals->sq == STORAGE_IN  && g_ShaderFlavour == SHADER_VERTEX)
      glsl_compile_error(ERROR_CUSTOM, 4, g_LineNumber, "vertex shaders do not support 'in' blocks");
   if (quals->sq == STORAGE_OUT && g_ShaderFlavour == SHADER_FRAGMENT)
      glsl_compile_error(ERROR_CUSTOM, 4, g_LineNumber, "fragment shaders do not support 'out' blocks");

   if (type->flavour == SYMBOL_ARRAY_TYPE &&
       type->u.array_type.member_type->flavour == SYMBOL_ARRAY_TYPE)
   {
      glsl_compile_error(ERROR_CUSTOM, 4, g_LineNumber, "Interface blocks cannot be arrays of arrays");
   }

   assert(is_array_of_blocks(type));

   const char *block_name = extract_block_name(type);

   if (type->flavour == SYMBOL_ARRAY_TYPE)
      complete_array_type_from_defaults(type, quals->sq, dflt);

   glsl_error_if_type_incomplete(type);

   check_singleton_is_instantiable(block_name, type);
   glsl_check_namespace_nonfunction(table, block_name);

   if (quals->lq) {
      if (quals->lq->qualified & LOC_QUALED && !(quals->sq == STORAGE_IN || quals->sq == STORAGE_OUT))
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "location qualifier may not be applied to %s blocks",
                                                            glsl_storage_qual_string(quals->sq));

      if (quals->lq->qualified & FORMAT_QUALED)
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "format qualifiers may not be applied to blocks");

      if (quals->lq->qualified & OFFSET_QUALED)
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "offset qualifier may not be applied to blocks");

      if (quals->lq->qualified & BINDING_QUALED) {
         if (g_ShaderVersion < GLSL_SHADER_VERSION(3, 10, 1))
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "binding qualifier not allowed in this language version");

         int binding = quals->lq->binding;
         int member_count = (type->flavour == SYMBOL_ARRAY_TYPE) ? type->u.array_type.member_count : 1;

         if (binding + member_count-1 > max_block_binding(quals->sq))
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "binding %d greater than max %s block bindings (%d)",
                                                               binding + member_count - 1,
                                                               glsl_storage_qual_string(quals->sq),
                                                               max_block_binding(quals->sq));
      }

      if (quals->sq == STORAGE_UNIFORM &&
          quals->lq->qualified & UNIF_QUALED &&
          quals->lq->unif_bits & LAYOUT_STD430)
      {
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "std430 may not be applied to uniform blocks");
      }
   }

   if(quals->invariant)
      glsl_compile_error(ERROR_SEMANTIC, 9, g_LineNumber, NULL);
   if(quals->iq != INTERP_SMOOTH && (quals->sq != STORAGE_IN && quals->sq != STORAGE_OUT))
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "'%s' invalid on block declaration", glsl_interp_qual_string(quals->iq));
   if(quals->aq != AUXILIARY_NONE && (quals->sq != STORAGE_IN && quals->sq != STORAGE_OUT))
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "'%s' invalid on block declaration", glsl_aux_qual_string(quals->aq));
   if(quals->pq != PREC_NONE)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "precision qualifier invalid on block declaration");

   if (!g_InGlobalScope)
      glsl_compile_error(ERROR_SEMANTIC, 7, g_LineNumber, NULL);

   Symbol *symbol = malloc_fast(sizeof(Symbol));
   SymbolType *ref_type = &primitiveTypes[PRIM_UINT];
   if (type->flavour == SYMBOL_ARRAY_TYPE)
      ref_type = make_array_type_internal(ref_type, type->u.array_type.member_count);
   glsl_symbol_construct_interface_block(symbol, block_name, ref_type, type, quals);
   glsl_symbol_table_insert(table, symbol);

   return symbol;
}

Symbol *glsl_commit_var_instance(SymbolTable *table, const char *name, SymbolType *type,
                                 Qualifiers *q, const_value *compile_time_value)
{
   check_singleton_is_instantiable(name, type);
   glsl_check_namespace_nonfunction(table, name);

   Symbol *block_symbol = NULL;
   if(is_array_of_blocks(type)) {
      // We are constructing the symbol for an interface block instance.
      SymbolType *block_type;

      if(type->flavour==SYMBOL_BLOCK_TYPE)
         block_type = type;
      else
         block_type = type->u.array_type.member_type;

      block_symbol = glsl_symbol_table_lookup(table, block_type->name);
      assert(block_symbol);
   }

   Symbol *symbol = malloc_fast(sizeof(Symbol));
   glsl_symbol_construct_var_instance(symbol, name, type, q, compile_time_value, block_symbol);

   glsl_symbol_table_insert(table, symbol);
   return symbol;
}

static bool is_arrays_of_opaque_type(SymbolType *type) {
   while (type->flavour == SYMBOL_ARRAY_TYPE) type = type->u.array_type.member_type;
   return glsl_prim_is_opaque_type(type);
}

static bool is_arrays_of_image_type(SymbolType *type) {
   while (type->flavour == SYMBOL_ARRAY_TYPE) type = type->u.array_type.member_type;
   return glsl_prim_is_prim_image_type(type);
}

static bool is_arrays_of_atomic_type(SymbolType *type) {
   while (type->flavour == SYMBOL_ARRAY_TYPE) type = type->u.array_type.member_type;
   return glsl_prim_is_prim_atomic_type(type);
}

static bool is_function_overloadable(Symbol *func) {
   /* You can overload everything in version 100 */
   if(g_ShaderVersion == GLSL_SHADER_VERSION(1,0,1)) return true;
   return !glsl_stdlib_is_stdlib_symbol(func);
}

Symbol *glsl_commit_singleton_function_declaration(SymbolTable *table, const char *name, SymbolType *type, bool definition, bool user_code)
{
   Symbol *symbol = malloc_fast(sizeof(Symbol));

   if (!g_InGlobalScope)
      glsl_compile_error(ERROR_CUSTOM, 12, g_LineNumber, NULL);

   /* Check that this doesn't have type void or use a 'gl_' name */
   check_singleton_is_instantiable(name, type);

   // Special case: main must have the signature "void main()"
   if (strcmp("main", name) == 0)
   {
      if (type->u.function_type.return_type != &primitiveTypes[PRIM_VOID] ||
          type->u.function_type.param_count != 0)
      {
         glsl_compile_error(ERROR_SEMANTIC, 17, g_LineNumber, NULL);
         return NULL;
      }
   }

   /* Check whether we are overloading or redeclaring an existing function*/
   Symbol *overload = glsl_symbol_table_lookup(table, name);
   if (overload)
   {
      if (overload->flavour != SYMBOL_FUNCTION_INSTANCE) {
         glsl_compile_error(ERROR_SEMANTIC, 13, g_LineNumber, "%s already defined as a non-function", name);
         return NULL;
      }
      if(user_code && !is_function_overloadable(overload)) {
         glsl_compile_error(ERROR_SEMANTIC, 19, g_LineNumber, NULL);
         return NULL;
      }

      // Check to see if we are actually re-declaring, rather than overloading
      Symbol *match = glsl_resolve_overload_using_prototype(overload, type);
      if (match)
      {
         if (!definition) {
            if (g_ShaderVersion == GLSL_SHADER_VERSION(1, 0, 1) && match->u.function_instance.has_prototype)
               glsl_compile_error(ERROR_SEMANTIC, 13, g_LineNumber, "Illegal redeclaration of function %s", name);
            match->u.function_instance.has_prototype = true;
         }

         // There is already a function with our signature in the symbol table, so don't add a new one.
         // Update parameter names to match this declaration, but only if the
         // function hasn't been defined. If it has then changing the type will
         // mess up its parameter storage, and we don't need the names.
         if (match->u.function_instance.function_def == NULL)
            match->type = type;

         // Return the existing symbol
         return match;
      }

      // We have a new overload. Chain the other overloads onto ours.
   }

   glsl_symbol_construct_function_instance(symbol, name, type, NULL, overload, !definition);

   // At this point we have a fresh declaration to insert into the symbol table.
   glsl_symbol_table_insert(table, symbol);
   return symbol;
}

void glsl_complete_array_from_init_type(SymbolType *type, SymbolType *init_type)
{
   if (init_type->flavour != SYMBOL_ARRAY_TYPE ||
        ( type->u.array_type.member_count != 0 &&
          type->u.array_type.member_count != init_type->u.array_type.member_count) )
   {
      // Type mismatch.
      glsl_compile_error(ERROR_SEMANTIC, 1, g_LineNumber, NULL);
   }

   if(type->u.array_type.member_type->flavour == SYMBOL_ARRAY_TYPE) {
      glsl_complete_array_from_init_type(type->u.array_type.member_type,
                                    init_type->u.array_type.member_type);
   } else {
      if (!glsl_shallow_match_nonfunction_types(init_type->u.array_type.member_type,
                                                type->u.array_type.member_type))
      {
         // Type mismatch.
         glsl_compile_error(ERROR_SEMANTIC, 1, g_LineNumber, NULL);
      }
   }

   glsl_complete_array_type(type, init_type->u.array_type.member_count);
}

static SymbolType *copy_type_where_incomplete(SymbolType *type) {
   if (type->flavour != SYMBOL_ARRAY_TYPE) return type;

   SymbolType *ret = malloc_fast(sizeof(SymbolType));
   memcpy(ret, type, sizeof(SymbolType));
   ret->u.array_type.member_type = copy_type_where_incomplete(type->u.array_type.member_type);
   return ret;
}

/* Check types for input and output types inside the pipeline (ie. not vertex in
 * or fragment out). The number of array levels allowed, and whether structs or
 * ints are allowed varies based on context */
static bool check_in_out_type(const SymbolType *ty, int arrays_ok, bool struct_ok, bool int_ok, bool matrix_ok)
{
   if (ty->flavour == SYMBOL_ARRAY_TYPE) {
      /* Struct are allowed in arrays only in the same circumstances that more array levels are */
      if (arrays_ok == 0) return false;
      else return check_in_out_type(ty->u.array_type.member_type, arrays_ok-1, (arrays_ok != 1), int_ok, matrix_ok);
   } else if (ty->flavour == SYMBOL_STRUCT_TYPE) {
      bool ok = struct_ok;
      for (unsigned i = 0; i < ty->u.struct_type.member_count; i++ ) {
         const SymbolType *sty = ty->u.struct_type.member[i].type;
         ok = ok && check_in_out_type(sty, 0, false, int_ok, matrix_ok);
      }
      return ok;
   } else {
      if (ty->flavour != SYMBOL_PRIMITIVE_TYPE) return false;
      bool i = primitiveTypeFlags[ty->u.primitive_type.index] & (PRIM_INT_TYPE | PRIM_UINT_TYPE);
      bool f = primitiveTypeFlags[ty->u.primitive_type.index] & PRIM_FLOAT_TYPE;
      bool m = primitiveTypeFlags[ty->u.primitive_type.index] & PRIM_MATRIX_TYPE;
      return (f || i) && (int_ok || !i) && (matrix_ok || !m);
   }
}

static bool format_supported(FormatQualifier format) {
   switch (format) {
      case FMT_RGBA32F:
      case FMT_RGBA16F:
      case FMT_R32F:
      case FMT_RGBA8:
      case FMT_RGBA8_SNORM:
      case FMT_RGBA32I:
      case FMT_RGBA16I:
      case FMT_RGBA8I:
      case FMT_R32I:
      case FMT_RGBA32UI:
      case FMT_RGBA16UI:
      case FMT_RGBA8UI:
      case FMT_R32UI:
         return true;

      /* Extended formats */
      case FMT_RG32F:
      case FMT_RG16F:
      case FMT_R11G11B10F:
      case FMT_R16F:
      case FMT_RGBA16:
      case FMT_RGB10A2:
      case FMT_RG16:
      case FMT_RG8:
      case FMT_R16:
      case FMT_R8:
      case FMT_RGBA16_SNORM:
      case FMT_RG16_SNORM:
      case FMT_RG8_SNORM:
      case FMT_R16_SNORM:
      case FMT_R8_SNORM:
      case FMT_RG32I:
      case FMT_RG16I:
      case FMT_RG8I:
      case FMT_R16I:
      case FMT_R8I:
      case FMT_RGB10A2UI:
      case FMT_RG32UI:
      case FMT_RG16UI:
      case FMT_RG8UI:
      case FMT_R16UI:
      case FMT_R8UI:
         return glsl_ext_status(GLSL_EXT_IMAGE_FORMATS) != GLSL_DISABLED;
   }
   unreachable();
   return false;
}

static bool format_valid_for_image_type(FormatQualifier format, SymbolType *type) {
   PrimitiveTypeIndex img_ret = glsl_prim_get_image_info(type->u.primitive_type.index)->return_type;
   PrimitiveTypeIndex img_ret_base = primitiveScalarTypeIndices[img_ret];

   switch (format) {
      case FMT_RGBA32F:
      case FMT_RGBA16F:
      case FMT_R32F:
      case FMT_RGBA8:
      case FMT_RGBA8_SNORM:
      case FMT_RG32F:
      case FMT_RG16F:
      case FMT_R11G11B10F:
      case FMT_R16F:
      case FMT_RGBA16:
      case FMT_RGB10A2:
      case FMT_RG16:
      case FMT_RG8:
      case FMT_R16:
      case FMT_R8:
      case FMT_RGBA16_SNORM:
      case FMT_RG16_SNORM:
      case FMT_RG8_SNORM:
      case FMT_R16_SNORM:
      case FMT_R8_SNORM:
         return (img_ret_base == PRIM_FLOAT);

      case FMT_RGBA32I:
      case FMT_RGBA16I:
      case FMT_RGBA8I:
      case FMT_R32I:
      case FMT_RG32I:
      case FMT_RG16I:
      case FMT_RG8I:
      case FMT_R16I:
      case FMT_R8I:
         return (img_ret_base == PRIM_INT);

      case FMT_RGBA32UI:
      case FMT_RGBA16UI:
      case FMT_RGBA8UI:
      case FMT_R32UI:
      case FMT_RGB10A2UI:
      case FMT_RG32UI:
      case FMT_RG16UI:
      case FMT_RG8UI:
      case FMT_R16UI:
      case FMT_R8UI:
         return (img_ret_base == PRIM_UINT);
   }
   unreachable();
   return false;
}

static SymbolType *strip_arrays(SymbolType *type) {
   while (type->flavour == SYMBOL_ARRAY_TYPE) type = type->u.array_type.member_type;
   return type;
}

Symbol *glsl_commit_variable_instance(SymbolTable *table, const PrecisionTable *prec, DeclDefaultState *dflt,
                                      QualList *quals, SymbolType *type, const char *name,
                                      ExprChain *array_size, Expr *initialiser)
{
   Qualifiers q;
   qualifiers_from_list(&q, quals);
   if (q.pq == PREC_NONE) q.pq = glsl_prec_get_prec(prec, type);

   const char *error_name = name ? name : "<empty declaration>";

   if (q.pq == PREC_UNKNOWN)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "type '%s' has unknown precision in declaration of '%s'", type->name, error_name);

   if (array_size != NULL)
      type = glsl_build_array_type(type, array_size);

   /* Validate ins and outs based on the type of shader. */
   if (q.sq == STORAGE_OUT || q.sq == STORAGE_IN) {
      bool vertex_in    = g_ShaderFlavour == SHADER_VERTEX   && q.sq == STORAGE_IN;
      bool fragment_out = g_ShaderFlavour == SHADER_FRAGMENT && q.sq == STORAGE_OUT;

      bool arrayed_iface = g_ShaderFlavour == SHADER_TESS_CONTROL || ((g_ShaderFlavour == SHADER_TESS_EVALUATION || g_ShaderFlavour == SHADER_GEOMETRY) && q.sq == STORAGE_IN);
      bool int_need_flat = (g_ShaderFlavour == SHADER_VERTEX   && q.sq == STORAGE_OUT) ||
                           (g_ShaderFlavour == SHADER_FRAGMENT && q.sq == STORAGE_IN);

      int arrays_ok = (arrayed_iface && q.aq != AUXILIARY_PATCH) ? 2 : (vertex_in ? 0 : 1);
      bool struct_ok = !(vertex_in || fragment_out) && g_ShaderVersion != GLSL_SHADER_VERSION(1,0,1);
      bool matrix_ok = !fragment_out;
      bool int_ok = (!int_need_flat || q.iq == INTERP_FLAT) &&
                    (g_ShaderVersion != GLSL_SHADER_VERSION(1,0,1));

      if (!check_in_out_type(type, arrays_ok, struct_ok, int_ok, matrix_ok))
         glsl_compile_error(ERROR_SEMANTIC, 8, g_LineNumber, "\"%s\"", name);

      if (vertex_in || fragment_out) {
         if (q.iq != INTERP_SMOOTH || q.aq != AUXILIARY_NONE)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Use of interpolation qualifiers on vertex input / fragment output");
      }
   }

   if (q.aq == AUXILIARY_PATCH) {
      if (!( (q.sq == STORAGE_IN  && g_ShaderFlavour == SHADER_TESS_EVALUATION) ||
             (q.sq == STORAGE_OUT && g_ShaderFlavour == SHADER_TESS_CONTROL)  ) )
      {
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Invalid use of patch qualifier");
      }
   }

   if (q.aq == AUXILIARY_SAMPLE) {
      if (q.sq == STORAGE_OUT && g_ShaderFlavour == SHADER_FRAGMENT)
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Invalid use of sample qualifier");
      if (q.sq == STORAGE_IN  && g_ShaderFlavour == SHADER_VERTEX)
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Invalid use of sample qualifier");
   }

   assert(!is_array_of_blocks(type));

   if (q.lq) {
      if (q.lq->qualified & BINDING_QUALED) {
         if (g_ShaderVersion < GLSL_SHADER_VERSION(3, 10, 1) )
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "binding qualifier not available in this language version");

         if (!is_arrays_of_opaque_type(type))
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "binding qualifier requires opaque type, found %s",
                                                               type->name);

         /* Image and sampler bindings count up for arrays. Atomic bindings stay the same */
         int max_binding, binding_used;
         if (is_arrays_of_image_type(type)) {
            max_binding = GLXX_CONFIG_MAX_IMAGE_UNITS - 1;
            binding_used = q.lq->binding + type->scalar_count - 1;
         } else if (is_arrays_of_atomic_type(type)) {
            max_binding = GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS - 1;
            binding_used = q.lq->binding;
         } else {
            max_binding = GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS - 1;
            binding_used = q.lq->binding + (type->scalar_count/2) - 1;
         }

         if (binding_used > max_binding)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "binding %d greater than max allowed for %s (%d)",
                                                               binding_used, type->name, max_binding);
      }

      if (q.lq->qualified & LOC_QUALED) {
         StorageQualifier sq = q.sq;
         if (sq != STORAGE_IN && sq != STORAGE_OUT && sq != STORAGE_UNIFORM)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "location qualifier not valid for storage class '%s'",
                                                               glsl_storage_qual_string(sq));

         if (g_ShaderVersion < GLSL_SHADER_VERSION(3, 10, 1)) {
            if (sq == STORAGE_IN && g_ShaderFlavour != SHADER_VERTEX)
               glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "input location qualifier only valid in vertex shader");
            if (sq == STORAGE_OUT && g_ShaderFlavour != SHADER_FRAGMENT)
               glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "output location qualifier only valid in fragment shader");
            if (sq == STORAGE_UNIFORM)
               glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "uniform location qualifier not valid in this language version");

         }
      }

      if (q.lq->qualified & FORMAT_QUALED) {
         if (!is_arrays_of_image_type(type))
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "format qualifier not valid for type %s", type->name);
         if (!format_valid_for_image_type(q.lq->format, strip_arrays(type)))
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "format qualifier inconsistent with type %s", type->name);
         if (!format_supported(q.lq->format))
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Format not supported. An extension may need to be enabled?");
      }

      if (q.lq->qualified & OFFSET_QUALED) {
         if (!is_arrays_of_atomic_type(type))
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "offset qualifier not valid for type %s", type->name);
         if (q.lq->offset & 3)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "offset %d insufficiently aligned", q.lq->offset);
      }

      if (q.lq->qualified & UNIF_QUALED)
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Block layouts may only used for variables in blocks");
   }

   if (glsl_type_contains(type, PRIM_IMAGE_TYPE) && (!q.lq || !(q.lq->qualified & FORMAT_QUALED)))
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Image type declaration requires a format qualifier");

   if (glsl_type_contains(type, PRIM_ATOMIC_TYPE)) {
      if (!q.lq || !(q.lq->qualified & BINDING_QUALED))
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "type %s requires binding qualifier", type->name);
      if (q.lq && (q.lq->qualified & LOC_QUALED))
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "location qualifier not valid for type %s", type->name);
      if (q.pq != PREC_HIGHP)
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "type %s is only available at highp", type->name);
   }

   if (q.mq != MEMORY_NONE && !is_arrays_of_image_type(type))
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Memory qualifiers only valid for image or block declarations");

   if (q.sq == STORAGE_BUFFER)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "%s: all buffer variables must be in blocks", error_name);

   if (type->flavour==SYMBOL_ARRAY_TYPE) {
      if (initialiser != NULL) {
         type = copy_type_where_incomplete(type);
         glsl_complete_array_from_init_type(type, initialiser->type);
      }

      complete_array_type_from_defaults(type, q.sq, dflt);

      glsl_error_if_type_incomplete(type);
   }

   if (is_arrays_of_atomic_type(type)) {
      /* Process the offset */
      assert(q.lq->qualified & BINDING_QUALED);
      if (q.lq->qualified & OFFSET_QUALED) dflt->atomic_offset[q.lq->binding] = q.lq->offset;
      else {
         q.lq->qualified = q.lq->qualified | OFFSET_QUALED;
         q.lq->offset = dflt->atomic_offset[q.lq->binding];
      }
      if (name != NULL)
         dflt->atomic_offset[q.lq->binding] += 4 * type->scalar_count;
   }

   if(q.invariant && q.sq != STORAGE_OUT) {
      if (g_ShaderVersion != GLSL_SHADER_VERSION(1,0,1))
         glsl_compile_error(ERROR_SEMANTIC, 9, g_LineNumber, NULL);
      else if (g_ShaderFlavour != SHADER_FRAGMENT || q.sq != STORAGE_IN)
         glsl_compile_error(ERROR_SEMANTIC, 9, g_LineNumber, NULL);
   }

   if(g_InGlobalScope && initialiser && initialiser->compile_time_value == NULL)
      glsl_compile_error(ERROR_SEMANTIC, 2, g_LineNumber, "Global variable initialisers must be constant expressions");

   if (q.sq == STORAGE_IN     || q.sq == STORAGE_OUT || q.sq == STORAGE_UNIFORM ||
       q.sq == STORAGE_BUFFER || q.sq == STORAGE_SHARED                             )
   {
      if (initialiser)
         glsl_compile_error(ERROR_SEMANTIC, 2, g_LineNumber, "Interface variables may not be initialised");
      if (!g_InGlobalScope)
         glsl_compile_error(ERROR_SEMANTIC, 7, g_LineNumber, NULL);
   }

   if (initialiser && !glsl_shallow_match_nonfunction_types(type, initialiser->type))
      glsl_compile_error(ERROR_SEMANTIC, 1, g_LineNumber, NULL);

   if(q.sq == STORAGE_CONST && glsl_type_contains_array(type) &&
      g_ShaderVersion == GLSL_SHADER_VERSION(1, 0, 1))
   {
      // Array cannot be const before 3.0.
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "const invalid for arrays in this language version");
      return NULL;
   }

   if (name == NULL) return NULL;

   /* XXX These checks only happen when declaring variables, contrary to what the spec says */

   if(q.sq != STORAGE_UNIFORM && glsl_type_contains_opaque(type))
      glsl_compile_error(ERROR_CUSTOM, 22, g_LineNumber, NULL);

   void *compile_time_value = NULL;
   if (q.sq == STORAGE_CONST) {
      if (!initialiser)
         glsl_compile_error(ERROR_SEMANTIC, 2, g_LineNumber, "const variable '%s' requires an initialiser", name);

      if (!initialiser->compile_time_value)
         glsl_compile_error(ERROR_SEMANTIC, 2, g_LineNumber, "const variable initialisers must be constant expressions");

      compile_time_value = initialiser->compile_time_value;
   }

   return glsl_commit_var_instance(table, name, type, &q, compile_time_value);
}

static void validate_member_layout(LayoutQualifier *member_lq, StorageQualifier sq)
{
   if (!member_lq) return;

   if(member_lq->qualified & LOC_QUALED && !(sq == STORAGE_IN || sq == STORAGE_OUT))
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "location layout may not be applied to %s block members",
                                                         glsl_storage_qual_string(sq));

   if(member_lq->qualified & BINDING_QUALED)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "binding layout may not be applied to block members");

   if (member_lq->qualified & OFFSET_QUALED)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "offset layout may not be applied to block members");

   if(member_lq->qualified & UNIF_QUALED) {
      const uint32_t packing_mask = LAYOUT_PACKED | LAYOUT_SHARED |
                                    LAYOUT_STD140 | LAYOUT_STD430;
      if(member_lq->unif_bits & packing_mask)
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "block layout may not be applied to members");
   }
}

static void glsl_map_member(Map *struct_map, StorageQualifier valid_sq,
                            const char *name, SymbolType *type, Qualifiers *quals)
{
   StructMember *member = malloc_fast(sizeof(StructMember));
   member->name   = name;
   member->type   = type;
   member->layout = quals->lq;
   member->prec   = quals->pq;
   member->memq   = quals->mq;
   member->interp = quals->iq;
   member->auxq   = quals->aq;

   check_singleton_is_instantiable(name, type);
   if (glsl_map_get(struct_map, name) != NULL)
      glsl_compile_error(ERROR_SEMANTIC, 13, g_LineNumber, "%s", name);

   glsl_map_put(struct_map, name, member);
}

static void glsl_map_and_check_members(Map *struct_map, StorageQualifier valid_sq,
                                       Statement *statement, LayoutQualifier *block_lq)
{
   assert(statement->flavour==STATEMENT_STRUCT_DECL);
   SymbolType *type = statement->u.struct_decl.type;
   Qualifiers q;
   qualifiers_from_list_context_sq(&q, statement->u.struct_decl.quals, valid_sq);

   if(block_lq) {
      LayoutQualifier *member_lq = q.lq;
      validate_member_layout(member_lq, valid_sq);
      q.lq = glsl_create_mixed_lq(member_lq, block_lq);
   }

   if(q.sq != STORAGE_NONE && q.sq != valid_sq)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "expected \"%s\", found \"%s\"",
                                                         glsl_storage_qual_string(valid_sq),
                                                         glsl_storage_qual_string(q.sq));

   if (q.mq != MEMORY_NONE && valid_sq != STORAGE_BUFFER)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "memory qualifier not valid in declaration of struct or block type");

   for(StatementChainNode *node=statement->u.struct_decl.members->first; node; node=node->next)
   {
      assert(node->statement->flavour==STATEMENT_STRUCT_MEMBER_DECL);

      if (node->statement->u.struct_member_decl.array_specifier != NULL)
         type = glsl_build_array_type(type, node->statement->u.struct_member_decl.array_specifier);

      glsl_map_member(struct_map, valid_sq, node->statement->u.struct_member_decl.name, type, &q);
   }
}

static SymbolType *glsl_build_struct_or_block_type(SymbolTypeFlavour flavour,
                                                   const char *name,
                                                   StatementChain *chain,
                                                   StorageQualifier valid_sq,
                                                   LayoutQualifier *block_lq)
{
   assert(flavour==SYMBOL_STRUCT_TYPE || flavour==SYMBOL_BLOCK_TYPE);

   Map *members = glsl_map_new();

   for(StatementChainNode *node=chain->first; node; node=node->next) {
      glsl_map_and_check_members(members, valid_sq, node->statement, block_lq);
   }

   SymbolType *resultType   = malloc_fast(sizeof(SymbolType));
   resultType->flavour      = flavour;
   resultType->name         = name ? name : "<anon>";
   resultType->scalar_count = 0;    /* Added as we go along */

   StructMember *memb = malloc_fast(sizeof(StructMember) * members->count);

   int i = 0;
   GLSL_MAP_FOREACH(map_entry, members) {
      StructMember *member = map_entry->v;

      /* If this is a buffer variable, and the last member then unsized array is allowed */
      if (valid_sq != STORAGE_BUFFER || i != (members->count - 1))
         glsl_error_if_type_incomplete(member->type);

      memb[i].name   = member->name;
      memb[i].type   = member->type;
      memb[i].layout = (flavour == SYMBOL_STRUCT_TYPE) ? NULL : member->layout;
      memb[i].prec   = member->prec;
      memb[i].memq   = member->memq;
      memb[i].interp = member->interp;
      memb[i].auxq   = member->auxq;

      resultType->scalar_count += member->type->scalar_count;

      ++i;
   }
   assert(i == members->count);

   if(flavour==SYMBOL_STRUCT_TYPE) {
      resultType->u.struct_type.member_count = members->count;
      resultType->u.struct_type.member       = memb;
   } else if(flavour==SYMBOL_BLOCK_TYPE) {
      resultType->u.block_type.member_count = members->count;
      resultType->u.block_type.member       = memb;
   }

   glsl_map_delete(members);

   return resultType;
}

SymbolType *glsl_build_struct_type(const char *name, StatementChain *chain)
{
   return glsl_build_struct_or_block_type(SYMBOL_STRUCT_TYPE, name, chain, STORAGE_NONE, NULL);
}

SymbolType *glsl_build_block_type(Qualifiers *quals, const char *name, StatementChain *chain)
{
   SymbolType *type = glsl_build_struct_or_block_type(SYMBOL_BLOCK_TYPE, name, chain,
                                                      quals->sq, quals->lq);
   if(glsl_type_contains_opaque(type))
      glsl_compile_error(ERROR_CUSTOM, 20, g_LineNumber, NULL);

   type->u.block_type.lq = quals->lq;
   type->u.block_type.layout = malloc_fast(sizeof(MemLayout));
   glsl_mem_calculate_block_layout(type->u.block_type.layout, type);
   type->u.block_type.has_named_instance = false;

   return type;
}

void glsl_commit_anonymous_block_members(SymbolTable *table, Symbol *symbol, MemoryQualifier mq)
{
   SymbolType *type = symbol->u.interface_block.block_data_type;

   assert(symbol->flavour==SYMBOL_INTERFACE_BLOCK);
   assert(type->flavour==SYMBOL_BLOCK_TYPE);

   for(unsigned i = 0; i < type->u.block_type.member_count; i++)
   {
      StructMember *member = &type->u.block_type.member[i];

      Qualifiers member_quals;
      member_quals.invariant = false;
      member_quals.sq        = symbol->u.interface_block.sq;
      member_quals.iq        = symbol->u.interface_block.iq;
      member_quals.aq        = symbol->u.interface_block.aq;
      member_quals.mq        = member->memq | mq;
      member_quals.lq        = member->layout;

      glsl_check_namespace_nonfunction(table, member->name);

      Symbol *member_symbol = malloc_fast(sizeof(Symbol));
      glsl_symbol_construct_var_instance(member_symbol, member->name, member->type, &member_quals, NULL, symbol);
      glsl_symbol_table_insert(table, member_symbol);
   }
}

static Symbol *build_function_param(const char *name, SymbolType *type, QualList *quals, ExprChain *size)
{
   Qualifiers q;
   ParamQualifier pq;

   param_quals_from_list(&q, &pq, quals);

   if (q.mq != MEMORY_NONE && !is_arrays_of_image_type(type))
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Memory qualifiers only valid for image or block declarations");

   if(size != NULL) {
      type = glsl_build_array_type(type, size);
      glsl_error_if_type_incomplete(type);
   }

   if (name != NULL)
      check_singleton_is_instantiable(name, type);
   else
      name = glsl_intern("$$anon", true);

   /* const qualifier and opaque types not allowed for 'out' params*/
   if(pq == PARAM_QUAL_INOUT || pq == PARAM_QUAL_OUT) {
      if(q.sq == STORAGE_CONST)
         glsl_compile_error(ERROR_CUSTOM, 10, g_LineNumber, NULL);

      if (glsl_type_contains_opaque(type))
         glsl_compile_error(ERROR_CUSTOM, 22, g_LineNumber, NULL);
   }

   assert(q.sq == STORAGE_NONE || q.sq == STORAGE_CONST);
   assert(!q.invariant);

   Symbol *symbol = malloc_fast(sizeof(Symbol));
   glsl_symbol_construct_param_instance(symbol, name, type, q.sq, pq, q.mq);

   return symbol;
}

SymbolType *glsl_build_function_type(QualList *return_quals, SymbolType *return_type, ParamList *params)
{
   if (return_quals) {
      /* Only precision qualifiers are allowed. TODO: They're currently ignored */
      for (QualListNode *n = return_quals->head; n; n=n->next) {
         if (n->q->flavour != QUAL_PREC)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "Only precision qualifiers allowed on return types");
      }
   }

   if (glsl_type_contains_array(return_type) &&
       g_ShaderVersion < GLSL_SHADER_VERSION(3, 0, 1))
   {
      glsl_compile_error(ERROR_CUSTOM, 19, g_LineNumber, "function returning array invalid in version 100");
   }

   unsigned void_count  = 0;
   unsigned param_count = 0;
   for (ParamList *l = params; l != NULL; l = l->next) {
      if (l->p->type == &primitiveTypes[PRIM_VOID]) {
         if (l->p->name != NULL || l->p->size != NULL)
            glsl_compile_error(ERROR_CUSTOM, 18, g_LineNumber, NULL);
         void_count++;
      } else
         param_count++;
   }

   /* void type can only be used in empty formal parameter list. */
   if (void_count > 1 || (void_count == 1 && param_count > 0))
      glsl_compile_error(ERROR_CUSTOM, 19, g_LineNumber, NULL);

   Symbol **functionParams = NULL;
   SymbolType *type = malloc_fast(sizeof(SymbolType));
   if (param_count > 0) {
      functionParams = malloc_fast(sizeof(Symbol*) * param_count);
      unsigned i = 0;
      for (ParamList *l = params; l != NULL; l = l->next, i++) {
         for (unsigned j=0; j<i; j++) {
            /* Rely on the fact that we've interned the strings to compare quickly */
            if (functionParams[j]->name == l->p->name)
               glsl_compile_error(ERROR_SEMANTIC, 13, g_LineNumber, "%s", l->p->name);
         }
         functionParams[i] = build_function_param(l->p->name, l->p->type, l->p->quals, l->p->size);
      }

      // Create the type name (working forwards!).
      StringBuilder *sb = glsl_sb_new();
      glsl_sb_append(sb, "(");
      for (i = 0; i < param_count; i++) {
         if (i > 0) glsl_sb_append(sb, ", ");
         glsl_sb_append(sb, "%s %s ", glsl_storage_qual_string(functionParams[i]->u.param_instance.storage_qual),
                                      glsl_param_qual_string(functionParams[i]->u.param_instance.param_qual));
         glsl_sb_append(sb, "%s", functionParams[i]->type->name);
      }
      glsl_sb_append(sb, ")");
      type->name = glsl_sb_content(sb);
   }
   else
      type->name = "()";

   type->flavour                     = SYMBOL_FUNCTION_TYPE;
   type->scalar_count                = 0;
   type->u.function_type.return_type = return_type;
   type->u.function_type.param_count = param_count;
   type->u.function_type.params      = functionParams;

   return type;
}

void glsl_instantiate_function_params(SymbolTable *table, SymbolType *fun)
{
   assert(fun->flavour == SYMBOL_FUNCTION_TYPE);

   // No need to check namespace as we have a clean scope for the function.
   for (unsigned i = 0; i < fun->u.function_type.param_count; i++)
      glsl_symbol_table_insert(table, fun->u.function_type.params[i]);
}
