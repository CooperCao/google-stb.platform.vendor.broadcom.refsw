/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#ifndef __GLSL_DFLOW_SANITIZE_VISITOR_H__
#define __GLSL_DFLOW_SANITIZE_VISITOR_H__

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

static INLINE void DFlowSanitizeVisitor_Destr(DFlowSanitizeVisitor *self)
{
   DFlowVisitor_Destr(self);
}

#endif /* __GLSL_DFLOW_SANITIZE_VISITOR_H__ */
