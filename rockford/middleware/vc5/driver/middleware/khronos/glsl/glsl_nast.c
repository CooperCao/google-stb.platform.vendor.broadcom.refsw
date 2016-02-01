/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_nast.h"
#include "glsl_fastmem.h"

static NStmt *glsl_nstmt_new_common(NStmtFlavour flavour, int line_num)
{
   NStmt *stmt = malloc_fast(sizeof(NStmt));
   stmt->flavour = flavour;
   stmt->line_num = line_num;
   return stmt;
}

const NStmt *glsl_nstmt_new(NStmtFlavour flavour, int line_num)
{
   return glsl_nstmt_new_common(flavour, line_num);
}

const NStmt *glsl_nstmt_new_var_decl(int line_num, Symbol *var, Expr *initializer)
{
   NStmt *stmt = glsl_nstmt_new_common(NSTMT_VAR_DECL, line_num);
   stmt->u.var_decl.var = var;
   stmt->u.var_decl.initializer = initializer;
   return stmt;
}

const NStmt *glsl_nstmt_new_function_def(int line_num, Symbol *header, NStmtList *body)
{
   NStmt *stmt = glsl_nstmt_new_common(NSTMT_FUNCTION_DEF, line_num);
   stmt->u.function_def.header = header;
   stmt->u.function_def.body = body;
   return stmt;
}

const NStmt *glsl_nstmt_new_assign(int line_num, Expr *lvalue, Expr *rvalue)
{
   NStmt *stmt = glsl_nstmt_new_common(NSTMT_ASSIGN, line_num);
   stmt->u.assign.lvalue = lvalue;
   stmt->u.assign.rvalue = rvalue;
   return stmt;
}

const NStmt *glsl_nstmt_new_function_call(int line_num, Expr *lvalue, Symbol *function, ExprChain *args)
{
   NStmt *stmt = glsl_nstmt_new_common(NSTMT_FUNCTION_CALL, line_num);
   stmt->u.function_call.lvalue = lvalue;
   stmt->u.function_call.function = function;
   stmt->u.function_call.args = args;
   return stmt;
}

const NStmt *glsl_nstmt_new_selection(int line_num, Expr *cond, NStmtList *if_true, NStmtList *if_false)
{
   NStmt *stmt = glsl_nstmt_new_common(NSTMT_SELECTION, line_num);
   stmt->u.selection.cond = cond;
   stmt->u.selection.if_true = if_true;
   stmt->u.selection.if_false = if_false;
   return stmt;
}

const NStmt *glsl_nstmt_new_switch(int line_num, Expr *cond, NStmtList *statements)
{
   NStmt *stmt = glsl_nstmt_new_common(NSTMT_SWITCH, line_num);
   stmt->u.switch_stmt.cond = cond;
   stmt->u.switch_stmt.statements = statements;
   return stmt;
}

const NStmt *glsl_nstmt_new_case(int line_num, Expr *expr)
{
   NStmt *stmt = glsl_nstmt_new_common(NSTMT_CASE, line_num);
   stmt->u.case_stmt.expr = expr;
   return stmt;
}

const NStmt *glsl_nstmt_new_iterator(int line_num, NStmtList *pre_cond_stmts, Expr *pre_cond_expr, NStmtList *body, NStmtList *post_cond_stmts, Expr *post_cond_expr, NStmtList *increment, Symbol *loop_index)
{
   NStmt *stmt = glsl_nstmt_new_common(NSTMT_ITERATOR, line_num);
   stmt->u.iterator.pre_cond_stmts  = pre_cond_stmts;
   stmt->u.iterator.pre_cond_expr   = pre_cond_expr;
   stmt->u.iterator.body            = body;
   stmt->u.iterator.post_cond_stmts = post_cond_stmts;
   stmt->u.iterator.post_cond_expr  = post_cond_expr;
   stmt->u.iterator.increment       = increment;
   stmt->u.iterator.loop_index      = loop_index;
   return stmt;
}

const NStmt *glsl_nstmt_new_return_expr(int line_num, Expr *expr)
{
   NStmt *stmt = glsl_nstmt_new_common(NSTMT_RETURN_EXPR, line_num);
   stmt->u.return_expr.expr = expr;
   return stmt;
}

NStmtList *glsl_nstmt_list_new(void)
{
   NStmtList *list = malloc_fast(sizeof(NStmtList));
   list->head = NULL;
   list->tail = NULL;
   return list;
}

void glsl_nstmt_list_add(NStmtList *list, const NStmt *value)
{
   NStmtListNode *node = malloc_fast(sizeof(NStmtListNode));
   node->v = value;
   node->next = NULL;
   if (list->tail) {
      list->tail->next = node;
      list->tail = node;
   } else {
      list->head = node;
      list->tail = node;
   }
}

void glsl_nstmt_list_add_list(NStmtList *dst, NStmtList *src)
{
   NStmtListNode *node;
   for (node = src->head; node; node = node->next)
      glsl_nstmt_list_add(dst, node->v);
}
