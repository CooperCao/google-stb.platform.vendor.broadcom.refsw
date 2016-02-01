/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_PREPRO_EXPAND_H
#define GLSL_PREPRO_EXPAND_H

#include "prepro/glsl_prepro_token.h"
#include "prepro/glsl_prepro_macro.h"

extern TokenSeq *glsl_expand(TokenSeq *ts, bool recursive);

#endif
