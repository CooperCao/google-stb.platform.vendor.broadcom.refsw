/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  BCG's scheduler

FILE DESCRIPTION

=============================================================================*/
#ifndef __GLSL_DFLOW_H__
#define __GLSL_DFLOW_H__

#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/glsl_dataflow.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_qpu_instr.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_registers.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_containers.h"

struct Scheduler_s;

////////////////////////////////////////////////////////////////////////////////////////

typedef struct ResetHelper_t
{
   DataflowVector m_originalNodes;
   // NodeVector     m_cppNodes;
} ResetHelper;

ResetHelper *ResetHelper_new(void);
void ResetHelper_delete(ResetHelper *self);

void ResetHelper_Constr(ResetHelper *self);
void ResetHelper_Cleanup(ResetHelper *self);
//void ResetHelper_DeleteNodes(ResetHelper *self);
void ResetHelper_ResetHelperPtrs(ResetHelper *self);

void ResetHelper_AddDataflow(ResetHelper *self, Dataflow *node);
//void ResetHelper_AddDFlowNode(ResetHelper *self, DFlowNode *node);

typedef union
{
   // DATAFLOW_CONST_BOOL
   struct ConstBool
   {
      const_bool  m_cbValue;
      uint32_t    m_cbIndex;
   } m_constBool;

   // DATAFLOW_CONST_FLOAT
   struct ConstFloat
   {
      const_float m_cfValue;
      uint32_t    m_cfIndex;
   } m_constFloat;

   // DATAFLOW_CONST_SAMPLER
   struct ConstSampler
   {
      const_int   m_csLocation;
      const char* m_csName;
   } m_constSampler;

   // DATAFLOW_UNIFORM, DATAFLOW_ATTRIBUTE, DATAFLOW_VARYING
   struct LinkableValue
   {
      const_int   m_lvRow;
      const char* m_lvName;
   } m_linkableValue;

   // DATAFLOW_CONST_INT
   struct ConstInt
   {
      const_int   m_ciValue;
      uint32_t    m_ciIndex;
   } m_constInt;

   struct IndexedUniformSampler
   {
      uint32_t    m_iusAddress;
   } m_indexedUniformSampler;
}  UniformData;

///////////////////////////////////////////////////////////////////////////////
// DFlowNode
///////////////////////////////////////////////////////////////////////////////

typedef enum
{
   DFlowNode_NOT_SCHEDULABLE,
   DFlowNode_NOTHING_TO_SCHEDULE,
   DFlowNode_SCHEDULED,
   DFlowNode_INSERTED_EXTRA_INSTR
} DFlowNode_ScheduleStatus;

typedef enum
{
   DFlowNode_NORMAL_RETIRE,
   DFlowNode_FULLY_RETIRE
} DFlowNode_RetireMode;

typedef enum
{
   DFlowNode_FOR_RESULT,
   DFlowNode_INTERMEDIATE
} DFlowNode_AllocRegMode;

typedef enum
{
   DFlowNode_NOT_SCHEDULED = 0x7fffffff
} DFlowNode_Error;

typedef enum
{
   DFlowNode_CONDITION_ARM  = 0xC0000000,
   DFlowNode_IF_ARM         = 0x40000000,
   DFlowNode_ELSE_ARM       = 0x80000000,
   DFlowNode_MASK_ARM       = ~0xC0000000
} DFlowNode_CondBits;

typedef enum
{
   // DATAFLOW_UNIFORM_OFFSET
   DFlowNode_ARG_LINKABLE_VALUE = 0,
   DFlowNode_ARG_OFFSET         = 1,

   // DATAFLOW_ARITH_NEGATE, DATAFLOW_LOGICAL_NOT
   // DATAFLOW_INTRINSIC_RSQRT, DATAFLOW_INTRINSIC_RCP, DATAFLOW_INTRINSIC_LOG2, DATAFLOW_INTRINSIC_EXP2
   // DATAFLOW_INTRINSIC_CEIL, DATAFLOW_INTRINSIC_FLOOR, DATAFLOW_INTRINSIC_SIGN
   // DATAFLOW_MOV,
   // DATAFLOW_FTOI_TRUNC, DATAFLOW_FTOI_NEAREST, DATAFLOW_ITOF,
   // DATAFLOW_UNPACK_COL_R, DATAFLOW_UNPACK_COL_G, DATAFLOW_UNPACK_COL_B, DATAFLOW_UNPACK_COL_A
   // DATAFLOW_UNPACK_16A, DATAFLOW_UNPACK_16A_F, DATAFLOW_UNPACK_16B, DATAFLOW_UNPACK_16B_F
   // DATAFLOW_UNPACK_8A, DATAFLOW_UNPACK_8B, DATAFLOW_UNPACK_8C, DATAFLOW_UNPACK_8D
   // DATAFLOW_UNPACK_8R
   // DATAFLOW_BITWISE_NOT
   DFlowNode_ARG_OPERAND        = 0,

   // DATAFLOW_MUL, DATAFLOW_ADD, DATAFLOW_SUB, DATAFLOW_RSUB,
   // DATAFLOW_LESS_THAN, DATAFLOW_LESS_THAN_EQUAL, DATAFLOW_GREATER_THAN, DATAFLOW_GREATER_THAN_EQUAL
   // DATAFLOW_EQUAL, DATAFLOW_NOT_EQUAL
   // DATAFLOW_LOGICAL_AND, DATAFLOW_LOGICAL_XOR, DATAFLOW_LOGICAL_OR
   // DATAFLOW_INTRINSIC_MIN, DATAFLOW_INTRINSIC_MAX
   // DATAFLOW_INTRINSIC_MINABS, DATAFLOW_INTRINSIC_MAXABS
   DFlowNode_ARG_LEFT           = 0,
   DFlowNode_ARG_RIGHT          = 1,

   // DATAFLOW_CONDITIONAL
   DFlowNode_ARG_COND           = 0,
   DFlowNode_ARG_TRUE_VALUE     = 1,
   DFlowNode_ARG_FALSE_VALUE    = 2,

   // DATAFLOW_TEX_SET_COORD_S, DATAFLOW_TEX_SET_COORD_T, DATAFLOW_TEX_SET_COORD_R
   // DATAFLOW_TEX_SET_LOD DATAFLOW_TEX_SET_BIAS
   DFlowNode_ARG_PARAM          = 0,
   DFlowNode_ARG_SAMPLER        = 1,

   // DATAFLOW_TEX_GET_CMP_R, DATAFLOW_TEX_GET_CMP_G, DATAFLOW_TEX_GET_CMP_B, DATAFLOW_TEX_GET_CMP_A (sampler only)
   // ARG_SAMPLER     = 1, (already defined)

   // DATAFLOW_FRAG_SUBMIT_STENCIL, DATAFLOW_FRAG_SUBMIT_Z, DATAFLOW_FRAG_SUBMIT_MS, DATAFLOW_FRAG_SUBMIT_ALL
   // DATAFLOW_FRAG_SUBMIT_R0, DATAFLOW_FRAG_SUBMIT_R1, DATAFLOW_FRAG_SUBMIT_R2, DATAFLOW_FRAG_SUBMIT_R3
   // DATAFLOW_TMU_SWAP
   // ARG_PARAM       = 0, (already defined)
   DFlowNode_ARG_DISCARD        = 2,

   // DATAFLOW_VERTEX_SET, DATAFLOW_VPM_READ_SETUP, DATAFLOW_VPM_WRITE_SETUP
   // ARG_PARAM       = 0, (already defined)

   // DATAFLOW_PACK_COL_R, DATAFLOW_PACK_COL_G, DATAFLOW_PACK_COL_B, DATAFLOW_PACK_COL_A,
   // DATAFLOW_PACK_16A, DATAFLOW_PACK_16B
   // ARG_OPERAND     = 0 (already defined)
   DFlowNode_ARG_BACKGROUND     = 1,

   // DATAFLOW_CONST_BOOL
   // DATAFLOW_CONST_FLOAT
   // DATAFLOW_CONST_SAMPLER
   // DATAFLOW_UNIFORM, DATAFLOW_ATTRIBUTE, DATAFLOW_VARYING
   // DATAFLOW_CONST_INT
   // DATAFLOW_INDEXED_UNIFORM_SAMPLER (2708A0), DATAFLOW_UNIFORM_ADDRESS (2708B0)
   DFlowNode_ARG_UNIFORM        = 0,

   // How many?
   DFlowNode_ARG_COUNT = 3
} DFlowNode_ArgIndex;

struct DFlowNode_s
{
   uint32_t          m_uniqueId;

   // Copied from underlying dataflow graph
   DataflowFlavour   m_flavour;
   BOOL_REP_T        m_boolRep;
   UniformData       m_uniform;
   uint32_t          m_sampler;     // Sampler number used by texture nodes

   // Our arguments constructed from dataflow graph
   DFlowNode         *m_args[DFlowNode_ARG_COUNT];

   // Caches
   NodeList          m_parents;
   NodeList          m_children;
   NodeList          m_ioChildren;
   NodeList          m_ioParents;

   int32_t           m_treeDepth;
   int32_t           m_registerDelta;
   int32_t           m_lifespanGuess;
   uint32_t          m_order;
   uint32_t          m_bushiness;

   ResetHelper       *m_resetHelper;

   // Filled out by the analysis visitor
   uint32_t          m_numRecursiveChildren;
   uint32_t          m_numReferences;

   // For visitor to manage unique node visiting
   uint32_t          m_visitorId;
   void             *m_visitorOpaqueData;

   // Subtree stuff
   DFlowNode        *m_replicant;
   bool              m_wantReplicate;
   bool              m_subtreeRoot;

   // Scheduler
   QPUOperand        m_result;
   QPUOperand        m_tempResult;
   bool              m_bypassed;
   bool              m_delayedCondition;
   bool              m_delayedLoadc;
   int32_t           m_slot;
   DFlowRegFile      m_regFile;
   uint32_t          m_schedPass;
   QPUResource      *m_condIfOutput;
   int32_t           m_sortOrder;
   bool              m_threadSwitch;      // For writing to texture S, true if we need a thread switch
   bool              m_tmuOrdered;

   uint32_t          m_issueCount;
};

// static members
extern uint32_t DFlowNode_lastVisitorId;
extern uint32_t DFlowNode_nextId;

void DFlowNode_Dataflow_Constr(DFlowNode *self, Dataflow *dataFlow, ResetHelper *rh);
void DFlowNode_Flavour_Constr(DFlowNode *self, DataflowFlavour flavour, ResetHelper *rh);

// Not needed yet
// void DFlowNode_PartialCopy(DFlowNode *self, const DFlowNode *rhs);

void DFlowNode_Destr(DFlowNode *self);

DFlowNode *DFlowNode_Flavour_new(DataflowFlavour flavour, ResetHelper *rh);
DFlowNode *DFlowNode_Dataflow_new(Dataflow *dataflow, ResetHelper *rh);

// The flavour of this dataflow node.
static INLINE DataflowFlavour DFlowNode_Flavour(const DFlowNode *self)
{
   return self->m_flavour;
}

static INLINE uint32_t DFlowNode_UniqueId(const DFlowNode *self)
{
   return self->m_uniqueId;
}

static INLINE void DFlowNode_SetSampler(DFlowNode *self, uint32_t sampler)
{
   self->m_sampler = sampler;
}

static INLINE void DFlowNode_SetTreeDepth(DFlowNode *self, int32_t val)
{
   self->m_treeDepth = val;
}

static INLINE const NodeList *DFlowNode_Parents(const DFlowNode *self)
{
   return &self->m_parents;
}

static INLINE const NodeList *DFlowNode_Children(const DFlowNode *self)
{
   return &self->m_children;
}

static INLINE const NodeList *DFlowNode_IoParents(const DFlowNode *self)
{
   return &self->m_ioParents;
}

static INLINE const NodeList *DFlowNode_IoChildren(const DFlowNode *self)
{
   return &self->m_ioChildren;
}

void DFlowNode_AddChildNode(DFlowNode *self, DFlowNode_ArgIndex ix, DFlowNode *child);
void DFlowNode_ReplaceChild(DFlowNode *self, DFlowNode *oldChild, DFlowNode *newChild);
void DFlowNode_RemoveParent(DFlowNode *self, DFlowNode *oldParent);
void DFlowNode_RemoveIoParent(DFlowNode *self, DFlowNode *oldParent);
void DFlowNode_AddParent(DFlowNode *self, DFlowNode *newParent);
void DFlowNode_AddIoParent(DFlowNode *self, DFlowNode *newParent);
void DFlowNode_AddExtraIoChild(DFlowNode *self, DFlowNode *node);

// DFlowNode *DFlowNode_DuplicateTree(DFlowNode *self);
void DFlowNode_MarkForReplication(DFlowNode *self, bool tf);

// Return number of children - including io children
uint32_t DFlowNode_NumChildren(const DFlowNode *self);

// Only valid after depth visitor has been run
static INLINE int32_t  DFlowNode_TreeDepth(const DFlowNode *self)
{
   return self->m_treeDepth;
}

// Only valid after analysis visitor has been run
static INLINE uint32_t DFlowNode_NumRecursiveChildren(const DFlowNode *self)
{
   return self->m_numRecursiveChildren;
}

static INLINE uint32_t DFlowNode_NumReferences(const DFlowNode *self)
{
   return self->m_numReferences;
}

DFlowNode_ScheduleStatus DFlowNode_AddToInstruction(DFlowNode *self, struct Scheduler_s *sched, QPUGenericInstr *gi);
DFlowNode_ScheduleStatus DFlowNode_DoDelayedACC5Move(DFlowNode *self, struct Scheduler_s *sched, QPUGenericInstr *gi);

static INLINE int32_t DFlowNode_Slot(const DFlowNode *self)
{
   return self->m_slot;
}

static INLINE void DFlowNode_SetSlot(DFlowNode *self, int32_t val)
{
   self->m_slot = val;
}

static INLINE bool DFlowNode_IsScheduled(const DFlowNode *self, int32_t curSlot)
{
   return self->m_bypassed || self->m_slot <= curSlot;
}

static INLINE bool DFlowNode_HasUnconsumedReference(const DFlowNode *self, int32_t curSlot)
{
   if (!DFlowNode_IsScheduled(self, curSlot))
      return true;

   // Node claims to be scheduled, but the optimization for condition nodes can cause a comparison node to look
   // scheduled when it is actually delayed.
   if (self->m_delayedCondition)
      return true;

   return false;
}

bool DFlowNode_IsSchedulable(const DFlowNode *self);
void DFlowNode_SetResultInt(DFlowNode *self, int8_t val);
void DFlowNode_SetResultFloat(DFlowNode *self, float val);
void DFlowNode_SetResultReg(DFlowNode *self, Register_Enum reg);

static INLINE DFlowRegFile DFlowNode_GetRegFile(const DFlowNode *self)
{
   return self->m_regFile;
}

static INLINE void DFlowNode_SetRegFile(DFlowNode *self, DFlowRegFile rf)
{
   self->m_regFile = rf;
}

static INLINE const DFlowNode *DFlowNode_GetArg_const(const DFlowNode *self, uint32_t i)
{
   return self->m_args[i];
}

static INLINE DFlowNode *DFlowNode_GetArg(DFlowNode *self, uint32_t i)
{
   return self->m_args[i];
}

static INLINE const UniformData *DFlowNode_GetConst_const(const DFlowNode *self)
{
   return &self->m_uniform;
}

static INLINE UniformData *DFlowNode_GetConst(DFlowNode *self)
{
   return &self->m_uniform;
}

static INLINE uint32_t DFlowNode_GetRegisterDelta(const DFlowNode *self)
{
   return self->m_registerDelta;
}

static INLINE void DFlowNode_SetOrder(DFlowNode *self, uint32_t order)
{
   self->m_order = order;
}

static INLINE uint32_t DFlowNode_GetOrder(const DFlowNode *self)
{
   return self->m_order;
}

// Return if the two calculations result in the same value
bool DFlowNode_IsTheSameValueAs(const DFlowNode *self, const DFlowNode *other);
bool DFlowNode_IsConstantConditionExpression(const DFlowNode *self);

// Replace node "other" with "this" in graph (clearly they should be the same)
void DFlowNode_ReplaceWith(DFlowNode *self, DFlowNode *other);

static INLINE int32_t DFlowNode_SortOrder(const DFlowNode *self)
{
   return self->m_sortOrder;
}

static INLINE void DFlowNode_SetSortOrder(DFlowNode *self, int32_t val)
{
   self->m_sortOrder = val;
}

static INLINE void DFlowNode_FlagForThreadSwitch(DFlowNode *self)
{
   self->m_threadSwitch = true;
}

// Used for schduling TMU ops
static INLINE void DFlowNode_FlagTMUOrdered(DFlowNode *self)
{
   self->m_tmuOrdered = true;
}

static INLINE bool DFlowNode_IsTMUOrdered(const DFlowNode *self)
{
   return self->m_tmuOrdered;
}

static INLINE int32_t DFlowNode_LifespanGuess(const DFlowNode *self)
{
   return self->m_lifespanGuess;
}

static INLINE int32_t DFlowNode_GetBushiness(const DFlowNode *self)
{
   return self->m_bushiness;
}

static INLINE void DFlowNode_SetBushiness(DFlowNode *self, int32_t bushiness)
{
   self->m_bushiness = bushiness;
}

Register_Enum DFlowNode_GetTextureReg(const DFlowNode *self);

bool DFlowNode_HasVaryCChild(const DFlowNode *self);

void DFlowNode_CalcRegisterDelta(DFlowNode *self);

static INLINE bool DFlowNode_IncrementIssueCount(DFlowNode *self)
{
   self->m_issueCount++;
   return self->m_issueCount < 10;
}

bool DFlowNode_CanShortcutOutputReg(DFlowNode *self);

// Static methods
bool DFlowNode_ListContains(const NodeList *set, DFlowNode *node);


#endif /* __GLSL_DFLOW_H__ */
