/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_SSA_CONVERT_H__
#define GLSL_SSA_CONVERT_H__

#include "glsl_map.h"
#include "glsl_ir_shader.h"
#include "glsl_symbols.h"
#include "glsl_basic_block.h"

VCOS_EXTERN_C_BEGIN

void glsl_ssa_convert(SSAShader *sh, BasicBlock *entry_block, const SymbolList *outs, Map *symbol_ids);

VCOS_EXTERN_C_END

#endif
