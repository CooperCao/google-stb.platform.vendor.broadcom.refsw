/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "prepro/glsl_prepro_token.h"
#include "prepro/glsl_prepro_macro.h"

extern TokenSeq *glsl_expand(TokenSeq *ts, bool recursive);
