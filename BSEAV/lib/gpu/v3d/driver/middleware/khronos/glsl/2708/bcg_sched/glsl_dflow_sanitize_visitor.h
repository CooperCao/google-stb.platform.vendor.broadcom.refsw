/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_forward.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include <stdio.h>

typedef struct DFlowSanitizeVisitor_s
{
   DFlowVisitor           m_base;
   uint32_t                m_pass;
   bool                    m_stuffRemoved;
   NodeVector              m_removeList;
} DFlowSanitizeVisitor;

void DFlowSanitizeVisitor_Constr(DFlowSanitizeVisitor *self, DFlowRecursionOptimizer *opt);
void DFlowSanitizeVisitor_Visit(DFlowSanitizeVisitor *self, DFlowNode *node);

static inline void DFlowSanitizeVisitor_Destr(DFlowSanitizeVisitor *self)
{
   DFlowVisitor_Destr(self);
}
