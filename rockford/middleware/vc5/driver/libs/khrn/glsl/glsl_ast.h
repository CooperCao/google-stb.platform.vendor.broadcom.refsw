/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_AST_H
#define GLSL_AST_H

#include "glsl_common.h"
#include "glsl_symbols.h"
#include "glsl_const_types.h"

//
// Forward declarations.
//

#define MAX_SWIZZLE_FIELD_COUNT 4
#define SWIZZLE_FIELD_XYZW_X 0
#define SWIZZLE_FIELD_XYZW_Y 1
#define SWIZZLE_FIELD_XYZW_Z 2
#define SWIZZLE_FIELD_XYZW_W 3
#define SWIZZLE_FIELD_RGBA_R 0
#define SWIZZLE_FIELD_RGBA_G 1
#define SWIZZLE_FIELD_RGBA_B 2
#define SWIZZLE_FIELD_RGBA_A 3
#define SWIZZLE_FIELD_STPQ_S 0
#define SWIZZLE_FIELD_STPQ_T 1
#define SWIZZLE_FIELD_STPQ_P 2
#define SWIZZLE_FIELD_STPQ_Q 3
#define SWIZZLE_SLOT_UNUSED 255

typedef char SWIZZLE_FIELD_FLAGS_T; // Bitmask. Must hold MAX_SWIZZLE_FIELD_COUNT flags

//
// Expressions.
//

/*
   _ExprChain

   Expression chain structure, for efficient appending.

   Represents a list {a[0], a[1], ..., a[count-1]} of pointers to Expr.
*/

struct _ExprChain
{
   /* If count == 0 then first,last == NULL, otherwise they are valid */
   ExprChainNode *first;
   ExprChainNode *last;

   uint32_t count;   /* The number of elements in the list */
};

struct _ExprChainNode
{
   Expr *expr;    /* The actual Expr. Never NULL */
   ExprChainNode *next;
   ExprChainNode *prev;
};

ExprChain *glsl_expr_chain_create(void);
ExprChain *glsl_expr_chain_append(ExprChain *chain, Expr *expr);
ExprChain *glsl_expr_chain_append_node(ExprChain *chain, ExprChainNode *node);

typedef enum
{
   PRIM_CONS_INVALID,
   PRIM_CONS_VECTOR_FROM_SCALAR,
   PRIM_CONS_MATRIX_FROM_SCALAR,
   PRIM_CONS_MATRIX_FROM_MATRIX,
   PRIM_CONS_FROM_COMPONENT_FLOW
} PrimConsFlavour;

struct _Expr
{
   /*
      flavour

      Explains how this expression was constructed syntactically.

      We extend the language to add intrinsics.

      TODO: we construct x *= y as x = x * y but this is wrong. The following don't exist:
         EXPR_MUL_ASSIGN,
         EXPR_DIV_ASSIGN,
         EXPR_ADD_ASSIGN,
         EXPR_SUB_ASSIGN,
   */
   ExprFlavour flavour;
   int         line_num;     /* The source line on which the expression appeared. */
   SymbolType *type;

   /* A pointer to the value of this expression, or NULL if the value is not
    * known at compile time.  The value is type->scalar_count const_values.
    * NOTE: For params this will be NULL even if they have the const qualifier,
    * because const params are readonly, but not compile time constants.  */
   const_value *compile_time_value;

   union
   {
      // EXPR_VALUE: no metadata because always a compile_time_value

      // EXPR_INSTANCE
      struct {
         /*
            symbol->flavour in {SYMBOL_VAR_INSTANCE,SYMBOL_PARAM_INSTANCE}
            symbol->type is a shallow_match for type
         */
         Symbol *symbol;         /* The variable of which this is an instance */
      } instance;

      // EXPR_SUBSCRIPT
      struct {
         Expr *aggregate;        /* The 'a' in a[i] */
         Expr *subscript;        /* The 'i' in a[i] */
      } subscript;

      // EXPR_FUNCTION_CALL
      struct {
         /* function->flavour == SYMBOL_FUNCTION_TYPE */
         Symbol    *function;
         ExprChain *args;        /* In order, i.e. leftmost first */
      } function_call;

      // EXPR_PRIM_CONSTRUCTOR_CALL
      struct {
         PrimConsFlavour flavour;   /* Type of constructor. For type see expr->type */
         ExprChain      *args;      /* In order, i.e. leftmost first */
      } prim_constructor_call;

      // EXPR_COMPOUND_CONSTRUCTOR_CALL
      struct {
         ExprChain *args;        /* In order, i.e. leftmost first */
      } compound_constructor_call;

      // EXPR_FIELD_SELECTOR
      struct {
         Expr       *aggregate;  /* The 'a' in a.f */
         const char *field;      /* The 'f' in a.f */
         int         field_no;
      } field_selector;

      // EXPR_SWIZZLE
      struct {
         Expr       *aggregate;        /* The 'a' in a.xxz */
         const char *field;            /* The text 'xxz' for a.xxz */
         /*
            Each slot contains the field number (0 <= n < MAX_SWIZZLE_FIELD_COUNT) that it is selecting,
            or SWIZZLE_SLOT_UNUSED.
            e.g. .xxz would be represented as { 0, 0, 2, SWIZZLE_SLOT_UNUSED }.
         */
         unsigned char swizzle_slots[MAX_SWIZZLE_FIELD_COUNT];
      } swizzle;

      // EXPR_POST_INC, EXPR_POST_DEC, EXPR_PRE_INC, EXPR_PRE_DEC, EXPR_ARITH_NEGATE, EXPR_LOGICAL_NOT
      struct {
         Expr *operand;
      } unary_op;

      // EXPR_MUL, EXPR_DIV, EXPR_ADD, EXPR_SUB
      // EXPR_LESS_THAN, EXPR_LESS_THAN_EQUAL, EXPR_GREATER_THAN, EXPR_GREATER_THAN_EQUAL
      // EXPR_EQUAL, EXPR_NOT_EQUAL
      // EXPR_LOGICAL_AND, EXPR_LOGICAL_XOR, EXPR_LOGICAL_OR
      struct {
         Expr *left;
         Expr *right;
      } binary_op;

      // EXPR_CONDITIONAL
      struct {
         Expr *cond;
         Expr *if_true;
         Expr *if_false;
      } cond_op;

      // EXPR_ASSIGN, EXPR_MUL_ASSIGN, EXPR_DIV_ASSIGN, EXPR_ADD_ASSIGN, EXPR_SUB_ASSIGN
      struct {
         Expr *lvalue;
         Expr *rvalue;
      } assign_op;

      // EXPR_SEQUENCE
      struct {
         Expr *all_these;
         Expr *then_this;
      } sequence;

      // EXPR_ARRAY_LENGTH
      struct {
         Expr *array;
      } array_length;

      // EXPR_INTRINSIC
      struct {
         glsl_intrinsic_index_t flavour;
         ExprChain             *args; // in order, i.e. leftmost first
      } intrinsic;
   } u;
};

bool glsl_is_lvalue(Expr *expr);
MemoryQualifier glsl_get_mem_flags(Expr *expr);

// On failure, these functions call glsl_compile_error() and return NULL.
Expr *glsl_expr_construct_const_value(PrimitiveTypeIndex type_index, const_value v);
Expr *glsl_expr_construct_instance(Symbol *symbol);
Expr *glsl_expr_construct_subscript(Expr* aggregate, Expr* subscript);
Expr *glsl_expr_construct_function_call(Symbol* overload_chain, ExprChain* args);
Expr *glsl_expr_construct_method_call(Expr* aggregate, CallContext* function_ctx);
Expr *glsl_expr_construct_constructor_call(SymbolType* type, ExprChain* args);
Expr *glsl_expr_construct_field_selector(Expr* aggregate, const char* field);
Expr *glsl_expr_construct_unary_op(ExprFlavour flavour, Expr* operand);
Expr *glsl_expr_construct_binary_op_arithmetic(ExprFlavour flavour, Expr* left, Expr* right);
Expr *glsl_expr_construct_binary_op_logical(ExprFlavour flavour, Expr* left, Expr* right);
Expr *glsl_expr_construct_binary_op_shift(ExprFlavour flavour, Expr* left, Expr* right);
Expr *glsl_expr_construct_binary_op_bitwise(ExprFlavour flavour, Expr* left, Expr* right);
Expr *glsl_expr_construct_binary_op_relational(ExprFlavour flavour, Expr* left, Expr* right);
Expr *glsl_expr_construct_binary_op_equality(ExprFlavour flavour, Expr* left, Expr* right);
Expr *glsl_expr_construct_cond_op(Expr *cond, Expr *if_true, Expr *if_false);
Expr *glsl_expr_construct_assign_op(Expr *lvalue, Expr *rvalue);
Expr *glsl_expr_construct_sequence(Expr *all_these, Expr *then_this);
Expr *glsl_expr_construct_array_length(Expr *array);
Expr *glsl_expr_construct_intrinsic(ExprFlavour flavour, ExprChain *args);


//
// Statements.
//

// Statement chain structure, for efficient appending.
struct _StatementChain
{
   StatementChainNode *first;    /* NULL if count == 0 */
   StatementChainNode *last;     /* NULL if count == 0 */

   int count;
};

struct _StatementChainNode
{
   Statement *statement;

   StatementChainNode *prev;
   StatementChainNode *next;
};

StatementChain *glsl_statement_chain_create(void);
StatementChain *glsl_statement_chain_append(StatementChain *chain, Statement *statement);

struct _Statement
{
   StatementFlavour flavour;  /* Type of statement. Somewhat documented below */

   int line_num;  /* The source line number on which this statement appeared. */

   union
   {
      // STATEMENT_AST
      // - The root of the AST, containing STATEMENT_FUNCTION_DEF and STATEMENT_DECL_LIST.
      // - However, after loop transforms, when STATEMENT_DECL_LIST are no longer necessary,
      //   compiler-generated variables will go straight into STATEMENT_VAR_DECL instead.
      struct
      {
         StatementChain *decls;
      } ast;

      // STATEMENT_DECL_LIST
      // - Contains a list of STATEMENT_VAR_DECL of the same type.
      // - Needed in loop statements where more than one STATEMENT_VAR_DECL is possible.
      struct
      {
         StatementChain *decls; // possibly containing 0 declarations
      } decl_list;

      // STATEMENT_FUNCTION_DEF
      struct
      {
         Symbol *header;
         Statement *body; // as STATEMENT_COMPOUND
      } function_def;

      // STATEMENT_VAR_DECL
      // - When contructed during parsing, these will always be inside STATEMENT_DECL_LIST.
      // - However, after loop transforms, when STATEMENT_DECL_LIST are no longer necessary,
      //   a later transformation will bring them out to statement context level.
      //   (i.e. splicing the list of STATEMENT_VAR_DECL into the place of the STATEMENT_DECL_LIST)
      struct
      {
         // base_type is the type which appears at the start of the declaration,
         // it differs from var->type only if the declarator specifies an array)
         QualList      *quals;
         SymbolType    *base_type;
         Symbol        *var;
         Expr          *initializer; // or NULL if no initializer
      } var_decl;

      struct
      {
         SymbolType     *type;
         QualList       *quals;
         StatementChain *members;
      } struct_decl;

      struct
      {
         const char *name;
         ExprChain  *array_specifier;
      } struct_member_decl;

      // STATEMENT_COMPOUND
      struct
      {
         StatementChain *statements; // possibly containing 0 statements
      } compound;

      // STATEMENT_EXPR
      struct
      {
         Expr *expr;
      } expr;

      // STATEMENT_SELECTION
      struct
      {
         Expr *cond;
         Statement *if_true; // *always* constructed as a STATEMENT_COMPOUND
         Statement *if_false; // *always* constructed as a STATEMENT_COMPOUND, or NULL if none
      } selection;

      // STATEMENT_SWITCH
      struct
      {
         Expr *cond;
         StatementChain *stmtChain;
      } switch_stmt;

      // STATEMENT_CASE
      struct
      {
         Expr *expr;
      } case_stmt;

      // STATEMENT_ITERATOR_FOR
      struct
      {
         Statement *init;
         Statement *cond_or_decl;
         Expr      *loop;          /* NULL if none provided */
         Statement *block; // *always* constructed as a STATEMENT_COMPOUND
      } iterator_for;

      // STATEMENT_ITERATOR_WHILE
      struct
      {
         Statement *cond_or_decl;
         Statement *block; // *always* constructed as a STATEMENT_COMPOUND
      } iterator_while;

      // STATEMENT_ITERATOR_DO_WHILE
      struct
      {
         Statement *block; // *always* constructed as a STATEMENT_COMPOUND
         Expr      *cond;
      } iterator_do_while;

      // STATEMENT_CONTINUE, STATEMENT_BREAK, STATEMENT_DISCARD, STATEMENT_RETURN: no metadata

      // STATEMENT_RETURN_EXPR
      struct {
         Expr *expr;
      } return_expr;

      // STATEMENT_PRECISION
      struct {
         PrecisionQualifier  prec;
         SymbolType         *type;
      } precision;

      // STATEMENT_QUALIFIER_DEFUALT
      struct {
         QualList *quals;
      } qualifier_default;

      // STATEMENT_QUALIFIER_AUGMENT
      struct {
         QualList   *quals;
         SymbolList *vars;
      } qualifier_augment;
   } u;
};


// On failure, these functions call glsl_compile_error() and return NULL.
Statement *glsl_statement_construct(StatementFlavour flavour);
Statement *glsl_statement_construct_ast(StatementChain* decls);
Statement *glsl_statement_construct_decl_list(StatementChain* decls);
Statement *glsl_statement_construct_function_def(Symbol* header, Statement* body);
Statement *glsl_statement_construct_var_decl(QualList *quals, SymbolType *type, Symbol* var, Expr* initializer);
Statement *glsl_statement_construct_compound(StatementChain* statements);
Statement *glsl_statement_construct_expr(Expr* expr);
Statement *glsl_statement_construct_selection(Expr* cond, Statement* if_true, Statement* if_false);
Statement *glsl_statement_construct_iterator_for(Statement *init, Statement *cond_or_decl, Expr *loop, Statement *block);
Statement *glsl_statement_construct_iterator_while(Statement* cond_or_decl, Statement* block);
Statement *glsl_statement_construct_iterator_do_while(Statement* block, Expr* cond);
Statement *glsl_statement_construct_return_expr(Expr* expr);
Statement *glsl_statement_construct_switch(Expr* cond, StatementChain* chain);
Statement *glsl_statement_construct_case(Expr* expr);
Statement *glsl_statement_construct_struct_decl(SymbolType *type, QualList *quals, StatementChain *members);
Statement *glsl_statement_construct_struct_member_decl(const char *name, ExprChain *size);
Statement *glsl_statement_construct_precision(PrecisionQualifier prec, SymbolType *type);
Statement *glsl_statement_construct_qualifier_default(QualList *quals);
Statement *glsl_statement_construct_qualifier_augment(QualList *quals, SymbolList *vars);

void glsl_ast_validate(Statement *ast, ShaderFlavour flavour, int version);

#endif // AST_H
