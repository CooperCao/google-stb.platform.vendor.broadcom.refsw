/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "prepro/glsl_prepro_macro.h"
#include "glsl_prepro_token.h"

extern MacroList *directive_macros;

extern void   glsl_directive_reset_macros(void);
extern Token *glsl_directive_next_token  (void);
