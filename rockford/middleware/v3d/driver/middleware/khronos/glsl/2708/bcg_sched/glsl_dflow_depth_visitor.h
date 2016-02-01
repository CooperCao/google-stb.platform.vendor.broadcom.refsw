/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#ifndef __GLSL_DFLOW_DEPTH_VISITOR_H__
#define __GLSL_DFLOW_DEPTH_VISITOR_H__

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include <stdio.h>

typedef struct DFlowDepthVisitor_s
{
   DFlowVisitor  m_base;
} DFlowDepthVisitor;

void DFlowDepthVisitor_Constr(DFlowDepthVisitor *self, DFlowRecursionOptimizer *opt);

static INLINE void DFlowDepthVisitor_Destr(DFlowDepthVisitor *self)
{
   DFlowVisitor_Destr(self);
}

void DFlowDepthVisitor_Visit(DFlowDepthVisitor *self, DFlowNode *node);

#endif /* __GLSL_DFLOW_DEPTH_VISITOR_H__ */
