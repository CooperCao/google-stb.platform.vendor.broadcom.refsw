/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_nast.h"
#include "glsl_ast.h"
#include "glsl_ast_print.h"
#include "glsl_ast_visitor.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_map.h"

#include "glsl_stdlib.auto.h"

static Expr *eval_expr(NStmtList *nstmts, Expr *expr, bool make_lvalue);
static void eval_stmt(NStmtList *nstmts, Statement *stmt);

/* Find expressions that have block-level side effects. This means changing to
 * a new block, or generating assignments within the block.
 * TODO: Change it so that assignments don't have to be trapped here */
static void vpost_expr_has_no_side_effects(Expr *expr, void *data)
{
   bool *result = data;
   switch (expr->flavour) {
      case EXPR_ASSIGN:
      case EXPR_LOGICAL_AND:
      case EXPR_LOGICAL_OR:
      case EXPR_CONDITIONAL:
      case EXPR_POST_INC:
      case EXPR_POST_DEC:
      case EXPR_PRE_INC:
      case EXPR_PRE_DEC:
         *result = false;
         break;
      case EXPR_FUNCTION_CALL: {
         Symbol    *function = expr->u.function_call.function;
         Statement *function_def = function->u.function_instance.function_def;
         Statement *body = function_def->u.function_def.body;

         // don't allow any out params
         for (unsigned i = 0; i < function->type->u.function_type.param_count; i++) {
            if (function->type->u.function_type.params[i]->u.param_instance.param_qual != PARAM_QUAL_IN)
               *result = false;
         }

         // check that the function body is a return statement
         assert(body->flavour == STATEMENT_COMPOUND);
         StatementChainNode *body_stmt = body->u.compound.statements->first;
         if (body_stmt && body_stmt->statement->flavour == STATEMENT_RETURN_EXPR)
            glsl_expr_accept_postfix(body_stmt->statement->u.return_expr.expr, result, vpost_expr_has_no_side_effects);
         else
            *result = false;
         break;
      }
      default:
         break;
   }
}

static bool expr_has_no_side_effects(Expr *expr) {
   bool result = true;
   glsl_expr_accept_postfix(expr, &result, vpost_expr_has_no_side_effects);
   return result;
}

static void add_assign_stmt(NStmtList *nstmts, Expr *lvalue, Expr *rvalue)
{
   glsl_nstmt_list_add(nstmts, glsl_nstmt_new_assign(lvalue, rvalue));
}

static ExprChain *eval_expr_list(NStmtList *nstmts, ExprChain *chain)
{
   ExprChain *copy = glsl_expr_chain_create();
   for (ExprChainNode *node = chain->first; node; node = node->next)
      glsl_expr_chain_append(copy, eval_expr(nstmts, node->expr, false));
   return copy;
}

// Recurse down the expression tree and add one statement for each expression.
// The return value is reference to temporary symbol which stores the value.
static Expr *eval_expr(NStmtList *nstmts, Expr *expr, bool make_lvalue)
{
   Expr *nexpr;

   // compile-time constant
   /* TODO: Array length expressions can both be constant and have side effects */
   if (expr->compile_time_value)
      return expr;

   // Samplers and images are immutable and (because they're only indexed by constants) side-effect free
   if (glsl_prim_is_prim_sampler_type(expr->type) || glsl_prim_is_prim_image_type(expr->type))
      return expr;

   if (!make_lvalue && expr_has_no_side_effects(expr)) {
      // optimisation - use the whole expression as-is if there are no side effects
      nexpr = expr;
   } else if (expr->flavour == EXPR_INSTANCE) {
      // optimisation - no need to copy since it has no children
      nexpr = expr;
   } else {
      nexpr = malloc_fast(sizeof(Expr));
      memcpy(nexpr, expr, sizeof(Expr));

      switch (expr->flavour) {
      case EXPR_ASSIGN: {
         Expr *lvalue = eval_expr(nstmts, expr->u.assign_op.lvalue, true);
         Expr *rvalue = eval_expr(nstmts, expr->u.assign_op.rvalue, false);
         add_assign_stmt(nstmts, lvalue, rvalue);
         return rvalue;
      }
      case EXPR_LOGICAL_AND:
      case EXPR_LOGICAL_OR: {
         Expr *temp = glsl_expr_construct_instance(expr->line_num, glsl_symbol_construct_temporary(expr->type));
         Expr *cond = (expr->flavour == EXPR_LOGICAL_AND ? temp : glsl_expr_construct_unary_op(EXPR_LOGICAL_NOT, expr->line_num, temp));
         NStmtList *other = glsl_nstmt_list_new();
         add_assign_stmt(nstmts, temp, eval_expr(nstmts, expr->u.binary_op.left, false));
         add_assign_stmt(other, temp, eval_expr(other, expr->u.binary_op.right, false));
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new_selection(cond, other, NULL));
         return temp;
      }
      case EXPR_CONDITIONAL: {
         Expr *temp = glsl_expr_construct_instance(expr->line_num, glsl_symbol_construct_temporary(expr->type));
         Expr *cond = eval_expr(nstmts, expr->u.cond_op.cond, false);
         NStmtList *if_true = glsl_nstmt_list_new();
         NStmtList *if_false = glsl_nstmt_list_new();
         add_assign_stmt(if_true,  temp, eval_expr(if_true,  expr->u.cond_op.if_true,  false));
         add_assign_stmt(if_false, temp, eval_expr(if_false, expr->u.cond_op.if_false, false));
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new_selection(cond, if_true, if_false));
         return temp;
      }
      case EXPR_POST_INC:
      case EXPR_POST_DEC:
      case EXPR_PRE_INC:
      case EXPR_PRE_DEC: {
         ExprFlavour op = (expr->flavour == EXPR_PRE_INC || expr->flavour == EXPR_POST_INC) ? EXPR_ADD : EXPR_SUB;
         Expr *lvalue = eval_expr(nstmts, expr->u.unary_op.operand, true);
         Expr *temp = glsl_expr_construct_instance(expr->line_num, glsl_symbol_construct_temporary(expr->type));
         ExprChain *ch = glsl_expr_chain_create();
         for (unsigned int i=0; i<expr->type->scalar_count; i++)
            glsl_expr_chain_append(ch, glsl_expr_construct_const_value(expr->line_num, PRIM_INT, 1));
         Expr *one = glsl_expr_construct_constructor_call(expr->line_num, lvalue->type, ch);
         if (expr->flavour == EXPR_POST_INC || expr->flavour == EXPR_POST_DEC)
            add_assign_stmt(nstmts, temp, lvalue);
         assert(lvalue->type->flavour == SYMBOL_PRIMITIVE_TYPE);
         add_assign_stmt(nstmts, lvalue, glsl_expr_construct_binary_op_arithmetic(op, expr->line_num, lvalue, one));
         if (expr->flavour == EXPR_PRE_INC || expr->flavour == EXPR_PRE_DEC)
            add_assign_stmt(nstmts, temp, lvalue);
         return temp;
      }
      case EXPR_FUNCTION_CALL: {
         unsigned int i;
         ExprChainNode *node;
         Symbol *function = expr->u.function_call.function;
         NStmtList *copy_back_params = glsl_nstmt_list_new();
         Expr *result = glsl_expr_construct_instance(expr->line_num, glsl_symbol_construct_temporary(expr->type));
         ExprChain *args = glsl_expr_chain_create();
         for (i = 0, node = expr->u.function_call.args->first; node; i++, node = node->next) {
            ParamQualifier qual = function->type->u.function_type.params[i]->u.param_instance.param_qual;
            if (qual == PARAM_QUAL_OUT || qual == PARAM_QUAL_INOUT) {
               Expr *temp = glsl_expr_construct_instance(expr->line_num, glsl_symbol_construct_temporary(node->expr->type));
               Expr *lvalue = eval_expr(nstmts, node->expr, true);
               // read the lvalue now
               add_assign_stmt(nstmts, temp, lvalue);
               glsl_expr_chain_append(args, temp);
               // write back the lvalue later
               add_assign_stmt(copy_back_params, lvalue, temp);
            } else {
               glsl_expr_chain_append(args, eval_expr(nstmts, node->expr, false));
            }
         }
         // call the function
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new_function_call(result, function, args));
         // copy back out params
         // Atomic memory functions don't obey the calling convention here
         if (!glsl_stdlib_is_stdlib_symbol(function) ||
             !(glsl_stdlib_function_properties[glsl_stdlib_function_index(function)] & GLSL_STDLIB_PROPERTY_ATOMIC_MEM))
            glsl_nstmt_list_add_list(nstmts, copy_back_params);
         return result;
      }
      case EXPR_PRIM_CONSTRUCTOR_CALL:
         nexpr->u.prim_constructor_call.args = eval_expr_list(nstmts, expr->u.prim_constructor_call.args);
         break;

      case EXPR_COMPOUND_CONSTRUCTOR_CALL:
         nexpr->u.compound_constructor_call.args = eval_expr_list(nstmts, expr->u.compound_constructor_call.args);
         break;

      case EXPR_SUBSCRIPT:
         nexpr->u.subscript.aggregate = eval_expr(nstmts, expr->u.subscript.aggregate, make_lvalue);
         nexpr->u.subscript.subscript = eval_expr(nstmts, expr->u.subscript.subscript, false);
         break;

      case EXPR_FIELD_SELECTOR:
         nexpr->u.field_selector.aggregate = eval_expr(nstmts, expr->u.field_selector.aggregate, make_lvalue);
         break;

      case EXPR_SWIZZLE:
         nexpr->u.swizzle.aggregate = eval_expr(nstmts, expr->u.swizzle.aggregate, make_lvalue);
         break;

      case EXPR_ARITH_NEGATE:
      case EXPR_LOGICAL_NOT:
      case EXPR_BITWISE_NOT:
         nexpr->u.unary_op.operand = eval_expr(nstmts, expr->u.unary_op.operand, false);
         break;

      case EXPR_MUL:
      case EXPR_DIV:
      case EXPR_REM:
      case EXPR_ADD:
      case EXPR_SUB:
      case EXPR_SHL:
      case EXPR_SHR:
      case EXPR_LESS_THAN:
      case EXPR_LESS_THAN_EQUAL:
      case EXPR_GREATER_THAN:
      case EXPR_GREATER_THAN_EQUAL:
      case EXPR_EQUAL:
      case EXPR_NOT_EQUAL:
      case EXPR_BITWISE_OR:
      case EXPR_BITWISE_AND:
      case EXPR_BITWISE_XOR:
      case EXPR_LOGICAL_XOR:
         nexpr->u.binary_op.left  = eval_expr(nstmts, expr->u.binary_op.left, false);
         nexpr->u.binary_op.right = eval_expr(nstmts, expr->u.binary_op.right, false);
         break;

      case EXPR_SEQUENCE:
         nexpr->u.sequence.all_these = eval_expr(nstmts, expr->u.sequence.all_these, false);
         nexpr->u.sequence.then_this = eval_expr(nstmts, expr->u.sequence.then_this, false);
         break;

      case EXPR_ARRAY_LENGTH:
         nexpr->u.array_length.array = eval_expr(nstmts, expr->u.array_length.array, false);
         break;

      case EXPR_INTRINSIC:
         nexpr->u.intrinsic.args = eval_expr_list(nstmts, expr->u.intrinsic.args);
         break;

      case EXPR_VALUE:        /* Always compile-time constant */
      case EXPR_INSTANCE:     /* Dealt with above */
      case EXPR_FLAVOUR_COUNT:
         unreachable();
         break;
      }
   }

   if (make_lvalue) {
      // don't store lvalue into temporaries
      return nexpr;
   } else {
      // evaluate the expression
      Expr *temp = glsl_expr_construct_instance(expr->line_num, glsl_symbol_construct_temporary(expr->type));
      add_assign_stmt(nstmts, temp, nexpr);
      return temp;
   }
}

static Expr *eval_root_expr(NStmtList *stmts, Expr *expr)
{
   if (expr == NULL)
      return NULL;

   // optimisation - no need to rewrite
   if (expr_has_no_side_effects(expr))
      return expr;

   return eval_expr(stmts, expr, false);
}

static Expr *eval_loop_pre_cond(NStmtList *nstmts, Statement *pre_cond)
{
   if (pre_cond) {
      switch(pre_cond->flavour) {
         case STATEMENT_EXPR:
            return eval_root_expr(nstmts, pre_cond->u.expr.expr);
         case STATEMENT_VAR_DECL: {
            Expr *initializer = pre_cond->u.var_decl.initializer;
            eval_stmt(nstmts, pre_cond);
            if (initializer && initializer->compile_time_value)
               return initializer;
            return glsl_expr_construct_instance(pre_cond->line_num, pre_cond->u.var_decl.var);
         }
         case STATEMENT_NULL:
            break;
         default:
            unreachable();
            break;
      }
   }
   return NULL;
}

static void eval_stmt_list(NStmtList *nstmts, StatementChain *stmts)
{
   for (StatementChainNode *node = stmts->first; node; node = node->next)
      eval_stmt(nstmts, node->statement);
}

static void eval_stmt(NStmtList *nstmts, Statement *stmt)
{
   if (stmt == NULL) return;

   switch(stmt->flavour) {
      case STATEMENT_AST:
         eval_stmt_list(nstmts, stmt->u.ast.decls);
         break;

      case STATEMENT_DECL_LIST:
         eval_stmt_list(nstmts, stmt->u.decl_list.decls);
         break;

      case STATEMENT_FUNCTION_DEF: {
         Symbol *function = stmt->u.function_def.header;
         NStmtList *body = glsl_nstmt_list_new();
         eval_stmt(body, stmt->u.function_def.body);
         const NStmt *def = glsl_nstmt_new_function_def(function, body);
         glsl_nstmt_list_add(nstmts, def);
         assert(function->flavour == SYMBOL_FUNCTION_INSTANCE);
         function->u.function_instance.function_norm_def = def;
         break;
      }
      case STATEMENT_VAR_DECL: {
         Symbol *var = stmt->u.var_decl.var;
         Expr *initializer = eval_root_expr(nstmts, stmt->u.var_decl.initializer);
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new_var_decl(var, initializer));
         break;
      }
      case STATEMENT_COMPOUND:
         eval_stmt_list(nstmts, stmt->u.compound.statements);
         break;

      case STATEMENT_EXPR: {
         Expr *expr = stmt->u.expr.expr;
         if (expr->flavour == EXPR_ASSIGN && expr_has_no_side_effects(expr->u.assign_op.rvalue)) {
            // optimisation - root assignment
            Expr *lvalue = eval_expr(nstmts, expr->u.assign_op.lvalue, true);
            Expr *rvalue = expr->u.assign_op.rvalue;
            add_assign_stmt(nstmts, lvalue, rvalue);
         } else {
            glsl_nstmt_list_add(nstmts, glsl_nstmt_new_expr(eval_root_expr(nstmts, stmt->u.expr.expr)));
         }
         break;
      }
      case STATEMENT_SELECTION: {
         Expr *cond = eval_root_expr(nstmts, stmt->u.selection.cond);
         NStmtList *if_true = glsl_nstmt_list_new();
         NStmtList *if_false = glsl_nstmt_list_new();
         eval_stmt(if_true, stmt->u.selection.if_true);
         eval_stmt(if_false, stmt->u.selection.if_false);
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new_selection(cond, if_true, if_false));
         break;
      }
      case STATEMENT_SWITCH: {
         Expr *cond = eval_root_expr(nstmts, stmt->u.switch_stmt.cond);
         NStmtList *statements = glsl_nstmt_list_new();
         eval_stmt_list(statements, stmt->u.switch_stmt.stmtChain);
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new_switch(cond, statements));
         break;
      }
      case STATEMENT_ITERATOR_FOR: {
         Statement *init = stmt->u.iterator_for.init;
         NStmtList *pre_cond_stmts = glsl_nstmt_list_new();
         Expr *pre_cond_expr = eval_loop_pre_cond(pre_cond_stmts, stmt->u.iterator_for.cond_or_decl);
         NStmtList *body = glsl_nstmt_list_new();
         NStmtList *increment = glsl_nstmt_list_new();
         eval_stmt(nstmts, init);
         eval_stmt(body, stmt->u.iterator_for.block);
         eval_root_expr(increment, stmt->u.iterator_for.loop);
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new_iterator(pre_cond_stmts, pre_cond_expr,
                                                             body, NULL, NULL, increment));
         break;
      }
      case STATEMENT_ITERATOR_WHILE: {
         NStmtList *pre_cond_stmts = glsl_nstmt_list_new();
         Expr *pre_cond_expr = eval_loop_pre_cond(pre_cond_stmts, stmt->u.iterator_while.cond_or_decl);
         NStmtList *body = glsl_nstmt_list_new();
         eval_stmt(body, stmt->u.iterator_while.block);
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new_iterator(pre_cond_stmts, pre_cond_expr,
                                                             body, NULL, NULL, NULL));
         break;
      }
      case STATEMENT_ITERATOR_DO_WHILE: {
         NStmtList *post_cond_stmts = glsl_nstmt_list_new();
         Expr *post_cond_expr = eval_root_expr(post_cond_stmts, stmt->u.iterator_do_while.cond);
         NStmtList *body = glsl_nstmt_list_new();
         eval_stmt(body, stmt->u.iterator_do_while.block);
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new_iterator(NULL, NULL, body, post_cond_stmts,
                                                             post_cond_expr, NULL));
         break;
      }
      case STATEMENT_CONTINUE:
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new(NSTMT_CONTINUE));
         break;

      case STATEMENT_BREAK:
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new(NSTMT_BREAK));
         break;

      case STATEMENT_CASE:
         assert(stmt->u.case_stmt.expr->compile_time_value);
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new_case(stmt->u.case_stmt.expr));
         break;

      case STATEMENT_DEFAULT:
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new(NSTMT_DEFAULT));
         break;

      case STATEMENT_DISCARD:
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new(NSTMT_DISCARD));
         break;

      case STATEMENT_RETURN:
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new(NSTMT_RETURN));
         break;

      case STATEMENT_RETURN_EXPR: {
         Expr *expr = eval_root_expr(nstmts, stmt->u.return_expr.expr);
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new_return_expr(expr));
         break;
      }
      case STATEMENT_BARRIER:
         glsl_nstmt_list_add(nstmts, glsl_nstmt_new(NSTMT_BARRIER));
         break;
      case STATEMENT_STRUCT_DECL:
      case STATEMENT_STRUCT_MEMBER_DECL:
      case STATEMENT_PRECISION:
      case STATEMENT_QUALIFIER_DEFAULT:
      case STATEMENT_QUALIFIER_AUGMENT:
      case STATEMENT_NULL:
         // ignore
         break;

      case STATEMENT_FLAVOUR_COUNT:
         unreachable();
         break;
   }
}

NStmtList *glsl_nast_build(Statement *ast)
{
   NStmtList *nstmts = glsl_nstmt_list_new();
   assert(ast->flavour == STATEMENT_AST);
   eval_stmt_list(nstmts, ast->u.ast.decls);
   return nstmts;
}
