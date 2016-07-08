/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_common.h"
#include "glsl_ast.h"
#include "glsl_ast_print.h"
#include "glsl_nast.h"
#include "glsl_nast_print.h"
#include "glsl_symbols.h"

#include <stdio.h>

#ifndef NDEBUG

static void print_indent(FILE* f, int indent)
{
   for (int i = 0; i < indent; i++)
      fprintf(f, "\t");
}

static void print_line(FILE* f, int indent, const char* text)
{
   print_indent(f, indent);
   fprintf(f, "%s\n", text);
}

static void print_expr_line(FILE* f, int indent, const char* pre_text, Expr* expr, const char* post_text)
{
   print_indent(f, indent);
   fprintf(f, "%s", pre_text);
   if (expr) {
      glsl_print_expr(f, expr, true);
   } else {
      fprintf(f, "(NULL)");
   }
   fprintf(f, "%s", post_text);
   fprintf(f, "\n");
}

static void print_statement_function_def(FILE* f, const NStmt* stmt, int indent)
{
   Symbol* header = stmt->u.function_def.header;
   NStmtList* body = stmt->u.function_def.body;

   print_indent(f, indent);
   fprintf(f, "%s %s(", header->type->u.function_type.return_type->name, header->name);
   for (unsigned i = 0; i < header->type->u.function_type.param_count; i++) {
      glsl_print_qualifiers(f, header->type->u.function_type.params[i]);
      fprintf(f, "%s %s", header->type->u.function_type.params[i]->type->name, header->type->u.function_type.params[i]->name);
      if (i < header->type->u.function_type.param_count-1) fprintf(f, ", ");
   }
   fprintf(f, ")\n");
   print_line(f, indent, "{");
   glsl_nast_print_statements(f, indent + 1, body);
   print_line(f, indent, "}");
}

static void print_statement_var_def(FILE* f, const NStmt* stmt, int indent)
{
   Symbol* var = stmt->u.var_decl.var;
   Expr* initializer = stmt->u.var_decl.initializer;

   print_indent(f, indent);
   glsl_print_qualifiers(f, var);
   fprintf(f, "%s %s", var->type->name, var->name);
   if (initializer) {
      fprintf(f, " = ");
      glsl_print_expr(f, initializer, true);
   }
   fprintf(f, ";\n");
}

static void print_statement_function_call(FILE* f, const NStmt* stmt, int indent)
{
   ExprChainNode* arg;
   print_indent(f, indent);
   glsl_print_expr(f, stmt->u.function_call.lvalue, true);
   fprintf(f, " = %s(", stmt->u.function_call.function->name);
   for (arg = stmt->u.function_call.args->first; arg; arg = arg->next) {
      glsl_print_expr(f, arg->expr, true);
      if (arg->next)
         fprintf(f, ", ");
   }
   fprintf(f, ");\n");
}

static void print_statement_selection(FILE* f, const NStmt* stmt, int indent)
{
   print_expr_line(f, indent, "if (", stmt->u.selection.cond, ") {");
   glsl_nast_print_statements(f, indent + 1, stmt->u.selection.if_true);
   if (stmt->u.selection.if_false && stmt->u.selection.if_false->head) {
      print_line(f, indent, "} else {");
      glsl_nast_print_statements(f, indent + 1, stmt->u.selection.if_false);
   }
   print_line(f, indent, "}");
}

static void print_statement_iterator(FILE* f, const NStmt* stmt, int indent)
{
   print_line(f, indent, "iterator {");
   if (stmt->u.iterator.loop_index) {
      print_line(f, indent + 1, "loop_index:");
      print_indent(f, indent + 2);
      fprintf(f, "%s\n", stmt->u.iterator.loop_index->name);
   }
   print_line(f, indent + 1, "pre_cond:");
   glsl_nast_print_statements(f, indent + 2, stmt->u.iterator.pre_cond_stmts);
   print_expr_line(f, indent + 2, "", stmt->u.iterator.pre_cond_expr, "");
   print_line(f, indent + 1, "body:");
   glsl_nast_print_statements(f, indent + 2, stmt->u.iterator.body);
   print_line(f, indent + 1, "post_cond:");
   glsl_nast_print_statements(f, indent + 2, stmt->u.iterator.post_cond_stmts);
   print_expr_line(f, indent + 2, "", stmt->u.iterator.post_cond_expr, "");
   print_line(f, indent + 1, "increment:");
   glsl_nast_print_statements(f, indent + 2, stmt->u.iterator.increment);
   print_line(f, indent, "}");
}

void glsl_nast_print_statement(FILE* f, int indent, const NStmt* stmt)
{
   switch (stmt->flavour) {
      case NSTMT_FUNCTION_DEF:
         print_statement_function_def(f, stmt, indent);
         break;
      case NSTMT_VAR_DECL:
         print_statement_var_def(f, stmt, indent);
         break;
      case NSTMT_ASSIGN:
         print_indent(f, indent);
         glsl_print_expr(f, stmt->u.assign.lvalue, true);
         fprintf(f, " = ");
         glsl_print_expr(f, stmt->u.assign.rvalue, true);
         fprintf(f, ";\n");
         break;
      case NSTMT_FUNCTION_CALL:
         print_statement_function_call(f, stmt, indent);
         break;
      case NSTMT_SELECTION:
         print_statement_selection(f, stmt, indent);
         break;
      case NSTMT_ITERATOR:
         print_statement_iterator(f, stmt, indent);
         break;
      case NSTMT_SWITCH:
         print_expr_line(f, indent, "switch (", stmt->u.switch_stmt.cond, ") {");
         glsl_nast_print_statements(f, indent + 1, stmt->u.switch_stmt.statements);
         print_line(f, indent, "}");
         break;
      case NSTMT_CASE:
         print_expr_line(f, indent - 1, "case ", stmt->u.case_stmt.expr, ":");
         break;
      case NSTMT_DEFAULT:
         print_line(f, indent - 1, "default:");
         break;
      case NSTMT_CONTINUE:
         print_line(f, indent, "continue;");
         break;
      case NSTMT_BREAK:
         print_line(f, indent, "break;");
         break;
      case NSTMT_DISCARD:
         print_line(f, indent, "discard;");
         break;
      case NSTMT_RETURN:
         print_line(f, indent, "return;");
         break;
      case NSTMT_RETURN_EXPR:
         print_expr_line(f, indent, "return ", stmt->u.return_expr.expr, ";");
         break;
      case NSTMT_BARRIER:
         print_line(f, indent, "barrier;");
         break;
      case NSTMT_FLAVOUR_COUNT:
         UNREACHABLE();
         break;
   }
   fflush(f);
}

void glsl_nast_print_statements(FILE* f, int indent, const NStmtList* statements)
{
   if (statements) {
      const NStmtListNode* node;
      for (node = statements->head; node; node = node->next)
         glsl_nast_print_statement(f, indent, node->v);
   }
}

#endif /* NEDBUG */
