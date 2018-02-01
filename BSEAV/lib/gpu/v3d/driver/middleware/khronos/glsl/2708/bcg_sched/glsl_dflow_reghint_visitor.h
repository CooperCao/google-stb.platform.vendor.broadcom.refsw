/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/glsl_common.h"

typedef struct DFlowRegHintVisitor_s
{
   DFlowVisitor  m_base;
} DFlowRegHintVisitor;

void DFlowRegHintVisitor_Constr(DFlowRegHintVisitor *self, DFlowRecursionOptimizer *opt);

static inline void DFlowRegHintVisitor_Destr(DFlowRegHintVisitor *self)
{
   DFlowVisitor_Destr(self);
}

void DFlowRegHintVisitor_Visit(DFlowRegHintVisitor *self, DFlowNode *node);
