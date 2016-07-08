/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_CHECK_H_INCLUDED
#define GLSL_CHECK_H_INCLUDED

#include "glsl_compiler.h"
#include "glsl_symbols.h"

bool     glsl_check_is_function_overloadable     (Symbol* func);
unsigned glsl_check_get_version_mask             (int version);
unsigned glsl_check_get_shader_mask              (ShaderFlavour flavour);

#endif
