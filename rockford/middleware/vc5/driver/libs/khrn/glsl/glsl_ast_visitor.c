/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdlib.h>

#include "glsl_common.h"
#include "glsl_ast_visitor.h"

void glsl_expr_accept(Expr* expr, void* data, ExprPreVisitor eprev, ExprPostVisitor epostv)
{
   if (expr)
   {
      ExprChainNode* node;

      if (eprev) expr = eprev(expr, data);
      if (!expr) return;

      switch (expr->flavour)
      {
         case EXPR_VALUE:
         case EXPR_INSTANCE:
            break;

         case EXPR_SUBSCRIPT:
            glsl_expr_accept(expr->u.subscript.aggregate, data, eprev, epostv);
            glsl_expr_accept(expr->u.subscript.subscript, data, eprev, epostv);
            break;

         case EXPR_FUNCTION_CALL:
            for (node = expr->u.function_call.args->first; node; node = node->next)
               glsl_expr_accept(node->expr, data, eprev, epostv);
            break;

         case EXPR_PRIM_CONSTRUCTOR_CALL:
            for (node = expr->u.prim_constructor_call.args->first; node; node = node->next)
               glsl_expr_accept(node->expr, data, eprev, epostv);
            break;

         case EXPR_COMPOUND_CONSTRUCTOR_CALL:
            for (node = expr->u.compound_constructor_call.args->first; node; node = node->next)
               glsl_expr_accept(node->expr, data, eprev, epostv);
            break;

         case EXPR_FIELD_SELECTOR:
            glsl_expr_accept(expr->u.field_selector.aggregate, data, eprev, epostv);
            break;

         case EXPR_SWIZZLE:
            glsl_expr_accept(expr->u.swizzle.aggregate, data, eprev, epostv);
            break;

         case EXPR_POST_INC:
         case EXPR_POST_DEC:
         case EXPR_PRE_INC:
         case EXPR_PRE_DEC:
         case EXPR_ARITH_NEGATE:
         case EXPR_LOGICAL_NOT:
         case EXPR_BITWISE_NOT:
            glsl_expr_accept(expr->u.unary_op.operand, data, eprev, epostv);
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
         case EXPR_LOGICAL_AND:
         case EXPR_LOGICAL_XOR:
         case EXPR_LOGICAL_OR:
         case EXPR_BITWISE_OR:
         case EXPR_BITWISE_AND:
         case EXPR_BITWISE_XOR:
            glsl_expr_accept(expr->u.binary_op.left, data, eprev, epostv);
            glsl_expr_accept(expr->u.binary_op.right, data, eprev, epostv);
            break;

         case EXPR_CONDITIONAL:
            glsl_expr_accept(expr->u.cond_op.cond, data, eprev, epostv);
            glsl_expr_accept(expr->u.cond_op.if_true, data, eprev, epostv);
            glsl_expr_accept(expr->u.cond_op.if_false, data, eprev, epostv);
            break;

         case EXPR_ASSIGN:
            glsl_expr_accept(expr->u.assign_op.lvalue, data, eprev, epostv);
            glsl_expr_accept(expr->u.assign_op.rvalue, data, eprev, epostv);
            break;

         case EXPR_SEQUENCE:
            glsl_expr_accept(expr->u.sequence.all_these, data, eprev, epostv);
            glsl_expr_accept(expr->u.sequence.then_this, data, eprev, epostv);
            break;

         case EXPR_ARRAY_LENGTH:
            glsl_expr_accept(expr->u.array_length.array, data, eprev, epostv);
            break;

         case EXPR_INTRINSIC:
            for (node = expr->u.intrinsic.args->first; node; node = node->next)
            {
               glsl_expr_accept(node->expr, data, eprev, epostv);
            }
            break;

         case EXPR_FLAVOUR_COUNT:
            unreachable();
            break;
      }

      if (epostv) epostv(expr, data);
   }
}

void glsl_expr_accept_prefix(Expr* expr, void* data, ExprPreVisitor eprev)
{
   glsl_expr_accept(expr, data, eprev, NULL);
}

void glsl_expr_accept_postfix(Expr* expr, void* data, ExprPostVisitor epostv)
{
   glsl_expr_accept(expr, data, NULL, epostv);
}

void glsl_statement_accept(Statement* statement, void* data, StatementPreVisitor sprev, ExprPreVisitor eprev, StatementPostVisitor spostv, ExprPostVisitor epostv)
{
   if (statement)
   {
      StatementChainNode* node;

      if (sprev) statement = sprev(statement, data);
      if (!statement) return;

      switch (statement->flavour)
      {
         case STATEMENT_AST:
            for (node = statement->u.ast.decls->first; node; node = node->next)
               glsl_statement_accept(node->statement, data, sprev, eprev, spostv, epostv);
            break;

         case STATEMENT_DECL_LIST:
            for (node = statement->u.decl_list.decls->first; node; node = node->next)
               glsl_statement_accept(node->statement, data, sprev, eprev, spostv, epostv);
            break;

         case STATEMENT_FUNCTION_DEF:
            glsl_statement_accept(statement->u.function_def.body, data, sprev, eprev, spostv, epostv);
            break;

         case STATEMENT_VAR_DECL:
            glsl_expr_accept(statement->u.var_decl.initializer, data, eprev, epostv);
            break;

         case STATEMENT_COMPOUND:
            for (node = statement->u.compound.statements->first; node; node = node->next)
               glsl_statement_accept(node->statement, data, sprev, eprev, spostv, epostv);
            break;

         case STATEMENT_EXPR:
            glsl_expr_accept(statement->u.expr.expr, data, eprev, epostv);
            break;

         case STATEMENT_SELECTION:
            glsl_expr_accept(statement->u.selection.cond, data, eprev, epostv);
            glsl_statement_accept(statement->u.selection.if_true, data, sprev, eprev, spostv, epostv);
            glsl_statement_accept(statement->u.selection.if_false, data, sprev, eprev, spostv, epostv);
            break;

         case STATEMENT_ITERATOR_FOR:
            glsl_statement_accept(statement->u.iterator_for.init, data, sprev, eprev, spostv, epostv);
            glsl_statement_accept(statement->u.iterator_for.cond_or_decl, data, sprev, eprev, spostv, epostv);
            glsl_expr_accept(statement->u.iterator_for.loop, data, eprev, epostv);
            glsl_statement_accept(statement->u.iterator_for.block, data, sprev, eprev, spostv, epostv);
            break;

         case STATEMENT_ITERATOR_WHILE:
            glsl_statement_accept(statement->u.iterator_while.cond_or_decl, data, sprev, eprev, spostv, epostv);
            glsl_statement_accept(statement->u.iterator_while.block, data, sprev, eprev, spostv, epostv);
            break;

         case STATEMENT_ITERATOR_DO_WHILE:
            glsl_statement_accept(statement->u.iterator_do_while.block, data, sprev, eprev, spostv, epostv);
            glsl_expr_accept(statement->u.iterator_do_while.cond, data, eprev, epostv);
            break;

         case STATEMENT_SWITCH:
            glsl_expr_accept(statement->u.switch_stmt.cond, data, eprev, epostv);
            for (node = statement->u.switch_stmt.stmtChain->first; node; node = node->next)
               glsl_statement_accept(node->statement, data, sprev, eprev, spostv, epostv);
            break;

         case STATEMENT_CASE:
            glsl_expr_accept(statement->u.case_stmt.expr, data, eprev, epostv);
            break;

         case STATEMENT_DEFAULT:
         case STATEMENT_CONTINUE:
         case STATEMENT_BREAK:
         case STATEMENT_DISCARD:
         case STATEMENT_RETURN:
            break;

         case STATEMENT_RETURN_EXPR:
            glsl_expr_accept(statement->u.return_expr.expr, data, eprev, epostv);
            break;

         case STATEMENT_PRECISION:
         case STATEMENT_QUALIFIER_DEFAULT:
         case STATEMENT_QUALIFIER_AUGMENT:
         case STATEMENT_BARRIER:
         case STATEMENT_NULL:
            break;

         default:
            unreachable();
            return;
      }

      if (spostv) spostv(statement, data);
   }
}

void glsl_statement_accept_prefix(Statement* statement, void* data, StatementPreVisitor sprev, ExprPreVisitor eprev)
{
   glsl_statement_accept(statement, data, sprev, eprev, NULL, NULL);
}

void glsl_statement_accept_postfix(Statement* statement, void* data, StatementPostVisitor spostv, ExprPostVisitor epostv)
{
   glsl_statement_accept(statement, data, NULL, NULL, spostv, epostv);
}
