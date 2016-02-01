/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_AST_VISITOR_H
#define GLSL_AST_VISITOR_H

#include "middleware/khronos/glsl/glsl_ast.h"


// All visitor functions must implement these signatures.
// data will be passed through the accept functions and back to the visitor.
typedef Expr* (*ExprPreVisitor)(Expr* expr, void* data);
typedef Statement* (*StatementPreVisitor)(Statement* statement, void* data);

// Prefix acceptor functions.
// - calls visitor for the current node as ev(statement, data) or sv(statement, data);
// - recurses on the children of the node *that the visitor passes back*, or not if the visitor passed NULL.
//   (this allows the prefix visitor to mutate the node)
// ev and sv can be NULL, in which case they will not be called.
void glsl_expr_accept_prefix(Expr* expr, void* data, ExprPreVisitor eprev);
void glsl_statement_accept_prefix(Statement* statement, void* data, StatementPreVisitor sprev, ExprPreVisitor eprev);

// All visitor functions must implement these signatures.
// data will be passed through the accept functions and back to the visitor.
typedef void (*ExprPostVisitor)(Expr* expr, void* data);
typedef void (*StatementPostVisitor)(Statement* statement, void* data);

// Postfix acceptor functions.
// - recurses on the children of the node;
// - calls visitor for the current node as ev(statement, data) or sv(statement, data).
// ev and sv can be NULL, in which case they will not be called.
void glsl_expr_accept_postfix(Expr* expr, void* data, ExprPostVisitor epostv);
void glsl_statement_accept_postfix(Statement* statement, void* data, StatementPostVisitor spostv, ExprPostVisitor epostv);

// Prefix AND postfix acceptor functions.
// As per the above.
void glsl_expr_accept(Expr* expr, void* data, ExprPreVisitor eprev, ExprPostVisitor epostv);
void glsl_statement_accept(Statement* statement, void* data, StatementPreVisitor sprev, ExprPreVisitor eprev, StatementPostVisitor spostv, ExprPostVisitor epostv);

#endif // AST_VISITOR_H