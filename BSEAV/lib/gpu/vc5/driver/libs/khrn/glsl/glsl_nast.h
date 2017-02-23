/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_NAST_H_INCLUDED
#define GLSL_NAST_H_INCLUDED

#include "glsl_common.h"

typedef struct _NStmt NStmt;
typedef struct _NStmtList NStmtList;
typedef struct _NStmtListNode NStmtListNode;

typedef enum {
   NSTMT_VAR_DECL,
   NSTMT_FUNCTION_DEF,
   NSTMT_ASSIGN,
   NSTMT_FUNCTION_CALL,
   NSTMT_EXPR,
   NSTMT_SELECTION,
   NSTMT_SWITCH,
   NSTMT_ITERATOR,
   NSTMT_CONTINUE,
   NSTMT_BREAK,
   NSTMT_CASE,
   NSTMT_DEFAULT,
   NSTMT_DISCARD,
   NSTMT_RETURN,
   NSTMT_RETURN_EXPR,
   NSTMT_BARRIER,
   NSTMT_FLAVOUR_COUNT
} NStmtFlavour;

// Normalised statement.  Any referenced expressions have no side-effects and no control-flow.
struct _NStmt {
   NStmtFlavour flavour;

   union {
      // NSTMT_VAR_DECL
      struct {
         Symbol *var;
         Expr   *initializer; // can be NULL
      } var_decl;

      // NSTMT_FUNCTION_DEF
      struct {
         Symbol    *header;
         NStmtList *body;
      } function_def;

      // NSTMT_ASSIGN
      struct {
         Expr *lvalue;
         Expr *rvalue;
      } assign;

      // NSTMT_FUNCTION_CALL
      struct {
         Expr      *lvalue;        // lvalue = function(args);
         Symbol    *function;
         ExprChain *args;
      } function_call;

      // NSTMT_EXPR
      struct {
         Expr *expr;
      } expr;

      // NSTMT_SELECTION
      struct {
         Expr      *cond;
         NStmtList *if_true;
         NStmtList *if_false;
      } selection;

      // NSTMT_SWITCH
      struct {
         Expr      *cond;
         NStmtList *statements;
      } switch_stmt;

      // NSTMT_CASE
      struct {
         Expr *expr;
      } case_stmt;

      // NSTMT_DEFAULT: no metadata

      // NSTMT_ITERATOR
      struct {
         // Generic loop construct with two conditions.
         // Statements are evaluated in this order during each iteration:
         NStmtList *pre_cond_stmts;
         Expr      *pre_cond_expr; // can be NULL
         NStmtList *body;
         NStmtList *post_cond_stmts;
         Expr      *post_cond_expr; // can be NULL
         NStmtList *increment;
      } iterator;

      // NSTMT_DISCARD, NSTMT_CONTINUE, NSTMT_BREAK, NSTMT_RETURN: no metadata

      // NSTMT_RETURN_EXPR
      struct {
         Expr *expr;
      } return_expr;
   } u;
};

struct _NStmtList {
   NStmtListNode *head;
   NStmtListNode *tail;
};

struct _NStmtListNode {
   const NStmt *v;
   NStmtListNode *next;
};

const NStmt *glsl_nstmt_new              (NStmtFlavour flavour);
const NStmt *glsl_nstmt_new_var_decl     (Symbol *var, Expr *initializer);
const NStmt *glsl_nstmt_new_function_def (Symbol *header, NStmtList *body);
const NStmt *glsl_nstmt_new_assign       (Expr *lvalue, Expr *rvalue);
const NStmt *glsl_nstmt_new_function_call(Expr *lvalue, Symbol *function, ExprChain *args);
const NStmt *glsl_nstmt_new_expr         (Expr *expr);
const NStmt *glsl_nstmt_new_selection    (Expr *cond, NStmtList *if_true, NStmtList *if_false);
const NStmt *glsl_nstmt_new_switch       (Expr *cond, NStmtList *statements);
const NStmt *glsl_nstmt_new_case         (Expr *expr);
const NStmt *glsl_nstmt_new_iterator     (NStmtList *pre_cond_stmts, Expr *pre_cond_expr, NStmtList *body,
                                          NStmtList *post_cond_stmts, Expr *post_cond_expr, NStmtList *increment);
const NStmt *glsl_nstmt_new_return_expr(Expr *expr);

NStmtList *glsl_nstmt_list_new(void);
void glsl_nstmt_list_add(NStmtList *list, const NStmt *value);
void glsl_nstmt_list_add_list(NStmtList *dst, NStmtList *src);

#endif
