/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_PREPRO_EXPAND_H
#define GLSL_PREPRO_EXPAND_H

#include "middleware/khronos/glsl/prepro/glsl_prepro_token.h"
#include "middleware/khronos/glsl/prepro/glsl_prepro_macro.h"

extern TokenSeq *glsl_expand(TokenSeq *ts, bool recursive);

#endif
