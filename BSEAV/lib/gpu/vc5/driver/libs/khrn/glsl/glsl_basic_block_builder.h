/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_BASICBLOCK_BUILDER_H_INCLUDED
#define GLSL_BASICBLOCK_BUILDER_H_INCLUDED

#include "glsl_nast.h"
#include "glsl_basic_block.h"

BasicBlock *glsl_basic_block_build(NStmtList *nast);

#endif
