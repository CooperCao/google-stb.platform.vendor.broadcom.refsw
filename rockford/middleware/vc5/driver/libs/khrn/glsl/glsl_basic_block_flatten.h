/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_BASICBLOCK_FLATTEN_H_INCLUDED
#define GLSL_BASICBLOCK_FLATTEN_H_INCLUDED

#include "glsl_basic_block.h"

bool glsl_basic_block_flatten_a_bit(BasicBlock *entry, Map *block_age_offsets);

// converts complex control flow graph into as few basic basics as possible
// using loop unrolling and conditional execution (i.e. guards).
BasicBlock *glsl_basic_block_flatten(BasicBlock *entry, Map *block_age_offsets);

#endif
