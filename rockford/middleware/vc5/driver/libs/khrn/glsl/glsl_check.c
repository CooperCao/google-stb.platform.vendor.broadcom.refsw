/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
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

bool glsl_check_is_function_overloadable(Symbol *func) {
   if(g_ShaderVersion == GLSL_SHADER_VERSION(1,0,1)) {
      /* You can overload builtins in version 100 */
      return true;
   }

   bool is_builtin = false;
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
   switch(version) {
   case GLSL_SHADER_VERSION(1,0,1):  return GLSL_STDLIB_PROPERTY_VERSION1;
   case GLSL_SHADER_VERSION(3,0,1):  return GLSL_STDLIB_PROPERTY_VERSION3;
   case GLSL_SHADER_VERSION(3,10,1): return GLSL_STDLIB_PROPERTY_VERSION31;
   case GLSL_SHADER_VERSION(3,20,1): return GLSL_STDLIB_PROPERTY_VERSION32;
   default: UNREACHABLE();           return 0;
   }
}

unsigned glsl_check_get_shader_mask(ShaderFlavour flavour) {
   switch(flavour) {
   case SHADER_VERTEX:          return GLSL_STDLIB_PROPERTY_VERTEX;
   case SHADER_TESS_CONTROL:    return GLSL_STDLIB_PROPERTY_TESS_C;
   case SHADER_TESS_EVALUATION: return GLSL_STDLIB_PROPERTY_TESS_E;
   case SHADER_GEOMETRY:        return GLSL_STDLIB_PROPERTY_GEOMETRY;
   case SHADER_FRAGMENT:        return GLSL_STDLIB_PROPERTY_FRAGMENT;
   case SHADER_COMPUTE:         return GLSL_STDLIB_PROPERTY_COMPUTE;
   default: UNREACHABLE();      return 0;
   }
}
