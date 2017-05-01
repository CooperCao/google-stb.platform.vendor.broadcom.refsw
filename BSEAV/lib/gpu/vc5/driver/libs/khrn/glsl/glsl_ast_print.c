/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_common.h"

#include "glsl_ast_print.h"
#include "glsl_intrinsic.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_symbols.h"

#include <stdio.h>

const char *glsl_param_qual_string(ParamQualifier pq) {
   switch (pq) {
      case PARAM_QUAL_IN:     return "in";
      case PARAM_QUAL_OUT:    return "out";
      case PARAM_QUAL_INOUT:  return "inout";
      default: unreachable(); return NULL;
   }
}

const char *glsl_storage_qual_string(StorageQualifier sq) {
   switch (sq) {
      case STORAGE_NONE:    return "";
      case STORAGE_CONST:   return "const";
      case STORAGE_UNIFORM: return "uniform";
      case STORAGE_IN:      return "in";
      case STORAGE_OUT:     return "out";
      case STORAGE_BUFFER:  return "buffer";
      case STORAGE_SHARED:  return "shared";
      default: unreachable(); return NULL;
   }
}

const char *glsl_interp_qual_string(InterpolationQualifier iq) {
   switch (iq) {
      case INTERP_SMOOTH:        return "smooth";
      case INTERP_NOPERSPECTIVE: return "noperspective";
      case INTERP_FLAT:          return "flat";
      default: unreachable();    return NULL;
   }
}

const char *glsl_aux_qual_string(AuxiliaryQualifier aq) {
   switch (aq) {
      case AUXILIARY_NONE:          return "";
      case AUXILIARY_CENTROID:      return "centroid";
      case AUXILIARY_PATCH:         return "patch";
      case AUXILIARY_SAMPLE:        return "sample";
      default: unreachable();       return NULL;
   }
}

#ifndef NDEBUG

//
// Utility functions.
//

// Helper function for the various argument-based expressions.
static inline void print_expr_chain(FILE* f, ExprChain* args, bool fully_evaluated)
{
   ExprChainNode* arg;
   for (arg = args->first; arg; arg = arg->next)
   {
      glsl_print_expr(f, arg->expr, fully_evaluated);
      if (arg->next) fprintf(f, ", ");
   }
}

// Adds a trailing space
void glsl_print_qualifiers(FILE* f, Symbol* symbol)
{
   switch (symbol->flavour)
   {
      case SYMBOL_PARAM_INSTANCE:
         fprintf(f, "%s ", glsl_storage_qual_string(symbol->u.param_instance.storage_qual));
         fprintf(f, "%s ", glsl_param_qual_string(symbol->u.param_instance.param_qual));
         return;
      case SYMBOL_VAR_INSTANCE:
         fprintf(f, "%s ", glsl_storage_qual_string(symbol->u.var_instance.storage_qual));
         fprintf(f, "%s ", glsl_interp_qual_string(symbol->u.var_instance.interp_qual));
         fprintf(f, "%s ", glsl_aux_qual_string(symbol->u.var_instance.aux_qual));
         return;
      default:
         unreachable();
         return;
   }
}

//
// Value printing.
//

void glsl_print_compile_time_value(FILE *f, SymbolType *type, const_value *compile_time_value)
{
   if (!compile_time_value) { return; }

   int prim_index;

   switch (type->flavour)
   {
      case SYMBOL_PRIMITIVE_TYPE:
         prim_index = type->u.primitive_type.index;
         switch (primitiveTypeFlags[prim_index] & (PRIM_SCALAR_TYPE | PRIM_VECTOR_TYPE | PRIM_MATRIX_TYPE))
         {
            case PRIM_SCALAR_TYPE:
               if (&primitiveTypes[PRIM_INT] == type || &primitiveTypes[PRIM_UINT] == type)
               {
                  fprintf(f, "%d", *compile_time_value);
               }
               else if (&primitiveTypes[PRIM_FLOAT] == type)
               {
                  fprintf(f, "%f", *(float*)compile_time_value);
               }
               else if (&primitiveTypes[PRIM_BOOL] == type)
               {
                  fprintf(f, *compile_time_value ? "true" : "false");
               }
               else
               {
                  unreachable();
               }
               return;
            case PRIM_VECTOR_TYPE:
               // fall
            case PRIM_MATRIX_TYPE:
               fprintf(f, "%s(", primitiveTypes[prim_index].name);
               for (unsigned i = 0; i < primitiveTypeSubscriptDimensions[prim_index]; i++)
               {
                  glsl_print_compile_time_value(f, primitiveTypeSubscriptTypes[prim_index], compile_time_value);
                  compile_time_value = compile_time_value + primitiveTypeSubscriptTypes[prim_index]->scalar_count;
                  if (i+1 < primitiveTypeSubscriptDimensions[prim_index]) fprintf(f, ", ");
               }
               fprintf(f, ")");
               return;
            default:
               unreachable();
               return;
         }

      case SYMBOL_STRUCT_TYPE:
      case SYMBOL_BLOCK_TYPE:
         fprintf(f, "%s(", type->name);
         for (unsigned i = 0; i < type->u.struct_type.member_count; i++)
         {
            glsl_print_compile_time_value(f, type->u.struct_type.member[i].type, compile_time_value);
            compile_time_value = compile_time_value + type->u.struct_type.member[i].type->scalar_count;
            if (i+1 < type->u.struct_type.member_count) fprintf(f, ", ");
         }
         fprintf(f, ")");
         return;

      default:
         // Nothing else can have a compile time value.
         unreachable();
         return;
   }
}


//
// Expression printing.
//

static void print_expr_value(FILE *f, Expr *expr, bool fully_evaluated)
{
   ((void)fully_evaluated);

   glsl_print_compile_time_value(f, expr->type, expr->compile_time_value);
}

static void print_expr_instance(FILE *f, Expr *expr, bool fully_evaluated)
{
   ((void)fully_evaluated);

   fprintf(f, "%s", expr->u.instance.symbol->name);
}

static void print_expr_subscript(FILE *f, Expr *expr, bool fully_evaluated)
{
   glsl_print_expr(f, expr->u.subscript.aggregate, fully_evaluated);
   fprintf(f, "[");
   glsl_print_expr(f, expr->u.subscript.subscript, fully_evaluated);
   fprintf(f, "]");
}

static void print_expr_function_call(FILE* f, Expr* expr, bool fully_evaluated)
{
   fprintf(f, "%s(", expr->u.function_call.function->name);
   print_expr_chain(f, expr->u.function_call.args, fully_evaluated);
   fprintf(f, ")");
}

static void print_expr_prim_constructor_call(FILE* f, Expr* expr, bool fully_evaluated)
{
   fprintf(f, "%s(", expr->type->name);
   print_expr_chain(f, expr->u.prim_constructor_call.args, fully_evaluated);
   fprintf(f, ")");
}

static void print_expr_compound_constructor_call(FILE* f, Expr* expr, bool fully_evaluated)
{
   fprintf(f, "%s(", expr->type->name);
   print_expr_chain(f, expr->u.compound_constructor_call.args, fully_evaluated);
   fprintf(f, ")");
}

static void print_expr_field_selector(FILE* f, Expr* expr, bool fully_evaluated)
{
   glsl_print_expr(f, expr->u.field_selector.aggregate, fully_evaluated);
   fprintf(f, ".%s", expr->u.field_selector.field);
}

static void print_expr_swizzle(FILE* f, Expr* expr, bool fully_evaluated)
{
   glsl_print_expr(f, expr->u.swizzle.aggregate, fully_evaluated);
   fprintf(f, ".%s", expr->u.swizzle.field);
}

static void print_expr_unary_op(FILE* f, Expr* expr, bool fully_evaluated)
{
   switch (expr->flavour)
   {
      case EXPR_POST_INC:
         glsl_print_expr(f, expr->u.unary_op.operand, fully_evaluated);
         fprintf(f, "++");
         return;
      case EXPR_POST_DEC:
         glsl_print_expr(f, expr->u.unary_op.operand, fully_evaluated);
         fprintf(f, "--");
         return;

      case EXPR_PRE_INC:
         fprintf(f, "++");
         glsl_print_expr(f, expr->u.unary_op.operand, fully_evaluated);
         return;
      case EXPR_PRE_DEC:
         fprintf(f, "--");
         glsl_print_expr(f, expr->u.unary_op.operand, fully_evaluated);
         return;
      case EXPR_ARITH_NEGATE:
         fprintf(f, "-");
         glsl_print_expr(f, expr->u.unary_op.operand, fully_evaluated);
         return;
      case EXPR_LOGICAL_NOT:
         fprintf(f, "!");
         glsl_print_expr(f, expr->u.unary_op.operand, fully_evaluated);
         return;
      case EXPR_BITWISE_NOT:
         fprintf(f, "~");
         glsl_print_expr(f, expr->u.unary_op.operand, fully_evaluated);
         return;
      default:
         unreachable();
         return;
   }
}

static void print_expr_binary_op(FILE* f, Expr* expr, bool fully_evaluated)
{
   const char* infix;

   switch (expr->flavour)
   {
      case EXPR_MUL:
         infix = "*";
         break;
      case EXPR_DIV:
         infix = "/";
         break;
      case EXPR_REM:
         infix = "%";
         break;
      case EXPR_ADD:
         infix = "+";
         break;
      case EXPR_SUB:
         infix = "-";
         break;
      case EXPR_SHL:
         infix = "<<";
         break;
      case EXPR_SHR:
         infix = ">>";
         break;
      case EXPR_LESS_THAN:
         infix = "<";
         break;
      case EXPR_LESS_THAN_EQUAL:
         infix = "<=";
         break;
      case EXPR_GREATER_THAN:
         infix = ">";
         break;
      case EXPR_GREATER_THAN_EQUAL:
         infix = ">=";
         break;
      case EXPR_EQUAL:
         infix = "==";
         break;
      case EXPR_NOT_EQUAL:
         infix = "!=";
         break;
      case EXPR_LOGICAL_AND:
         infix = "&&";
         break;
      case EXPR_LOGICAL_XOR:
         infix = "^^";
         break;
      case EXPR_LOGICAL_OR:
         infix = "||";
         break;
      case EXPR_BITWISE_AND:
         infix = "&";
         break;
      case EXPR_BITWISE_XOR:
         infix = "^";
         break;
      case EXPR_BITWISE_OR:
         infix = "|";
         break;
      default:
         unreachable();
         return;
   }

   glsl_print_expr(f, expr->u.binary_op.left, fully_evaluated);
   fprintf(f, " %s ", infix);
   glsl_print_expr(f, expr->u.binary_op.right, fully_evaluated);
}

static void print_expr_cond_op(FILE* f, Expr* expr, bool fully_evaluated)
{
   glsl_print_expr(f, expr->u.cond_op.cond, fully_evaluated);
   fprintf(f, " ? ");
   glsl_print_expr(f, expr->u.cond_op.if_true, fully_evaluated);
   fprintf(f, " : ");
   glsl_print_expr(f, expr->u.cond_op.if_false, fully_evaluated);
}

static void print_expr_assign_op(FILE* f, Expr* expr, bool fully_evaluated)
{
   glsl_print_expr(f, expr->u.assign_op.lvalue, fully_evaluated);
   fprintf(f, " = ");
   glsl_print_expr(f, expr->u.assign_op.rvalue, fully_evaluated);
}

static void print_expr_sequence(FILE* f, Expr* expr, bool fully_evaluated)
{
   glsl_print_expr(f, expr->u.sequence.all_these, fully_evaluated);
   fprintf(f, ", ");
   glsl_print_expr(f, expr->u.sequence.then_this, fully_evaluated);
}

static void print_expr_array_length(FILE *f, Expr *expr, bool fully_evaluated) {
   glsl_print_expr(f, expr->u.array_length.array, fully_evaluated);
   fprintf(f, ".length()");
}

static void print_expr_intrinsic(FILE* f, Expr* expr, bool fully_evaluated)
{
   fprintf(f, "$$%s(", glsl_intrinsic_name(expr->u.intrinsic.flavour));
   print_expr_chain(f, expr->u.intrinsic.args, fully_evaluated);
   fprintf(f, ")");
}

void glsl_print_expr(FILE* f, Expr* expr, bool fully_evaluated)
{
   // Don't print out the whole expression if we are allowed to compress it.
   if (fully_evaluated && expr->compile_time_value)
   {
      glsl_print_compile_time_value(f, expr->type, expr->compile_time_value);
      return;
   }

   switch (expr->flavour)
   {
      case EXPR_VALUE:
         print_expr_value(f, expr, fully_evaluated);
         return;

      case EXPR_INSTANCE:
         print_expr_instance(f, expr, fully_evaluated);
         return;

      case EXPR_SUBSCRIPT:
         print_expr_subscript(f, expr, fully_evaluated);
         return;

      case EXPR_FUNCTION_CALL:
         print_expr_function_call(f, expr, fully_evaluated);
         return;

      case EXPR_PRIM_CONSTRUCTOR_CALL:
         print_expr_prim_constructor_call(f, expr, fully_evaluated);
         return;

      case EXPR_COMPOUND_CONSTRUCTOR_CALL:
         print_expr_compound_constructor_call(f, expr, fully_evaluated);
         return;

      case EXPR_FIELD_SELECTOR:
         print_expr_field_selector(f, expr, fully_evaluated);
         return;

      case EXPR_SWIZZLE:
         print_expr_swizzle(f, expr, fully_evaluated);
         return;

      case EXPR_POST_INC:
      case EXPR_POST_DEC:
      case EXPR_PRE_INC:
      case EXPR_PRE_DEC:
      case EXPR_ARITH_NEGATE:
      case EXPR_LOGICAL_NOT:
      case EXPR_BITWISE_NOT:
         // The postfix operators don't really need the brackets, but never mind.
         fprintf(f, "(");
         print_expr_unary_op(f, expr, fully_evaluated);
         fprintf(f, ")");
         return;

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
      case EXPR_BITWISE_AND:
      case EXPR_BITWISE_XOR:
      case EXPR_BITWISE_OR:
         fprintf(f, "(");
         print_expr_binary_op(f, expr, fully_evaluated);
         fprintf(f, ")");
         return;

      case EXPR_CONDITIONAL:
         fprintf(f, "(");
         print_expr_cond_op(f, expr, fully_evaluated);
         fprintf(f, ")");
         return;

      case EXPR_ASSIGN:
         fprintf(f, "(");
         print_expr_assign_op(f, expr, fully_evaluated);
         fprintf(f, ")");
         return;

      case EXPR_SEQUENCE:
         fprintf(f, "(");
         print_expr_sequence(f, expr, fully_evaluated);
         fprintf(f, ")");
         return;

      case EXPR_ARRAY_LENGTH:
         fprintf(f, "(");
         print_expr_array_length(f, expr, fully_evaluated);
         fprintf(f, ")");
         return;

      case EXPR_INTRINSIC:
         print_expr_intrinsic(f, expr, fully_evaluated);
         return;

      case EXPR_FLAVOUR_COUNT:
         unreachable();
         return;
   }
}


//
// Statement printing.
//

static void print_statement_ast(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   StatementChainNode* decl;
   for (decl = statement->u.ast.decls->first; decl; decl = decl->next)
   {
      glsl_print_statement(f, decl->statement, fully_evaluated, indent_depth, suppress_semicolon);
      if (decl->next) fprintf(f, "\n");
   }
}

static void print_statement_decl_list(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   StatementChainNode* decl;
   for (decl = statement->u.decl_list.decls->first; decl; decl = decl->next)
   {
      glsl_print_statement(f, decl->statement, fully_evaluated, indent_depth, suppress_semicolon);
      if (decl->next) fprintf(f, "\n");
   }
}

static void print_statement_function_def(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   Symbol *header = statement->u.function_def.header;
   Statement *body = statement->u.function_def.body;

   for (unsigned i = 0; i < indent_depth; i++) fprintf(f, "\t");

   fprintf(f, "%s %s(", header->type->u.function_type.return_type->name, header->name);

   for (unsigned i = 0; i < header->type->u.function_type.param_count; i++)
   {
      glsl_print_qualifiers(f, header->type->u.function_type.params[i]);
      fprintf(f, "%s %s", header->type->u.function_type.params[i]->type->name, header->type->u.function_type.params[i]->name);
      if (i < header->type->u.function_type.param_count-1) fprintf(f, ", ");
   }

   fprintf(f, ")\n");

   glsl_print_statement(f, body, fully_evaluated, indent_depth, suppress_semicolon);
}

static void print_statement_var_def(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   Symbol* var = statement->u.var_decl.var;
   Expr* initializer = statement->u.var_decl.initializer;

   for (unsigned i = 0; i < indent_depth; i++) fprintf(f, "\t");

   glsl_print_qualifiers(f, var);
   fprintf(f, "%s %s", var->type->name, var->name);

   if (initializer)
   {
      fprintf(f, " = ");
      glsl_print_expr(f, initializer, fully_evaluated);
   }

   if (!suppress_semicolon) fprintf(f, ";");
}

static void print_statement_compound(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   unsigned int i;
   StatementChainNode* s;

   for (i = 0; i < indent_depth; i++) fprintf(f, "\t");
   fprintf(f, "{\n");

   for (s = statement->u.compound.statements->first; s; s = s->next)
   {
      glsl_print_statement(f, s->statement, fully_evaluated, indent_depth + 1, suppress_semicolon);
      fprintf(f, "\n");
   }

   for (i = 0; i < indent_depth; i++) fprintf(f, "\t");
   fprintf(f, "}");

#if 0
   // Dumps count.
   fprintf(f, " [count:%d]", statement->compound.statements->count);
#endif
}

static void print_statement_expr(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   for (unsigned i = 0; i < indent_depth; i++) fprintf(f, "\t");
   glsl_print_expr(f, statement->u.expr.expr, fully_evaluated);
   if (!suppress_semicolon) fprintf(f, ";");
}

static void print_statement_selection(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   for (unsigned i = 0; i < indent_depth; i++) fprintf(f, "\t");
   fprintf(f, "if (");
   glsl_print_expr(f, statement->u.selection.cond, fully_evaluated);
   fprintf(f, ")");

   if (STATEMENT_COMPOUND == statement->u.selection.if_true->flavour)
   {
      fprintf(f, "\n");
      glsl_print_statement(f, statement->u.selection.if_true, fully_evaluated, indent_depth, suppress_semicolon);
   }
   else
   {
      fprintf(f, " ");
      glsl_print_statement(f, statement->u.selection.if_true, fully_evaluated, 0, suppress_semicolon);
   }

   if (statement->u.selection.if_false)
   {
      fprintf(f, "\n");
      for (unsigned i = 0; i < indent_depth; i++) fprintf(f, "\t");
      fprintf(f, "else");

      if (STATEMENT_COMPOUND == statement->u.selection.if_true->flavour)
      {
         fprintf(f, "\n");
         glsl_print_statement(f, statement->u.selection.if_false, fully_evaluated, indent_depth, suppress_semicolon);
      }
      else
      {
         fprintf(f, " ");
         glsl_print_statement(f, statement->u.selection.if_false, fully_evaluated, 0, suppress_semicolon);
      }
   }
}

static void print_statement_iterator_for(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   for (unsigned i = 0; i < indent_depth; i++) fprintf(f, "\t");
   fprintf(f, "for (");
   glsl_print_statement(f, statement->u.iterator_for.init, fully_evaluated, 0, true);
   fprintf(f, "; ");
   glsl_print_statement(f, statement->u.iterator_for.cond_or_decl, fully_evaluated, 0, true);
   fprintf(f, "; ");
   glsl_print_expr(f, statement->u.iterator_for.loop, fully_evaluated);
   fprintf(f, ")\n");

   glsl_print_statement(f, statement->u.iterator_for.block, fully_evaluated, indent_depth, suppress_semicolon);
}

static void print_statement_iterator_while(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   for (unsigned i = 0; i < indent_depth; i++) fprintf(f, "\t");
   fprintf(f, "while (");
   glsl_print_statement(f, statement->u.iterator_while.cond_or_decl, fully_evaluated, 0, true);
   fprintf(f, ")\n");

   glsl_print_statement(f, statement->u.iterator_while.block, fully_evaluated, indent_depth, suppress_semicolon);
}

static void print_statement_iterator_do_while(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   unsigned int i;

   for (i = 0; i < indent_depth; i++) fprintf(f, "\t");
   fprintf(f, "do\n");

   glsl_print_statement(f, statement->u.iterator_do_while.block, fully_evaluated, indent_depth, suppress_semicolon);

   for (i = 0; i < indent_depth; i++) fprintf(f, "\t");
   fprintf(f, "while (");
   glsl_print_expr(f, statement->u.iterator_do_while.cond, fully_evaluated);
   fprintf(f, ")");

   if (!suppress_semicolon) fprintf(f, ";");
}

static void print_statement_jump(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   for (unsigned i = 0; i < indent_depth; i++) fprintf(f, "\t");

   switch (statement->flavour)
   {
      case STATEMENT_CONTINUE:
         fprintf(f, "continue");
         break;
      case STATEMENT_BREAK:
         fprintf(f, "break");
         break;
      case STATEMENT_DISCARD:
         fprintf(f, "discard");
         break;
      case STATEMENT_RETURN:
         fprintf(f, "return");
         break;
      case STATEMENT_RETURN_EXPR:
         fprintf(f, "return ");
         glsl_print_expr(f, statement->u.return_expr.expr, fully_evaluated);
         break;
      default:
         unreachable();
         return;
   }

   if (!suppress_semicolon) fprintf(f, ";");
}

static void print_statement_null(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   ((void)statement);
   ((void)fully_evaluated);

   for (unsigned i = 0; i < indent_depth; i++) fprintf(f, "\t");
   if (!suppress_semicolon) fprintf(f, ";");
}

void glsl_print_statement(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   switch (statement->flavour)
   {
      case STATEMENT_AST:
         print_statement_ast(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      case STATEMENT_DECL_LIST:
         print_statement_decl_list(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      case STATEMENT_FUNCTION_DEF:
         print_statement_function_def(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      case STATEMENT_VAR_DECL:
         print_statement_var_def(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      case STATEMENT_COMPOUND:
         print_statement_compound(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      case STATEMENT_EXPR:
         print_statement_expr(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      case STATEMENT_SELECTION:
         print_statement_selection(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      case STATEMENT_ITERATOR_FOR:
         print_statement_iterator_for(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      case STATEMENT_ITERATOR_WHILE:
         print_statement_iterator_while(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      case STATEMENT_ITERATOR_DO_WHILE:
         print_statement_iterator_do_while(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      case STATEMENT_CONTINUE:
      case STATEMENT_BREAK:
      case STATEMENT_DISCARD:
      case STATEMENT_RETURN:
      case STATEMENT_RETURN_EXPR:
         print_statement_jump(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      case STATEMENT_NULL:
         print_statement_null(f, statement, fully_evaluated, indent_depth, suppress_semicolon);
         break;

      default:
         unreachable();
         break;
   }

   // Add statement dump commands here if you need them.
#if 0
   // Dumps flags.
   fprintf(f, " [%1d]", statement->flags);
#endif
}

#else
/*
   keep Metaware happy by providing an exported symbol
*/

void glsl_print_statement(FILE* f, Statement* statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon)
{
   (void)(f);
   (void)(statement);
   (void)(fully_evaluated);
   (void)(indent_depth);
   (void)(suppress_semicolon);

   unreachable();
}
#endif // _DEBUG
