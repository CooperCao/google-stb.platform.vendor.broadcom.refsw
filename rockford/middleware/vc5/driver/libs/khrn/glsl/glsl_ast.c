/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_common.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "glsl_ast.h"
#include "glsl_ast_visitor.h"
#include "glsl_builders.h"
#include "glsl_const_operators.h"
#include "glsl_errors.h"
#include "glsl_extensions.h"
#include "glsl_fastmem.h"
#include "glsl_globals.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"
#include "glsl_symbols.h"
#include "glsl_trace.h"
#include "glsl_layout.h"

//
// Utilities.
//

static bool swizzle_valid_as_lvalue(unsigned char *swizzle_slots)
{
   // Swizzles that contain repeats are not allowed as lvalues (eg. a.xx)
   SWIZZLE_FIELD_FLAGS_T swizzle_fields_in_use = 0;

   for (int i = 0; i < MAX_SWIZZLE_FIELD_COUNT; i++)
   {
      if (swizzle_slots[i] != SWIZZLE_SLOT_UNUSED)
      {
         SWIZZLE_FIELD_FLAGS_T swizzle_field_flag = 1 << swizzle_slots[i];
         if (swizzle_fields_in_use & swizzle_field_flag)
            return false;

         swizzle_fields_in_use |= swizzle_field_flag;
      }
   }
   return true;
}

MemoryQualifier glsl_get_mem_flags(Expr *expr) {
   switch (expr->flavour) {
      case EXPR_INSTANCE:
      {
         Symbol *symbol = expr->u.instance.symbol;

         if (symbol->flavour == SYMBOL_TEMPORARY)
            return MEMORY_NONE;

         assert(symbol->flavour == SYMBOL_VAR_INSTANCE ||
                symbol->flavour == SYMBOL_PARAM_INSTANCE);

         if (symbol->flavour == SYMBOL_VAR_INSTANCE)
            return symbol->u.var_instance.mem_qual;
         else
            return symbol->u.param_instance.mem_qual;
      }

      case EXPR_SUBSCRIPT:
         return glsl_get_mem_flags(expr->u.subscript.aggregate);

      case EXPR_FIELD_SELECTOR:
      {
         Expr *aggregate = expr->u.field_selector.aggregate;
         MemoryQualifier mq = glsl_get_mem_flags(aggregate);
         MemoryQualifier field_mq;
         SymbolType *t = aggregate->type;
         assert(t->flavour == SYMBOL_STRUCT_TYPE || t->flavour == SYMBOL_BLOCK_TYPE);
         if (t->flavour == SYMBOL_STRUCT_TYPE)
            field_mq = t->u.struct_type.member[expr->u.field_selector.field_no].memq;
         else
            field_mq = t->u.block_type.member[expr->u.field_selector.field_no].memq;

         return mq | field_mq;
      }

      case EXPR_SWIZZLE:
         return glsl_get_mem_flags(expr->u.swizzle.aggregate);

      default:
         return MEMORY_NONE;
   }
}

bool glsl_is_lvalue(Expr *expr)
{
   switch (expr->flavour)
   {
      case EXPR_INSTANCE:
      {
         Symbol *symbol = expr->u.instance.symbol;

         assert(symbol->flavour == SYMBOL_VAR_INSTANCE ||
                symbol->flavour == SYMBOL_PARAM_INSTANCE);

         MemoryQualifier mq;
         StorageQualifier sq;
         if (symbol->flavour == SYMBOL_VAR_INSTANCE) {
            mq = symbol->u.var_instance.mem_qual;
            sq = symbol->u.var_instance.storage_qual;
         } else {
            mq = symbol->u.param_instance.mem_qual;
            sq = symbol->u.param_instance.storage_qual;
         }

         if (mq & MEMORY_READONLY)
            return false;

         switch (sq)
         {
            case STORAGE_CONST:
            case STORAGE_UNIFORM:
            case STORAGE_IN:
               return false;

            case STORAGE_OUT:
            case STORAGE_SHARED:
            case STORAGE_BUFFER:
            case STORAGE_NONE:
               return true;

            default:
               unreachable();
               return false;
         }
      }

      case EXPR_SUBSCRIPT:
         return glsl_is_lvalue(expr->u.subscript.aggregate);

      case EXPR_FIELD_SELECTOR:
      {
         SymbolType *t = expr->u.field_selector.aggregate->type;
         assert(t->flavour == SYMBOL_STRUCT_TYPE || t->flavour == SYMBOL_BLOCK_TYPE);
         if (t->flavour == SYMBOL_STRUCT_TYPE) {
            if (t->u.struct_type.member[expr->u.field_selector.field_no].memq & MEMORY_READONLY)
               return false;
         } else {
            if (t->u.block_type.member[expr->u.field_selector.field_no].memq & MEMORY_READONLY)
               return false;
         }
         return glsl_is_lvalue(expr->u.field_selector.aggregate);
      }

      case EXPR_SWIZZLE:
         if (!swizzle_valid_as_lvalue(expr->u.swizzle.swizzle_slots))
            return false;

         return glsl_is_lvalue(expr->u.swizzle.aggregate);

      default:
         return false;
   }
}

static bool valid_for_atomic(Expr *expr)
{
   switch (expr->flavour)
   {
      case EXPR_INSTANCE:
      {
         Symbol *symbol = expr->u.instance.symbol;

         assert(symbol->flavour == SYMBOL_VAR_INSTANCE ||
                symbol->flavour == SYMBOL_PARAM_INSTANCE);

         if (symbol->flavour != SYMBOL_VAR_INSTANCE) return false;

         StorageQualifier sq = symbol->u.var_instance.storage_qual;
         return sq == STORAGE_BUFFER || sq == STORAGE_SHARED;
      }

      case EXPR_SUBSCRIPT:
         return valid_for_atomic(expr->u.subscript.aggregate);

      case EXPR_FIELD_SELECTOR:
         return valid_for_atomic(expr->u.field_selector.aggregate);

      case EXPR_SWIZZLE:
         return valid_for_atomic(expr->u.swizzle.aggregate);

      default:
         return false;
   }
}


/*
   void check_returns(SymbolType* return_type, Statement* statement)

   Checks for:
   - void function with value return statement;
   - non-void function with plain return statement;
   - non-void function value return statement type mismatch.
*/
static void spostv_check_returns(Statement *s, void *data) {
   SymbolType *function_type = data;

   if (s->flavour == STATEMENT_RETURN ||
       s->flavour == STATEMENT_RETURN_EXPR)
   {
      SymbolType *returned_type = &primitiveTypes[PRIM_VOID];

      if (s->flavour == STATEMENT_RETURN_EXPR)
         returned_type = s->u.return_expr.expr->type;

      if (!glsl_shallow_match_nonfunction_types(returned_type, function_type))
      {
         glsl_compile_error(ERROR_SEMANTIC, 1, s->line_num, "return type %s does not match function definition %s",
                            returned_type->name, function_type->name);
      }
   }
}

static void check_returns(SymbolType *return_type, Statement *statement) {
   glsl_statement_accept(statement, return_type, NULL, NULL, spostv_check_returns, NULL);
}

struct validate_data {
   ShaderFlavour flavour;
   int           version;

   int loop_depth;
   int switch_depth;
};

static Statement *sprev_mark_entries(Statement *s, void *data) {
   struct validate_data *info = data;
   switch (s->flavour) {
      case STATEMENT_ITERATOR_FOR:
      case STATEMENT_ITERATOR_WHILE:
      case STATEMENT_ITERATOR_DO_WHILE:
         info->loop_depth++;
         break;
      case STATEMENT_SWITCH:
         info->switch_depth++;
         break;
      default:
         break;
   }
   return s;
}

static void reject_ins_outs(const Statement *s) {
   assert(s->flavour == STATEMENT_VAR_DECL);

   if (s->u.var_decl.quals == NULL) return;

   for (QualListNode *n = s->u.var_decl.quals->head; n; n=n->next) {
      if (n->q->flavour == QUAL_STORAGE && ( n->q->u.storage == STORAGE_IN ||
                                             n->q->u.storage == STORAGE_OUT  ) )
         glsl_compile_error(ERROR_SEMANTIC, 15, s->line_num, "in and out invalid in compute shaders");
   }
}

static void validate_qualifier_default(const Statement *s, ShaderFlavour flavour, int version) {
   StorageQualifier sq = STORAGE_NONE;
   for (QualListNode *qn = s->u.qualifier_default.quals->head; qn; qn=qn->next)
      if (qn->q->flavour == QUAL_STORAGE) sq = qn->q->u.storage;

   for (QualListNode *qn = s->u.qualifier_default.quals->head; qn; qn=qn->next) {
      if (qn->q->flavour == QUAL_LAYOUT) {
         for (LayoutIDList *idn = qn->q->u.layout; idn; idn=idn->next) {
            switch(idn->l->id) {
               case LQ_SHARED:
               case LQ_PACKED:
               case LQ_STD140:
               case LQ_STD430:
               case LQ_ROW_MAJOR:
               case LQ_COLUMN_MAJOR:
                  if (sq != STORAGE_UNIFORM && sq != STORAGE_BUFFER)
                     glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "Block qualifier only valid for uniform and buffer interfaces");
                  break;
               case LQ_TRIANGLES:
                  if (sq != STORAGE_IN || (flavour != SHADER_TESS_EVALUATION && flavour != SHADER_GEOMETRY))
                     glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "'trianlges'qualifier only valid for tess evaluation or geometry input");
                  break;
               case LQ_QUADS:
               case LQ_ISOLINES:
               case LQ_SPACING_EQUAL:
               case LQ_SPACING_FRACTIONAL_EVEN:
               case LQ_SPACING_FRACTIONAL_ODD:
               case LQ_CW:
               case LQ_CCW:
               case LQ_POINT_MODE:
                  if (sq != STORAGE_IN || flavour != SHADER_TESS_EVALUATION)
                     glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "Tessellation mode qualifier only valid for tess evaluation input");
                  break;
               case LQ_POINTS:
                  if (flavour != SHADER_GEOMETRY || (sq != STORAGE_IN && sq != STORAGE_OUT))
                     glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "'points' qualifiers only valid for geometry input/output");
                  break;
               case LQ_LINES:
               case LQ_LINES_ADJACENCY:
               case LQ_TRIANGLES_ADJACENCY:
                  if (flavour != SHADER_GEOMETRY || sq != STORAGE_IN)
                     glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "Primitive type only valid for geometry input");
                  break;
               case LQ_INVOCATIONS:
                  if (flavour != SHADER_GEOMETRY || sq != STORAGE_IN)
                     glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "'invocations' qualifier only valid for geometry input");
                  break;
               case LQ_EARLY_FRAGMENT_TESTS:
                  if (flavour != SHADER_FRAGMENT || sq != STORAGE_IN || version < GLSL_SHADER_VERSION(3,10,1))
                     glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "'early_fragment_tests' layout only valid for fragment input (version >= 310)");
                  break;
               case LQ_SIZE_X:
               case LQ_SIZE_Y:
               case LQ_SIZE_Z:
                  if (flavour != SHADER_COMPUTE || sq != STORAGE_IN)
                     glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "Compute size qualifiers only valid for compute input");
                  break;
               case LQ_VERTICES:
                  if (flavour != SHADER_TESS_CONTROL || sq != STORAGE_OUT)
                     glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "'vertices' qualifiers only valid for tess control output");
                  break;
               case LQ_LINE_STRIP:
               case LQ_TRIANGLE_STRIP:
                  if (flavour != SHADER_GEOMETRY || sq != STORAGE_OUT)
                     glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "Primitive type only valid for geometry output");
                  break;
               case LQ_MAX_VERTICES:
                  if (flavour != SHADER_GEOMETRY || sq != STORAGE_OUT)
                     glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "'max_vertices' only valid for geometry output");
                  break;
               default:
                  glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "Layout qualifier not valid for default declarations");
            }
         }
      }
   }
}

/* Determine whether the given variable may be declared invariant. Outputs can
   always be invariant. Additionally fragment shader inputs may be invariant in
   language version 100.  */
bool invariant_decl_valid(Symbol *var, ShaderFlavour flavour, int version) {
   if (var->flavour != SYMBOL_VAR_INSTANCE) return false;

   StorageQualifier sq = var->u.var_instance.storage_qual;
   if (sq == STORAGE_OUT) return true;
   if (version == GLSL_SHADER_VERSION(1, 0, 1) &&
       sq == STORAGE_IN && flavour == SHADER_FRAGMENT) return true;

   return false;
}

static void validate_qualifier_augment(const Statement *s, ShaderFlavour flavour, int version) {
   int invariant = 0, precise = 0, other = 0;
   for (QualListNode *qn = s->u.qualifier_augment.quals->head; qn; qn = qn->next) {
      if      (qn->q->flavour == QUAL_INVARIANT) invariant++;
      else if (qn->q->flavour == QUAL_PRECISE)   precise++;
      else other++;
   }

   if (other > 0)
      glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "Invalid requalification");

   if (invariant > 1 || precise > 1)
      glsl_compile_error(ERROR_CUSTOM, 15, s->line_num, "repeated qualifier");

   if (invariant > 0) {
      for (SymbolListNode *n = s->u.qualifier_augment.vars->head; n; n=n->next) {
         if(!invariant_decl_valid(n->s, flavour, version))
            glsl_compile_error(ERROR_SEMANTIC, 34, s->line_num, "%s", n->s->name);
      }
   }
}

static void spostv_check_and_mark_exits(Statement *s, void *data) {
   /* First check that writeonly isn't misapplied */
   MemoryQualifier mq = MEMORY_NONE;
   switch (s->flavour) {
      case STATEMENT_VAR_DECL:
         if (s->u.var_decl.initializer)
            mq = glsl_get_mem_flags(s->u.var_decl.initializer);
         break;
      case STATEMENT_EXPR:
         mq = glsl_get_mem_flags(s->u.expr.expr);
         break;
      case STATEMENT_SELECTION:
         mq = glsl_get_mem_flags(s->u.selection.cond);
         break;
      case STATEMENT_ITERATOR_FOR:
         if (s->u.iterator_for.loop)
            mq = glsl_get_mem_flags(s->u.iterator_for.loop);
         break;
      case STATEMENT_ITERATOR_DO_WHILE:
         mq = glsl_get_mem_flags(s->u.iterator_do_while.cond);
         break;
      case STATEMENT_SWITCH:
         mq = glsl_get_mem_flags(s->u.switch_stmt.cond);
         break;
      case STATEMENT_CASE:
         mq = glsl_get_mem_flags(s->u.case_stmt.expr);
         break;
      case STATEMENT_RETURN_EXPR:
         mq = glsl_get_mem_flags(s->u.return_expr.expr);
         break;
      default:
         /* No checking needed */
         break;
   }
   if (mq & MEMORY_WRITEONLY)
      glsl_compile_error(ERROR_SEMANTIC, 5, s->line_num, NULL);

   struct validate_data *info = data;
   if (s->flavour == STATEMENT_DISCARD && info->flavour != SHADER_FRAGMENT)
      glsl_compile_error(ERROR_CUSTOM, 12, s->line_num, NULL);
   if (s->flavour == STATEMENT_VAR_DECL && info->flavour == SHADER_COMPUTE)
      reject_ins_outs(s);
   if (s->flavour == STATEMENT_QUALIFIER_DEFAULT)
      validate_qualifier_default(s, info->flavour, info->version);
   if (s->flavour == STATEMENT_FUNCTION_DEF)
      check_returns(s->u.function_def.header->type->u.function_type.return_type, s->u.function_def.body);
   if (s->flavour == STATEMENT_QUALIFIER_AUGMENT)
      validate_qualifier_augment(s, info->flavour, info->version);

   /* Now check nesting */
   switch (s->flavour) {
      case STATEMENT_ITERATOR_FOR:
      case STATEMENT_ITERATOR_WHILE:
      case STATEMENT_ITERATOR_DO_WHILE:
         info->loop_depth--;
         break;
      case STATEMENT_SWITCH:
         info->switch_depth--;
         break;

      case STATEMENT_BREAK:
         if (info->loop_depth == 0 && info->switch_depth == 0)
            glsl_compile_error(ERROR_CUSTOM, 23, s->line_num, NULL);
         break;
      case STATEMENT_CONTINUE:
         if (info->loop_depth == 0)
            glsl_compile_error(ERROR_CUSTOM, 23, s->line_num, NULL);
         break;
      default:
         break;
   }
}

static Expr *expr_chain_get_expr(ExprChain *c, int n) {
   ExprChainNode *cn = c->first;
   for (int i=0; i<n; i++) cn = cn->next;
   return cn->expr;
}

static void ensure_arg_n_const(Symbol *called, ExprChain *args, int n) {
   /* Functions arguments are 1 based, so get the n-1-th expression */
   Expr *arg = expr_chain_get_expr(args, n-1);
   if (!arg->compile_time_value)
      glsl_compile_error(ERROR_SEMANTIC, 15, arg->line_num, "in argument to function %s", called->name);
}

static void epostv_validate(Expr *e, void *data) {
   /* Validate that reading things doesn't violate writeonly */
   switch(e->flavour) {
      case EXPR_VALUE:         /* Doesn't read anything */
      case EXPR_ARRAY_LENGTH:
      case EXPR_INTRINSIC:     /* Should already be valid. We constructed these. */
      case EXPR_FUNCTION_CALL: /* Validated elsewhere */
         break;

      case EXPR_INSTANCE:
      case EXPR_SUBSCRIPT:
      case EXPR_FIELD_SELECTOR:
      case EXPR_SWIZZLE:
         break;               /* These can't be validated without more context. Parent must do it */

      case EXPR_POST_INC:
      case EXPR_POST_DEC:
      case EXPR_PRE_INC:
      case EXPR_PRE_DEC:
      case EXPR_ARITH_NEGATE:
      case EXPR_LOGICAL_NOT:
      case EXPR_BITWISE_NOT:
      {
         MemoryQualifier mq = glsl_get_mem_flags(e->u.unary_op.operand);
         if (mq & MEMORY_WRITEONLY)
            glsl_compile_error(ERROR_SEMANTIC, 5, e->line_num, NULL);
         break;
      }

      case EXPR_MUL:
      case EXPR_DIV:
      case EXPR_REM:
      case EXPR_ADD:
      case EXPR_SUB:
      case EXPR_LESS_THAN:
      case EXPR_LESS_THAN_EQUAL:
      case EXPR_GREATER_THAN:
      case EXPR_GREATER_THAN_EQUAL:
      case EXPR_EQUAL:
      case EXPR_NOT_EQUAL:
      case EXPR_LOGICAL_AND:
      case EXPR_LOGICAL_XOR:
      case EXPR_LOGICAL_OR:
      case EXPR_BITWISE_AND:
      case EXPR_BITWISE_XOR:
      case EXPR_BITWISE_OR:
      case EXPR_SHL:
      case EXPR_SHR:
      {
         MemoryQualifier lmq = glsl_get_mem_flags(e->u.binary_op.left);
         MemoryQualifier rmq = glsl_get_mem_flags(e->u.binary_op.right);
         if ( (lmq & MEMORY_WRITEONLY) || (rmq & MEMORY_WRITEONLY) )
            glsl_compile_error(ERROR_SEMANTIC, 5, e->line_num, NULL);
         break;
      }

      case EXPR_PRIM_CONSTRUCTOR_CALL:
         for (ExprChainNode *node = e->u.prim_constructor_call.args->first; node; node=node->next) {
            MemoryQualifier mq = glsl_get_mem_flags(node->expr);
            if (mq & MEMORY_WRITEONLY)
               glsl_compile_error(ERROR_SEMANTIC, 5, e->line_num, NULL);
         }
         break;

      case EXPR_COMPOUND_CONSTRUCTOR_CALL:
         for (ExprChainNode *node = e->u.compound_constructor_call.args->first; node; node=node->next) {
            MemoryQualifier mq = glsl_get_mem_flags(node->expr);
            if (mq & MEMORY_WRITEONLY)
               glsl_compile_error(ERROR_SEMANTIC, 5, e->line_num, NULL);
         }
         break;

      case EXPR_ASSIGN:
      {
         MemoryQualifier rmq = glsl_get_mem_flags(e->u.assign_op.rvalue);
         if (rmq & MEMORY_WRITEONLY)
            glsl_compile_error(ERROR_SEMANTIC, 5, e->line_num, NULL);
         break;
      }
      case EXPR_CONDITIONAL:
      {
         MemoryQualifier cmq = glsl_get_mem_flags(e->u.cond_op.cond);
         MemoryQualifier tmq = glsl_get_mem_flags(e->u.cond_op.if_true);
         MemoryQualifier fmq = glsl_get_mem_flags(e->u.cond_op.if_false);
         if ( (cmq & MEMORY_WRITEONLY) || (tmq & MEMORY_WRITEONLY) || (fmq & MEMORY_WRITEONLY) )
            glsl_compile_error(ERROR_SEMANTIC, 5, e->line_num, NULL);
         break;
      }
      case EXPR_SEQUENCE:
      {
         MemoryQualifier lmq = glsl_get_mem_flags(e->u.sequence.all_these);
         MemoryQualifier rmq = glsl_get_mem_flags(e->u.sequence.then_this);
         if ( (lmq & MEMORY_WRITEONLY) || (rmq & MEMORY_WRITEONLY) )
            glsl_compile_error(ERROR_SEMANTIC, 5, e->line_num, NULL);
         break;
      }
      default:
         unreachable();
         break;
   }

   struct validate_data *d = data;
   if (e->flavour == EXPR_FUNCTION_CALL) {
      Symbol *called = e->u.function_call.function;
      if (called->u.function_instance.function_def == NULL)
         glsl_compile_error(ERROR_CUSTOM, 21, e->line_num, "%s", called->name);

      if (glsl_stdlib_is_stdlib_symbol(called)) {
         uint64_t fn_props = glsl_stdlib_function_properties[glsl_stdlib_function_index(called)];
         if (fn_props & GLSL_STDLIB_PROPERTY_ATOMIC_MEM) {
            if (!valid_for_atomic(e->u.function_call.args->first->expr))
               glsl_compile_error(ERROR_CUSTOM, 21, e->line_num, NULL);
         }

         bool gath_off_const = d->version < GLSL_SHADER_VERSION(3,20,1) && glsl_ext_status(GLSL_EXT_GPU_SHADER5) == GLSL_DISABLED;
         if (fn_props & (GLSL_STDLIB_PROPERTY_ARG3_CONST | (gath_off_const ? GLSL_STDLIB_PROPERTY_ARG3_GATHER_OFFSET : 0)))
            ensure_arg_n_const(called, e->u.function_call.args, 3);
         if (fn_props & (GLSL_STDLIB_PROPERTY_ARG4_CONST | (gath_off_const ? GLSL_STDLIB_PROPERTY_ARG4_GATHER_OFFSET : 0)))
            ensure_arg_n_const(called, e->u.function_call.args, 4);
         if (fn_props & (GLSL_STDLIB_PROPERTY_ARG5_CONST))
            ensure_arg_n_const(called, e->u.function_call.args, 5);
      }
   }

   if (d->version < GLSL_SHADER_VERSION(3,0,1)) {
      switch(e->flavour) {
         case EXPR_BITWISE_AND:
         case EXPR_BITWISE_OR:
         case EXPR_BITWISE_XOR:
         case EXPR_BITWISE_NOT:
         case EXPR_SHL:
         case EXPR_SHR:
         case EXPR_REM:
            glsl_compile_error(ERROR_SEMANTIC, 4, e->line_num, "operator not supported before GLSL version 300 es");

         case EXPR_ASSIGN:
         case EXPR_CONDITIONAL:
            if (glsl_type_contains_array(e->type))
               glsl_compile_error(ERROR_SEMANTIC, 4, e->line_num, "operator not supported for arrays before GLSL version 300 es");
            break;
         case EXPR_EQUAL:
         case EXPR_NOT_EQUAL:
            if (glsl_type_contains_array(e->u.binary_op.left->type))
               glsl_compile_error(ERROR_SEMANTIC, 4, e->line_num, "operator not supported for arrays before GLSL version 300 es");
            break;
         default:
            break;
      }
   }

   if (e->flavour == EXPR_SUBSCRIPT) {
      /* Check that all interface blocks are indexed by constant expressions */
      SymbolType *aggregate_type = e->u.subscript.aggregate->type;
      while (aggregate_type->flavour == SYMBOL_ARRAY_TYPE)
         aggregate_type = aggregate_type->u.array_type.member_type;
      if (aggregate_type->flavour == SYMBOL_BLOCK_TYPE && !e->u.subscript.subscript->compile_time_value) {
         bool shader5 = d->version >= GLSL_SHADER_VERSION(3,20,1) || glsl_ext_status(GLSL_EXT_GPU_SHADER5) != GLSL_DISABLED;
         if (!shader5 || e->u.subscript.aggregate->u.instance.symbol->u.var_instance.storage_qual == STORAGE_BUFFER)
            glsl_compile_error(ERROR_SEMANTIC, 15, e->line_num, "indexing array of interface blocks");
      }
   }
}

static void ast_validate_recursive(Statement *ast, ShaderFlavour flavour, int version) {
   struct validate_data d = { .flavour = flavour, .version = version, .loop_depth = 0, .switch_depth = 0 };
   glsl_statement_accept(ast, &d, sprev_mark_entries, NULL, spostv_check_and_mark_exits, epostv_validate);
}

static void function_check_recursion(Symbol *s, SymbolList *active);

static void epostv_check_recursion(Expr *e, void *data) {
   SymbolList *active = data;
   if (e->flavour == EXPR_FUNCTION_CALL) {
      if (glsl_symbol_list_contains(active, e->u.function_call.function))
         glsl_compile_error(ERROR_SEMANTIC, 30, e->line_num, "recursive call to function <%s>", e->u.function_call.function->name);

      function_check_recursion(e->u.function_call.function, active);
   }
}

static void function_check_recursion(Symbol *f, SymbolList *active) {
   glsl_symbol_list_append(active, f);
   glsl_statement_accept(f->u.function_instance.function_def->u.function_def.body, active, NULL, NULL, NULL, epostv_check_recursion);
   glsl_symbol_list_pop(active);
}

static void ast_validate_recursion(Statement *ast) {
   SymbolList *active = glsl_symbol_list_new();
   for (StatementChainNode *n = ast->u.ast.decls->first; n; n=n->next) {
      Statement *s = n->statement;
      if (s->flavour == STATEMENT_FUNCTION_DEF) function_check_recursion(s->u.function_def.header, active);
   }
}

void glsl_ast_validate(Statement *ast, ShaderFlavour flavour, int version) {
   ast_validate_recursive(ast, flavour, version);

   ast_validate_recursion(ast);

   // Find main function definition.
   Statement *shader_main = NULL;
   for (StatementChainNode *decl = ast->u.ast.decls->first; decl; decl = decl->next) {
      if (decl->statement->flavour == STATEMENT_FUNCTION_DEF) {
         Symbol *h = decl->statement->u.function_def.header;
         if (strcmp(h->name, "main") == 0) {
            // This is good enough as main cannot be overloaded.
            assert(h->u.function_instance.next_overload == NULL);
            shader_main = decl->statement;
            break;
         }
      }
   }

   if (!shader_main || !shader_main->u.function_def.body) {
      // Missing main function definition.
      glsl_compile_error(ERROR_CUSTOM, 13, ast->line_num, NULL);
   }
}

/////////////////////
//
// Functions for constructing Expr objects
//
/////////////////////

Expr *glsl_expr_construct_const_value(int line_num, PrimitiveTypeIndex type_index, const_value v)
{
   assert(type_index == PRIM_INT  || type_index == PRIM_UINT ||
          type_index == PRIM_BOOL || type_index == PRIM_FLOAT);

   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = EXPR_VALUE;
   expr->line_num = line_num;
   expr->type     = &primitiveTypes[type_index];
   expr->compile_time_value = malloc_fast(sizeof(const_value));
   *expr->compile_time_value = v;

   return expr;
}

Expr *glsl_expr_construct_instance(int line_num, Symbol *symbol)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = EXPR_INSTANCE;
   expr->line_num = line_num;
   expr->type     = symbol->type;
   expr->u.instance.symbol = symbol;

   switch (symbol->flavour)
   {
      case SYMBOL_VAR_INSTANCE:
         expr->compile_time_value = symbol->u.var_instance.compile_time_value;
         break;
      case SYMBOL_PARAM_INSTANCE:
      case SYMBOL_TEMPORARY:
         expr->compile_time_value = NULL;
         break;
      default:
         // Identifier is not var or param.
         glsl_compile_error(ERROR_CUSTOM, 1, line_num, "%s", symbol->name);
         return NULL;
   }

   return expr;
}

Expr *glsl_expr_construct_subscript(int line_num, Expr *aggregate, Expr *subscript)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = EXPR_SUBSCRIPT;
   expr->line_num = line_num;
   expr->u.subscript.aggregate = aggregate;
   expr->u.subscript.subscript = subscript;
   expr->compile_time_value = NULL;

   expr->type = NULL;

   if (subscript->type != &primitiveTypes[PRIM_INT] &&
       subscript->type != &primitiveTypes[PRIM_UINT]  )
   {
      // subscript is not integral.
      glsl_compile_error(ERROR_CUSTOM, 2, line_num, NULL);
      return NULL;
   }

   unsigned aggregate_size = 0;
   if (aggregate->type->flavour == SYMBOL_PRIMITIVE_TYPE )
   {
      int aggregate_index = aggregate->type->u.primitive_type.index;
      /* expr->type will be set to NULL if this type can't be subscripted */
      expr->type = primitiveTypeSubscriptTypes[aggregate_index];
      aggregate_size = primitiveTypeSubscriptDimensions[aggregate_index];
   }
   else if (aggregate->type->flavour == SYMBOL_ARRAY_TYPE)
   {
      expr->type = aggregate->type->u.array_type.member_type;
      aggregate_size = aggregate->type->u.array_type.member_count;
   }

   if (expr->type == NULL) {
      // aggregate cannot be subscripted.
      glsl_compile_error(ERROR_CUSTOM, 3, line_num, NULL);
      return NULL;
   }

   if (glsl_type_contains(expr->type, PRIM_SAMPLER_TYPE | PRIM_IMAGE_TYPE) && subscript->compile_time_value == NULL)
      glsl_compile_error(ERROR_CUSTOM, 3, line_num, "Indexing type %s requires constant expression", expr->type->name);

   if (subscript->compile_time_value) {
      if (subscript->type == &primitiveTypes[PRIM_INT]) {
         int v = const_signed_from_value(subscript->compile_time_value[0]);
         if (v < 0) {
            glsl_compile_error(ERROR_SEMANTIC, 20, line_num, "array subscript (%d) cannot be negative", v);
            return NULL;
         }
      }

      unsigned subscript_value = subscript->compile_time_value[0];

      // Bounds check. aggregate_size == 0 for unsized SSBO arrays
      if (aggregate_size > 0 && subscript_value >= aggregate_size) {
         glsl_compile_error(ERROR_SEMANTIC, 20, line_num, "subscript %d beyond array bound %d",
                                                          subscript_value, aggregate_size-1);
         return NULL;
      }

      // Propagate constant values
      if (aggregate->compile_time_value) {
         expr->compile_time_value = aggregate->compile_time_value + (expr->type->scalar_count * subscript_value);
      }
   }

   return expr;
}

static bool expr_chain_all_compile_time_value(const ExprChain *c) {
   for (ExprChainNode *n = c->first; n != NULL; n = n->next)
      if (n->expr->compile_time_value == NULL) return false;

   return true;
}

static bool expr_chain_all_primitive_type(const ExprChain *c) {
   for (ExprChainNode *n = c->first; n != NULL; n = n->next)
      if (n->expr->type->flavour != SYMBOL_PRIMITIVE_TYPE) return false;

   return true;
}

Expr *glsl_expr_construct_function_call(int line_num, Symbol *function, ExprChain *args)
{
   // Note that each function symbol points to the last function symbol
   // with the same name.
   // We create a prototype from the arg types and then traverse the chain
   // until we find the correct overload.

   /* The symbol is not actually a function */
   if(function->flavour != SYMBOL_FUNCTION_INSTANCE)
      glsl_compile_error(ERROR_CUSTOM, 9, line_num, "%s", function->name);

   function = glsl_resolve_overload_using_arguments(function, args);
   if(!function) {
      /* There's no match for the arguments */
      glsl_compile_error(ERROR_CUSTOM, 11, line_num, NULL);
      return NULL;
   }

   /* There exists such a function which we can call - setup the expr */
   Expr *expr                     = malloc_fast(sizeof(*expr));
   expr->type                     = function->type->u.function_type.return_type;
   expr->flavour                  = EXPR_FUNCTION_CALL;
   expr->line_num                 = line_num;
   expr->u.function_call.function = function;
   expr->u.function_call.args     = args;
   expr->compile_time_value       = NULL;

   /* Fold any function call with constant arguments when there is a folding function. */
   if(function->u.function_instance.folding_function && expr_chain_all_compile_time_value(args)) {
      /* This function call can be constant folded */
      FoldingFunction folding_function = function->u.function_instance.folding_function;
      expr->compile_time_value = malloc_fast(expr->type->scalar_count * sizeof(const_value));
      switch(expr->u.function_call.function->type->u.function_type.param_count) {
      case 1:
         ((void (*)(void*, void*))folding_function)
            (
             expr->compile_time_value, // result
             args->first->expr->compile_time_value // argument 1
            );
         break;
      case 2:
         ((void (*)(void*, void*, void*))folding_function)
            (
             expr->compile_time_value, // result
             args->first->expr->compile_time_value, // argument 1
             args->first->next->expr->compile_time_value // argument 2
            );
         break;
      case 3:
         ((void (*)(void*, void*, void*, void*))folding_function)
            (
             expr->compile_time_value, // result
             args->first->expr->compile_time_value, // argument 1
             args->first->next->expr->compile_time_value, // argument 2
             args->first->next->next->expr->compile_time_value // argument 3
            );
         break;
      case 4:
         ((void (*)(void*, void*, void*, void*, void*))folding_function)
            (
             expr->compile_time_value, // result
             args->first->expr->compile_time_value, // argument 1
             args->first->next->expr->compile_time_value, // argument 2
             args->first->next->next->expr->compile_time_value, // argument 3
             args->first->next->next->next->expr->compile_time_value // argument 4
            );
         break;
      default:
         // No built in functions take more than 4 arguments.
         unreachable();
         return NULL;
      }
      return expr;
   }

   return expr;
}

Expr *glsl_expr_construct_method_call(int line_num, Expr *aggregate, CallContext *function_ctx)
{
   const char *method_name;

   switch(function_ctx->flavour) {
   case CALL_CONTEXT_FUNCTION:
      method_name = function_ctx->u.function.symbol->name;
      break;
   case CALL_CONTEXT_CONSTRUCTOR:
      method_name = function_ctx->u.constructor.type->name;
      break;
   case CALL_CONTEXT_INTRINSIC:
      /* If we ever want to have our own intrinsic methods, handling
         is needed here; a lookup array to convert them back into
         names would probably be sufficient */
      glsl_compile_error(ERROR_LEXER_PARSER, 2, line_num, NULL);
      return NULL;
   default:
      unreachable();
      return NULL;
   }

   if (aggregate->type->flavour != SYMBOL_ARRAY_TYPE || strcmp(method_name, "length") != 0)
      glsl_compile_error(ERROR_LEXER_PARSER, 2, line_num, NULL);

   return glsl_expr_construct_array_length(line_num, aggregate);
}


static Expr *glsl_expr_construct_prim_constructor_call(int line_num, PrimitiveTypeIndex return_index, ExprChain *args)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = EXPR_PRIM_CONSTRUCTOR_CALL;
   expr->line_num = line_num;
   expr->type = &primitiveTypes[return_index];
   expr->u.prim_constructor_call.flavour = PRIM_CONS_INVALID;
   expr->u.prim_constructor_call.args    = args;
   expr->compile_time_value = NULL;

   PRIMITIVE_TYPE_FLAGS_T return_flags = primitiveTypeFlags[return_index];

   // Quick checks.
   if (!args || args->count <= 0)
      glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Too few arguments");

   if (!expr_chain_all_primitive_type(args))
      glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Arguments must be primitive types");

   if ( ! (return_flags & (PRIM_SCALAR_TYPE | PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE)) )
      glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "No constructor for type %s", primitiveTypes[return_index].name);

   // Peek at first argument.
   ExprChainNode *arg = args->first;
   PrimitiveTypeIndex arg_index = arg->expr->type->u.primitive_type.index;
   PRIMITIVE_TYPE_FLAGS_T arg_flags = primitiveTypeFlags[arg_index];

   if (!glsl_conversion_valid(arg_index, return_index))
      glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Invalid type conversion");

   /* TODO: Do these even need a flavour? */
   if ((return_flags & PRIM_VECTOR_TYPE) && args->count == 1 && (arg_flags & PRIM_SCALAR_TYPE))
      expr->u.prim_constructor_call.flavour = PRIM_CONS_VECTOR_FROM_SCALAR;
   else if ((return_flags & PRIM_MATRIX_TYPE) && args->count == 1 && (arg_flags & PRIM_SCALAR_TYPE))
      expr->u.prim_constructor_call.flavour = PRIM_CONS_MATRIX_FROM_SCALAR;
   else if ((return_flags & PRIM_MATRIX_TYPE) && args->count == 1 && (arg_flags & PRIM_MATRIX_TYPE))
      expr->u.prim_constructor_call.flavour = PRIM_CONS_MATRIX_FROM_MATRIX;
   else
      expr->u.prim_constructor_call.flavour = PRIM_CONS_FROM_COMPONENT_FLOW;

   /* For a component flow, validate the arguments beyond the first */
   if (expr->u.prim_constructor_call.flavour == PRIM_CONS_FROM_COMPONENT_FLOW) {
      int return_components = primitiveTypes[return_index].scalar_count;
      int arg_components = 0;

      for ( ; arg != NULL && arg_components < return_components; arg=arg->next) {
         arg_index = arg->expr->type->u.primitive_type.index;
         arg_flags = primitiveTypeFlags[arg_index];

         // Ensure that matrix constructors reject matrix arguments.
         if ((return_flags & PRIM_MATRIX_TYPE) && (arg_flags & PRIM_MATRIX_TYPE))
            glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Argument may not be a matrix type");

         if (!glsl_conversion_valid(arg_index, return_index))
            glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Invalid type conversion");

         arg_components += primitiveTypes[arg_index].scalar_count;
      }

      if (arg)
         glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Unused argument");

      if (return_components > arg_components)
         glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Too few arguments");
   }

   // If we can decide a value at compile time, do so.
   bool args_all_compile_time_values = expr_chain_all_compile_time_value(args);
   if (args_all_compile_time_values)
   {
      expr->compile_time_value = malloc_fast(primitiveTypes[return_index].scalar_count * sizeof(const_value));
      const_value *dst = expr->compile_time_value;

      switch (expr->u.prim_constructor_call.flavour) {
         case PRIM_CONS_VECTOR_FROM_SCALAR:
         {
            // Convert scalar.
            const_value *src = arg->expr->compile_time_value;
            const_value val = glsl_single_scalar_type_conversion(return_index, arg_index, src[0]);

            int return_dimension = primitiveTypeSubscriptDimensions[return_index];
            for (int j = 0; j < return_dimension; j++) dst[j] = val;
            break;
         }
         case PRIM_CONS_MATRIX_FROM_SCALAR:
         {
            const_value *src = arg->expr->compile_time_value;
            const_value val = glsl_single_scalar_type_conversion(return_index, arg_index, src[0]);

            int columns = glsl_prim_matrix_type_subscript_dimensions(return_index, 0);
            int rows    = glsl_prim_matrix_type_subscript_dimensions(return_index, 1);
            for (int i = 0; i < columns; i++) {
               for (int j = 0; j < rows; j++) {
                  *dst = (i == j) ? val : CONST_FLOAT_ZERO;
                  dst = dst + 1;
               }
            }
            break;
         }
         case PRIM_CONS_MATRIX_FROM_MATRIX:
         {
            int arg_cols = glsl_prim_matrix_type_subscript_dimensions(arg_index, 0);
            int arg_rows = glsl_prim_matrix_type_subscript_dimensions(arg_index, 1);
            int return_cols = glsl_prim_matrix_type_subscript_dimensions(return_index, 0);
            int return_rows = glsl_prim_matrix_type_subscript_dimensions(return_index, 1);

            for (int i = 0; i < return_cols; i++) {
               // Set reader to m[i][j] where i selects columns.
               const_value *src = (arg->expr->compile_time_value + (arg_rows * i));

               for (int j = 0; j < return_rows; j++) {
                  if (i < arg_cols && j < arg_rows)
                     *dst = glsl_single_scalar_type_conversion(return_index, arg_index, src[j]);
                  else
                     *dst = (i == j) ? CONST_FLOAT_ONE : CONST_FLOAT_ZERO;
                  dst++;
               }
            }
            break;
         }
         case PRIM_CONS_FROM_COMPONENT_FLOW:
         {
            int return_components = primitiveTypes[return_index].scalar_count;
            int arg_components = 0;

            /* We know all arguments are used, so loop over them */
            for (ExprChainNode *arg=args->first; arg != NULL; arg=arg->next) {
               arg_index = arg->expr->type->u.primitive_type.index;
               arg_flags = primitiveTypeFlags[arg_index];
               int size = primitiveTypes[arg_index].scalar_count;
               const_value *src = arg->expr->compile_time_value;
               for (int i=0; i<size && arg_components < return_components; i++) {
                  *dst = glsl_single_scalar_type_conversion(return_index, arg_index, src[i]);
                  dst++;
                  arg_components++;
               }
            }
            break;
         }
         case PRIM_CONS_INVALID:
            unreachable();
            break;
      }
   }

   return expr;
}

static Expr *glsl_expr_construct_type_constructor_call(int line_num, SymbolType *type, ExprChain *args)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = EXPR_COMPOUND_CONSTRUCTOR_CALL;
   expr->line_num = line_num;
   expr->type     = type;
   expr->u.compound_constructor_call.args = args;
   expr->compile_time_value = NULL;

   assert(type->flavour == SYMBOL_STRUCT_TYPE);

   if (!args || args->count != type->u.struct_type.member_count)
      glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Expected %d args, found %d", type->u.struct_type.member_count,
                                                                                    args ? args->count : 0);

   // Match the types of all arguments.
   ExprChainNode *arg;
   unsigned int i;
   for (i = 0, arg = args->first; arg; i++, arg = arg->next)
   {
      // Match types.
      if (!glsl_shallow_match_nonfunction_types(arg->expr->type, type->u.struct_type.member[i].type))
         glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Invalid type conversion");
   }

   // If we can decide a value at compile time, do so.
   if (expr_chain_all_compile_time_value(args)) {
      expr->compile_time_value = malloc_fast(type->scalar_count * sizeof(const_value));
      const_value *dst = expr->compile_time_value;

      // Copy values.
      for (arg = args->first; arg; arg = arg->next) {
         memcpy(dst, arg->expr->compile_time_value, arg->expr->type->scalar_count * sizeof(const_value));
         dst = dst + arg->expr->type->scalar_count;
      }
   }

   return expr;
}

static Expr *glsl_expr_construct_array_constructor_call(int line_num, SymbolType *type, ExprChain *args)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = EXPR_COMPOUND_CONSTRUCTOR_CALL;
   expr->line_num = line_num;
   expr->u.compound_constructor_call.args = args;
   expr->compile_time_value = NULL;

   assert(type->flavour==SYMBOL_ARRAY_TYPE);

   // Quick checks.
   if (!args || args->count <= 0)
      glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Arguments required");

   SymbolType *type_in = type->u.array_type.member_type;
   SymbolType *arg_type = args->first->expr->type;
   if (type_in->flavour == SYMBOL_ARRAY_TYPE) {
      if (arg_type->flavour != SYMBOL_ARRAY_TYPE)
         glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "expected argument of type %s but found %s",
                                                         type_in->name, arg_type->name);

      glsl_complete_array_from_init_type(type_in, arg_type);
   }

   if(!type->u.array_type.member_count)
      glsl_complete_array_type(type, args->count);

   expr->type = type;

   if (args->count != expr->type->u.array_type.member_count)
      glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Expected %d args, found %d",
                         args->count, expr->type->u.array_type.member_count);

   // Match the types of all arguments.
   for (ExprChainNode *arg = args->first; arg; arg = arg->next) {
      if (!glsl_shallow_match_nonfunction_types(arg->expr->type, expr->type->u.array_type.member_type))
         glsl_compile_error(ERROR_SEMANTIC, 3, line_num, "Invalid type conversion");
   }

   // If we can decide a value at compile time, do so.
   if (expr_chain_all_compile_time_value(args)) {
      expr->compile_time_value = malloc_fast(expr->type->scalar_count * sizeof(const_value));
      const_value *dst = expr->compile_time_value;

      // Copy values.
      for (ExprChainNode *arg = args->first; arg; arg = arg->next) {
         memcpy(dst, arg->expr->compile_time_value, arg->expr->type->scalar_count * sizeof(const_value));
         dst = dst + arg->expr->type->scalar_count;
      }
   }

   return expr;
}

Expr *glsl_expr_construct_constructor_call(int line_num, SymbolType *type, ExprChain *args)
{
   switch(type->flavour) {
      case SYMBOL_PRIMITIVE_TYPE:
         return glsl_expr_construct_prim_constructor_call(line_num, type->u.primitive_type.index, args);

      case SYMBOL_STRUCT_TYPE:
         return glsl_expr_construct_type_constructor_call(line_num, type, args);

      case SYMBOL_ARRAY_TYPE:
         return glsl_expr_construct_array_constructor_call(line_num, type, args);

      default:
         unreachable();
         return NULL;
   }
}

static unsigned char swizzle_value(char c) {
   switch (c) {
      case 'x': return SWIZZLE_FIELD_XYZW_X;
      case 'y': return SWIZZLE_FIELD_XYZW_Y;
      case 'z': return SWIZZLE_FIELD_XYZW_Z;
      case 'w': return SWIZZLE_FIELD_XYZW_W;

      case 'r': return SWIZZLE_FIELD_RGBA_R;
      case 'g': return SWIZZLE_FIELD_RGBA_G;
      case 'b': return SWIZZLE_FIELD_RGBA_B;
      case 'a': return SWIZZLE_FIELD_RGBA_A;

      case 's': return SWIZZLE_FIELD_STPQ_S;
      case 't': return SWIZZLE_FIELD_STPQ_T;
      case 'p': return SWIZZLE_FIELD_STPQ_P;
      case 'q': return SWIZZLE_FIELD_STPQ_Q;

      default: unreachable(); return '\0';
   }
}

Expr *glsl_expr_construct_field_selector(int line_num, Expr *aggregate, const char *field)
{
   int member_count;
   StructMember *member;

   Expr *expr = malloc_fast(sizeof(Expr));
   expr->line_num = line_num;

   switch (aggregate->type->flavour)
   {
      case SYMBOL_STRUCT_TYPE:
      case SYMBOL_BLOCK_TYPE:
         if(aggregate->type->flavour==SYMBOL_STRUCT_TYPE) {
            member_count = aggregate->type->u.struct_type.member_count;
            member       = aggregate->type->u.struct_type.member;
         } else {
            member_count = aggregate->type->u.block_type.member_count;
            member       = aggregate->type->u.block_type.member;
         }

         expr->flavour = EXPR_FIELD_SELECTOR;
         expr->u.field_selector.aggregate = aggregate;
         expr->u.field_selector.field     = field;
         expr->u.field_selector.field_no  = -1;
         expr->compile_time_value = NULL;

         // Work out field number if valid
         for (int i=0; i<member_count; i++) {
            if (strcmp(member[i].name, field) == 0) {
               expr->u.field_selector.field_no = i;
               expr->type = member[i].type;
               break;
            }
         }

         if (expr->u.field_selector.field_no == -1) {
            glsl_compile_error(ERROR_SEMANTIC, 25, line_num, "%s does not name a field of type %s",
                                                             field, aggregate->type->name);
         }

         if (aggregate->compile_time_value) {
            int field_offset = 0;
            for (int i=0; i<expr->u.field_selector.field_no; i++)
               field_offset += member[i].type->scalar_count;
            expr->compile_time_value = aggregate->compile_time_value + field_offset;
         }

         return expr;

      case SYMBOL_PRIMITIVE_TYPE:
         expr->flavour = EXPR_SWIZZLE;
         expr->u.swizzle.aggregate = aggregate;
         expr->u.swizzle.field     = field;
         for (int j=0; j < MAX_SWIZZLE_FIELD_COUNT; j++)
            expr->u.swizzle.swizzle_slots[j] = SWIZZLE_SLOT_UNUSED;
         expr->compile_time_value = NULL;

         PRIMITIVE_TYPE_FLAGS_T primitive_flags;
         primitive_flags = primitiveTypeFlags[aggregate->type->u.primitive_type.index];

         if (!(primitive_flags & PRIM_VECTOR_TYPE))
            glsl_compile_error(ERROR_SEMANTIC, 26, line_num, "%s does not support swizzling", aggregate->type->name);

         int len = strlen(field);
         if (len > MAX_SWIZZLE_FIELD_COUNT)
            glsl_compile_error(ERROR_SEMANTIC, 26, line_num, "%s too long", field);

         // Decode swizzle into swizzle_slots[].
         int seen_xyzw = 0, seen_rgba = 0, seen_stpq = 0;
         for (int i=0; i<len; i++) {
            switch (field[i]) {
               case 'x': case 'y': case 'z': case 'w':
                  seen_xyzw++;
                  break;
               case 'r': case 'g': case 'b': case 'a':
                  seen_rgba++;
                  break;
               case 's': case 't': case 'p': case 'q':
                  seen_stpq++;
                  break;

               default:
                  glsl_compile_error(ERROR_SEMANTIC, 26, line_num, "%c not a swizzle", field[i]);
                  return NULL;
            }
            expr->u.swizzle.swizzle_slots[i] = swizzle_value(field[i]);
            if (expr->u.swizzle.swizzle_slots[i] >= primitiveTypes[aggregate->type->u.primitive_type.index].scalar_count)
            {
               glsl_compile_error(ERROR_SEMANTIC, 26, line_num, "component too large for aggregate");
               return NULL;
            }
         }

         if ((seen_xyzw != len) && (seen_rgba != len) && (seen_stpq != len))
         {
            glsl_compile_error(ERROR_SEMANTIC, 26, line_num, "Elements must be from the same set");
            return NULL;
         }

         PRIMITIVE_TYPE_FLAGS_T base_idx = primitiveScalarTypeIndices[aggregate->type->u.primitive_type.index];
         expr->type = glsl_prim_vector_type(base_idx, len);

         // Work out value, if possible.
         if (aggregate->compile_time_value)
         {
            expr->compile_time_value = malloc_fast(expr->type->scalar_count * sizeof(const_value));

            for (int i = 0; i < len; i++) {
               void *dst = expr->compile_time_value + i;
               void *src = aggregate->compile_time_value + expr->u.swizzle.swizzle_slots[i];
               memcpy(dst, src, sizeof(const_value));
            }
         }

         return expr;
      default:
         break; // fail
   }

   // Illegal field selector.
   glsl_compile_error(ERROR_SEMANTIC, 26, line_num, NULL);
   return NULL;
}

Expr *glsl_expr_construct_unary_op(ExprFlavour flavour, int line_num, Expr *operand)
{
   //  unary PLUS is trivial, just return the operand:
   if (flavour == EXPR_ADD)
      return operand;

   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = flavour;
   expr->line_num = line_num;
   expr->u.unary_op.operand = operand;
   expr->type = operand->type;
   expr->compile_time_value = NULL;

   if (operand->type->flavour != SYMBOL_PRIMITIVE_TYPE) goto error;

   int prim_index = operand->type->u.primitive_type.index;

   switch (flavour)
   {
      /* Arithmetic unaries */
      case EXPR_POST_INC:
      case EXPR_POST_DEC:
      case EXPR_PRE_INC:
      case EXPR_PRE_DEC:
         // EXPR_POST_INC, EXPR_POST_DEC, EXPR_PRE_INC, EXPR_PRE_DEC cannot be
         //  applied to constants.
         if (!glsl_is_lvalue(operand)) {
            // Not an l-value.
            glsl_compile_error(ERROR_SEMANTIC, 27, line_num, NULL);
            return NULL;
         }
         /* Fall through */
      case EXPR_ARITH_NEGATE:
         if ( !(primitiveTypeFlags[prim_index] & PRIM_ARITHMETIC)) goto error;
         break;

      /* Logical unaries */
      case EXPR_LOGICAL_NOT:
         if ( !(primitiveTypeFlags[prim_index] & PRIM_LOGICAL)) goto error;
         break;

      /* Bitwise unaries */
      case EXPR_BITWISE_NOT:
         if ( !(primitiveTypeFlags[prim_index] & PRIM_BITWISE)) goto error;
         break;

      default:
         goto error;
   }

   if (operand->compile_time_value) {
      switch (flavour)
      {
         case EXPR_ARITH_NEGATE:
         case EXPR_BITWISE_NOT:
         {
            const_value (*component_op)(const_value);

            expr->compile_time_value = malloc_fast(expr->type->scalar_count * sizeof(const_value));

            const_value *src = operand->compile_time_value;
            const_value *dst = expr->compile_time_value;

            int n = primitiveTypes[prim_index].scalar_count;

            if (flavour == EXPR_ARITH_NEGATE) {
               switch (primitiveTypeFlags[prim_index] & (PRIM_INT_TYPE | PRIM_UINT_TYPE | PRIM_FLOAT_TYPE))
               {
                  case PRIM_INT_TYPE:
                  case PRIM_UINT_TYPE: // complies with spec
                     component_op = op_i_negate;
                     break;
                  case PRIM_FLOAT_TYPE:
                     component_op = op_f_negate;
                     break;
                  default:
                     unreachable();
                     return NULL;
               }
            } else {
               assert(flavour == EXPR_BITWISE_NOT);
               component_op = op_bitwise_not;
            }

            for (int i = 0; i < n; i++)
            {
               dst[i] = component_op(src[i]);
            }
            break;
         }

         case EXPR_LOGICAL_NOT:
            expr->compile_time_value = malloc_fast(expr->type->scalar_count * sizeof(const_value));
            // This code assumes we do not act on vectors.
            assert(prim_index == PRIM_BOOL);
            *expr->compile_time_value = op_logical_not(*operand->compile_time_value);
            break;

         default:
            break;
      }
   }

   return expr;

error:
   // Operator not supported.
   glsl_compile_error(ERROR_SEMANTIC, 4, line_num, NULL);
   return NULL;
}

// Applies dst = component_op(src_left, src_right) n times.
// dst is incremented.
// src_left, src_right are incremented iff their locks are not set.
static inline void apply_component_wise(
                         const_value (*component_op)(const_value, const_value),
                         int n, const_value *dst,
                         const_value *src_left,  bool lock_src_left,
                         const_value *src_right, bool lock_src_right)
{
   for (int i = 0; i < n; i++) {
      dst[i] = component_op(*src_left, *src_right);

      if (!lock_src_left) src_left = src_left + 1;
      if (!lock_src_right) src_right = src_right + 1;
   }
}

Expr *glsl_expr_construct_binary_op_arithmetic(ExprFlavour flavour, int line_num, Expr *left, Expr *right)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = flavour;
   expr->line_num = line_num;
   expr->u.binary_op.left  = left;
   expr->u.binary_op.right = right;

   expr->compile_time_value = NULL;

   if (left->type->flavour != SYMBOL_PRIMITIVE_TYPE || right->type->flavour != SYMBOL_PRIMITIVE_TYPE)
      goto fail;

   int left_index  = left->type->u.primitive_type.index;
   int right_index = right->type->u.primitive_type.index;
   PRIMITIVE_TYPE_FLAGS_T left_flags  = primitiveTypeFlags[left_index];
   PRIMITIVE_TYPE_FLAGS_T right_flags = primitiveTypeFlags[right_index];

   if (!(left_flags & PRIM_ARITHMETIC) || !(right_flags & PRIM_ARITHMETIC))
      goto fail;

   bool types_match = ( (left_flags & PRIM_FLOAT_TYPE) && (right_flags & PRIM_FLOAT_TYPE) ) ||
                      ( (left_flags & PRIM_INT_TYPE)   && (right_flags & PRIM_INT_TYPE)   ) ||
                      ( (left_flags & PRIM_UINT_TYPE)  && (right_flags & PRIM_UINT_TYPE) );
   if (!types_match) goto fail;

   /* rem is not supported for floats (because division gives no remainder) */
   if (flavour == EXPR_REM && (left_flags & PRIM_FLOAT_TYPE))
      goto fail;

   // Infer type, and value if possible.
   // From spec (1.00 rev 14 p46), the two operands must be:
   // 1 - the same type (the type being integer scalar/vector, float scalar/vector/matrix),
   // 2 - or one can be a scalar float and the other a float vector or matrix,
   // 3 - or one can be a scalar integer and the other an integer vector,
   // 4 - or, for multiply, one can be a float vector and the other a float matrix with the same dimensional size.
   // All operations are component-wise except EXPR_MUL involving at least one matrix (cases 1 and 4).

   /* Int matrix types don't exist */
   assert( !(left_flags & PRIM_MATRIX_TYPE) || left_flags & PRIM_FLOAT_TYPE );
   assert( !(right_flags & PRIM_MATRIX_TYPE) || right_flags & PRIM_FLOAT_TYPE );
   /* SCALAR, VECTOR, MATRIX should be exhaustive */
   assert(left_flags & (PRIM_SCALAR_TYPE | PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE));
   assert(right_flags & (PRIM_SCALAR_TYPE | PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE));

   bool left_scalar  = !!(left_flags & PRIM_SCALAR_TYPE);
   bool right_scalar = !!(right_flags & PRIM_SCALAR_TYPE);

   if (left->type == right->type)         expr->type = left->type;
   else if (left_scalar && !right_scalar) expr->type = right->type;
   else if (!left_scalar && right_scalar) expr->type = left->type;
   else if (flavour == EXPR_MUL)
   {
      if ( (left_flags & PRIM_MATRIX_TYPE) && (right_flags & PRIM_VECTOR_TYPE)
         && glsl_prim_matrix_type_subscript_dimensions(left_index, 0) == primitiveTypeSubscriptDimensions[right_index])
      {
         expr->type = &primitiveTypes[glsl_prim_matrix_type_subscript_vector(left_index, 1)];
      }
      else if ( (right_flags & PRIM_MATRIX_TYPE) && (left_flags & PRIM_VECTOR_TYPE)
         && primitiveTypeSubscriptDimensions[left_index] == glsl_prim_matrix_type_subscript_dimensions(right_index, 1))
      {
         expr->type = &primitiveTypes[glsl_prim_matrix_type_subscript_vector(right_index, 0)];
      }
      else if ( (right_flags & PRIM_MATRIX_TYPE) && (left_flags & PRIM_MATRIX_TYPE)
         && glsl_prim_matrix_type_subscript_dimensions(left_index, 0) == glsl_prim_matrix_type_subscript_dimensions(right_index, 1))
      {
         // NOTE: The dimensions of the result derive from the arguments' in a non-standard way!
         int dim0 = glsl_prim_matrix_type_subscript_dimensions(right_index, 0);
         int dim1 = glsl_prim_matrix_type_subscript_dimensions(left_index, 1);
         expr->type = &primitiveTypes[primitiveMatrixTypeIndices[dim0][dim1]];
      }
      else
         goto fail;
   }
   else
      goto fail;

   if (left->compile_time_value && right->compile_time_value)
   {
      expr->compile_time_value = malloc_fast(expr->type->scalar_count * sizeof(const_value));

      /* Operations with a matrix and a matrix or vector are linear algebraic... */
      if (flavour == EXPR_MUL && (
            ((left_flags & PRIM_MATRIX_TYPE) && (right_flags & PRIM_MATRIX_TYPE)) ||
            ((left_flags & PRIM_MATRIX_TYPE) && (right_flags & PRIM_VECTOR_TYPE)) ||
            ((left_flags & PRIM_VECTOR_TYPE) && (right_flags & PRIM_MATRIX_TYPE))  ) )
      {
         int dim0, dim1, dim2;

         if ( (left_flags & PRIM_MATRIX_TYPE) && (right_flags & PRIM_VECTOR_TYPE) )
         {
            dim0 = 1;
            dim1 = glsl_prim_matrix_type_subscript_dimensions(left_index, 1);
            dim2 = primitiveTypeSubscriptDimensions[right_index];
         }
         else if ( (right_flags & PRIM_MATRIX_TYPE) && (left_flags & PRIM_VECTOR_TYPE) )
         {
            dim0 = glsl_prim_matrix_type_subscript_dimensions(right_index, 0);
            dim1 = 1;
            dim2 = primitiveTypeSubscriptDimensions[left_index];
         }
         else /* ( (right_flags & PRIM_MATRIX_TYPE) && (left_flags & PRIM_MATRIX_TYPE) */
         {
            dim0 = glsl_prim_matrix_type_subscript_dimensions(right_index, 0);
            dim1 = glsl_prim_matrix_type_subscript_dimensions(left_index, 1);
            dim2 = glsl_prim_matrix_type_subscript_dimensions(left_index, 0);
         }

         op_mul__const_matXxY__const_matZxY__const_matXxZ(dim0,
                                                          dim1,
                                                          dim2,
                                                          expr->compile_time_value,
                                                          left->compile_time_value,
                                                          right->compile_time_value);
      }
      else
      {
         // Find the correct component-wise operation to apply.
         const_value (*component_op)(const_value, const_value);

         switch (left_flags & (PRIM_INT_TYPE | PRIM_FLOAT_TYPE | PRIM_UINT_TYPE))
         {
            case PRIM_INT_TYPE:
               switch (flavour)
               {
                  case EXPR_MUL: component_op = op_i_mul; break;
                  case EXPR_DIV: component_op = op_i_div; break;
                  case EXPR_REM: component_op = op_i_rem; break;
                  case EXPR_ADD: component_op = op_i_add; break;
                  case EXPR_SUB: component_op = op_i_sub; break;
                  default: unreachable(); return NULL;
               }
               break;
            case PRIM_UINT_TYPE:
               switch (flavour)
               {
                  case EXPR_MUL: component_op = op_u_mul; break;
                  case EXPR_DIV: component_op = op_u_div; break;
                  case EXPR_REM: component_op = op_u_rem; break;
                  case EXPR_ADD: component_op = op_i_add; break;
                  case EXPR_SUB: component_op = op_i_sub; break;
                  default: unreachable(); return NULL;
               }
               break;
            case PRIM_FLOAT_TYPE:
               switch (flavour)
               {
                  case EXPR_MUL: component_op = op_f_mul; break;
                  case EXPR_DIV: component_op = op_f_div; break;
                  case EXPR_REM: unreachable(); break;
                  case EXPR_ADD: component_op = op_f_add; break;
                  case EXPR_SUB: component_op = op_f_sub; break;
                  default: unreachable(); return NULL;
               }
               break;
            default:
               unreachable();
               return NULL;
         }

         /* ... everything else is component wise */
         apply_component_wise(component_op,
                              primitiveTypes[expr->type->u.primitive_type.index].scalar_count,
                              expr->compile_time_value,
                              left->compile_time_value,  left_scalar,
                              right->compile_time_value, right_scalar);
      }
   }

   return expr;

fail:
   // Operator not supported.
   glsl_compile_error(ERROR_SEMANTIC, 4, line_num, NULL);
   return NULL;
}

Expr *glsl_expr_construct_binary_op_logical(ExprFlavour flavour, int line_num, Expr *left, Expr *right)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = flavour;
   expr->line_num = line_num;
   expr->type     = &primitiveTypes[PRIM_BOOL];
   expr->u.binary_op.left  = left;
   expr->u.binary_op.right = right;

   expr->compile_time_value = NULL;

   int left_index = left->type->u.primitive_type.index;
   if (left->type != right->type) goto fail;
   if (left->type->flavour != SYMBOL_PRIMITIVE_TYPE) goto fail;
   if ( !(primitiveTypeFlags[left_index] & PRIM_LOGICAL) ) goto fail;

   if (left->compile_time_value && right->compile_time_value)
   {
      const_value l = *left->compile_time_value;
      const_value r = *right->compile_time_value;

      expr->compile_time_value = malloc_fast(expr->type->scalar_count * sizeof(const_value));

      // This code assumes we do not act on vectors.
      assert(left_index == PRIM_BOOL);

      switch (flavour)
      {
         case EXPR_LOGICAL_AND:
            *expr->compile_time_value = op_logical_and(l, r);
            break;
         case EXPR_LOGICAL_XOR:
            *expr->compile_time_value = op_logical_xor(l, r);
            break;
         case EXPR_LOGICAL_OR:
            *expr->compile_time_value = op_logical_or(l, r);
            break;
         default:
            unreachable();
            return NULL;
      }
   }
   return expr;

fail:
   // Operator not supported.
   glsl_compile_error(ERROR_SEMANTIC, 4, line_num, NULL);
   return NULL;
}

Expr *glsl_expr_construct_binary_op_bitwise(ExprFlavour flavour, int line_num, Expr *left, Expr *right)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = flavour;
   expr->line_num = line_num;
   expr->u.binary_op.left  = left;
   expr->u.binary_op.right = right;

   expr->compile_time_value = NULL;

   if (left->type->flavour != SYMBOL_PRIMITIVE_TYPE || right->type->flavour != SYMBOL_PRIMITIVE_TYPE)
      goto error;

   PRIMITIVE_TYPE_FLAGS_T left_flags  = primitiveTypeFlags[left->type->u.primitive_type.index];
   PRIMITIVE_TYPE_FLAGS_T right_flags = primitiveTypeFlags[right->type->u.primitive_type.index];

   if ( !(left_flags  & PRIM_BITWISE) ) goto error;
   if ( !(right_flags & PRIM_BITWISE) ) goto error;

   /* Can't mix int and uint types in bitwise expressions */
   if ((left_flags & PRIM_INT_TYPE)  && (right_flags & PRIM_UINT_TYPE)) goto error;
   if ((left_flags & PRIM_UINT_TYPE) && (right_flags & PRIM_INT_TYPE))  goto error;

   bool left_vector  = !!(left_flags & PRIM_VECTOR_TYPE);
   bool right_vector = !!(right_flags & PRIM_VECTOR_TYPE);
   if (left_vector && right_vector && left->type != right->type) goto error;

   if      (left_vector)  expr->type = left->type;
   else if (right_vector) expr->type = right->type;
   else {
      assert(left->type == right->type);
      expr->type = left->type;
   }

   if (left->compile_time_value && right->compile_time_value)
   {
      const_value (*component_op)(const_value, const_value);
      expr->compile_time_value = malloc_fast(expr->type->scalar_count * sizeof(const_value));

      switch (flavour) {
         case EXPR_BITWISE_AND: component_op = op_bitwise_and; break;
         case EXPR_BITWISE_XOR: component_op = op_bitwise_xor; break;
         case EXPR_BITWISE_OR:  component_op = op_bitwise_or; break;
         default: unreachable(); return NULL;
      }
      apply_component_wise(component_op,
                           primitiveTypes[expr->type->u.primitive_type.index].scalar_count,
                           expr->compile_time_value,
                           left->compile_time_value, !left_vector,
                           right->compile_time_value, !right_vector);
   }

   return expr;

   // Operator not supported.
error:
   glsl_compile_error(ERROR_SEMANTIC, 4, line_num, NULL);
   return NULL;
}

Expr *glsl_expr_construct_binary_op_shift(ExprFlavour flavour, int line_num, Expr *left, Expr *right)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = flavour;
   expr->line_num = line_num;
   expr->u.binary_op.left  = left;
   expr->u.binary_op.right = right;

   expr->compile_time_value = NULL;

   if (right->type->flavour != SYMBOL_PRIMITIVE_TYPE || left->type->flavour != SYMBOL_PRIMITIVE_TYPE)
      goto error;

   PRIMITIVE_TYPE_FLAGS_T left_flags  = primitiveTypeFlags[left->type->u.primitive_type.index];
   PRIMITIVE_TYPE_FLAGS_T right_flags = primitiveTypeFlags[right->type->u.primitive_type.index];

   if (!(left_flags & PRIM_BITWISE))
      goto error;
   if (!(left_flags & (PRIM_INT_TYPE | PRIM_UINT_TYPE)))
      goto error;
   if (!(right_flags & (PRIM_INT_TYPE | PRIM_UINT_TYPE)))
      goto error;
   if ((left_flags & PRIM_SCALAR_TYPE) && !(right_flags & PRIM_SCALAR_TYPE))
      goto error;
   if ((left_flags & PRIM_VECTOR_TYPE) && (right_flags & PRIM_VECTOR_TYPE) && (left->type->scalar_count!=right->type->scalar_count))
      goto error;
   if ((left_flags & PRIM_MATRIX_TYPE) || (right_flags & PRIM_MATRIX_TYPE))
      goto error;

   expr->type = left->type;

   if (left->compile_time_value && right->compile_time_value)
   {
      const_value (*component_op)(const_value, const_value);
      expr->compile_time_value = malloc_fast(expr->type->scalar_count * sizeof(const_value));

      switch (flavour)
      {
         case EXPR_SHL:
            component_op = op_bitwise_shl;
            break;
         case EXPR_SHR:
            if(left_flags & PRIM_INT_TYPE)
               component_op = op_i_bitwise_shr;
            else
               component_op = op_u_bitwise_shr;
            break;
         default:
            unreachable();
            return NULL;
      }
      apply_component_wise(
         component_op,
         primitiveTypes[expr->type->u.primitive_type.index].scalar_count,
         expr->compile_time_value,
         left->compile_time_value,
         false,
         right->compile_time_value,
         (right_flags & PRIM_SCALAR_TYPE) ? true : false);
   }

   return expr;

error:
   // Operator not supported.
   glsl_compile_error(ERROR_SEMANTIC, 4, line_num, NULL);
   return NULL;
}

Expr *glsl_expr_construct_binary_op_relational(ExprFlavour flavour, int line_num, Expr *left, Expr *right)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = flavour;
   expr->line_num = line_num;
   expr->type     = &primitiveTypes[PRIM_BOOL];
   expr->u.binary_op.left  = left;
   expr->u.binary_op.right = right;

   expr->compile_time_value = NULL;

   if (left->type != right->type) goto fail;

   // Only need look at left type as they are identical anyway.
   if (SYMBOL_PRIMITIVE_TYPE != left->type->flavour)
      goto fail;

   int left_index = left->type->u.primitive_type.index;
   int left_flags = primitiveTypeFlags[left_index];

   // Only need look at left type as they are identical anyway.
   if ( !(left_flags & PRIM_RELATIONAL))
      goto fail;

   if (left->compile_time_value && right->compile_time_value)
   {
      const_value (*component_op)(const_value, const_value);
      expr->compile_time_value = malloc_fast(expr->type->scalar_count * sizeof(const_value));

      // This code assumes we do not act on vectors.
      assert(left_flags & PRIM_SCALAR_TYPE);

      switch (left_flags & (PRIM_INT_TYPE | PRIM_UINT_TYPE | PRIM_FLOAT_TYPE))
      {
         case PRIM_INT_TYPE:
            switch (flavour)
            {
               case EXPR_LESS_THAN:
                  component_op = op_i_less_than;
                  break;
               case EXPR_LESS_THAN_EQUAL:
                  component_op = op_i_less_than_equal;
                  break;
               case EXPR_GREATER_THAN:
                  component_op = op_i_greater_than;
                  break;
               case EXPR_GREATER_THAN_EQUAL:
                  component_op = op_i_greater_than_equal;
                  break;
               default:
                  unreachable();
                  return NULL;
            }
            break;
         case PRIM_UINT_TYPE:
            switch (flavour)
            {
               case EXPR_LESS_THAN:
                  component_op = op_u_less_than;
                  break;
               case EXPR_LESS_THAN_EQUAL:
                  component_op = op_u_less_than_equal;
                  break;
               case EXPR_GREATER_THAN:
                  component_op = op_u_greater_than;
                  break;
               case EXPR_GREATER_THAN_EQUAL:
                  component_op = op_u_greater_than_equal;
                  break;
               default:
                  unreachable();
                  return NULL;
            }
            break;
         case PRIM_FLOAT_TYPE:
            switch (flavour)
            {
               case EXPR_LESS_THAN:
                  component_op = op_f_less_than;
                  break;
               case EXPR_LESS_THAN_EQUAL:
                  component_op = op_f_less_than_equal;
                  break;
               case EXPR_GREATER_THAN:
                  component_op = op_f_greater_than;
                  break;
               case EXPR_GREATER_THAN_EQUAL:
                  component_op = op_f_greater_than_equal;
                  break;
               default:
                  unreachable();
                  return NULL;
            }
            break;
         default:
            unreachable();
            return NULL;
      }

      *expr->compile_time_value = component_op(*left->compile_time_value,
                                               *right->compile_time_value);
   }

   return expr;

fail:
   // Operator not supported.
   glsl_compile_error(ERROR_SEMANTIC, 4, line_num, NULL);
   return NULL;
}

static bool compile_time_values_equal(SymbolType  *a_type,
                                      const_value *a_value,
                                      SymbolType  *b_type,
                                      const_value *b_value)
{
   assert(glsl_shallow_match_nonfunction_types(a_type, b_type));
   assert(a_type->scalar_count == b_type->scalar_count);

   bool result = false;

   if(a_type->flavour == SYMBOL_PRIMITIVE_TYPE) {
      const int                    a_index      = a_type->u.primitive_type.index;
      const PRIMITIVE_TYPE_FLAGS_T a_flags      = primitiveTypeFlags[a_index];

      switch(a_flags & (PRIM_BOOL_TYPE|PRIM_INT_TYPE|PRIM_UINT_TYPE|PRIM_FLOAT_TYPE)) {
      case PRIM_FLOAT_TYPE:
         /* floats need a more detailed comparison to determine equality */
         for(unsigned i=0; i<a_type->scalar_count; ++i) {
            result = op_f_equal(a_value[i], b_value[i]);
            if(!result) break;
         }
         break;
      default:
         result = (memcmp(a_value, b_value, a_type->scalar_count * sizeof(const_value)) == 0);
         break;
      }

   } else if(a_type->flavour == SYMBOL_STRUCT_TYPE) {
      const unsigned n = a_type->u.struct_type.member_count;
      for(unsigned i=0;i<n;++i) {
         unsigned scalar_count = a_type->u.struct_type.member[i].type->scalar_count;
         result = compile_time_values_equal(
                           a_type->u.struct_type.member[i].type, a_value,
                           b_type->u.struct_type.member[i].type, b_value);
         if(!result) break;

         a_value += scalar_count;
         b_value += scalar_count;
      }
   } else if(a_type->flavour == SYMBOL_ARRAY_TYPE) {
      unsigned n            = a_type->u.array_type.member_count;
      unsigned scalar_count = a_type->scalar_count / n;
      for(unsigned i=0;i<n;++i) {
         result = compile_time_values_equal(
                        a_type->u.array_type.member_type, a_value,
                        b_type->u.array_type.member_type, b_value);
         if(!result) break;

         a_value += scalar_count;
         b_value += scalar_count;
      }
   } else {
      unreachable();
   }
   return result;
}

Expr *glsl_expr_construct_binary_op_equality(ExprFlavour flavour, int line_num, Expr *left, Expr *right)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = flavour;
   expr->line_num = line_num;
   expr->u.binary_op.left  = left;
   expr->u.binary_op.right = right;
   expr->type = &primitiveTypes[PRIM_BOOL];

   expr->compile_time_value = NULL;

   if (!glsl_shallow_match_nonfunction_types(left->type, right->type)) goto fail;

   if (glsl_type_contains_opaque(left->type)) goto fail;

   if (left->compile_time_value && right->compile_time_value) {
      bool equal = compile_time_values_equal(left->type,  left->compile_time_value,
                                             right->type, right->compile_time_value);

      expr->compile_time_value = malloc_fast(expr->type->scalar_count * sizeof(const_value));

      switch (flavour) {
         case EXPR_EQUAL:     *expr->compile_time_value =  equal ? 1 : 0; break;
         case EXPR_NOT_EQUAL: *expr->compile_time_value = !equal ? 1 : 0; break;
         default: unreachable(); return NULL;
      }
   }

   return expr;

fail:
   // Operator not supported.
   glsl_compile_error(ERROR_SEMANTIC, 4, line_num, NULL);
   return NULL;
}

Expr *glsl_expr_construct_cond_op(int line_num, Expr *cond, Expr *if_true, Expr *if_false)
{
   Expr *expr = malloc_fast(sizeof(Expr));
   expr->flavour = EXPR_CONDITIONAL;
   expr->line_num = line_num;
   expr->u.cond_op.cond = cond;
   expr->u.cond_op.if_true = if_true;
   expr->u.cond_op.if_false = if_false;
   expr->compile_time_value = NULL;

   expr->type = if_true->type;

   // cond must be a (scalar) bool.
   if (cond->type != &primitiveTypes[PRIM_BOOL])
      glsl_compile_error(ERROR_SEMANTIC, 4, line_num, "First operand of ?: must have type bool");

   // if_true and if_false must have same type.
   if (!glsl_shallow_match_nonfunction_types(if_true->type, if_false->type))
      glsl_compile_error(ERROR_SEMANTIC, 1, line_num, NULL);

   if (glsl_type_contains_opaque(if_true->type))
      glsl_compile_error(ERROR_SEMANTIC, 4, line_num, "?: operator not supported for samplers");

   if (cond->compile_time_value     != NULL &&
       if_true->compile_time_value  != NULL &&
       if_false->compile_time_value != NULL  )
   {
      if (*cond->compile_time_value)
         expr->compile_time_value = if_true->compile_time_value;
      else
         expr->compile_time_value = if_false->compile_time_value;
   }

   return expr;
}

Expr *glsl_expr_construct_assign_op(int line_num, Expr *lvalue, Expr *rvalue)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = EXPR_ASSIGN;
   expr->line_num = line_num;
   expr->u.assign_op.lvalue = lvalue;
   expr->u.assign_op.rvalue = rvalue;
   expr->type = rvalue->type;

   // Assignments can never have a compile_time_value as the lvalue cannot be constant,
   // and thus they cannot form part of constant expressions.
   expr->compile_time_value = NULL;

   if (!glsl_is_lvalue(lvalue))
      glsl_compile_error(ERROR_SEMANTIC, 27, line_num, NULL);

   if (!glsl_shallow_match_nonfunction_types(lvalue->type, rvalue->type))
      glsl_compile_error(ERROR_SEMANTIC, 1, line_num, NULL);

   return expr;
}

Expr *glsl_expr_construct_sequence(int line_num, Expr *all_these, Expr *then_this)
{
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = EXPR_SEQUENCE;
   expr->line_num = line_num;
   expr->u.sequence.all_these = all_these;
   expr->u.sequence.then_this = then_this;
   expr->type = then_this->type;
   expr->compile_time_value = NULL;

   /* The rules are weird. Sequence operators only form constant expressions
    * in version 100, and only when all their arguments are constant.       */
   if (g_ShaderVersion == GLSL_SHADER_VERSION(1, 0, 1))
      if (all_these->compile_time_value && then_this->compile_time_value)
         expr->compile_time_value = then_this->compile_time_value;

   return expr;
}

Expr *glsl_expr_construct_array_length(int line_num, Expr *array) {
   Expr *expr     = malloc_fast(sizeof(Expr));
   expr->flavour  = EXPR_ARRAY_LENGTH;
   expr->line_num = line_num;
   expr->type     = &primitiveTypes[PRIM_INT];
   expr->u.array_length.array = array;
   expr->compile_time_value   = NULL;

   if (array->type->u.array_type.member_count != 0) {
      expr->compile_time_value = malloc_fast(sizeof(const_value));
      expr->compile_time_value[0] = array->type->u.array_type.member_count;
   }

   return expr;
}


Statement *glsl_statement_construct(StatementFlavour flavour, int line_num)
{
   Statement *statement = malloc_fast(sizeof(Statement));
   statement->line_num  = line_num;
   statement->flavour   = flavour;
   return statement;
}

Statement *glsl_statement_construct_ast(int line_num, StatementChain *decls)
{
   Statement *statement = glsl_statement_construct(STATEMENT_AST, line_num);
   statement->u.ast.decls = decls;
   return statement;
}

Statement *glsl_statement_construct_decl_list(int line_num, StatementChain *decls)
{
   Statement *statement = glsl_statement_construct(STATEMENT_DECL_LIST, line_num);
   statement->u.decl_list.decls = decls;
   return statement;
}

Statement *glsl_statement_construct_function_def(int line_num, Symbol *header, Statement *body)
{
   Statement *statement = glsl_statement_construct(STATEMENT_FUNCTION_DEF, line_num);
   statement->u.function_def.header = header;
   statement->u.function_def.body   = body;
   return statement;
}

Statement *glsl_statement_construct_precision(int line_num, PrecisionQualifier prec, SymbolType *type) {
   Statement *statement = glsl_statement_construct(STATEMENT_PRECISION, line_num);
   statement->u.precision.prec = prec;
   statement->u.precision.type = type;
   return statement;
}

Statement *glsl_statement_construct_qualifier_default(int line_num, QualList *quals) {
   Statement *statement = glsl_statement_construct(STATEMENT_QUALIFIER_DEFAULT, line_num);
   statement->u.qualifier_default.quals = quals;
   return statement;
}

Statement *glsl_statement_construct_qualifier_augment(int line_num, QualList *quals, SymbolList *vars) {
   Statement *statement = glsl_statement_construct(STATEMENT_QUALIFIER_AUGMENT, line_num);
   statement->u.qualifier_augment.quals = quals;
   statement->u.qualifier_augment.vars  = vars;
   return statement;
}

Statement *glsl_statement_construct_var_decl(int line_num, QualList *quals, SymbolType *base_type,
                                             Symbol *var, Expr *initializer)
{
   Statement *statement = glsl_statement_construct(STATEMENT_VAR_DECL, line_num);
   statement->u.var_decl.quals       = quals;
   statement->u.var_decl.base_type   = base_type;
   statement->u.var_decl.var         = var;
   statement->u.var_decl.initializer = initializer;
   return statement;
}

Statement *glsl_statement_construct_struct_decl(int line_num, SymbolType *type, QualList *quals, StatementChain *members)
{
   Statement *statement = glsl_statement_construct(STATEMENT_STRUCT_DECL, line_num);
   statement->u.struct_decl.type    = type;
   statement->u.struct_decl.quals   = quals;
   statement->u.struct_decl.members = members;
   return statement;
}

Statement *glsl_statement_construct_struct_member_decl(int line_num, const char *name, ExprChain *size) {
   Statement *statement = glsl_statement_construct(STATEMENT_STRUCT_MEMBER_DECL, line_num);
   statement->u.struct_member_decl.name            = name;
   statement->u.struct_member_decl.array_specifier = size;
   return statement;
}

Statement *glsl_statement_construct_compound(int line_num, StatementChain *statements) {
   Statement *statement = glsl_statement_construct(STATEMENT_COMPOUND, line_num);
   statement->u.compound.statements = statements;
   return statement;
}

Statement *glsl_statement_construct_expr(int line_num, Expr *expr) {
   Statement *statement = glsl_statement_construct(STATEMENT_EXPR, line_num);
   statement->u.expr.expr = expr;
   return statement;
}

static inline Statement *promote_to_statement_compound(Statement *s)
{
   if (!s) return NULL;
   if (s->flavour == STATEMENT_COMPOUND) return s;

   return glsl_statement_construct_compound(s->line_num, glsl_statement_chain_append(glsl_statement_chain_create(), s));
}

Statement *glsl_statement_construct_selection(int line_num, Expr *cond, Statement *if_true, Statement *if_false)
{
   Statement *statement = glsl_statement_construct(STATEMENT_SELECTION, line_num);
   statement->u.selection.cond     = cond;
   statement->u.selection.if_true  = promote_to_statement_compound(if_true);
   statement->u.selection.if_false = promote_to_statement_compound(if_false);

   // cond must be a (scalar) bool.
   if (cond->type != &primitiveTypes[PRIM_BOOL])
      glsl_compile_error(ERROR_SEMANTIC, 1, line_num, "if condition must be bool, found %s", cond->type->name);

   return statement;
}

Statement *glsl_statement_construct_switch(int line_num, Expr *cond, StatementChain *chain)
{
   Statement *statement = glsl_statement_construct(STATEMENT_SWITCH, line_num);
   statement->u.switch_stmt.cond      = cond;
   statement->u.switch_stmt.stmtChain = chain;

   // cond must be a (scalar) int.
   if (cond->type != &primitiveTypes[PRIM_INT] && cond->type != &primitiveTypes[PRIM_UINT])
      glsl_compile_error(ERROR_SEMANTIC, 1, line_num, "switch condition requires int type, found %s", cond->type->name);

   /* If the switch was empty then we're done */
   if(!chain->first) return statement;

   /* TODO: Must we support switch statements with no cases in? */
   if(chain->first->statement->flavour!=STATEMENT_CASE &&
      chain->first->statement->flavour!=STATEMENT_DEFAULT )
   {
      glsl_compile_error(ERROR_LEXER_PARSER, 1, line_num, "Statements in switch before first case");
   }
   if (chain->last->statement->flavour==STATEMENT_CASE ||
       chain->last->statement->flavour==STATEMENT_DEFAULT )
   {
      glsl_compile_error(ERROR_LEXER_PARSER, 1, line_num, "Switch statements may not end with a case or default");
   }

   bool dflt_label = false;
   for(StatementChainNode *node=chain->first; node; node=node->next)
   {
      if(node->statement->flavour==STATEMENT_CASE)
      {
         if(!node->statement->u.case_stmt.expr->compile_time_value ||
             node->statement->u.case_stmt.expr->type != cond->type   )
         {
            glsl_compile_error(ERROR_CUSTOM, 26, line_num, "Case labels must be constant and match the condition type");
         }

         const_value case_label = *node->statement->u.case_stmt.expr->compile_time_value;
         StatementChainNode *node2;
         for(node2=node->next; node2; node2=node2->next)
         {
            if(node2->statement->flavour==STATEMENT_CASE && case_label==*node2->statement->u.case_stmt.expr->compile_time_value)
               /* Case labels cannot repeat values */
               glsl_compile_error(ERROR_CUSTOM, 26, line_num, "Duplicate case value %d", case_label);
         }
      }

      if(node->statement->flavour==STATEMENT_DEFAULT) {
         if(dflt_label)
            glsl_compile_error(ERROR_CUSTOM, 26, line_num, "Duplicate default labels");

         dflt_label = true;
      }
   }

   return statement;
}

Statement *glsl_statement_construct_case(int line_num, Expr *expr) {
   Statement *statement = glsl_statement_construct(STATEMENT_CASE, line_num);
   statement->u.case_stmt.expr = expr;
   return statement;
}

static bool cond_or_decl_is_bool(Statement *cond_or_decl) {
   assert(cond_or_decl->flavour == STATEMENT_VAR_DECL ||
          cond_or_decl->flavour == STATEMENT_EXPR);

   SymbolType *type;
   if (cond_or_decl->flavour == STATEMENT_VAR_DECL) {
      type = cond_or_decl->u.var_decl.var->type;
   } else {
      type = cond_or_decl->u.expr.expr->type;
   }

   if (type->flavour != SYMBOL_PRIMITIVE_TYPE) return false;
   if (type->u.primitive_type.index != PRIM_BOOL) return false;
   return true;
}

Statement *glsl_statement_construct_iterator_for(int line_num, Statement *init, Statement *cond_or_decl, Expr *loop, Statement *block)
{
   Statement *statement = glsl_statement_construct(STATEMENT_ITERATOR_FOR, line_num);

   /* TODO: Should we just leave this NULL instead? */
   if (cond_or_decl == NULL)
      cond_or_decl = glsl_statement_construct_expr(line_num, glsl_expr_construct_const_value(line_num, PRIM_BOOL, 1));

   statement->u.iterator_for.init = init;
   statement->u.iterator_for.cond_or_decl = cond_or_decl;
   statement->u.iterator_for.loop = loop;
   statement->u.iterator_for.block = promote_to_statement_compound(block);

   if (!cond_or_decl_is_bool(cond_or_decl))
      glsl_compile_error(ERROR_SEMANTIC, 1, statement->line_num, "loop condition must be a boolean");

   return statement;
}

Statement *glsl_statement_construct_iterator_while(int line_num, Statement *cond_or_decl, Statement *block)
{
   Statement *statement = glsl_statement_construct(STATEMENT_ITERATOR_WHILE, line_num);
   statement->u.iterator_while.cond_or_decl = cond_or_decl;
   statement->u.iterator_while.block = promote_to_statement_compound(block);

   if (!cond_or_decl_is_bool(cond_or_decl)) {
      glsl_compile_error(ERROR_SEMANTIC, 1, statement->line_num, "loop condition must be a boolean");
      return NULL;
   }

   return statement;
}

Statement *glsl_statement_construct_iterator_do_while(int line_num, Statement *block, Expr *cond)
{
   Statement *statement = glsl_statement_construct(STATEMENT_ITERATOR_DO_WHILE, line_num);
   statement->u.iterator_do_while.block = promote_to_statement_compound(block);
   statement->u.iterator_do_while.cond  = cond;

   if (cond->type->flavour != SYMBOL_PRIMITIVE_TYPE ||
       cond->type->u.primitive_type.index != PRIM_BOOL )
   {
      glsl_compile_error(ERROR_SEMANTIC, 1, statement->line_num, "loop condition must be a boolean");
   }

   return statement;
}

Statement *glsl_statement_construct_return_expr(int line_num, Expr *expr) {
   Statement *statement = glsl_statement_construct(STATEMENT_RETURN_EXPR, line_num);
   statement->u.return_expr.expr = expr;
   return statement;
}



//
// Chain constructors.
//

ExprChain *glsl_expr_chain_create(void)
{
   ExprChain *chain = malloc_fast(sizeof(ExprChain));

   chain->count = 0;

   chain->first = NULL;
   chain->last = NULL;

   return chain;
}

ExprChain *glsl_expr_chain_append_node(ExprChain *chain, ExprChainNode *node)
{
   node->next = NULL;
   node->prev = chain->last;

   if (!chain->first) chain->first = node;
   if (chain->last) chain->last->next = node;
   chain->last = node;

   chain->count++;

   return chain;
}

ExprChain *glsl_expr_chain_append(ExprChain *chain, Expr *expr)
{
   ExprChainNode *node = malloc_fast(sizeof(ExprChainNode));

   node->expr = expr;

   return glsl_expr_chain_append_node(chain, node);
}

StatementChain *glsl_statement_chain_create(void)
{
   StatementChain *chain = malloc_fast(sizeof(StatementChain));

   chain->count = 0;

   chain->first = NULL;
   chain->last = NULL;

   return chain;
}

StatementChain *glsl_statement_chain_append(StatementChain *chain, Statement *statement)
{
   StatementChainNode *node = malloc_fast(sizeof(StatementChainNode));
   node->statement = statement;
   node->prev = chain->last;
   node->next = NULL;

   if (!chain->first) chain->first = node;
   if (chain->last) chain->last->next = node;
   chain->last = node;

   chain->count++;

   return chain;
}

StatementChain *glsl_statement_chain_cat(StatementChain *a, StatementChain *b)
{
   if (b->count == 0) return a;

   assert(b->first && b->last);

   if (a->count == 0)
      a->first = b->first;
   else {
      a->last->next = b->first;
      b->first->prev = a->last;
   }
   a->last = b->last;
   a->count += b->count;

   return a;
}
