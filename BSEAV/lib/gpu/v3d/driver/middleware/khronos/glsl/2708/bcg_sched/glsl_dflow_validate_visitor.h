/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_forward.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include <stdio.h>

typedef struct DFlowValidateVisitor_s
{
   DFlowVisitor  m_base;
} DFlowValidateVisitor;

void DFlowValidateVisitor_Constr(DFlowValidateVisitor *self, DFlowRecursionOptimizer *opt);
void DFlowValidateVisitor_Visit(DFlowValidateVisitor *self, DFlowNode *node);

static inline void DFlowValidateVisitor_Destr(DFlowValidateVisitor *self)
{
   DFlowVisitor_Destr(self);
}
