/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_common.h"
#include "glsl_symbol_table.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "glsl_check.h"
#include "glsl_map.h"
#include "glsl_scoped_map.h"
#include "glsl_symbols.h"
#include "glsl_ast.h"
#include "glsl_errors.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"
#include "glsl_const_types.h"
#include "glsl_fastmem.h"
#include "glsl_intern.h"
#include "glsl_dataflow.h"
#include "glsl_shader_interfaces.h"
#include "glsl_extensions.h"

SymbolTable *glsl_symbol_table_new(void) {
   SymbolTable *table = malloc_fast(sizeof(SymbolTable));
   table->map = glsl_scoped_map_new();
   glsl_symbol_table_enter_scope(table);
   return table;
}

SymbolTable *glsl_symbol_table_populate(SymbolTable *table, ShaderFlavour flavour, int version)
{
   int active_mask = glsl_check_get_version_mask(version) |
                     glsl_check_get_shader_mask (flavour);

   active_mask |= glsl_ext_get_symbol_mask();

   // Put types in the outmost scope. Must be called before other stdlib functions.
   glsl_stdlib_populate_symbol_table_with_types(table, active_mask);

   // Put const functions in the outermost scope.
   glsl_stdlib_populate_symbol_table_with_functions(table, active_mask);

   // Create the global scope.
   glsl_symbol_table_enter_scope(table);

   // Put built in vars in the global scope.
   glsl_stdlib_populate_symbol_table_with_variables(table, active_mask);

   return table;
}

void glsl_symbol_table_enter_scope(SymbolTable *table)
{
   glsl_scoped_map_push_scope(table->map);
}

void glsl_symbol_table_exit_scope(SymbolTable *table)
{
   glsl_scoped_map_pop_scope(table->map);
}

Symbol *glsl_symbol_table_lookup(SymbolTable *table, const char *name)
{
   return glsl_scoped_map_get(table->map, name);
}

Symbol *glsl_symbol_table_lookup_in_current_scope(SymbolTable *table, const char *name)
{
   return glsl_map_get(table->map->scopes->map, name);
}

void glsl_symbol_table_insert(SymbolTable *table, Symbol *symbol)
{
   glsl_scoped_map_put(table->map, symbol->name, symbol);
}

void glsl_symbol_print(Symbol *s) {
   switch(s->flavour) {
   case SYMBOL_TYPE:              printf("type ");            break;
   case SYMBOL_INTERFACE_BLOCK:   printf("interface block "); break;
   case SYMBOL_VAR_INSTANCE:      printf("var ");             break;
   case SYMBOL_PARAM_INSTANCE:    printf("param ");           break;
   case SYMBOL_FUNCTION_INSTANCE: printf("function ");        break;
   case SYMBOL_TEMPORARY:         printf("temporary ");       break;
   }
   printf("%s\n", s->name);
}

void glsl_symbol_table_print(SymbolTable *table)
{
   int indent = 0;
   for (ScopeList *scope = table->map->scopes; scope; scope = scope->next, indent++) {
      for (MapNode *n = scope->map->head; n; n = n->next) {
         int i;
         for (i=0; i<indent; i++)
            printf("   ");
         glsl_symbol_print(n->v);
      }
   }
}
