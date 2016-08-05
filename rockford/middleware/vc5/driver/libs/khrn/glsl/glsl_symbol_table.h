/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_SYMBOL_TABLE_H
#define GLSL_SYMBOL_TABLE_H

#include "glsl_common.h"
#include "glsl_compiler.h"
#include "glsl_scoped_map.h"
#include "glsl_shader_interfaces.h"

typedef struct _SymbolTable {
   ScopedMap* map;
} SymbolTable;

// Creates a new symbol table.
SymbolTable* glsl_symbol_table_new(void);

// Populates table with built in symbols.
SymbolTable *glsl_symbol_table_populate(SymbolTable *table, ShaderFlavour flavour, int version);

// Populate with some more symbols once we know the shader language version
void glsl_symbol_table_populate_version(SymbolTable* table);

// Enters a new scope.
void glsl_symbol_table_enter_scope(SymbolTable* table);

// Exits a scope, clearing everything encountered in this scope so far.
void glsl_symbol_table_exit_scope(SymbolTable* table);

// Looks up a symbol by name in the whole table.
Symbol* glsl_symbol_table_lookup(SymbolTable* table, const char* name);

// Looks up a symbol by name in the most recent scope.
Symbol* glsl_symbol_table_lookup_in_current_scope(SymbolTable* table, const char* name);

// Inserts a symbol into the table.
// Fails if the name is NULL.
void glsl_symbol_table_insert(SymbolTable* table, Symbol* symbol);

// debug helper function
void glsl_symbol_table_print(SymbolTable* table);

#endif
