/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_common.h"

#include "glsl_check.h"
#include "glsl_globals.h"
#include "glsl_stdlib.auto.h"
#include "glsl_symbols.h"

bool glsl_check_is_function_overloadable(Symbol* func) {
   bool is_builtin;

   if(g_ShaderVersion == GLSL_SHADER_VERSION(1,0,1)) {
      /* You can overload builtins in version 100 */
      return true;
   }

   is_builtin = false;
   for ( ; func; func = func->u.function_instance.next_overload) {
      if(glsl_stdlib_is_stdlib_symbol(func)) {
         is_builtin = true;
         break;
      }
   }

   if(!is_builtin) {
      return true;
   }
   if(!glsl_parsing_user_code()) {
      return true;
   }
   return false;
}

unsigned glsl_check_get_version_mask(int version) {
   unsigned mask;
   mask = 0;
   switch(version) {
   case GLSL_SHADER_VERSION(1,0,1):
      mask |= (GLSL_STDLIB_PROPERTY_VERSION3_ONLY | GLSL_STDLIB_PROPERTY_VERSION31_ONLY);
      break;
   case GLSL_SHADER_VERSION(3,0,1):
      mask |= (GLSL_STDLIB_PROPERTY_VERSION1_ONLY | GLSL_STDLIB_PROPERTY_VERSION31_ONLY);
      break;
   case GLSL_SHADER_VERSION(3,20,1):      /* Just use 310-stdlib for 320. TODO: Fix */
   case GLSL_SHADER_VERSION(3,10,1):
      mask |= GLSL_STDLIB_PROPERTY_VERSION1_ONLY;
      break;
   default:
      UNREACHABLE();
   }
   return mask;
}

unsigned glsl_check_get_shader_mask(ShaderFlavour flavour) {
   int mask;
   mask = 0;
   switch(flavour) {
   case SHADER_VERTEX:
      mask |= GLSL_STDLIB_PROPERTY_FRAGMENT_ONLY | GLSL_STDLIB_PROPERTY_COMPUTE_ONLY;
      break;
   case SHADER_FRAGMENT:
      mask |= GLSL_STDLIB_PROPERTY_VERTEX_ONLY | GLSL_STDLIB_PROPERTY_COMPUTE_ONLY;
      break;
   case SHADER_COMPUTE:
      mask |= GLSL_STDLIB_PROPERTY_VERTEX_ONLY | GLSL_STDLIB_PROPERTY_FRAGMENT_ONLY;
      break;
   default:
      UNREACHABLE();
   }
   return mask;
}

/* Determine whether the given variable may be declared invariant. Outputs can
   always be invariant. Additionally fragment shader inputs may be invariant in
   language version 100.  */
bool glsl_check_is_invariant_decl_valid(Symbol *var) {
   if (var->flavour != SYMBOL_VAR_INSTANCE) return false;

   StorageQualifier sq = var->u.var_instance.storage_qual;
   if (sq == STORAGE_OUT) return true;
   if (g_ShaderVersion == GLSL_SHADER_VERSION(1, 0, 1) &&
       sq == STORAGE_IN && g_ShaderFlavour == SHADER_FRAGMENT) return true;

   return false;
}
