/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/

#ifndef __GLSL_BCG_SCHED_4_H__
#define __GLSL_BCG_SCHED_4_H__

#include "interface/khronos/common/khrn_client_vector.h"
#include "middleware/khronos/glsl/glsl_common.h"
#include "vcfw/rtos/abstract/rtos_abstract_mem.h"

typedef enum
{
   Scheduler_DEFAULT,
   Scheduler_ALT_SORT,
   Scheduler_LAST_GASP
} Scheduler_Strategy;

bool bcg_schedule(Dataflow *root, uint32_t type, bool *allow_thread, Scheduler_Strategy strategy, MEM_HANDLE_T *mh_code,
                  MEM_HANDLE_T *mh_uniform_map, uint32_t *vary_map, uint32_t *vary_count, void **resetHelper);
void bcg_schedule_cleanup(void **resetHelper);

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_registers.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_qpu_instr.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_containers.h"

struct DFlowRecursionOptimizer_s;

///////////////////////////////////////////////////////////////////////////////
// GIVector -- vector of instructions
///////////////////////////////////////////////////////////////////////////////

typedef struct GIVector_s
{
   uint32_t       m_end;
   VectorBase     m_vector;
} GIVector;

typedef QPUGenericInstr       *GIVector_iterator;
typedef const QPUGenericInstr *GIVector_const_iterator;

// Functions
void GIVector_Constr(GIVector *self, uint32_t capacity);
void GIVector_push_back(GIVector *self, const QPUGenericInstr *node);

// Inline functions
static INLINE void GIVector_Destr(GIVector *self)
{
   VectorBase_Destr(&self->m_vector);
}

static INLINE void GIVector_clear(GIVector *self)
{
   self->m_end = 0;
   VectorBase_Clear(&self->m_vector);
}

static INLINE QPUGenericInstr *GIVector_lindex(GIVector *self, uint32_t i)
{
   GIVector_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (GIVector_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE const QPUGenericInstr *GIVector_const_lindex(const GIVector *self, uint32_t i)
{
   GIVector_const_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (GIVector_const_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE QPUGenericInstr *GIVector_index(GIVector *self, uint32_t i)
{
   GIVector_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (GIVector_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE const QPUGenericInstr *GIVector_const_index(const GIVector *self, uint32_t i)
{
   GIVector_const_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (GIVector_const_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE uint32_t GIVector_size(const GIVector *self)
{
   return self->m_end;
}

// Iterator
static INLINE GIVector_iterator GIVector_begin(GIVector *self)
{
   GIVector_iterator   data  = (GIVector_iterator)self->m_vector.data;

   return &data[0];
}

static INLINE GIVector_const_iterator GIVector_const_begin(const GIVector *self)
{
   GIVector_const_iterator   data  = (GIVector_const_iterator)self->m_vector.data;

   return &data[0];
}

static INLINE GIVector_iterator GIVector_end(GIVector *self)
{
   GIVector_iterator   data  = (GIVector_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static INLINE GIVector_const_iterator GIVector_const_end(const GIVector *self)
{
   GIVector_const_iterator   data  = (GIVector_const_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static INLINE void GIVector_next(GIVector_iterator *iter)
{
   (*iter)++;
}

static INLINE void GIVector_const_next(GIVector_const_iterator *iter)
{
   (*iter)++;
}

// Iterator methods
static INLINE QPUGenericInstr *GIVector_star(GIVector_iterator self)
{
   return self;
}

static INLINE const QPUGenericInstr *GIVector_const_star(GIVector_const_iterator self)
{
   return self;
}

///////////////////////////////////////////////////////////////////////////////
// Uniform
///////////////////////////////////////////////////////////////////////////////

typedef struct Uniform_s
{
   uint32_t m_type;
   uint32_t m_value;
} Uniform;

static INLINE void Uniform_Constr(Uniform *self, uint32_t uniformType, uint32_t value)
{
   self->m_type  = uniformType;
   self->m_value = value;
}

///////////////////////////////////////////////////////////////////////////////
// UniformVector
///////////////////////////////////////////////////////////////////////////////

typedef struct UniformVector_s
{
   uint32_t       m_end;
   VectorBase     m_vector;
} UniformVector;

typedef Uniform       *UniformVector_iterator;
typedef const Uniform *UniformVector_const_iterator;

// Functions
void UniformVector_Constr(UniformVector *self, uint32_t capacity);
void UniformVector_push_back(UniformVector *self, const Uniform *node);

// Inline functions
static INLINE void UniformVector_Destr(UniformVector *self)
{
   VectorBase_Destr(&self->m_vector);
}

static INLINE void UniformVector_clear(UniformVector *self)
{
   self->m_end = 0;
   VectorBase_Clear(&self->m_vector);
}

static INLINE Uniform *UniformVector_lindex(UniformVector *self, uint32_t i)
{
   UniformVector_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (UniformVector_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE const Uniform *UniformVector_const_lindex(const UniformVector *self, uint32_t i)
{
   UniformVector_const_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (UniformVector_const_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE Uniform *UniformVector_index(UniformVector *self, uint32_t i)
{
   UniformVector_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (UniformVector_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE const Uniform *UniformVector_const_index(const UniformVector *self, uint32_t i)
{
   UniformVector_const_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (UniformVector_const_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE uint32_t UniformVector_size(const UniformVector *self)
{
   return self->m_end;
}

// Iterator
static INLINE UniformVector_iterator UniformVector_begin(UniformVector *self)
{
   UniformVector_iterator   data  = (UniformVector_iterator)self->m_vector.data;

   return &data[0];
}

static INLINE UniformVector_const_iterator UniformVector_const_begin(const UniformVector *self)
{
   UniformVector_const_iterator   data  = (UniformVector_const_iterator)self->m_vector.data;

   return &data[0];
}

static INLINE UniformVector_iterator UniformVector_end(UniformVector *self)
{
   UniformVector_iterator   data  = (UniformVector_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static INLINE UniformVector_const_iterator UniformVector_const_end(const UniformVector *self)
{
   UniformVector_const_iterator   data  = (UniformVector_const_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static INLINE void UniformVector_next(UniformVector_iterator *iter)
{
   (*iter)++;
}

static INLINE void UniformVector_const_next(UniformVector_const_iterator *iter)
{
   (*iter)++;
}

// Iterator methods
static INLINE Uniform *UniformVector_star(UniformVector_iterator self)
{
   return self;
}

static INLINE const Uniform *UniformVector_const_star(UniformVector_const_iterator self)
{
   return self;
}

///////////////////////////////////////////////////////////////////////////////
// VaryingVector
///////////////////////////////////////////////////////////////////////////////

typedef struct VaryingVector_s
{
   uint32_t       m_end;
   VectorBase     m_vector;
} VaryingVector;

typedef int32_t       *VaryingVector_iterator;
typedef const int32_t *VaryingVector_const_iterator;

// Functions
void VaryingVector_Constr(VaryingVector *self, uint32_t capacity);
void VaryingVector_push_back(VaryingVector *self, const int32_t *node);

// Inline functions
static INLINE void VaryingVector_Destr(VaryingVector *self)
{
   VectorBase_Destr(&self->m_vector);
}

static INLINE void VaryingVector_clear(VaryingVector *self)
{
   self->m_end = 0;
   VectorBase_Clear(&self->m_vector);
}

static INLINE int32_t *VaryingVector_lindex(VaryingVector *self, uint32_t i)
{
   VaryingVector_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (VaryingVector_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE const int32_t *VaryingVector_const_lindex(const VaryingVector *self, uint32_t i)
{
   VaryingVector_const_iterator resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (VaryingVector_const_iterator)self->m_vector.data;

   return &resultList[i];
}

static INLINE int32_t VaryingVector_index(VaryingVector *self, uint32_t i)
{
   VaryingVector_iterator resultList;

   if (i >= self->m_end)
      return 0;

   resultList = (VaryingVector_iterator)self->m_vector.data;

   return resultList[i];
}

static INLINE int32_t VaryingVector_const_index(const VaryingVector *self, uint32_t i)
{
   VaryingVector_const_iterator resultList;

   if (i >= self->m_end)
      return 0;

   resultList = (VaryingVector_const_iterator)self->m_vector.data;

   return resultList[i];
}

static INLINE uint32_t VaryingVector_size(const VaryingVector *self)
{
   return self->m_end;
}

// Iterator
static INLINE VaryingVector_iterator VaryingVector_begin(VaryingVector *self)
{
   VaryingVector_iterator   data  = (VaryingVector_iterator)self->m_vector.data;

   return &data[0];
}

static INLINE VaryingVector_const_iterator VaryingVector_const_begin(const VaryingVector *self)
{
   VaryingVector_const_iterator   data  = (VaryingVector_const_iterator)self->m_vector.data;

   return &data[0];
}

static INLINE VaryingVector_iterator VaryingVector_end(VaryingVector *self)
{
   VaryingVector_iterator   data  = (VaryingVector_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static INLINE VaryingVector_const_iterator VaryingVector_const_end(const VaryingVector *self)
{
   VaryingVector_const_iterator   data  = (VaryingVector_const_iterator)self->m_vector.data;

   return &data[self->m_end];
}

static INLINE void VaryingVector_next(VaryingVector_iterator *iter)
{
   (*iter)++;
}

static INLINE void VaryingVector_const_next(VaryingVector_const_iterator *iter)
{
   (*iter)++;
}

// Iterator methods
static INLINE int32_t *VaryingVector_star(VaryingVector_iterator self)
{
   return self;
}

static INLINE const int32_t *VaryingVector_const_star(VaryingVector_const_iterator self)
{
   return self;
}

///////////////////////////////////////////////////////////////////////////////
// Scheduler
///////////////////////////////////////////////////////////////////////////////


typedef enum
{
   Scheduler_MAX_NUM_TEXTURE_UNITS = 2
} Scheduler_Constant;

typedef struct Scheduler_s
{
   bool               m_isFragmentShader;
   bool               m_allowThread;
   Scheduler_Strategy m_strategy;     // Try anything to schedule the code - minimal register pressure

   GIVector          m_schedule;
   QPUResources      m_resources;

   UniformVector     m_uniforms;
   VaryingVector     m_varyings;
   uint32_t          m_nextTextureUniform[Scheduler_MAX_NUM_TEXTURE_UNITS];

   NodeList          m_schedulable;
   NodeList          m_notSchedulable;

   uint32_t          m_maxThreadSwitches;
   uint32_t          m_numThreadSwitches;

   DFlowNode        *m_activeNode;             // When we want to stick with one instruction.
   DFlowNode        *m_currentFlagsCondition;  // The condition that set the current flag state

   bool              m_needsACC5Move;
   DFlowNode        *m_ACC5MoveNode;
} Scheduler;


void Scheduler_Constr(Scheduler *self, bool isFragment, bool allowThread,  Scheduler_Strategy strategy, uint32_t maxThreadSwitches);
void Scheduler_Destr(Scheduler *self);

bool Scheduler_Schedule(Scheduler *self, DFlowNode *root, struct DFlowRecursionOptimizer_s *opt);
int32_t Scheduler_FirstEmptySlot(const Scheduler *self);
void Scheduler_WriteCode(Scheduler *self, void *code, size_t bufSize);
void Scheduler_WriteUniforms(Scheduler *self, void *uniforms, size_t bufSize);
void Scheduler_WriteVaryings(Scheduler *self, void *varyings, size_t bufSize);
void Scheduler_TransmitOutput(Scheduler *self, uint32_t type);
void Scheduler_TransmitInfo(Scheduler *self, uint32_t type);
void Scheduler_InsertThreadSwitch(Scheduler *self);
bool Scheduler_CanInsertThreadSwitch(const Scheduler *self);
bool Scheduler_AllThreadSwitchesDone(const Scheduler *self);

static INLINE uint32_t Scheduler_CodeByteSize(const Scheduler *self)
{
   return GIVector_size(&self->m_schedule) * sizeof(uint64_t);
}

static INLINE uint32_t Scheduler_UniformsByteSize(const Scheduler *self)
{
   return UniformVector_size(&self->m_uniforms) * sizeof(Uniform);
}

static INLINE uint32_t Scheduler_VaryingsByteSize(const Scheduler *self)
{
   return VaryingVector_size(&self->m_varyings) * sizeof(int32_t);
}

static INLINE QPUResources *Scheduler_Resources(Scheduler *self)
{
   return &self->m_resources;
}

static INLINE UniformVector *Scheduler_Uniforms(Scheduler *self)
{
   return &self->m_uniforms;
}

static INLINE VaryingVector *Scheduler_Varyings(Scheduler *self)
{
   return &self->m_varyings;
}

static INLINE void Scheduler_SetNextTextureUniform(Scheduler *self, uint32_t unit, uint32_t value)
{
   self->m_nextTextureUniform[unit] = value;
}

static INLINE uint32_t Scheduler_GetNextTextureUniform(Scheduler *self, uint32_t unit)
{
   return self->m_nextTextureUniform[unit];
}

static INLINE bool Scheduler_IsFragmentShader(const Scheduler *self)
{
   return self->m_isFragmentShader;
}

static INLINE bool Scheduler_AllowThreadswitch(const Scheduler *self)
{
   return self->m_allowThread;
}

// Register the current instruction to be scheduled with the scheduler (used for e.g. writing tXs registers when spilling)
static INLINE void Scheduler_SetActiveNode(Scheduler *self, DFlowNode *node)
{
   self->m_activeNode = node;
}

static INLINE DFlowNode *Scheduler_CurrentFlagsCondition(const Scheduler *self)
{
   return self->m_currentFlagsCondition;
}

static INLINE void Scheduler_SetCurrentFlagsCondition(Scheduler *self, DFlowNode *val)
{
   self->m_currentFlagsCondition = val;
}

static INLINE void Scheduler_SetNeedsACC5Mov(Scheduler *self, DFlowNode *node, bool tf)
{
   self->m_needsACC5Move = tf;
   self->m_ACC5MoveNode  = node;
}

static INLINE void Scheduler_SetNeedsACC5MovDef(Scheduler *self, DFlowNode *node)
{
   self->m_needsACC5Move = true;
   self->m_ACC5MoveNode  = node;
}

static INLINE bool Scheduler_NeedsACC5Mov(const Scheduler *self, DFlowNode **node)
{
   *node = self->m_ACC5MoveNode;
   return self->m_needsACC5Move;
}

static INLINE bool Scheduler_LastGasp(const Scheduler *self)
{
   return self->m_strategy == Scheduler_LAST_GASP;
}

#endif /* __GLSL_BCG_SCHED_4_H__ */
