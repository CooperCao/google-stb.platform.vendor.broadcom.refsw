/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include <stdio.h>

typedef struct DFlowOptimizeVisitor_s
{
   DFlowVisitor  m_base;
   uint32_t       m_pass;
   NodeSet        m_removeList;
} DFlowOptimizeVisitor;

void DFlowOptimizeVisitor_Constr(DFlowOptimizeVisitor *self, DFlowRecursionOptimizer *opt);
void DFlowOptimizeVisitor_Visit(DFlowOptimizeVisitor *self, DFlowNode *node);

static inline void DFlowOptimizeVisitor_Destr(DFlowOptimizeVisitor *self)
{
   DFlowVisitor_Destr(self);
}
