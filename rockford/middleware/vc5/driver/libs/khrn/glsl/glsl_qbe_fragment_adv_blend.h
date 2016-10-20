/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_QBE_FRAGMENT_ADV_BLEND_H__
#define GLSL_QBE_FRAGMENT_ADV_BLEND_H__

#include "glsl_backflow.h"

void adv_blend_blend(Backflow *res[4], Backflow *src[4], Backflow *dst[4], uint32_t mode);
void adv_blend_read_tlb(Backflow *res[4][4], const FragmentBackendState *s, SchedBlock *block, int rt);


#endif
