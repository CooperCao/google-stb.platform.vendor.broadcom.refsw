/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#ifndef __GLSL_DFLOW_VALIDATE_VISITOR_H__
#define __GLSL_DFLOW_VALIDATE_VISITOR_H__

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

static INLINE void DFlowValidateVisitor_Destr(DFlowValidateVisitor *self)
{
   DFlowVisitor_Destr(self);
}



#endif /* __GLSL_DFLOW_VALIDATE_VISITOR_H__ */
