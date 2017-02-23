/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#ifndef __GLSL_DFLOW_ANALYZE_VISITOR_H__
#define __GLSL_DFLOW_ANALYZE_VISITOR_H__

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_containers.h"
#include "middleware/khronos/glsl/glsl_common.h"

typedef struct TextureSetup_s
{
   DFlowNode *m_request;
   DFlowNode *m_s;
   DFlowNode *m_t;
   DFlowNode *m_r;
   DFlowNode *m_b;

   // Cache of number of FIFO writes (i.e. how many of m_s through m_b are valid)
   uint32_t   m_writeCount;

   // Assign a unit number to this transaction
   uint32_t   m_unit;
} TextureSetup;

static INLINE void TextureSetup_Constr(TextureSetup *self)
{
   self->m_request    = NULL;
   self->m_s          = NULL;
   self->m_t          = NULL;
   self->m_r          = NULL;
   self->m_b          = NULL;
   self->m_writeCount = 0;
   self->m_unit       = 0;
}

static INLINE void TextureSetup_Destr(TextureSetup *self)
{
   UNUSED(self);
}

static INLINE void TextureSetup_SetRequest(TextureSetup *self, DFlowNode *node)
{
   self->m_request = node;
}

static INLINE void TextureSetup_SetS(TextureSetup *self, DFlowNode *node)
{
   self->m_s = node;
   self->m_writeCount += 1;
}

static INLINE void TextureSetup_SetT(TextureSetup *self, DFlowNode *node)
{
   self->m_t = node;
   self->m_writeCount += 1;
}

static INLINE void TextureSetup_SetR(TextureSetup *self, DFlowNode *node)
{
   self->m_r = node;
   self->m_writeCount += 1;
}

static INLINE void TextureSetup_SetB(TextureSetup *self, DFlowNode *node)
{
   self->m_b = node;
   self->m_writeCount += 1;
}

static INLINE void TextureSetup_SetUnit(TextureSetup *self, uint32_t unit)
{
   self->m_unit = unit;

   if (self->m_request != NULL)
      DFlowNode_SetSampler(self->m_request, unit);

   if (self->m_s != NULL)
      DFlowNode_SetSampler(self->m_s, unit);

   if (self->m_t != NULL)
      DFlowNode_SetSampler(self->m_t, unit);

   if (self->m_r != NULL)
      DFlowNode_SetSampler(self->m_r, unit);

   if (self->m_b != NULL)
      DFlowNode_SetSampler(self->m_b, unit);
}

static INLINE DFlowNode *TextureSetup_GetRequest(const TextureSetup *self)
{
   return self->m_request;
}

static INLINE DFlowNode *TextureSetup_GetS(const TextureSetup *self)
{
   return self->m_s;
}

static INLINE DFlowNode *TextureSetup_GetT(const TextureSetup *self)
{
   return self->m_t;
}

static INLINE DFlowNode *TextureSetup_GetR(const TextureSetup *self)
{
   return self->m_r;
}

static INLINE DFlowNode *TextureSetup_GetB(const TextureSetup *self)
{
   return self->m_b;
}

static INLINE uint32_t TextureSetup_GetWriteCount(const TextureSetup *self)
{
   return self->m_writeCount;
}

static INLINE uint32_t TextureSetup_GetUnit(const TextureSetup *self)
{
   return self->m_unit;
}

///////////////////////////////////////////////////////////////////////////////
// TextureSetupVector
///////////////////////////////////////////////////////////////////////////////

typedef struct TextureSetupVector_s
{
   uint32_t       m_end;
   VectorBase     m_vector;
} TextureSetupVector;

typedef TextureSetup       *TextureSetupVector_iterator;
typedef const TextureSetup *TextureSetupVector_const_iterator;

typedef int (*TextureSetup_pred)(const TextureSetup *i, const TextureSetup *j);

// Functions
void TextureSetupVector_Constr(TextureSetupVector *self, uint32_t capacity);
void TextureSetupVector_push_back(TextureSetupVector *self, const TextureSetup *node);

// Inline functions
static INLINE void TextureSetupVector_Destr(TextureSetupVector *self)
{
   VectorBase_Destr(&self->m_vector);
}

static INLINE void TextureSetupVector_clear(TextureSetupVector *self)
{
   self->m_end = 0;
   VectorBase_Clear(&self->m_vector);
}

static INLINE TextureSetup *TextureSetupVector_lindex(TextureSetupVector *self, uint32_t i)
{
   TextureSetupVector_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (TextureSetupVector_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE const TextureSetup *TextureSetupVector_const_lindex(const TextureSetupVector *self, uint32_t i)
{
   TextureSetupVector_const_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (TextureSetupVector_const_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE TextureSetup *TextureSetupVector_index(TextureSetupVector *self, uint32_t i)
{
   TextureSetupVector_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (TextureSetupVector_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE const TextureSetup *TextureSetupVector_const_index(const TextureSetupVector *self, uint32_t i)
{
   TextureSetupVector_const_iterator resultList;
   if (i >= self->m_end)
      return NULL;

   resultList = (TextureSetupVector_const_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE uint32_t TextureSetupVector_size(const TextureSetupVector *self)
{
   return self->m_end;
}

// Iterator
static INLINE TextureSetupVector_iterator TextureSetupVector_begin(TextureSetupVector *self)
{
   TextureSetupVector_iterator   data  = (TextureSetupVector_iterator)self->m_vector.data;

   return &data[0];
}

static INLINE TextureSetupVector_const_iterator TextureSetupVector_const_begin(const TextureSetupVector *self)
{
   TextureSetupVector_const_iterator   data  = (TextureSetupVector_const_iterator)self->m_vector.data;

   return &data[0];
}

static INLINE TextureSetupVector_iterator TextureSetupVector_end(TextureSetupVector *self)
{
   TextureSetupVector_iterator   data  = (TextureSetupVector_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static INLINE TextureSetupVector_const_iterator TextureSetupVector_const_end(const TextureSetupVector *self)
{
   TextureSetupVector_const_iterator   data  = (TextureSetupVector_const_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static INLINE void TextureSetupVector_next(TextureSetupVector_iterator *iter)
{
   (*iter)++;
}

static INLINE void TextureSetupVector_const_next(TextureSetupVector_const_iterator *iter)
{
   (*iter)++;
}

// Iterator methods
static INLINE TextureSetup *TextureSetupVector_star(TextureSetupVector_iterator self)
{
   return self;
}

static INLINE const TextureSetup *TextureSetupVector_const_star(TextureSetupVector_const_iterator self)
{
   return self;
}

///////////////////////////////////////////////////////////////////////////////
// NodesAtLevel
///////////////////////////////////////////////////////////////////////////////

typedef struct NodesAtLevel_s
{
   uint32_t *m_nodes;
   uint32_t m_size;
} NodesAtLevel;

static INLINE void NodesAtLevel_Constr(NodesAtLevel *self, uint32_t capacity)
{
   self->m_size  = capacity;
   self->m_nodes = (uint32_t *)bcg_glsl_malloc(sizeof(uint32_t) * capacity);
   memset(self->m_nodes, 0, sizeof(uint32_t) * capacity);
}

static INLINE void NodesAtLevel_Destr(NodesAtLevel *self)
{
   bcg_glsl_free(self->m_nodes);
}

static INLINE uint32_t NodesAtLevel_size(const NodesAtLevel *self)
{
   return self->m_size;
}

static INLINE void NodesAtLevel_resize(NodesAtLevel *self, uint32_t capacity)
{
   uint32_t i;
   uint32_t *oldNodes = self->m_nodes;

   // Realloc
   self->m_nodes = (uint32_t *)bcg_glsl_malloc(sizeof(uint32_t) * capacity);
   khrn_memcpy(self->m_nodes, oldNodes, self->m_size);

   // Zero out new entries
   for (i = self->m_size; i < capacity; ++i)
      self->m_nodes[i] = 0;

   self->m_size  = capacity;
}

static INLINE void NodesAtLevel_inc(NodesAtLevel *self, uint32_t i)
{
   self->m_nodes[i]++;
}

static INLINE uint32_t NodesAtLevel_index(const NodesAtLevel *self, uint32_t i)
{
   return self->m_nodes[i];
}

///////////////////////////////////////////////////////////////////////////////
// DFlowAnalyzeVisitor
///////////////////////////////////////////////////////////////////////////////

typedef struct DFlowAnalyzeVisitor_s
{
   DFlowVisitor        m_base;
   ResetHelper         *m_resetHelper;

   NodesAtLevel         m_nodesAtLevel;
   uint32_t             m_maxNodesOnOneLevel;
   TextureSetupVector   m_textureSetups;
   DFlowNode           *m_pointCoord[2];
   DFlowNode           *m_varyings[32];
   NodeList             m_extraVpmWriteDeps;
   NodeList             m_extraMovNodeList;
} DFlowAnalyzeVisitor;

void DFlowAnalyzeVisitor_Constr(DFlowAnalyzeVisitor *self, DFlowRecursionOptimizer *opt, ResetHelper *rh);
void DFlowAnalyzeVisitor_Visit(DFlowAnalyzeVisitor *self, DFlowNode *node);

static INLINE void DFlowAnalyzeVisitor_Destr(DFlowAnalyzeVisitor *self)
{
   DFlowVisitor_Destr(self);
}

// Return number of levels in the graph
static INLINE uint32_t DFlowAnalyzeVisitor_NumLevels(const DFlowAnalyzeVisitor *self)
{
   return NodesAtLevel_size(&self->m_nodesAtLevel);
}

// Return number of nodes at this level in the graph
static INLINE uint32_t DFlowAnalyzeVisitor_GetNodesAtLevel(const DFlowAnalyzeVisitor *self, uint32_t level)
{
   return NodesAtLevel_index(&self->m_nodesAtLevel, level);
}

// Return the maximum number of nodes at any level
static INLINE uint32_t DFlowAnalyzeVisitor_MaxNodesOnOneLevel(const DFlowAnalyzeVisitor *self)
{
   return self->m_maxNodesOnOneLevel;
}

// Returns texture groups
static INLINE const TextureSetupVector *DFlowAnalyzeVisitor_TextureSetups(const DFlowAnalyzeVisitor *self)
{
   return &self->m_textureSetups;
}

///////////////////////////////////////////////////////////////////////////////
// DFlowDependentReadVisitor
///////////////////////////////////////////////////////////////////////////////

typedef struct DFlowDependentReadVisitor_s
{
   DFlowVisitor  m_base;

   bool           m_isDependent;
} DFlowDependentReadVisitor;

void DFlowDependentReadVisitor_Constr(DFlowDependentReadVisitor *self, DFlowRecursionOptimizer *opt);
void DFlowDependentReadVisitor_Visit(DFlowDependentReadVisitor *self, DFlowNode *node);

static INLINE void DFlowDependentReadVisitor_Destr(DFlowDependentReadVisitor *self)
{
   DFlowVisitor_Destr(self);
}

static INLINE bool DFlowDependentReadVisitor_IsDependent(const DFlowDependentReadVisitor *self)
{
   return self->m_isDependent;
}

///////////////////////////////////////////////////////////////////////////////
// DFlowOptimizeCandidateVisitor
///////////////////////////////////////////////////////////////////////////////

typedef struct DFlowOptimizeCandidateVisitor_s
{
   DFlowVisitor  m_base;
   NodeSet        m_candidates[2];
   uint32_t       m_current;
   uint32_t       m_next;
} DFlowOptimizeCandidateVisitor;

void DFlowOptimizeCandidateVisitor_Constr(DFlowOptimizeCandidateVisitor *self, DFlowRecursionOptimizer *opt);

static INLINE void DFlowOptimizeCandidateVisitor_Destr(DFlowOptimizeCandidateVisitor *self)
{
   DFlowVisitor_Destr(self);
}

static INLINE uint32_t DFlowOptimizeCandidateVisitor_size(DFlowOptimizeCandidateVisitor *self)
{
   return NodeSet_size(&self->m_candidates[self->m_current]);
}

void DFlowOptimizeCandidateVisitor_Visit(DFlowOptimizeCandidateVisitor *self, DFlowNode *node);
void DFlowOptimizeCandidateVisitor_AddParents(DFlowOptimizeCandidateVisitor *self, DFlowNode *node);
void DFlowOptimizeCandidateVisitor_Remove(DFlowOptimizeCandidateVisitor *self, DFlowNode *node);
void DFlowOptimizeCandidateVisitor_Next(DFlowOptimizeCandidateVisitor *self);
void DFlowOptimizeCandidateVisitor_Flatten(DFlowOptimizeCandidateVisitor *self, NodeVector *flat);

///////////////////////////////////////////////////////////////////////////////
// DFlowCombineVisitor
///////////////////////////////////////////////////////////////////////////////

typedef struct DFlowCombiner_s
{
   NodeVectorMap                  m_nodeMap;
   DFlowOptimizeCandidateVisitor *m_candidates;
   uint32_t                       m_numCombines;
} DFlowCombiner;

void DFlowCombiner_Constr(DFlowCombiner *self, DFlowOptimizeCandidateVisitor *candidates);
void DFlowCombiner_Visit(DFlowCombiner *self);
void DFlowCombiner_Accept(DFlowCombiner *self, DFlowNode *node);

void DFlowCombiner_Destr(DFlowCombiner *self);

static INLINE uint32_t DFlowCombiner_GetNumCombines(const DFlowCombiner *self)
{
   return self->m_numCombines;
}

///////////////////////////////////////////////////////////////////////////////
// DFlowSimplifier
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
   Simplifier_NONE,
   Simplifier_X_TIMES_ZERO,
   Simplifier_X_TIMES_ONE,
   Simplifier_ZERO_TIMES_X,
   Simplifier_ONE_TIMES_X,
   Simplifier_X_PLUS_ZERO,
   Simplifier_ZERO_PLUS_X,
   Simplifier_IF_X_THEN_Y_ELSE_Y
} Simplifier_Type;

typedef struct Simplification_s
{
   Simplifier_Type   m_type;
   DFlowNode        *m_node;
} Simplification;

typedef struct SimplificatioVector_s
{
   uint32_t       m_end;
   VectorBase     m_vector;
} SimplificationVector;

// Functions
void SimplificationVector_Constr(SimplificationVector *self, uint32_t capacity);
void SimplificationVector_push_back(SimplificationVector *self, const Simplification *node);

// Inline functions
static INLINE void SimplificationVector_Destr(SimplificationVector *self)
{
   VectorBase_Destr(&self->m_vector);
}

static INLINE Simplification *SimplificationVector_index(SimplificationVector *self, uint32_t i)
{
   Simplification *resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (Simplification *)self->m_vector.data;

   return &resultList[i];
}

static INLINE const Simplification *SimplificationVector_const_index(const SimplificationVector *self, uint32_t i)
{
   Simplification *resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (Simplification *)self->m_vector.data;

   return &resultList[i];
}

static INLINE uint32_t SimplificationVector_size(const SimplificationVector *self)
{
   return self->m_end;
}

typedef struct DFlowSimplifier_s
{
   DFlowOptimizeCandidateVisitor *m_candidates;
   SimplificationVector           m_simplifications;
} DFlowSimplifier;

void DFlowSimplifier_Constr(DFlowSimplifier *self, DFlowOptimizeCandidateVisitor *candidates);
void DFlowSimplifier_Destr(DFlowSimplifier *self);
void DFlowSimplifier_Visit(DFlowSimplifier *self);
void DFlowSimplifier_Accept(DFlowSimplifier *self, DFlowNode *node);

static INLINE uint32_t DFlowSimplifier_GetNumRemoved(const DFlowSimplifier *self)
{
   return SimplificationVector_size(&self->m_simplifications);
}

///////////////////////////////////////////////////////////////////////////////
// DBushinessVisitor
///////////////////////////////////////////////////////////////////////////////

typedef struct DFlowBushinessVisitor_s
{
   DFlowVisitor  m_base;
} DFlowBushinessVisitor;

void DFlowBushinessVisitor_Constr(DFlowBushinessVisitor *self, DFlowRecursionOptimizer *opt);

static INLINE void DFlowBushinessVisitor_Destr(DFlowBushinessVisitor *self)
{
   DFlowVisitor_Destr(self);
}

void DFlowBushinessVisitor_Visit(DFlowBushinessVisitor *self, DFlowNode *node);

///////////////////////////////////////////////////////////////////////////////
// DFlowSubtreeDetectionVisitor
///////////////////////////////////////////////////////////////////////////////

typedef struct DFlowSubtreeDetectionVisitor_s
{
   DFlowVisitor  m_base;
   NodeVector     m_nodeList;
} DFlowSubtreeDetectionVisitor;

void DFlowSubtreeDetectionVisitor_Constr(DFlowSubtreeDetectionVisitor *self, DFlowRecursionOptimizer *opt);
void DFlowSubtreeDetectionVisitor_Visit(DFlowSubtreeDetectionVisitor *self, DFlowNode *node);

static INLINE void DFlowSubtreeDetectionVisitor_Destr(DFlowSubtreeDetectionVisitor *self)
{
   DFlowVisitor_Destr(self);
}

#endif /* __GLSL_DFLOW_ANALYZE_VISITOR_H__ */
