/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_containers.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include <stdio.h>

typedef struct DFlowFlattenVisitor_s
{
   DFlowVisitor   m_base;
   NodeList       *m_nodes;
} DFlowFlattenVisitor;

void DFlowFlattenVisitor_Constr(DFlowFlattenVisitor *self, DFlowRecursionOptimizer *opt, NodeList *nodes);

static inline void DFlowFlattenVisitor_Destr(DFlowFlattenVisitor *self)
{
   DFlowVisitor_Destr(self);
}

void DFlowFlattenVisitor_Visit(DFlowFlattenVisitor *self, DFlowNode *node);
