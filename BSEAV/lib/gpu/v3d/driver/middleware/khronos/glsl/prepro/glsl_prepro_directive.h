/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_PREPRO_DIRECTIVE_H
#define GLSL_PREPRO_DIRECTIVE_H

#include "middleware/khronos/glsl/prepro/glsl_prepro_macro.h"

extern MacroList *directive_macros;

extern void glsl_directive_allow_version(void);
extern void glsl_directive_allow_extension(void);
extern void glsl_directive_reset_macros(void);

extern Token *glsl_directive_next_token(void);

#endif
