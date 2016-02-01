/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  prepro
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_PREPRO_EVAL_H
#define GLSL_PREPRO_EVAL_H

#include "prepro/glsl_prepro_token.h"

/* Evaluate sequence s and return the value. If any tokens remain, rem points to them */
extern int glsl_eval_evaluate(TokenSeq *s, TokenSeq **rem);

#endif
