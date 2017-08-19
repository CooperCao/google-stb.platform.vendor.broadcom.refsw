/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_symbols.h"
#include "glsl_symbol_table.h"
#include "libs/khrn/glxx/glxx_int_config.h"

typedef struct param {
   const char *name;
   SymbolType *type;
   QualList *quals;
   ExprChain *size;
} Param;

typedef struct param_list {
   Param *p;
   struct param_list *next;
} ParamList;

typedef struct {
   int input_size;
   int output_size;
   uint32_t buffer_layout;
   uint32_t uniform_layout;
   unsigned atomic_offset[GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS];
} DeclDefaultState;

/* Symbol builders. */

// Overwrites set bits in lq_global with lq_local as appropriate.
uint32_t glsl_layout_combine_block_bits(uint32_t a, uint32_t b);
LayoutQualifier *glsl_create_mixed_lq(const LayoutQualifier *lq_local, const LayoutQualifier *lq_global);

// Inserts the given STATEMENT_FUNCTION_DEF into the last function symbol we added to the main symbol table.
void glsl_insert_function_definition(Statement *statement);

// Adds the struct type 'type' to the named symbol table.
void    glsl_commit_struct_type(SymbolTable *table, SymbolType *type);
Symbol *glsl_commit_block_type (SymbolTable *table, DeclDefaultState *dflt, SymbolType *type, Qualifiers *quals);
void glsl_commit_anonymous_block_members(SymbolTable *table, Symbol *symbol, MemoryQualifier mq);

// Specializes glsl_commit_singleton() for function instances.
// At this point we're only committing a declaration,
// because even if we're parsing a definition, we've only seen the prototype so far.
Symbol *glsl_commit_singleton_function_declaration(SymbolTable *table, const char *name, SymbolType *type, bool definition, bool user_code);
Symbol *glsl_commit_variable_instance(SymbolTable *table, const PrecisionTable *prec, DeclDefaultState *dflt,
                                      QualList *quals, SymbolType *type, const char *name,
                                      ExprChain *array_size, Expr *initialiser);
Symbol *glsl_commit_var_instance(SymbolTable *table, const char *name, SymbolType *type, Qualifiers *q, const_value *compile_time_value);

// builds an array type that may be an incomplete type (if size==NULL).
SymbolType *glsl_build_array_type(SymbolType *member_type, ExprChain *array_specifier);
// completes an existing array type or does nothing if type already complete.
void glsl_complete_array_type(SymbolType *type, int member_count);
// completes an array type by making it match init_type, or throws an error
void glsl_complete_array_from_init_type(SymbolType *type, SymbolType *init_type);
// Throw a compile error if the type is not complete
void glsl_error_if_type_incomplete(SymbolType *type);

// Aggregates the struct symbol table into a new SymbolType, and then clears it for the next struct.
SymbolType *glsl_build_struct_type(const char *name, StatementChain *chain);
// same for (interface) blocks:
SymbolType *glsl_build_block_type(Qualifiers *quals, const char *name, StatementChain* chain);
// Aggregates the function symbol table into a new SymbolType, and then clears it for the next function.
SymbolType *glsl_build_function_type(QualList *return_quals, SymbolType *return_type, ParamList *params);

// Instantiates all function params in the main symbol table.
void glsl_instantiate_function_params(SymbolTable *table, SymbolType *fun);
