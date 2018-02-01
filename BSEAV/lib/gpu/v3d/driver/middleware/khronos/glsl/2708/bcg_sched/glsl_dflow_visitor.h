/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_containers.h"

///////////////////////////////////////////////////////////////////////////////
// DFlowRecursionOptimizer
///////////////////////////////////////////////////////////////////////////////
typedef struct DFlowRecursionOptimizer_s
{
   NodeVector  m_topDown;
   NodeVector  m_bottomUp;
   NodeVector  m_breadthFirst;

   // Private
   DFlowNode   *m_root;
} DFlowRecursionOptimizer;

void DFlowRecursionOptimizer_Constr(DFlowRecursionOptimizer *self);
void DFlowRecursionOptimizer_Destr(DFlowRecursionOptimizer *self);

static inline DFlowNode *DFlowRecursionOptimizer_Root(const DFlowRecursionOptimizer *self)
{
   return self->m_root;
}

static inline void DFlowRecursionOptimizer_SetRoot(DFlowRecursionOptimizer *self, DFlowNode *val)
{
   self->m_root = val;
}

void DFlowRecursionOptimizer_Invalidate(DFlowRecursionOptimizer *self);

///////////////////////////////////////////////////////////////////////////////
// DFlowVisitor
///////////////////////////////////////////////////////////////////////////////

typedef void (*PFN_DFlowVisitor_Destroy)(void *self);
typedef void (*PFN_DFlowVisitor_Accept)(void *self, DFlowNode *node);

typedef struct DFlowVisitor_s
{
   PFN_DFlowVisitor_Destroy   Destroy;
   PFN_DFlowVisitor_Accept    Accept;

   DFlowRecursionOptimizer *m_opt;
} DFlowVisitor;

static inline void DFlowVisitor_Constr(void *me, PFN_DFlowVisitor_Destroy destroy, PFN_DFlowVisitor_Accept accept, DFlowRecursionOptimizer *opt)
{
   DFlowVisitor  *self = (DFlowVisitor *)me;

   self->Destroy = destroy;
   self->Accept  = accept;
   self->m_opt   = opt;
}

static inline void DFlowVisitor_Destr(void *me)
{
   DFlowVisitor  *self = (DFlowVisitor *)me;

   self->Destroy(me);
}

static inline void DFlowVisitor_Accept(void *me, DFlowNode *node)
{
   DFlowVisitor  *self = (DFlowVisitor *)me;

   self->Accept(me, node);
}

static inline void DFlowVisitor_InvalidateRecursionOptimizer(void *me)
{
   DFlowVisitor  *self = (DFlowVisitor *)me;

   DFlowRecursionOptimizer_Invalidate(self->m_opt);
}

void DFlowVisitor_VisitBottomUp(void *me, DFlowNode *root);
void DFlowVisitor_VisitTopDown(void *me, DFlowNode *root);
void DFlowVisitor_VisitBreadthFirst(void *me, DFlowNode *root);
