/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_BUILDERS_H
#define GLSL_BUILDERS_H

#include "glsl_symbols.h"
#include "glsl_symbol_table.h"

// State for the current function declaration/definition.
typedef struct {
   SymbolTable *Params;
   int          ParamCount;
   int          VoidCount;
} FunctionBuilder;

/* Symbol builders. */

// Overwrites set bits in lq_global with lq_local as appropriate.
LayoutQualifier* glsl_create_mixed_lq(const LayoutQualifier* lq_local, const LayoutQualifier* lq_global);

// Readies the function builder for the next function.
void glsl_reinit_function_builder(FunctionBuilder *fb);

// Inserts the given STATEMENT_FUNCTION_DEF into the last function symbol we added to the main symbol table.
void glsl_insert_function_definition(Statement* statement);

// Adds the struct type 'type' to the named symbol table.
void    glsl_commit_struct_type(SymbolTable *table, SymbolType *type);
Symbol *glsl_commit_block_type (SymbolTable *table, SymbolType *type, Qualifiers *quals);
void glsl_commit_anonymous_block_members(SymbolTable *table, Symbol *symbol, MemoryQualifier mq);

void glsl_commit_function_param(FunctionBuilder *fb, const char *name, SymbolType *type, QualList *quals, ExprChain *size);

// Specializes glsl_commit_singleton() for function instances.
// At this point we're only committing a declaration,
// because even if we're parsing a definition, we've only seen the prototype so far.
Symbol *glsl_commit_singleton_function_declaration(SymbolTable *table, const char *name, SymbolType *type, bool definition);
Symbol *glsl_commit_variable_instance(SymbolTable *table, const PrecisionTable *prec, unsigned *atomic_offset,
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
SymbolType *glsl_build_function_type(QualList *return_quals, SymbolType *return_type, FunctionBuilder *args);

// Instantiates all function params in the main symbol table.
void glsl_instantiate_function_params(SymbolTable *table, SymbolType *fun);

#endif // BUILDERS_H
