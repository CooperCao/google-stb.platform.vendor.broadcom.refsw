/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/
#ifndef __GLSL_DFLOW_REGHINT_VISITOR_H__
#define __GLSL_DFLOW_REGHINT_VISITOR_H__

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/glsl_common.h"

typedef struct DFlowRegHintVisitor_s
{
   DFlowVisitor  m_base;
} DFlowRegHintVisitor;

void DFlowRegHintVisitor_Constr(DFlowRegHintVisitor *self, DFlowRecursionOptimizer *opt);

static INLINE void DFlowRegHintVisitor_Destr(DFlowRegHintVisitor *self)
{
   DFlowVisitor_Destr(self);
}

void DFlowRegHintVisitor_Visit(DFlowRegHintVisitor *self, DFlowNode *node);

#endif // __GLSL_DFLOW_REGHINT_VISITOR_H__
