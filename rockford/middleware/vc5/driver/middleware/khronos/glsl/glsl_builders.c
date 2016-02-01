/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdlib.h>
#include <string.h>

#include "glsl_common.h"
#include "glsl_builders.h"
#include "glsl_check.h"
#include "glsl_errors.h"
#include "glsl_globals.h"
#include "glsl_trace.h"
#include "glsl_fastmem.h"
#include "glsl_ast.h"
#include "glsl_ast_print.h"
#include "glsl_intern.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_uniform_layout.h"
#include "glsl_precision.h"
#include "glsl_symbol_table.h"
#include "glsl_stringbuilder.h"
#include "glsl_quals.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

void glsl_error_if_type_incomplete(SymbolType *type)
{
   while (type->flavour == SYMBOL_ARRAY_TYPE) {
      if(type->u.array_type.member_count==0)
         glsl_compile_error(ERROR_CUSTOM, 28, g_LineNumber, NULL);
      type = type->u.array_type.member_type;
   }
}

void glsl_reinit_function_builder(FunctionBuilder *fb)
{
   fb->Params = glsl_symbol_table_new();
   fb->ParamCount = 0;
   fb->VoidCount = 0;
}

static void glsl_check_namespace_nonfunction(SymbolTable *table, const char *name)
{
   Symbol *sym = glsl_symbol_table_lookup_in_current_scope(table, name);
   if (sym)
      glsl_compile_error(ERROR_SEMANTIC, 22, g_LineNumber, "%s", name);
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

   if(lq_local->qualified & UNIF_QUALED)
   {
      uint32_t packing_mask = LAYOUT_PACKED | LAYOUT_SHARED | LAYOUT_STD140 | LAYOUT_STD430;
      uint32_t matrix_order_mask = LAYOUT_ROW_MAJOR | LAYOUT_COLUMN_MAJOR;

      res->qualified |= UNIF_QUALED;

      if(lq_local->unif_bits & matrix_order_mask) {
         res->unif_bits &= ~matrix_order_mask;
         res->unif_bits |= (lq_local->unif_bits & matrix_order_mask);
      }

      if(lq_local->unif_bits & packing_mask) {
         res->unif_bits &= ~packing_mask;
         res->unif_bits |= (lq_local->unif_bits & packing_mask);
      }
   }

   return res;
}

void glsl_insert_function_definition(Statement* statement)
{
   assert(statement->flavour == STATEMENT_FUNCTION_DEF);

   Symbol *header_sym = statement->u.function_def.header;

   if (header_sym->u.function_instance.function_def)
   {
      glsl_compile_error(ERROR_SEMANTIC, 22, g_LineNumber, "function %s already defined", header_sym->name);
      return;
   }

   header_sym->u.function_instance.function_def = statement;
}

void glsl_commit_struct_type(SymbolTable *table, SymbolType *type)
{
   Symbol *symbol = malloc_fast(sizeof(Symbol));
   glsl_symbol_construct_type(symbol, type);
   glsl_check_namespace_nonfunction(table, type->name);
   glsl_symbol_table_insert(table, symbol);

   TRACE(("added symbol <%s> as new struct type (%d scalars)\n", type->name, type->scalar_count));
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

static bool is_array_of_blocks(const SymbolType *type) {
   while (type->flavour == SYMBOL_ARRAY_TYPE) type = type->u.array_type.member_type;
   return type->flavour == SYMBOL_BLOCK_TYPE;
}

static const char *extract_block_name(const SymbolType *type) {
   while (type->flavour == SYMBOL_ARRAY_TYPE) type = type->u.array_type.member_type;
   return type->name;
}

static bool storage_valid_for_block(StorageQualifier sq) {
   if (sq == STORAGE_UNIFORM) return true;
   if (sq == STORAGE_BUFFER) return g_ShaderVersion >= GLSL_SHADER_VERSION(3, 10, 1);
   if (sq == STORAGE_IN || sq == STORAGE_OUT) return g_ShaderVersion >= GLSL_SHADER_VERSION(3, 20, 1);
   return false;
}

static int max_block_binding(StorageQualifier sq) {
   assert(sq == STORAGE_UNIFORM || sq == STORAGE_BUFFER);
   if (sq == STORAGE_UNIFORM) return GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS - 1;
   else                       return GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS - 1;
}

Symbol *glsl_commit_block_type(SymbolTable *table, FullySpecType *fs_type)
{
   SymbolType *type = fs_type->type;

   if(!storage_valid_for_block(fs_type->quals.sq))
      glsl_compile_error(ERROR_CUSTOM, 4, g_LineNumber, "interface %s does not support blocks",
                                                        glsl_storage_qual_string(fs_type->quals.sq));

   if (type->flavour == SYMBOL_ARRAY_TYPE &&
       type->u.array_type.member_type->flavour == SYMBOL_ARRAY_TYPE)
   {
      glsl_compile_error(ERROR_CUSTOM, 4, g_LineNumber, "Interface blocks cannot be arrays of arrays");
   }

   assert(is_array_of_blocks(type));

   const char *block_name = extract_block_name(type);
   glsl_error_if_type_incomplete(type);

   check_singleton_is_instantiable(block_name, type);
   glsl_check_namespace_nonfunction(table, block_name);

   if (fs_type->quals.lq->qualified & LOC_QUALED)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "location qualifier may not be applied to blocks");

   if (fs_type->quals.lq->qualified & OFFSET_QUALED)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "offset qualifier may not be applied to blocks");

   if (fs_type->quals.lq->qualified & BINDING_QUALED) {
      if (g_ShaderVersion < GLSL_SHADER_VERSION(3, 10, 1))
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "binding qualifier not allowed in this language version");

      int binding = fs_type->quals.lq->binding;
      int member_count = (fs_type->type->flavour == SYMBOL_ARRAY_TYPE) ? fs_type->type->u.array_type.member_count : 1;

      if (binding + member_count-1 > max_block_binding(fs_type->quals.sq))
         glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "binding %d greater than max %s block bindings (%d)",
                                                            binding + member_count - 1,
                                                            glsl_storage_qual_string(fs_type->quals.sq),
                                                            max_block_binding(fs_type->quals.sq));

   }

   if (fs_type->quals.sq == STORAGE_UNIFORM &&
       fs_type->quals.lq->qualified & UNIF_QUALED &&
       fs_type->quals.lq->unif_bits & LAYOUT_STD430)
   {
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "std430 may not be applied to uniform blocks");
   }
   if(fs_type->quals.invariant) {
      glsl_compile_error(ERROR_SEMANTIC, 34, g_LineNumber, NULL);
   }
   if (!g_InGlobalScope) {
      glsl_compile_error(ERROR_SEMANTIC, 47, g_LineNumber, NULL);
   }

   Symbol *symbol = malloc_fast(sizeof(Symbol));
   glsl_symbol_construct_interface_block(symbol, block_name, fs_type);  // Checks storage class is valid
   glsl_symbol_table_insert(table, symbol);
   glsl_shader_interfaces_update(g_ShaderInterfaces, symbol);

   return symbol;
}

Symbol *glsl_commit_var_instance(SymbolTable *table, const char *name,
                                 FullySpecType *fs_type, const_value *compile_time_value,
                                 Symbol *block_symbol)
{
   Symbol *symbol = malloc_fast(sizeof(Symbol));

   check_singleton_is_instantiable(name, fs_type->type);
   glsl_check_namespace_nonfunction(table, name);

   SymbolType *type = fs_type->type;
   if(is_array_of_blocks(type)) {
      // We are constructing the symbol for an interface block instance.
      SymbolType *block_type;

      if(type->flavour==SYMBOL_BLOCK_TYPE)
         block_type = type;
      else
         block_type = type->u.array_type.member_type;

      assert(block_symbol == NULL);
      block_symbol = glsl_symbol_table_lookup(table, block_type->name);
      assert(block_symbol);
   }

   glsl_symbol_construct_var_instance(symbol, name, fs_type, compile_time_value, block_symbol);

   glsl_symbol_table_insert(table, symbol);
   glsl_shader_interfaces_update(g_ShaderInterfaces, symbol);
   return symbol;
}

void glsl_commit_function_param(FunctionBuilder *fb, const char *name, SymbolType *type, QualList *quals, ExprChain *size)
{
   Symbol *symbol;
   Qualifiers q;
   ParamQualifier pq;

   param_quals_from_list(&q, &pq, quals);

   if(size != NULL) {
      type = glsl_build_array_type(type, size);
      glsl_error_if_type_incomplete(type);
   }

   // Ignore nameless parameters with void type, for now at least.
   if (type == &primitiveTypes[PRIM_VOID] && name == NULL) {
      fb->VoidCount++;
      return;
   }

   if (name == NULL) {
      char c[16];
      snprintf(c, sizeof(c), "$$anon%d", fb->ParamCount);
      name = glsl_intern(c, true);
   }

   check_singleton_is_instantiable(name, type);
   glsl_check_namespace_nonfunction(fb->Params, name);

   if( (q.sq == STORAGE_CONST) && (pq == PARAM_QUAL_INOUT || pq == PARAM_QUAL_OUT)) {
      // const type qualifier cannot be used with out or inout parameter qualifier.
      glsl_compile_error(ERROR_CUSTOM, 10, g_LineNumber, NULL);
      return;
   }

   assert(q.sq == STORAGE_NONE || q.sq == STORAGE_CONST);
   assert(!q.invariant);

   symbol = malloc_fast(sizeof(Symbol));
   glsl_symbol_construct_param_instance(symbol, name, type, q.sq, pq);
   glsl_symbol_table_insert(fb->Params, symbol);

   TRACE(("added symbol <%s> as function parameter %d\n", name, fb->ParamCount));
   fb->ParamCount++;
}

Symbol *glsl_commit_singleton_function_declaration(SymbolTable *table, const char *name, SymbolType *type, bool definition)
{
   Symbol *symbol = malloc_fast(sizeof(Symbol));

   if (!g_InGlobalScope)
      glsl_compile_error(ERROR_CUSTOM, 24, g_LineNumber, NULL);

   /* Check that this doesn't have type void or use a 'gl_' name */
   check_singleton_is_instantiable(name, type);

   // Special case: main must have the signature "void main()"
   if (strcmp("main", name) == 0)
   {
      if (type->u.function_type.return_type != &primitiveTypes[PRIM_VOID] ||
          type->u.function_type.param_count != 0)
      {
         glsl_compile_error(ERROR_SEMANTIC, 29, g_LineNumber, NULL);
         return NULL;
      }
   }

   /* Check whether we are overloading or redeclaring an existing function*/
   Symbol *overload = glsl_symbol_table_lookup(table, name);
   if (overload)
   {

      if (overload->flavour != SYMBOL_FUNCTION_INSTANCE)
      {
         glsl_compile_error(ERROR_SEMANTIC, 22, g_LineNumber, "%s already defined as a non-function", name);
         return NULL;
      }
      if(!glsl_check_is_function_overloadable(overload)) {
         glsl_compile_error(ERROR_SEMANTIC, 31, g_LineNumber, NULL);
         return NULL;
      }

      // Check to see if we are actually re-declaring, rather than overloading
      Symbol *match = glsl_resolve_overload_using_prototype(overload, type);
      if (match)
      {
         if (!definition) {
            if (g_ShaderVersion == GLSL_SHADER_VERSION(1, 0, 1) && match->u.function_instance.has_prototype)
               glsl_compile_error(ERROR_SEMANTIC, 22, g_LineNumber, "Illegal redeclaration of function %s", name);
            match->u.function_instance.has_prototype = true;
         }

         // There is already a function with our signature in the symbol table, so don't add a new one.
         TRACE(("declaration of function symbol <%s> matches existing declaration\n", name));
         // Update parameter names to match this declaration, but only if the
         // function hasn't been defined. If it has then changing the type will
         // mess up its parameter storage, and we don't need the names.
         if (match->u.function_instance.function_def == NULL)
            match->type = type;

         // Return the existing symbol
         return match;
      }

      // We have a new overload. Chain the other overloads onto ours.
      TRACE(("declaration of function symbol <%s> is new overload\n", name));
   }

   glsl_symbol_construct_function_instance(symbol, name, type, NULL, overload, !definition);

   // At this point we have a fresh declaration to insert into the symbol table.
   TRACE(("adding symbol <%s> as function\n", name));
   glsl_symbol_table_insert(table, symbol);
   return symbol;
}

void glsl_complete_array_type(SymbolType *type, int member_count)
{
   assert(type->flavour==SYMBOL_ARRAY_TYPE);

   SymbolType *member_type = type->u.array_type.member_type;
   if (member_count <= 0)
   {
      // Array size must be greater than zero.
      glsl_compile_error(ERROR_SEMANTIC, 17, g_LineNumber, NULL);
      return;
   }

   if(type->u.array_type.member_count) //type already complete
      assert(type->u.array_type.member_count == (unsigned)member_count);

   // Write the complete typename
   type->name = NULL;
   if(member_type->name) {
      type->name = glsl_intern(asprintf_fast("%s[%d]", member_type->name, member_count), false);
   }
   type->u.array_type.member_count = member_count;
   type->scalar_count = member_count*member_type->scalar_count;
}

/* build preliminary (perhaps incomplete) type */
static SymbolType *make_array_type(SymbolType *member_type, Expr* size)
{
   if (member_type == &primitiveTypes[PRIM_VOID]) {
      glsl_compile_error(ERROR_CUSTOM, 18, g_LineNumber, NULL);
      return NULL;
   }

   if (g_ShaderVersion < GLSL_SHADER_VERSION(3, 10, 1) &&
       member_type->flavour == SYMBOL_ARRAY_TYPE)
   {
      // members cannot be arrays: multi-dimensional array not allowed
      glsl_compile_error(ERROR_CUSTOM, 27, g_LineNumber, NULL);
      return NULL;
   }

   SymbolType *type                = malloc_fast(sizeof(SymbolType));
   type->flavour                   = SYMBOL_ARRAY_TYPE;
   type->u.array_type.member_type  = member_type;
   type->u.array_type.member_count = 0;
   type->name                      = NULL;

   if (size != NULL) {
      if (size->type != &primitiveTypes[PRIM_INT] && size->type != &primitiveTypes[PRIM_UINT])
         glsl_compile_error(ERROR_SEMANTIC, 15, g_LineNumber, "found type %s", size->type->name);
      if (!size->compile_time_value)
         glsl_compile_error(ERROR_SEMANTIC, 15, g_LineNumber, "not a constant expression");

      glsl_complete_array_type(type, *(int*)size->compile_time_value);
      type->name = asprintf_fast("%s[%d]", member_type->name, type->u.array_type.member_count);
   } else {
      type->name = asprintf_fast("%s[]", member_type->name);
   }

   return type;
}

/* build preliminary (perhaps incomplete) type */
SymbolType *glsl_build_array_type(SymbolType *member_type, ExprChain *array_specifier) {
   SymbolType *ret = member_type;
   for (ExprChainNode *n = array_specifier->last; n != NULL; n = n->prev) {
      ret = make_array_type(ret, n->expr);
   }
   return ret;
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

static bool is_array(const SymbolType *ty) { return (ty->flavour == SYMBOL_ARRAY_TYPE); }
static bool is_struct(const SymbolType *ty) { return (ty->flavour == SYMBOL_STRUCT_TYPE); }

static bool is_integer(const SymbolType *ty)
{
   return ((ty->flavour == SYMBOL_PRIMITIVE_TYPE ) &&
           (primitiveTypeFlags[ty->u.primitive_type.index] & (PRIM_INT_TYPE | PRIM_UINT_TYPE)));
}

static bool is_float(const SymbolType *ty)
{
   return ((ty->flavour == SYMBOL_PRIMITIVE_TYPE) &&
           (primitiveTypeFlags[ty->u.primitive_type.index] & PRIM_FLOAT_TYPE));
}

static bool is_matrix(const SymbolType *ty)
{
   return ((ty->flavour == SYMBOL_PRIMITIVE_TYPE) &&
           (primitiveTypeFlags[ty->u.primitive_type.index] & PRIM_MATRIX_TYPE));
}

static void check_vo_fi_type_es2(const SymbolType *ty, const char *name)
{
   if (is_array(ty)) ty = ty->u.array_type.member_type;

   if (!is_float(ty))
      glsl_compile_error(ERROR_SEMANTIC, 48, g_LineNumber, "\"%s\"", name);
}

/*
   Vertex Output and Fragment Input are identical.
   - floats
   - int BUT only if declared FLAT
   - structs of the above
   - arrays of the above
*/
static void check_vo_fi_type(SymbolType *ty, int has_flat, const char *name)
{
   if (is_array(ty)) {
      if (is_array(ty->u.array_type.member_type) || is_struct(ty->u.array_type.member_type))
         glsl_compile_error(ERROR_SEMANTIC, 48, g_LineNumber, "\"%s\"", name);

      check_vo_fi_type(ty->u.array_type.member_type, has_flat, name);
   }
   else if (is_float(ty))
      /* Ok */;
   else if (is_integer(ty))
   {
      /* Must have FLAT qualifier */
      if (!has_flat)
         glsl_compile_error(ERROR_SEMANTIC, 48, g_LineNumber, "\"%s\" must be qualified flat", name);
   }
   else if (is_struct(ty))
   {
      for (unsigned i = 0; i < ty->u.struct_type.member_count; i++ )
      {
         SymbolType *sty = ty->u.struct_type.member[i].type;
         if (is_array(sty) || is_struct(sty))
            glsl_compile_error(ERROR_SEMANTIC, 48, g_LineNumber, "\"%s\"", name);

         check_vo_fi_type(sty, has_flat, ty->u.struct_type.member[i].name);
      }
   }
   else
      glsl_compile_error(ERROR_SEMANTIC, 48, g_LineNumber, "\"%s\"", name);
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

Symbol *glsl_commit_variable_instance(SymbolTable *table, const PrecisionTable *prec, unsigned *atomic_offset,
                                      QualList *quals, SymbolType *type, const char *name,
                                      ExprChain *array_size, Expr *initialiser)
{
   Qualifiers q;
   qualifiers_from_list(&q, quals);
   if (q.pq == PREC_NONE) q.pq = glsl_prec_get_prec(prec, type);

   const char *error_name = name ? name : "<empty declaration>";

   if (array_size != NULL)
      type = glsl_build_array_type(type, array_size);

   /* Validate global ins and outs based on the type of shader. */
   if (g_ShaderFlavour == SHADER_VERTEX && q.sq == STORAGE_IN)
   {
      bool ints_allowed = (g_ShaderVersion >= GLSL_SHADER_VERSION(3,0,1));
      /* Supports floats but no arrays. Ints are OK from ES3 */
      if ( !is_float(type) && !(is_integer(type) && ints_allowed) )
         glsl_compile_error(ERROR_SEMANTIC, 49, g_LineNumber, "\"%s\"", error_name);
   }
   else if (g_ShaderFlavour == SHADER_FRAGMENT && q.sq == STORAGE_OUT)
   {
      /* Only possible in ES3. Supports floats, ints, arrays but not matrices */
      const SymbolType *base_type = type;
      if (is_array(base_type)) base_type = base_type->u.array_type.member_type;

      if ( !(is_integer(base_type) || is_float(base_type)) || is_matrix(base_type))
         glsl_compile_error(ERROR_SEMANTIC, 50, g_LineNumber, "\"%s\"", error_name);
      if (q.tq == TYPE_QUAL_CENTROID)
         glsl_compile_error(ERROR_SEMANTIC, 15, g_LineNumber, "Use of centroid out in a fragment shader");
   }
   else if ((g_ShaderFlavour == SHADER_VERTEX   && q.sq == STORAGE_OUT) ||
            (g_ShaderFlavour == SHADER_FRAGMENT && q.sq == STORAGE_IN)    )
   {
      if (g_ShaderVersion == GLSL_SHADER_VERSION(1,0,1))
         check_vo_fi_type_es2(type, error_name);
      else
         check_vo_fi_type(type, q.tq == TYPE_QUAL_FLAT, error_name);
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
            max_binding = GLXX_CONFIG_MAX_TEXTURE_UNITS - 1;
            binding_used = q.lq->binding + type->scalar_count - 1;
         }

         if (binding_used > max_binding)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "binding %d greater than max allowed for %s (%d)",
                                                               binding_used, type->name, max_binding);
      }

      if (q.lq->qualified & LOC_QUALED) {
         StorageQualifier sq = q.sq;
         if (sq != STORAGE_IN && sq != STORAGE_OUT && sq != STORAGE_UNIFORM)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "location qualifier not valid for %s",
                                                               glsl_storage_qual_string(sq));

         if (g_ShaderVersion < GLSL_SHADER_VERSION(3, 10, 1)) {
            if (sq == STORAGE_IN && g_ShaderFlavour != SHADER_VERTEX)
               glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "input location qualifier only valid in vertex shader");
            if (sq == STORAGE_OUT && g_ShaderFlavour != SHADER_FRAGMENT)
               glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "output location qualifier only valid in fragment shader");
         }
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

   if (glsl_type_contains(type, PRIM_ATOMIC_TYPE) && (!q.lq || !(q.lq->qualified & BINDING_QUALED)))
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "%s type requires binding qualifier", type->name);

   if (q.sq == STORAGE_BUFFER)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "%s: all buffer variables must be in blocks", error_name);

   if (q.sq == STORAGE_SHARED)
      glsl_compile_error(WARNING, 3, g_LineNumber, "Shared storage qualifier");

   if (type->flavour==SYMBOL_ARRAY_TYPE) {
      if (initialiser != NULL)
         glsl_complete_array_from_init_type(type, initialiser->type);

      glsl_error_if_type_incomplete(type);
   }

   if (is_arrays_of_atomic_type(type)) {
      /* Process the offset */
      assert(q.lq->qualified & BINDING_QUALED);
      if (q.lq->qualified & OFFSET_QUALED) atomic_offset[q.lq->binding] = q.lq->offset;
      else {
         q.lq->qualified = q.lq->qualified | OFFSET_QUALED;
         q.lq->offset = atomic_offset[q.lq->binding];
      }
      if (name != NULL)
         atomic_offset[q.lq->binding] += 4 * type->scalar_count;
   }

   if(q.invariant && q.sq != STORAGE_OUT) {
      if (g_ShaderVersion != GLSL_SHADER_VERSION(1,0,1))
         glsl_compile_error(ERROR_SEMANTIC, 34, g_LineNumber, NULL);
      else if (g_ShaderVersion != SHADER_FRAGMENT || q.sq != STORAGE_IN)
         glsl_compile_error(ERROR_SEMANTIC, 34, g_LineNumber, NULL);
   }

   if(g_InGlobalScope && initialiser && initialiser->compile_time_value == NULL)
      glsl_compile_error(ERROR_SEMANTIC, 13, g_LineNumber, "Global variable initialisers must be constant expressions");

   if (q.sq == STORAGE_IN     || q.sq == STORAGE_OUT || q.sq == STORAGE_UNIFORM ||
       q.sq == STORAGE_BUFFER || q.sq == STORAGE_SHARED                             )
   {
      if (initialiser)
         glsl_compile_error(ERROR_SEMANTIC, 13, g_LineNumber, "Interface variables may not be initialised");
      if (!g_InGlobalScope)
         glsl_compile_error(ERROR_SEMANTIC, 47, g_LineNumber, NULL);
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

   if (name != NULL) {
      /* XXX These checks only happen when declaring variables, contrary to what the spec says */
      /* TODO: Ensure that all these checks are applied correctly when name != NULL */
      if(q.sq != STORAGE_UNIFORM && glsl_type_contains_opaque(type) && name != NULL)
         glsl_compile_error(ERROR_CUSTOM, 22, g_LineNumber, NULL);

      void *compile_time_value = NULL;
      if (q.sq == STORAGE_CONST) {
         if (!initialiser)
            glsl_compile_error(ERROR_SEMANTIC, 13, g_LineNumber, "const variable '%s' requires an initialiser", name);

         if (!initialiser->compile_time_value)
            glsl_compile_error(ERROR_SEMANTIC, 13, g_LineNumber, "const variable initialisers must be constant expressions");

         compile_time_value = initialiser->compile_time_value;
         TRACE_CONSTANT(fs_type->type, compile_time_value);
      }

      FullySpecType fs_type = { .quals = q, .type = type };
      TRACE(("adding symbol <%s> as instance\n", name));
      return glsl_commit_var_instance(table, name, &fs_type, compile_time_value, NULL);
   } else
      return NULL;
}

static void validate_member_layout(LayoutQualifier *member_lq)
{
   if (!member_lq) return;

   if(member_lq->qualified & LOC_QUALED)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "location layout may not be applied to block members");

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
                            const char *name, FullySpecType *fs_type)
{
   StructMember *member = malloc_fast(sizeof(StructMember));
   member->name   = name;
   member->type   = fs_type->type;
   member->layout = fs_type->quals.lq;
   member->prec   = fs_type->quals.pq;

   check_singleton_is_instantiable(name, fs_type->type);
   if (glsl_map_get(struct_map, name) != NULL)
      glsl_compile_error(ERROR_SEMANTIC, 22, g_LineNumber, "%s", name);

   glsl_map_put(struct_map, name, member);
}

static void glsl_map_and_check_members(Map *struct_map, StorageQualifier valid_sq,
                                       Statement *statement, LayoutQualifier *block_lq)
{
   assert(statement->flavour==STATEMENT_STRUCT_DECL);
   FullySpecType fs_type;
   fs_type.type = statement->u.struct_decl.type;
   qualifiers_from_list(&fs_type.quals, statement->u.struct_decl.quals);

   if(block_lq) {
      LayoutQualifier *member_lq = fs_type.quals.lq;
      validate_member_layout(member_lq);
      fs_type.quals.lq = glsl_create_mixed_lq(member_lq, block_lq);
   }

   if(fs_type.quals.sq != STORAGE_NONE && fs_type.quals.sq != valid_sq)
      glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "expected \"%s\", found \"%s\"",
                                                         glsl_storage_qual_string(valid_sq),
                                                         glsl_storage_qual_string(fs_type.quals.sq));

   for(StatementChainNode *node=statement->u.struct_decl.members->first; node; node=node->next)
   {
      assert(node->statement->flavour==STATEMENT_STRUCT_MEMBER_DECL);

      if (node->statement->u.struct_member_decl.array_specifier != NULL)
         fs_type.type = glsl_build_array_type(fs_type.type, node->statement->u.struct_member_decl.array_specifier);

      if (fs_type.type->flavour == SYMBOL_ARRAY_TYPE)
         glsl_error_if_type_incomplete(fs_type.type);

      glsl_map_member(struct_map, valid_sq, node->statement->u.struct_member_decl.name, &fs_type);
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
   resultType->name         = name;
   resultType->scalar_count = 0;    /* Added as we go along */

   StructMember *memb = malloc_fast(sizeof(StructMember) * members->count);

   int i;
   MapNode *map_node;
   for (map_node = members->head, i = 0; map_node; map_node = map_node->next, i++)
   {
      StructMember *member = map_node->v;
      memb[i].name   = member->name;
      memb[i].type   = member->type;
      memb[i].layout = (flavour == SYMBOL_STRUCT_TYPE) ? NULL : member->layout;
      memb[i].prec   = member->prec;

      resultType->scalar_count += member->type->scalar_count;
   }
   assert(i == members->count);

   if(flavour==SYMBOL_STRUCT_TYPE) {
      resultType->u.struct_type.member_count = members->count;
      resultType->u.struct_type.member       = memb;
   } else if(flavour==SYMBOL_BLOCK_TYPE) {
      resultType->u.block_type.member_count = members->count;
      resultType->u.block_type.member       = memb;
   }

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
   type->u.block_type.layout = malloc_fast(sizeof(MEMBER_LAYOUT_T));
   calculate_block_layout(type->u.block_type.layout, type);
   type->u.block_type.has_named_instance = false;

   return type;
}

void glsl_commit_anonymous_block_members(SymbolTable *table, Symbol *symbol)
{
   SymbolType *type = symbol->type;

   assert(symbol->flavour==SYMBOL_INTERFACE_BLOCK);
   assert(type->flavour==SYMBOL_BLOCK_TYPE);

   for(unsigned i = 0; i < type->u.block_type.member_count; i++)
   {
      FullySpecType fs_member_type;
      const char *member_name        = type->u.block_type.member[i].name;
      fs_member_type.type            = type->u.block_type.member[i].type;
      fs_member_type.quals.invariant = false;
      fs_member_type.quals.sq        = symbol->u.interface_block.sq;
      fs_member_type.quals.tq        = TYPE_QUAL_NONE;
      fs_member_type.quals.lq        = type->u.block_type.member[i].layout;

      glsl_commit_var_instance(table, member_name, &fs_member_type, NULL, symbol);
   }
}

SymbolType *glsl_build_function_type(QualList *return_quals, SymbolType *return_type, FunctionBuilder *args)
{
   Symbol **functionParams = NULL;

   SymbolType *type = malloc_fast(sizeof(SymbolType));

   if (args->VoidCount > 1 || (args->VoidCount == 1 && args->ParamCount > 0))
   {
      // void type can only be used in empty formal parameter list.
      glsl_compile_error(ERROR_CUSTOM, 19, g_LineNumber, NULL);
      return NULL;
   }

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
      return NULL;
   }

   if (args->ParamCount > 0)
   {
      functionParams = (Symbol **)malloc_fast(sizeof(Symbol*) * args->ParamCount);

      // Populate the parameter array.
      int i;
      MapNode *map_node;
      for (map_node = args->Params->map->scopes->map->head, i = 0; map_node; map_node = map_node->next, i++)
      {
         Symbol* symbol = (Symbol*)map_node->v;
         assert(SYMBOL_PARAM_INSTANCE == symbol->flavour);
         functionParams[i] = symbol;
      }
      assert(i == args->ParamCount);

      // Create the type name (working forwards!).
      StringBuilder *sb = glsl_sb_new();
      glsl_sb_append(sb, "(");
      for (i = 0; i < args->ParamCount; i++) {
         if (i > 0) glsl_sb_append(sb, ", ");
         glsl_sb_append(sb, "%s %s ", glsl_storage_qual_string(functionParams[i]->u.param_instance.storage_qual),
                                      glsl_param_qual_string(functionParams[i]->u.param_instance.param_qual));
         glsl_sb_append(sb, "%s", functionParams[i]->type->name);
      }
      glsl_sb_append(sb, ")");
      type->name = glsl_sb_content(sb);
   }
   else
   {
      type->name = "()";
   }

   type->flavour                     = SYMBOL_FUNCTION_TYPE;
   type->scalar_count                = 0;
   type->u.function_type.return_type = return_type;
   type->u.function_type.param_count = args->ParamCount;
   type->u.function_type.params      = functionParams;

   return type;
}

void glsl_instantiate_function_params(SymbolTable *table, SymbolType* fun)
{
   assert(fun->flavour == SYMBOL_FUNCTION_TYPE);

   for (unsigned i = 0; i < fun->u.function_type.param_count; i++)
   {
      // No need to check namespace as we have a clean scope for the function.
      glsl_symbol_table_insert(table, fun->u.function_type.params[i]);
   }
}
