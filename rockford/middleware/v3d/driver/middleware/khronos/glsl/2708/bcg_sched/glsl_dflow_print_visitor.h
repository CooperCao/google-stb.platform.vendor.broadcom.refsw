/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#ifndef __GLSL_DFLOW_PRINT_VISITOR_H__
#define __GLSL_DFLOW_PRINT_VISITOR_H__

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include <stdio.h>

typedef struct DFlowPrintVisitor_s
{
   DFlowVisitor   m_base;
   FILE          *m_fp;
} DFlowPrintVisitor;

typedef enum
{
   DFlowPrintVisitor_EDGE_SOLID,
   DFlowPrintVisitor_EDGE_DASHED,
   DFlowPrintVisitor_EDGE_SEQUENCE,
   DFlowPrintVisitor_EDGE_STYLE_COUNT,
   DFlowPrintVisitor_EDGE_PARENT
} DFlowPrintVisitor_EdgeStyle;

void DFlowPrintVisitor_Constr(DFlowPrintVisitor *self, DFlowRecursionOptimizer *opt, const char *filename);
void DFlowPrintVisitor_Destr(DFlowPrintVisitor *self);
void DFlowPrintVisitor_Visit(DFlowPrintVisitor *self, DFlowNode *node);

#endif /* __GLSL_DFLOW_PRINT_VISITOR_H__ */
