/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include <stdio.h>

typedef struct DFlowDepthVisitor_s
{
   DFlowVisitor  m_base;
} DFlowDepthVisitor;

void DFlowDepthVisitor_Constr(DFlowDepthVisitor *self, DFlowRecursionOptimizer *opt);

static inline void DFlowDepthVisitor_Destr(DFlowDepthVisitor *self)
{
   DFlowVisitor_Destr(self);
}

void DFlowDepthVisitor_Visit(DFlowDepthVisitor *self, DFlowNode *node);
