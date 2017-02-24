/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "prepro/glsl_prepro_token.h"

/* Evaluate sequence s and return the value. If any tokens remain, rem points to them */
extern int glsl_eval_evaluate(TokenSeq *s, TokenSeq **rem);
