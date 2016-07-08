/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_PREPRO_DIRECTIVE_H
#define GLSL_PREPRO_DIRECTIVE_H

#include "prepro/glsl_prepro_macro.h"
#include "glsl_prepro_token.h"

extern MacroList *directive_macros;

extern void      glsl_directive_reset_macros(void);
extern TokenSeq *glsl_directive_next_token  (void);
#endif
