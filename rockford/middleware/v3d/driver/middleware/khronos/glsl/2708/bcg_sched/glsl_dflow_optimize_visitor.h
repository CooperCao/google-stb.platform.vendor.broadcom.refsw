/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#ifndef __GLSL_DFLOW_OPTIMIZE_VISITOR_H__
#define __GLSL_DFLOW_OPTIMIZE_VISITOR_H__

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

static INLINE void DFlowOptimizeVisitor_Destr(DFlowOptimizeVisitor *self)
{
   DFlowVisitor_Destr(self);
}

#endif /* __GLSL_DFLOW_OPTIMIZE_VISITOR_H__ */
