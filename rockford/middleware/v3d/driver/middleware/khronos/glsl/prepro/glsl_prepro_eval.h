/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_PREPRO_EVAL_H
#define GLSL_PREPRO_EVAL_H

#include "middleware/khronos/glsl/prepro/glsl_prepro_token.h"
#include "middleware/khronos/glsl/prepro/glsl_prepro_macro.h"

extern void glsl_eval_set_sequence(TokenSeq *_seq);
extern bool glsl_eval_has_sequence(void);
extern int glsl_eval_evaluate(void);

#endif
