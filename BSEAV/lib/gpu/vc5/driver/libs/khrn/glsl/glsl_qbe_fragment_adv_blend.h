/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_backflow.h"

void adv_blend_blend(Backflow *res[4], Backflow *src[4], Backflow *dst[4], uint32_t mode);
void adv_blend_read_tlb(Backflow *res[4][4], const FragmentBackendState *s, SchedBlock *block, int rt);
