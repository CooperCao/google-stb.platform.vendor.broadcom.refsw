/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_options.h"
#include "interface/khronos/common/khrn_client_vector.h"

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_qpu_instr.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_bcg_sched.h"
#include "middleware/khronos/glsl/2708/glsl_allocator_4.h"

uint32_t DFlowNode_lastVisitorId = 0;
uint32_t DFlowNode_nextId = 0;

///////////////////////////////////////////////////////////////////////////////
// OptionalBypass
///////////////////////////////////////////////////////////////////////////////
typedef struct OptionalBypass_s
{
   DFlowNode   *m_node;
} OptionalBypass;

static INLINE void OptionalBypass_Constr(OptionalBypass *self)
{
   self->m_node = NULL;
}

static INLINE void OptionalBypass_Set(OptionalBypass *self, DFlowNode *node)
{
   assert(self->m_node == NULL);
   assert(node != NULL);
   self->m_node = node;
}

static INLINE bool OptionalBypass_IsSet(const OptionalBypass *self)
{
   return self->m_node == NULL ? 0 : 1;
}

static INLINE DFlowNode *OptionalBypass_Get(OptionalBypass *self)
{
   assert(self->m_node != NULL);
   return self->m_node;
}

///////////////////////////////////////////////////////////////////////////////
// NodeResult
///////////////////////////////////////////////////////////////////////////////
typedef struct NodeResult_s
{
   DFlowNode *m_node;
   QPUOperand m_result;
} NodeResult;

void NodeResult_Constr(NodeResult *self, DFlowNode *node, const QPUOperand *result)
{
   self->m_node = node;
   QPUOperand_ConstrCopy(&self->m_result, result);
}

void NodeResult_Copy(NodeResult *self, const NodeResult *rhs)
{
   assert(rhs != NULL);
   self->m_node = rhs->m_node;
   QPUOperand_ConstrCopy(&self->m_result, &rhs->m_result);
}

///////////////////////////////////////////////////////////////////////////////
// NodeResultList
///////////////////////////////////////////////////////////////////////////////
#define NodeResult_CAPACITY 4

typedef struct NodeResultList_s
{
   uint32_t       m_end;
   VectorBase     m_vector;
} NodeResultList;

void NodeResultList_Constr(NodeResultList *self)
{
   self->m_end = 0;
   VectorBase_Constr(&self->m_vector, sizeof(NodeResult));
}

void NodeResultList_Destr(NodeResultList *self)
{
   VectorBase_Destr(&self->m_vector);
}

NodeResult *NodeResultList_index(NodeResultList *self, uint32_t i)
{
   NodeResult  *resultList;

   if (i >= self->m_end)
      return NULL;

   resultList = (NodeResult *)self->m_vector.data;

   return &resultList[i];
}

void NodeResultList_push_back(NodeResultList *self, const NodeResult *result)
{
   if (VectorBase_Extend(&self->m_vector, NodeResult_CAPACITY * sizeof(NodeResult)))
   {
      NodeResult  *resultList = (NodeResult *)self->m_vector.data;
      uint32_t    i = self->m_end;

      self->m_end++;
      NodeResult_Copy(&resultList[i], result);
   }
}

uint32_t NodeResultList_size(NodeResultList *self)
{
   return self->m_end;
}

///////////////////////////////////////////////////////////////////////////////
// InstrState
///////////////////////////////////////////////////////////////////////////////
typedef struct InstrState_s
{
   // New state
   Scheduler            *m_scheduler;

   int32_t               m_slot;
   bool                  m_setNextTextureUniform;
   uint32_t              m_nextTextureUniformUnit;
   uint32_t              m_nextTextureUniformValue;

   QPUGenericInstr       m_gi;
   Register_File         m_outputRegistersFree;

   // The original state to restore on error
   QPUGenericInstr      *m_originalInstr;

   bool                  m_textureRead;
   bool                  m_insertThreadSwitch;

   OptionalBypass        m_optionalBypass;
   NodeResultList        m_nodeResults;

// private:
   QPUResources          m_resources;

   bool                  m_isPendingUniform;
   Uniform               m_pendingUniform;

   bool                  m_isPendingVarying;
   int32_t               m_pendingVarying;
} InstrState;

void InstrState_Constr(InstrState *self, QPUGenericInstr *origInstr, Scheduler *sched)
{
   self->m_scheduler               = sched;
   self->m_slot                    = Scheduler_FirstEmptySlot(sched);
   self->m_setNextTextureUniform   = false;
   self->m_nextTextureUniformUnit  = 0;
   self->m_nextTextureUniformValue = 0;

   QPUGenericInstr_ConstrCopy(&self->m_gi, origInstr);

   self->m_outputRegistersFree     = QPUGenericInstr_GetFreeOutputRegisters(origInstr);
   self->m_originalInstr           = origInstr;
   self->m_textureRead             = false;
   self->m_insertThreadSwitch      = false;

   OptionalBypass_Constr(&self->m_optionalBypass);
   NodeResultList_Constr(&self->m_nodeResults);

   self->m_resources               = *Scheduler_Resources(sched);

   self->m_isPendingUniform        = false;
   // m_pendingUniform only valid when m_isPendingUniform == true
   self->m_isPendingVarying        = false;
   // m_pendingVarying only valid when m_isPendingVarying == true
}

void InstrState_Destr(InstrState *self)
{
   NodeResultList_Destr(&self->m_nodeResults);
}

DFlowNode_ScheduleStatus InstrState_CommitChanges(InstrState *self)
{
   uint32_t i;

   // One final check that the instruction is valid
   if (QPUGenericInstr_GetCoding(&self->m_gi) == 0)
      return DFlowNode_NOT_SCHEDULABLE;

   QPUGenericInstr_ConstrCopy(self->m_originalInstr, &self->m_gi);

   *Scheduler_Resources(self->m_scheduler) = self->m_resources;

   if (self->m_setNextTextureUniform)
      Scheduler_SetNextTextureUniform(self->m_scheduler, self->m_nextTextureUniformUnit, self->m_nextTextureUniformValue);

   if (self->m_insertThreadSwitch)
      Scheduler_InsertThreadSwitch(self->m_scheduler);

   for (i = 0; i < NodeResultList_size(&self->m_nodeResults); i++)
   {
      NodeResult  *result = NodeResultList_index(&self->m_nodeResults, i);

      if (result != NULL)
         result->m_node->m_result = result->m_result;
   }

   if (OptionalBypass_IsSet(&self->m_optionalBypass))
      OptionalBypass_Get(&self->m_optionalBypass)->m_bypassed = true;

   if (self->m_isPendingUniform)
   {
      UniformVector_push_back(Scheduler_Uniforms(self->m_scheduler), &self->m_pendingUniform);
      self->m_isPendingUniform = false;
   }

   if (self->m_isPendingVarying)
   {
      VaryingVector_push_back(Scheduler_Varyings(self->m_scheduler), &self->m_pendingVarying);
      self->m_isPendingVarying = false;
   }

   return DFlowNode_SCHEDULED;
}

void InstrState_AddNodeResult(InstrState *self, DFlowNode *node, const QPUOperand *result)
{
   NodeResult  nodeResult;
   NodeResult_Constr(&nodeResult, node, result);

   NodeResultList_push_back(&self->m_nodeResults, &nodeResult);
}

DFlowNode_ScheduleStatus InstrState_CommitChangesOp(InstrState *self, DFlowNode *resultNode, const QPUOperand *resultOperand)
{
   InstrState_AddNodeResult(self, resultNode, resultOperand);
   return InstrState_CommitChanges(self);
}

DFlowNode_ScheduleStatus InstrState_CommitChangesRes(InstrState *self, DFlowNode *resultNode, QPUResource *result)
{
   QPUOperand  resultOperand;
   QPUOperand_ConstrResource(&resultOperand, result);
   return InstrState_CommitChangesOp(self, resultNode, &resultOperand);
}

DFlowNode_ScheduleStatus InstrState_CommitChangesReg(InstrState *self, DFlowNode *resultNode, Register_Enum result)
{
   QPUOperand  resultOperand;
   QPUOperand_ConstrReg(&resultOperand, result);
   return InstrState_CommitChangesOp(self, resultNode, &resultOperand);
}

bool InstrState_IsFragmentShader(InstrState *self)
{
   return Scheduler_IsFragmentShader(self->m_scheduler);
}

bool InstrState_IsRegisterInUse(const InstrState *self, Register_Enum reg)
{
   return QPUResource_IsReferenced(QPUResources_GetResourceConst(&self->m_resources, reg));
}

void InstrState_SetWrittenAt(InstrState *self, Register_Enum reg, int32_t extraDelay)
{
   QPUResource_SetWrittenAt(QPUResources_GetResource(&self->m_resources, reg), self->m_slot + extraDelay);
}

void InstrState_SetWrittenAtDef(InstrState *self, Register_Enum reg)
{
   InstrState_SetWrittenAt(self, reg, 0);
}

void InstrState_SetReferenced(InstrState *self, Register_Enum reg, DFlowNode *owner, int32_t count)
{
   QPUResource_SetReferenced(QPUResources_GetResource(&self->m_resources, reg), owner, count);
}

void InstrState_SetReferencedDef(InstrState *self, Register_Enum reg, DFlowNode *owner)
{
   InstrState_SetReferenced(self, reg, owner, 1);
}

void InstrState_AddUniform(InstrState *self, uint32_t uniformType, uint32_t value)
{
   self->m_isPendingUniform = true;
   Uniform_Constr(&self->m_pendingUniform, uniformType, value);
}

void InstrState_AddUniformRead(InstrState *self, DFlowNode *from)
{
   uint32_t type, val;

   switch (from->m_flavour)
   {
   case DATAFLOW_CONST_INT :
      type = BACKEND_UNIFORM_LITERAL;
      val = const_float_from_int(from->m_uniform.m_constInt.m_ciValue);
      break;
   case DATAFLOW_CONST_FLOAT :
      type = BACKEND_UNIFORM_LITERAL;
      val = from->m_uniform.m_constFloat.m_cfValue;
      break;
   case DATAFLOW_UNIFORM :
      type = BACKEND_UNIFORM;
      val = from->m_uniform.m_linkableValue.m_lvRow;
      break;
   case DATAFLOW_UNIFORM_ADDRESS :
      type = BACKEND_UNIFORM_ADDRESS;
      val = from->m_uniform.m_indexedUniformSampler.m_iusAddress;
      //val = from->m_uniform.m_indexedUniformSampler.m_size << 16 | from->m_args[DFlowNode_ARG_UNIFORM]->m_uniform.m_linkableValue.m_row;
      break;
   case DATAFLOW_UNIFORM_OFFSET :
      UNREACHABLE(); // TODO?
      break;
   default:
      UNREACHABLE();
      return;
   }

   // Ensure we've not added a uniform in this instruction already
   //assert(m_uniforms.size() == m_scheduler->Uniforms().size());
   assert(!self->m_isPendingUniform);
   InstrState_AddUniform(self, type, val);
}

bool InstrState_IsReadableOp(const InstrState *self, const QPUOperand *op)
{
   return QPUOperand_IsReadable(op, self->m_slot, &self->m_resources);
}

bool InstrState_IsReadableReg(InstrState *self, Register_Enum reg)
{
   QPUOperand  op;
   QPUOperand_ConstrReg(&op, reg);

   return InstrState_IsReadableOp(self, &op);
}

DFlowNode *InstrState_GetOwner(const InstrState *self, Register_Enum reg)
{
   return QPUResources_GetOwner(&self->m_resources, reg);
}

bool InstrState_AddVaryingRead(InstrState *self, DFlowNode *node)
{
   int32_t vary;

   // Need access to R5. We'll check whether there is anything in R5 that hasn't been consumed yet.
   // If there is, we'll move it somewhere before overwriting it.
   if (InstrState_IsRegisterInUse(self, Register_ACC5))
   {
      if (!InstrState_IsReadableReg(self, Register_ACC5))
         return false;

      Scheduler_SetNeedsACC5Mov(self->m_scheduler, InstrState_GetOwner(self, Register_ACC5), true);
      return false;
   }

   vary = node->m_uniform.m_linkableValue.m_lvRow;

   if (vary >= 32)
   {
      assert(!self->m_isPendingVarying);
      self->m_isPendingVarying = true;
      self->m_pendingVarying = vary - 32;
   }

   // Find the VARY_C node
   {
      DFlowNode               *varyc = NULL;
      NodeList_const_iterator  iter;
      const NodeList          *ioParents = DFlowNode_IoParents(node);

      for (iter = NodeList_const_begin(ioParents); iter != NodeList_const_end(ioParents); NodeList_const_next(&iter))
      {
         DFlowNode   *starIter = NodeList_const_star(iter);

         if (DFlowNode_Flavour(starIter) == DATAFLOW_VARYING_C)
         {
            varyc = starIter;
            break;
         }
      }

      // Mark ACC5 in use to prevent further varying reads until vary c is read
      InstrState_SetWrittenAtDef(self, Register_ACC5);
      InstrState_SetReferencedDef(self, Register_ACC5, varyc);

      {
         QPUOperand  acc5;
         QPUOperand_ConstrReg(&acc5, Register_ACC5);
         InstrState_AddNodeResult(self, varyc, &acc5);
      }
   }

   return true;
}

bool InstrState_IsWritable(InstrState *self, Register_Enum r)
{
   bool  ret = true;

   if (Register_IsTMUSetupWrite(r))
   {
      if (Scheduler_FirstEmptySlot(self->m_scheduler) < 2)
         ret = false;

      // Prevent TMU swap from overlapping
      if (!InstrState_IsReadableReg(self, Register_VIRTUAL_TMUWRITE))
         ret = false;
   }

   return ret;
}

bool InstrState_InsertThreadSwitch(InstrState *self)
{
   if (Scheduler_CanInsertThreadSwitch(self->m_scheduler))
   {
      self->m_insertThreadSwitch = true;
      // Prevent any further stuff in this instruction (NOTE: some instructions could
      // go in the slot, but we would have to check that r0->r5 were not written explicitly
      // or implicitly)
      QPUGenericInstr_SetFull(&self->m_gi);
   }

   return self->m_insertThreadSwitch;
}

uint32_t InstrState_GetNextTextureUniform(const InstrState *self, uint32_t unit)
{
   return Scheduler_GetNextTextureUniform(self->m_scheduler, unit);
}

void InstrState_SetNextTextureUniform(InstrState *self, uint32_t unit, uint32_t value)
{
   self->m_setNextTextureUniform   = true;
   self->m_nextTextureUniformUnit  = unit;
   self->m_nextTextureUniformValue = value;
}

QPUResource *InstrState_GetResource(InstrState *self, Register_Enum reg)
{
   return QPUResources_GetResource(&self->m_resources, reg);
}

const QPUResource *InstrState_GetResourceConst(const InstrState *self, Register_Enum reg)
{
   return QPUResources_GetResourceConst(&self->m_resources, reg);
}

void InstrState_RetireOp(InstrState *self, DFlowNode *node, const QPUOperand *op)
{
   if (QPUOperand_IsBypass(op))
   {
      DFlowNode *child;

      // Special handling for bypass nodes (unpacks)
      assert(node != NULL);
      child = NodeList_front(DFlowNode_Children(node));
      assert(QPUOperand_GetType(&child->m_result) != QPUOperand_BYPASS);
      InstrState_RetireOp(self, child, &child->m_result);
   }
   else
      QPUOperand_Retire(op, &self->m_resources);
}

void InstrState_RetireReg(InstrState *self, DFlowNode *node, Register_Enum reg)
{
   QPUOperand  op;
   QPUOperand_ConstrReg(&op, reg);
   InstrState_RetireOp(self, node, &op);
}

void InstrState_MoveRefCount(InstrState *self, DFlowNode *node, const QPUOperand *from, const QPUOperand *to)
{
   // We have to deal with a special case here.
   // If we are moving from an unpack (bypassed) node, we have to do something a little different
   if (QPUOperand_IsRegister(from))
   {
      if (QPUOperand_IsBypass(&node->m_result))
      {
         // We don't actually want to move the ref count as the underlying bypass node isn't actually ours
         NodeList_const_iterator iter;
         DFlowNode *valueNode = NodeList_front(DFlowNode_Children(node));
         const NodeList *parents = DFlowNode_Parents(node);

         assert(valueNode == InstrState_GetOwner(self, QPUOperand_ValueRegister(from)));
         // We need to retire enough times to cover all our unscheduled parents
         for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
         {
            DFlowNode   *starIter = NodeList_const_star(iter);

            if (DFlowNode_HasUnconsumedReference(starIter, self->m_slot))
               InstrState_RetireOp(self, valueNode, &valueNode->m_result);
         }

         return;
      }
   }

   QPUResources_MoveRefCount(&self->m_resources, QPUOperand_ValueRegister(from), QPUOperand_ValueRegister(to));
   QPUResource_FullyUnreference(InstrState_GetResource(self, QPUOperand_ValueRegister(from)));
}

QPUResource *InstrState_FindFreeResource(InstrState *self, DFlowRegFile file, QPUResources_Preference pref)
{
   return QPUResources_FindFreeResource(&self->m_resources, file, self->m_outputRegistersFree, pref);
}

///////////////////////////////////////////////////////////////////////////////
// DFlowNode
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE
///////////////////////////////////////////////////////////////////////////////

// static methods
static void     DFlowNode_AddToList(NodeList *set, DFlowNode *node) { NodeList_push_back(set, node); }

static uint32_t DFlowNode_RemoveFromList(NodeList *set, DFlowNode *node);
static void     DFlowNode_SetWrittenAt(InstrState *state, Register_Enum reg, int32_t extraDelay /*= 0*/);


// non-static (in the class sense) methods
void DFlowNode_Initialize(DFlowNode *self, DataflowFlavour flavour, BOOL_REP_T boolRep, ResetHelper *rh);

static DFlowNode_ScheduleStatus DFlowNode_InsertExtraMov(DFlowNode *self, const QPUOperand *operand, InstrState *state,
                                                         DFlowNode_RetireMode retMode /* = DFlowNode_FULLY_RETIRE */, QPUResources_Preference pref/* = QPUResources_PREFER_ACC */);


static void DFlowNode_AddChildren(DFlowNode *self, Dataflow *dataFlow);
static void DFlowNode_AddChild(DFlowNode *self, DFlowNode_ArgIndex ix, Dataflow *dataFlow);

static bool DFlowNode_LooksSchedulable(DFlowNode *self, QPUGenericInstr *genericInstr);

static DFlowNode_ScheduleStatus DFlowNode_SetupForTwoOperands(DFlowNode *self, QPUOperand *leftOp, QPUOperand *rightOp, InstrState *state);

static bool DFlowNode_SetupForSingleOperand(DFlowNode *self, QPUOperand *operand, InstrState *state);

static void DFlowNode_SetReferenced(DFlowNode *self, InstrState *state, Register_Enum reg, int32_t overrideCount /* = 0 */);

static QPUResource *DFlowNode_AllocateOutputReg(DFlowNode *self, InstrState *state, DFlowNode_AllocRegMode mode, QPUResources_Preference p);
static DFlowNode_ScheduleStatus DFlowNode_SpecialRegWrite(DFlowNode *self, DFlowNode *from, Register_Enum to, InstrState *state, DFlowNode *conditional /*  = NULL */);

static DFlowNode_ScheduleStatus DFlowNode_TextureWrite(DFlowNode *self, InstrState *state, Scheduler *sched);

static DFlowNode_ScheduleStatus DFlowNode_ReadSpecialRegister(DFlowNode *self, InstrState *state, Register_Enum from);
static void DFlowNode_RetireOurChildren(DFlowNode *self, InstrState *state);

static bool DFlowNode_ExtraMovNeeded(const QPUOperand *leftOp, const QPUOperand *rightOp);

static bool DFlowNode_SetPackedMov(DFlowNode *self, InstrState *state, const QPUOperand *from, const QPUOperand *to);

static DFlowNode_ScheduleStatus DFlowNode_DoConstFloat(DFlowNode *self, InstrState *state);
static DFlowNode_ScheduleStatus DFlowNode_DoConstInt(DFlowNode *self, InstrState *state);
static bool DFlowNode_SetConditionFlags(DFlowNode *self, InstrState *state, DFlowNode *conditionNode);

static const QPUOperand *DFlowNode_GetInput(DFlowNode *self, InstrState *state, DFlowNode *from);
static const QPUOperand *DFlowNode_PeekInput(DFlowNode *self, InstrState *state, DFlowNode *from);

static AOP_Enum DFlowNode_GetAddOp(const DFlowNode *self);
static MOP_Enum DFlowNode_GetMulOp(const DFlowNode *self);
static VirtualUnpack_Enum DFlowNode_GetUnpackCode(const DFlowNode *self);

static Register_Enum  DFlowNode_GetSFUReg(const DFlowNode *self);
static Register_Enum  DFlowNode_GetIOInputReg(const DFlowNode *self);
static Register_Enum  DFlowNode_GetIOOutputReg(const DFlowNode *self);
static CondCode_Enum  DFlowNode_GetCondCode(const DFlowNode *self);
static Sig_Enum       DFlowNode_GetSignal(const DFlowNode *self);
static bool           DFlowNode_GetSpecificOutputReg(DFlowNode *self, InstrState *state, QPUResource **output);
static bool           DFlowNode_CanShortcutPack(DFlowNode *self, InstrState *state);
static bool           DFlowNode_IsMulOperation(const DFlowNode *self);
static Mul_Pack_Enum  DFlowNode_GetMulPackCode(const DFlowNode *self);
static RegA_Pack_Enum DFlowNode_GetRegAPackCode(const DFlowNode *self);

static int32_t        DFlowNode_RealRegistersRead(DFlowNode *self);
static int32_t        DFlowNode_RealRegistersWritten(DFlowNode *self);
static bool           DFlowNode_RecordUniformsAndVaryings(DFlowNode *self, const QPUOperand *operand, InstrState *state);

///////////////////////////////////////////////////////////////////////////////
// Public
///////////////////////////////////////////////////////////////////////////////

void DFlowNode_Initialize(DFlowNode *self, DataflowFlavour flavour, BOOL_REP_T boolRep, ResetHelper *resetHelper)
{
   uint32_t i;

   self->m_uniqueId = DFlowNode_nextId++;

   NodeList_Constr(&self->m_parents);
   NodeList_Constr(&self->m_children);
   NodeList_Constr(&self->m_ioChildren);
   NodeList_Constr(&self->m_ioParents);

   self->m_flavour = flavour;
   self->m_boolRep = boolRep;

   self->m_sampler               = 0;
   self->m_visitorId             = 0;
   self->m_treeDepth             = -1;
   self->m_registerDelta         = 0;
   self->m_lifespanGuess         = 0;
   self->m_order                 = 0;
   self->m_resetHelper           = resetHelper;
   self->m_visitorOpaqueData     = NULL;
   self->m_replicant             = NULL;
   self->m_wantReplicate         = false;
   self->m_subtreeRoot           = false;
   self->m_numRecursiveChildren  = 0;
   self->m_numReferences         = 0;
   self->m_slot                  = DFlowNode_NOT_SCHEDULED;
   QPUOperand_ConstrReg(&self->m_result, Register_NOP);
   QPUOperand_ConstrReg(&self->m_tempResult, Register_NOP);
   self->m_bypassed              = false;
   self->m_delayedCondition      = false;
   self->m_delayedLoadc          = false;
   self->m_schedPass             = 0;
   self->m_sortOrder             = 0;
   self->m_threadSwitch          = false;
   self->m_tmuOrdered            = false;
   self->m_issueCount            = 0;
   self->m_regFile               = DFlowRegFile_ANY;

   for (i = 0; i < DFlowNode_ARG_COUNT; ++i)
      self->m_args[i] = NULL;
}

void DFlowNode_Flavour_Constr(DFlowNode *self, DataflowFlavour flavour, ResetHelper *rh)
{
   DFlowNode_Initialize(self, flavour, BOOL_REP_NONE, rh);
}

void DFlowNode_Dataflow_Constr(DFlowNode *self, Dataflow *dataFlow, ResetHelper *rh)
{
   DataflowChainNode *n;

   assert(dataFlow->bcg_helper == NULL);

   DFlowNode_Initialize(self, dataFlow->flavour, dataFlow->bool_rep, rh);

   if (self->m_resetHelper)
   {
      ResetHelper_AddDataflow(self->m_resetHelper, dataFlow);
      //ResetHelper_AddDFlowNode(self->m_resetHelper, self);
   }

   // Set the back-pointer in the dataflow so we know which C++ DFlowNode it relates to
   dataFlow->bcg_helper = (void*)self;

   // Copy and mirror arguments and create child list
   DFlowNode_AddChildren(self, dataFlow);

   // Build ioChild list
   for (n = dataFlow->iodependencies.first; n != NULL; n = n->next)
   {
      DFlowNode *node = NULL;
      if (n->dataflow->bcg_helper == NULL)
         node = DFlowNode_Dataflow_new(n->dataflow, self->m_resetHelper);
      else
         node = (DFlowNode*)n->dataflow->bcg_helper;

      DFlowNode_AddToList(&self->m_ioChildren, node);
   }

   // Build parents list
   for (n = dataFlow->dependents.first; n != NULL; n = n->next)
   {
      DFlowNode *node = NULL;
      if (n->dataflow->bcg_helper == NULL)
         node = DFlowNode_Dataflow_new(n->dataflow, self->m_resetHelper);
      else
         node = (DFlowNode*)n->dataflow->bcg_helper;

      DFlowNode_AddToList(&self->m_parents, node);
   }

   // Build ioParents list
   for (n = dataFlow->iodependents.first; n != NULL; n = n->next)
   {
      DFlowNode *node = NULL;
      if (n->dataflow->bcg_helper == NULL)
         node = DFlowNode_Dataflow_new(n->dataflow, self->m_resetHelper);
      else
         node = (DFlowNode*)n->dataflow->bcg_helper;

      DFlowNode_AddToList(&self->m_ioParents, node);
   }
}

DFlowNode *DFlowNode_Flavour_new(DataflowFlavour flavour, ResetHelper *rh)
{
   DFlowNode   *node = (DFlowNode *)bcg_glsl_malloc(sizeof(DFlowNode));

   DFlowNode_Flavour_Constr(node, flavour, rh);

   return node;
}

DFlowNode *DFlowNode_Dataflow_new(Dataflow *dataflow, ResetHelper *rh)
{
   DFlowNode   *node = (DFlowNode *)bcg_glsl_malloc(sizeof(DFlowNode));

   DFlowNode_Dataflow_Constr(node, dataflow, rh);

   return node;
}

#if 0

DFlowNode *DFlowNode_DuplicateTree(DFlowNode *self)
{
   DFlowNode *newNode;
   DFlowNode *node;
   uint32_t  i;

   if (!self->m_wantReplicate)
      return self;

   assert(self->m_replicant == NULL);

   newNode = (DFlowNode *)bcg_glsl_malloc(sizeof(DFlowNode));
   // TODO DFlowNode_PartialCopy(newNode, self);

   // TODO -- I don't think these are necessary
   //newNode->m_children.clear();
   //newNode->m_ioChildren.clear();
   //newNode->m_parents.clear();
   //newNode->m_ioParents.clear();

   //for (uint32_t i = 0; i < ARG_COUNT; ++i)
   //   newNode->m_args[i] = NULL;

   // Store the cloned node in the original node
   self->m_replicant = newNode;

   // Copy and mirror arguments and create child list
   for (i = 0; i < DFlowNode_ARG_COUNT; ++i)
   {
      if (self->m_args[i] != NULL)
      {
         if (self->m_args[i]->m_replicant == NULL)
            node = DFlowNode_DuplicateTree(self->m_args[i]);
         else
            node = self->m_args[i]->m_replicant;

         DFlowNode_AddToList(&newNode->m_children, node);

         newNode->m_args[i] = node;
      }
   }

   // Build ioChild list
   {
      NodeList_iterator iter;

      for (iter = NodeList_begin(&self->m_ioChildren); iter != NodeList_end(&self->m_ioChildren); NodeList_next(&iter))
      {
         DFlowNode   *starIter = NodeList_star(iter);

         if (starIter->m_replicant == NULL)
            node = DFlowNode_DuplicateTree(starIter);
         else
            node = starIter->m_replicant;

         DFlowNode_AddToList(&newNode->m_ioChildren, node);
      }

      // Build parents list
      for (iter = NodeList_begin(&self->m_parents); iter != NodeList_end(&self->m_parents); NodeList_next(&iter))
      {
         DFlowNode   *starIter = NodeList_star(iter);

         if (starIter->m_replicant == NULL)
            node = DFlowNode_DuplicateTree(starIter);
         else
            node = starIter->m_replicant;

         DFlowNode_AddToList(&newNode->m_parents, node);
      }

      // Build ioParents list
      for (iter = NodeList_begin(&self->m_ioParents); iter != NodeList_end(&self->m_ioParents); NodeList_next(&iter))
      {
         DFlowNode   *starIter = NodeList_star(iter);

         if (starIter->m_replicant == NULL)
            node = DFlowNode_DuplicateTree(starIter);
         else
            node = starIter->m_replicant;

         DFlowNode_AddToList(&newNode->m_ioParents, node);
      }
   }

   return newNode;
}

#endif

void DFlowNode_Destr(DFlowNode *self)
{
   NodeList_Destr(&self->m_parents);
   NodeList_Destr(&self->m_children);
   NodeList_Destr(&self->m_ioChildren);
   NodeList_Destr(&self->m_ioParents);
}

void DFlowNode_AddChild(DFlowNode *self, DFlowNode_ArgIndex ix, Dataflow *dataFlow)
{
   DFlowNode   *node = NULL;

   if (dataFlow != NULL)
   {
      if (dataFlow->bcg_helper == NULL)
         node = DFlowNode_Dataflow_new(dataFlow, self->m_resetHelper);
      else
         node = (DFlowNode*)dataFlow->bcg_helper;

      DFlowNode_AddToList(&self->m_children, node);
   }

   self->m_args[ix] = node;
}

void DFlowNode_AddChildNode(DFlowNode *self, DFlowNode_ArgIndex ix, DFlowNode *child)
{
   DFlowNode_AddToList(&self->m_children, child);
   self->m_args[ix] = child;
   DFlowNode_AddParent(child, self);
}

void DFlowNode_AddChildren(DFlowNode *self, Dataflow *dataFlow)
{
   uint32_t i;

   for (i = 0; i < DFlowNode_ARG_COUNT; ++i)
      self->m_args[i] = NULL;

   switch (dataFlow->flavour)
   {
   case DATAFLOW_UNIFORM_OFFSET:
      DFlowNode_AddChild(self, DFlowNode_ARG_LINKABLE_VALUE, dataFlow->d.linkable_value_offset.linkable_value);
      DFlowNode_AddChild(self, DFlowNode_ARG_OFFSET,         dataFlow->d.linkable_value_offset.offset);
      break;

   case DATAFLOW_ARITH_NEGATE:
   case DATAFLOW_INTRINSIC_RSQRT:
   case DATAFLOW_INTRINSIC_RCP:
   case DATAFLOW_INTRINSIC_LOG2:
   case DATAFLOW_INTRINSIC_EXP2:
   case DATAFLOW_INTRINSIC_CEIL:
   case DATAFLOW_INTRINSIC_FLOOR:
   case DATAFLOW_INTRINSIC_SIGN:
   case DATAFLOW_MOV:
   case DATAFLOW_FTOI_TRUNC:
   case DATAFLOW_FTOI_NEAREST:
   case DATAFLOW_ITOF:
   case DATAFLOW_UNPACK_COL_R:
   case DATAFLOW_UNPACK_COL_G:
   case DATAFLOW_UNPACK_COL_B:
   case DATAFLOW_UNPACK_COL_A:
   case DATAFLOW_UNPACK_16A:
   case DATAFLOW_UNPACK_16A_F:
   case DATAFLOW_UNPACK_16B:
   case DATAFLOW_UNPACK_16B_F:
   case DATAFLOW_UNPACK_8A:
   case DATAFLOW_UNPACK_8B:
   case DATAFLOW_UNPACK_8C:
   case DATAFLOW_UNPACK_8D:
   case DATAFLOW_UNPACK_8R:
   case DATAFLOW_LOGICAL_NOT:
   case DATAFLOW_BITWISE_NOT:
      DFlowNode_AddChild(self, DFlowNode_ARG_OPERAND, dataFlow->d.unary_op.operand);
      break;

   case DATAFLOW_MUL:
   case DATAFLOW_ADD:
   case DATAFLOW_SUB:
   case DATAFLOW_RSUB:
   case DATAFLOW_LESS_THAN:
   case DATAFLOW_LESS_THAN_EQUAL:
   case DATAFLOW_GREATER_THAN:
   case DATAFLOW_GREATER_THAN_EQUAL:
   case DATAFLOW_EQUAL:
   case DATAFLOW_NOT_EQUAL:
   case DATAFLOW_LOGICAL_AND:
   case DATAFLOW_LOGICAL_XOR:
   case DATAFLOW_LOGICAL_OR:
   case DATAFLOW_INTRINSIC_MIN:
   case DATAFLOW_INTRINSIC_MAX:
   case DATAFLOW_INTRINSIC_MINABS:
   case DATAFLOW_INTRINSIC_MAXABS:
   case DATAFLOW_BITWISE_AND:
   case DATAFLOW_BITWISE_OR:
   case DATAFLOW_BITWISE_XOR:
   case DATAFLOW_V8MULD:
   case DATAFLOW_V8MIN:
   case DATAFLOW_V8MAX:
   case DATAFLOW_V8ADDS:
   case DATAFLOW_V8SUBS:
   case DATAFLOW_INTEGER_ADD:
   case DATAFLOW_SHIFT_RIGHT:
   case DATAFLOW_LOGICAL_SHR:
   case DATAFLOW_UNPACK_PLACEHOLDER_R:
   case DATAFLOW_UNPACK_PLACEHOLDER_B:
      DFlowNode_AddChild(self, DFlowNode_ARG_LEFT,        dataFlow->d.binary_op.left);
      DFlowNode_AddChild(self, DFlowNode_ARG_RIGHT,       dataFlow->d.binary_op.right);
      break;

   case DATAFLOW_CONDITIONAL:
      DFlowNode_AddChild(self, DFlowNode_ARG_COND,        dataFlow->d.cond_op.cond);
      DFlowNode_AddChild(self, DFlowNode_ARG_TRUE_VALUE,  dataFlow->d.cond_op.true_value);
      DFlowNode_AddChild(self, DFlowNode_ARG_FALSE_VALUE, dataFlow->d.cond_op.false_value);
      break;

   case DATAFLOW_TEX_SET_COORD_S:
   case DATAFLOW_TEX_SET_COORD_T:
   case DATAFLOW_TEX_SET_COORD_R:
   case DATAFLOW_TEX_SET_LOD:
   case DATAFLOW_TEX_SET_BIAS:
   case DATAFLOW_TEX_SET_DIRECT:
      DFlowNode_AddChild(self, DFlowNode_ARG_PARAM,       dataFlow->d.texture_lookup_set.param);
      DFlowNode_AddChild(self, DFlowNode_ARG_SAMPLER,     dataFlow->d.texture_lookup_set.sampler);
      break;

   case DATAFLOW_TEX_GET_CMP_R:
   case DATAFLOW_TEX_GET_CMP_G:
   case DATAFLOW_TEX_GET_CMP_B:
   case DATAFLOW_TEX_GET_CMP_A:
      DFlowNode_AddChild(self, DFlowNode_ARG_SAMPLER,     dataFlow->d.texture_lookup_get.sampler);
      break;

   case DATAFLOW_FRAG_SUBMIT_STENCIL:
   case DATAFLOW_FRAG_SUBMIT_Z:
   case DATAFLOW_FRAG_SUBMIT_MS:
   case DATAFLOW_FRAG_SUBMIT_ALL:
   case DATAFLOW_FRAG_SUBMIT_R0:
   case DATAFLOW_FRAG_SUBMIT_R1:
   case DATAFLOW_FRAG_SUBMIT_R2:
   case DATAFLOW_FRAG_SUBMIT_R3:
   case DATAFLOW_TMU_SWAP:
      DFlowNode_AddChild(self, DFlowNode_ARG_PARAM,       dataFlow->d.fragment_set.param);
      DFlowNode_AddChild(self, DFlowNode_ARG_DISCARD,     dataFlow->d.fragment_set.discard);
      break;

   case DATAFLOW_VERTEX_SET:
   case DATAFLOW_VPM_READ_SETUP:
   case DATAFLOW_VPM_WRITE_SETUP:
      DFlowNode_AddChild(self, DFlowNode_ARG_PARAM,       dataFlow->d.vertex_set.param);
      break;

   case DATAFLOW_PACK_COL_REPLICATE:
      DFlowNode_AddChild(self, DFlowNode_ARG_OPERAND,     dataFlow->d.pack.operand);
      break;

   case DATAFLOW_PACK_COL_R:
   case DATAFLOW_PACK_COL_G:
   case DATAFLOW_PACK_COL_B:
   case DATAFLOW_PACK_COL_A:
   case DATAFLOW_PACK_16A:
   case DATAFLOW_PACK_16B:
      DFlowNode_AddChild(self, DFlowNode_ARG_OPERAND,     dataFlow->d.pack.operand);
      DFlowNode_AddChild(self, DFlowNode_ARG_BACKGROUND,  dataFlow->d.pack.background);
      break;

   case DATAFLOW_CONST_BOOL:
      self->m_uniform.m_constBool.m_cbValue = dataFlow->u.const_bool.value;
      self->m_uniform.m_constBool.m_cbIndex = dataFlow->u.const_bool.index;
      break;

   case DATAFLOW_CONST_FLOAT:
      self->m_uniform.m_constFloat.m_cfValue = dataFlow->u.const_float.value;
      self->m_uniform.m_constFloat.m_cfIndex = dataFlow->u.const_float.index;
      break;

   case DATAFLOW_CONST_SAMPLER:
      self->m_uniform.m_constSampler.m_csLocation = dataFlow->u.const_sampler.location;
      self->m_uniform.m_constSampler.m_csName     = dataFlow->u.const_sampler.name;
      break;

   case DATAFLOW_UNIFORM:
   case DATAFLOW_ATTRIBUTE:
   case DATAFLOW_VARYING:
      self->m_uniform.m_linkableValue.m_lvRow  = dataFlow->u.linkable_value.row;
      self->m_uniform.m_linkableValue.m_lvName = dataFlow->u.linkable_value.name;
      break;

   case DATAFLOW_CONST_INT:
      self->m_uniform.m_constInt.m_ciValue = dataFlow->u.const_int.value;
      self->m_uniform.m_constInt.m_ciIndex = dataFlow->u.const_int.index;
      break;

   case DATAFLOW_UNIFORM_ADDRESS:
      //AddChild(ARG_UNIFORM,     dataFlow->d.indexed_uniform_sampler.uniform);
      self->m_uniform.m_indexedUniformSampler.m_iusAddress = dataFlow->u.indexed_uniform_sampler.size << 16 |
                                                          dataFlow->d.indexed_uniform_sampler.uniform->u.linkable_value.row;
      break;

   case DATAFLOW_VARYING_C:
   case DATAFLOW_FRAG_GET_X:
   case DATAFLOW_FRAG_GET_Y:
   case DATAFLOW_FRAG_GET_Z:
   case DATAFLOW_FRAG_GET_W:
   case DATAFLOW_FRAG_GET_PC_X:
   case DATAFLOW_FRAG_GET_PC_Y:
   case DATAFLOW_FRAG_GET_FF:
   case DATAFLOW_FRAG_GET_COL:
   case DATAFLOW_SCOREBOARD_WAIT:
      break;

   default:
      assert(0);
      break;
   }
}



uint32_t DFlowNode_NumChildren(const DFlowNode *self)
{
   return NodeList_size(DFlowNode_Children(self)) + NodeList_size(DFlowNode_IoChildren(self));
}

CondCode_Enum DFlowNode_GetCondCode(const DFlowNode *self)
{
   switch (self->m_boolRep)
   {
   default              :
   case BOOL_REP_NONE   : assert(0); return CondCode_NEVER;
   case BOOL_REP_NEG    : return CondCode_NC;
   case BOOL_REP_NEG_N  : return CondCode_NS;
   case BOOL_REP_BOOL_N :
   case BOOL_REP_ZERO   : return CondCode_ZC;
   case BOOL_REP_BOOL   :
   case BOOL_REP_ZERO_N : return CondCode_ZS;
   }
}

Mul_Pack_Enum DFlowNode_GetMulPackCode(const DFlowNode *self)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_PACK_COL_REPLICATE :
   case DATAFLOW_PACK_COL_R  :
   case DATAFLOW_PACK_COL_G  :
   case DATAFLOW_PACK_COL_B  :
   case DATAFLOW_PACK_COL_A  :
      return (Mul_Pack_Enum)(Mul_Pack_8R + (self->m_flavour - DATAFLOW_PACK_COL_REPLICATE));

   default:
      return Mul_Pack_NONE;
   }
}

RegA_Pack_Enum DFlowNode_GetRegAPackCode(const DFlowNode *self)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_PACK_COL_REPLICATE :
   case DATAFLOW_PACK_COL_R  :
   case DATAFLOW_PACK_COL_G  :
   case DATAFLOW_PACK_COL_B  :
   case DATAFLOW_PACK_COL_A  :
      assert(0);   // These are float operations, cannot be done with regA packing
      return (RegA_Pack_Enum)(RegA_Pack_8R_S + (self->m_flavour - DATAFLOW_PACK_COL_REPLICATE));
   case DATAFLOW_PACK_16A    :
   case DATAFLOW_PACK_16B    :
      return (RegA_Pack_Enum)(RegA_Pack_16A + (self->m_flavour - DATAFLOW_PACK_16A));

   default:
      return RegA_Pack_NONE;
   }
}

VirtualUnpack_Enum DFlowNode_GetUnpackCode(const DFlowNode *self)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_UNPACK_COL_R: return VirtualUnpack_COL_R;
   case DATAFLOW_UNPACK_COL_G: return VirtualUnpack_COL_G;
   case DATAFLOW_UNPACK_COL_B: return VirtualUnpack_COL_B;
   case DATAFLOW_UNPACK_COL_A: return VirtualUnpack_COL_A;
   case DATAFLOW_UNPACK_16A  : return VirtualUnpack_16A;
   case DATAFLOW_UNPACK_16A_F: return VirtualUnpack_16A_F;
   case DATAFLOW_UNPACK_16B  : return VirtualUnpack_16B;
   case DATAFLOW_UNPACK_16B_F: return VirtualUnpack_16B_F;
   case DATAFLOW_UNPACK_8A   : return VirtualUnpack_8A;
   case DATAFLOW_UNPACK_8B   : return VirtualUnpack_8B;
   case DATAFLOW_UNPACK_8C   : return VirtualUnpack_8C;
   case DATAFLOW_UNPACK_8D   : return VirtualUnpack_8D;
   case DATAFLOW_UNPACK_8R   : return VirtualUnpack_8R;
   default                   : return VirtualUnpack_NONE;
   }
}

bool DFlowNode_CheckPackUnpackClash(DFlowNode *self, const QPUOperand *from)
{
   if (QPUOperand_IsRegister(from) && QPUOperand_GetUnpack(from) != VirtualUnpack_NONE)
   {
      bool unpackFloat = VirtualUnpack_IsFloat(QPUOperand_GetUnpack(from));

      // If it's a float unpack, it must be done in the ADD unit
      if (unpackFloat)
      {
         if (DFlowNode_GetMulPackCode(self) != Mul_Pack_NONE)
            return false;
      }
   }

   return true;
}

bool DFlowNode_IsAddOperation(const DFlowNode *self)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_ADD               :
   case DATAFLOW_SUB               :
   case DATAFLOW_RSUB              :
   case DATAFLOW_LESS_THAN         :
   case DATAFLOW_LESS_THAN_EQUAL   :
   case DATAFLOW_GREATER_THAN      :
   case DATAFLOW_GREATER_THAN_EQUAL:
   case DATAFLOW_EQUAL             :
   case DATAFLOW_NOT_EQUAL         :
   case DATAFLOW_LOGICAL_AND       :
   case DATAFLOW_LOGICAL_XOR       :
   case DATAFLOW_LOGICAL_OR        :
   case DATAFLOW_BITWISE_NOT       :
   case DATAFLOW_BITWISE_AND       :
   case DATAFLOW_BITWISE_OR        :
   case DATAFLOW_BITWISE_XOR       :
   case DATAFLOW_V8ADDS            :
   case DATAFLOW_V8SUBS            :
   case DATAFLOW_INTEGER_ADD       :
   case DATAFLOW_INTRINSIC_MIN     :
   case DATAFLOW_INTRINSIC_MAX     :
   case DATAFLOW_INTRINSIC_MINABS  :
   case DATAFLOW_INTRINSIC_MAXABS  :
   case DATAFLOW_FTOI_TRUNC        :
   case DATAFLOW_FTOI_NEAREST      :
   case DATAFLOW_ITOF              :
   case DATAFLOW_SHIFT_RIGHT       :
   case DATAFLOW_LOGICAL_SHR       :
      return true;

   default:
      return false;
   }
}

bool DFlowNode_IsMulOperation(const DFlowNode *self)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_MUL               :
   case DATAFLOW_V8MULD            :
   case DATAFLOW_V8MIN             :
   case DATAFLOW_V8MAX             :
   case DATAFLOW_V8ADDS            :
   case DATAFLOW_V8SUBS            :
      return true;

   default:
      return false;
   }
}

AOP_Enum DFlowNode_GetAddOp(const DFlowNode *self)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_ADD               : return AOP_FADD;
   case DATAFLOW_SUB               :
   case DATAFLOW_RSUB              :
   case DATAFLOW_LESS_THAN         :
   case DATAFLOW_LESS_THAN_EQUAL   :
   case DATAFLOW_GREATER_THAN      :
   case DATAFLOW_GREATER_THAN_EQUAL:
   case DATAFLOW_EQUAL             :
   case DATAFLOW_NOT_EQUAL         : return AOP_FSUB;
   case DATAFLOW_LOGICAL_AND       : return AOP_AND;
   case DATAFLOW_LOGICAL_XOR       : return AOP_XOR;
   case DATAFLOW_LOGICAL_OR        : return AOP_OR;
   case DATAFLOW_BITWISE_NOT       : return AOP_NOT;
   case DATAFLOW_BITWISE_AND       : return AOP_AND;
   case DATAFLOW_BITWISE_OR        : return AOP_OR;
   case DATAFLOW_BITWISE_XOR       : return AOP_XOR;
   case DATAFLOW_V8ADDS            : return AOP_V8ADDS;
   case DATAFLOW_V8SUBS            : return AOP_V8SUBS;
   case DATAFLOW_INTEGER_ADD       : return AOP_ADD;
   case DATAFLOW_INTRINSIC_MIN     : return AOP_FMIN;
   case DATAFLOW_INTRINSIC_MAX     : return AOP_FMAX;
   case DATAFLOW_INTRINSIC_MINABS  : return AOP_FMINABS;
   case DATAFLOW_INTRINSIC_MAXABS  : return AOP_FMAXABS;
   case DATAFLOW_FTOI_TRUNC        : return AOP_FTOI;
   case DATAFLOW_FTOI_NEAREST      : return AOP_FTOI;
   case DATAFLOW_ITOF              : return AOP_ITOF;
   case DATAFLOW_SHIFT_RIGHT       : return AOP_SHR;
   case DATAFLOW_LOGICAL_SHR       : return AOP_ASR;

   default:
      UNREACHABLE();
      return AOP_NOP;
   }
}

MOP_Enum DFlowNode_GetMulOp(const DFlowNode *self)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_MUL               : return MOP_FMUL;
   case DATAFLOW_V8MULD            : return MOP_V8MULD;
   case DATAFLOW_V8MIN             : return MOP_V8MIN;
   case DATAFLOW_V8MAX             : return MOP_V8MAX;
   case DATAFLOW_V8ADDS            : return MOP_V8ADDS;
   case DATAFLOW_V8SUBS            : return MOP_V8SUBS;

   default:
      UNREACHABLE();
      return MOP_NOP;
   }
}


Register_Enum DFlowNode_GetSFUReg(const DFlowNode *self)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_INTRINSIC_RCP   : return Register_SFU_RECIP;
   case DATAFLOW_INTRINSIC_RSQRT : return Register_SFU_RECIPSQRT;
   case DATAFLOW_INTRINSIC_LOG2  : return Register_SFU_LOG;
   case DATAFLOW_INTRINSIC_EXP2  : return Register_SFU_EXP;

   default:
      UNREACHABLE();
      return Register_NOP;
   }
}

Register_Enum DFlowNode_GetIOInputReg(const DFlowNode *self)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_FRAG_GET_X   : return Register_X_PIXEL_COORD;
   case DATAFLOW_FRAG_GET_Y   : return Register_Y_PIXEL_COORD;
   case DATAFLOW_FRAG_GET_Z   : return Register_RB15;
   case DATAFLOW_FRAG_GET_W   : return Register_RA15;
   case DATAFLOW_FRAG_GET_FF  : return Register_REV_FLAG;
   case DATAFLOW_VARYING_C    : return Register_ACC5;

   default:
      UNREACHABLE();
      return Register_NOP;
   }
}

Register_Enum DFlowNode_GetIOOutputReg(const DFlowNode *self)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_FRAG_SUBMIT_STENCIL: return Register_TLB_STENCIL_SETUP;
   case DATAFLOW_FRAG_SUBMIT_Z      : return Register_TLB_Z;
   case DATAFLOW_FRAG_SUBMIT_MS     : return Register_TLB_COLOUR_MS;
   case DATAFLOW_FRAG_SUBMIT_ALL    : return Register_TLB_COLOUR_ALL;

   default:
      UNREACHABLE();
      return Register_NOP;
   }
}

Register_Enum DFlowNode_GetTextureReg(const DFlowNode *self)
{
   uint32_t add = self->m_sampler == 1 ? 4 : 0;

   switch (self->m_flavour)
   {
   case DATAFLOW_TEX_SET_DIRECT  :
   case DATAFLOW_TEX_SET_COORD_S : return (Register_Enum)(Register_TMU0_S + add);
   case DATAFLOW_TEX_SET_COORD_T : return (Register_Enum)(Register_TMU0_T + add);
   case DATAFLOW_TEX_SET_COORD_R : return (Register_Enum)(Register_TMU0_R + add);
   case DATAFLOW_TEX_SET_BIAS    :
   case DATAFLOW_TEX_SET_LOD     : return (Register_Enum)(Register_TMU0_B + add);

   default:
      UNREACHABLE();
      return Register_NOP;
   }
}

Sig_Enum DFlowNode_GetSignal(const DFlowNode *self)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_FRAG_GET_COL   : return Sig_COLLOAD;
   case DATAFLOW_TEX_GET_CMP_R  : return self->m_sampler == 1 ? Sig_LDTMU1 : Sig_LDTMU0;

   default:
      UNREACHABLE();
      return Sig_NONE;
   }
}


DFlowNode_ScheduleStatus DFlowNode_ReadSpecialRegister(DFlowNode *self, InstrState *state, Register_Enum from)
{
   if (from == Register_UNIFORM_READ)
      // We don't copy uniforms to registers any more, as the schedules are better if we don't
      // && (NodeList_size(DFlowNode_Parents(self)) <= 1 || Scheduler_LastGasp(state->m_scheduler)))
   {
      DFlowNode_SetResultReg(self, Register_UNIFORM_READ);
      return DFlowNode_NOTHING_TO_SCHEDULE;
   }
   else if (from == Register_VARYING_READ && NodeList_size(DFlowNode_Parents(self)) <= 1) // Varyings won't work without the number of parents check
   {
      DFlowNode_SetResultReg(self, Register_VARYING_READ);
      return DFlowNode_NOTHING_TO_SCHEDULE;
   }
   else
   {
      QPUResource *output;
      QPUOperand  src;
      QPUOperand  dest;

      if (state->m_outputRegistersFree == Register_FILE_NONE)
         return DFlowNode_NOT_SCHEDULABLE;

      // If child results are no longer needed, free them up
      DFlowNode_RetireOurChildren(self, state);

      output = DFlowNode_AllocateOutputReg(self, state, DFlowNode_FOR_RESULT, QPUResources_PREFER_REG);
      if (output == NULL)
         return DFlowNode_NOT_SCHEDULABLE;

      QPUOperand_ConstrReg(&src, from);
      QPUOperand_ConstrReg(&dest, QPUResource_Name(output));

      if (QPUGenericInstr_SetMovOpOp(&state->m_gi, &src, &dest))
      {
         // A couple of special cases
         if (from == Register_UNIFORM_READ)
            InstrState_AddUniformRead(state, self);
         else if (from == Register_VARYING_READ)
         {
            if (!InstrState_AddVaryingRead(state, self))
               return DFlowNode_NOT_SCHEDULABLE;
         }

         return InstrState_CommitChangesOp(state, self, &dest);
      }

      return DFlowNode_NOT_SCHEDULABLE;
   }
}

DFlowNode_ScheduleStatus DFlowNode_DoConstInt(DFlowNode *self, InstrState *state)
{
   float val = (float)self->m_uniform.m_constInt.m_ciValue;

   bool fsUniformConsts = InstrState_IsFragmentShader(state) && khrn_options.fs_uniform_consts;
   bool vsUniformConsts = !InstrState_IsFragmentShader(state) && khrn_options.vs_uniform_consts;

   if (fsUniformConsts || vsUniformConsts)
   {
      return DFlowNode_ReadSpecialRegister(self, state, Register_UNIFORM_READ);
   }
   else
   {
      if (QPUInstr_IsValidSmallImmediateFloat(val))
      {
         DFlowNode_SetResultFloat(self, val);
         return DFlowNode_NOTHING_TO_SCHEDULE;
      }
      else if ((InstrState_IsFragmentShader(state) && khrn_options.fs_uniform_long_immed) ||
              (!InstrState_IsFragmentShader(state) && khrn_options.vs_uniform_long_immed))
      {
         return DFlowNode_ReadSpecialRegister(self, state, Register_UNIFORM_READ);
      }
      else
      {
         QPUResource *output;
         QPUOperand  dest;

         // Long immediate
         if (state->m_outputRegistersFree == Register_FILE_NONE)
            return DFlowNode_NOT_SCHEDULABLE;

         // If child results are no longer needed, free them up
         DFlowNode_RetireOurChildren(self, state);

         output = DFlowNode_AllocateOutputReg(self, state, DFlowNode_FOR_RESULT, QPUResources_PREFER_ACC);
         if (output == NULL)
            return DFlowNode_NOT_SCHEDULABLE;

         QPUOperand_ConstrReg(&dest, QPUResource_Name(output));

         if (QPUGenericInstr_SetMovFloatOp(&state->m_gi, val, &dest))
         {
            if (QPUGenericInstr_SetAddOutputRegister(&state->m_gi, QPUResource_Name(output)))
               return InstrState_CommitChangesOp(state, self, &dest);
         }
         return DFlowNode_NOT_SCHEDULABLE;
      }
   }
}

DFlowNode_ScheduleStatus DFlowNode_DoConstFloat(DFlowNode *self, InstrState *state)
{
   float val = *(float*)&self->m_uniform.m_constFloat.m_cfValue;

   bool fsUniformConsts =  InstrState_IsFragmentShader(state) && khrn_options.fs_uniform_consts;
   bool vsUniformConsts = !InstrState_IsFragmentShader(state) && khrn_options.vs_uniform_consts;

   if (fsUniformConsts || vsUniformConsts)
   {
      return DFlowNode_ReadSpecialRegister(self, state, Register_UNIFORM_READ);
   }
   else
   {
      if (QPUInstr_IsValidSmallImmediateFloat(val))
      {
         DFlowNode_SetResultFloat(self, val);
         return DFlowNode_NOTHING_TO_SCHEDULE;
      }
      else if ((InstrState_IsFragmentShader(state) && khrn_options.fs_uniform_long_immed) ||
              (!InstrState_IsFragmentShader(state) && khrn_options.vs_uniform_long_immed))
      {
         return DFlowNode_ReadSpecialRegister(self, state, Register_UNIFORM_READ);
      }
      else
      {
         QPUResource *output;

         // Long immediate
         if (state->m_outputRegistersFree == Register_FILE_NONE)
            return DFlowNode_NOT_SCHEDULABLE;

         // If child results are no longer needed, free them up
         DFlowNode_RetireOurChildren(self, state);

         output = DFlowNode_AllocateOutputReg(self, state, DFlowNode_FOR_RESULT, QPUResources_PREFER_ACC);
         if (output == NULL)
            return DFlowNode_NOT_SCHEDULABLE;

         if (QPUGenericInstr_SetMovFloatRes(&state->m_gi, val, output))
         {
            if (QPUGenericInstr_SetAddOutputRegisterRes(&state->m_gi, output))
               return InstrState_CommitChangesRes(state, self, output);
         }
         return DFlowNode_NOT_SCHEDULABLE;
      }
   }
}

// Do some early checks to see if this looks schedulable in the given gi.
// Will save us having to copy the state only to find something later.
bool DFlowNode_LooksSchedulable(DFlowNode *self, QPUGenericInstr *gi)
{
   switch (self->m_flavour)
   {
   default :
      return true;

   case DATAFLOW_SCOREBOARD_WAIT:
      if (QPUGenericInstr_SigUsed(gi))
         return false;
      break;

   case DATAFLOW_MUL               :
   case DATAFLOW_V8MULD            :
   case DATAFLOW_V8MIN             :
   case DATAFLOW_V8MAX             :
      if (QPUGenericInstr_MulUsed(gi) || QPUGenericInstr_GetFreeOutputRegisters(gi) == Register_FILE_NONE)
         return false;
      break;

   case DATAFLOW_ADD               :
   case DATAFLOW_SUB               :
   case DATAFLOW_RSUB              :   // Rev
   case DATAFLOW_BITWISE_AND       :
   case DATAFLOW_BITWISE_OR        :
   case DATAFLOW_BITWISE_XOR       :
   case DATAFLOW_V8ADDS            :
   case DATAFLOW_V8SUBS            :
   case DATAFLOW_INTEGER_ADD       :
   case DATAFLOW_INTRINSIC_MIN     :
   case DATAFLOW_INTRINSIC_MAX     :
   case DATAFLOW_INTRINSIC_MINABS  :
   case DATAFLOW_INTRINSIC_MAXABS  :
   case DATAFLOW_SHIFT_RIGHT       :
   case DATAFLOW_LOGICAL_SHR       :
   case DATAFLOW_LESS_THAN         :
   case DATAFLOW_LESS_THAN_EQUAL   :   // Rev
   case DATAFLOW_GREATER_THAN      :   // Rev
   case DATAFLOW_GREATER_THAN_EQUAL:
   case DATAFLOW_EQUAL             :
   case DATAFLOW_NOT_EQUAL         :
   case DATAFLOW_LOGICAL_AND       :
   case DATAFLOW_LOGICAL_XOR       :
   case DATAFLOW_LOGICAL_OR        :
   case DATAFLOW_FTOI_TRUNC        :
   case DATAFLOW_FTOI_NEAREST      :
   case DATAFLOW_ITOF              :
   case DATAFLOW_BITWISE_NOT       :
      if (QPUGenericInstr_AdderUsed(gi) || QPUGenericInstr_GetFreeOutputRegisters(gi) == Register_FILE_NONE)
         return false;
      break;

   case DATAFLOW_TEX_SET_COORD_S:
   case DATAFLOW_TEX_SET_COORD_T:
   case DATAFLOW_TEX_SET_COORD_R:
   case DATAFLOW_TEX_SET_BIAS   :
   case DATAFLOW_TEX_SET_LOD    :
   case DATAFLOW_VPM_READ_SETUP :
   case DATAFLOW_VPM_WRITE_SETUP:
      if (QPUGenericInstr_GetFreeOutputRegisters(gi) == Register_FILE_NONE)
         return false;
      break;
   }

   return true;
}

DFlowNode_ScheduleStatus DFlowNode_DoDelayedACC5Move(DFlowNode *self, Scheduler *sched, QPUGenericInstr *gi)
{
   // Take a working copy of the instruction and the register state so we can easily restore the original after a failure
   InstrState state;
   QPUOperand acc5;

   InstrState_Constr(&state, gi, sched);
   QPUOperand_ConstrReg(&acc5, Register_ACC5);

   return DFlowNode_InsertExtraMov(self, &acc5, &state, DFlowNode_FULLY_RETIRE, QPUResources_PREFER_ACC);
}

DFlowNode_ScheduleStatus DFlowNode_HandleDelayedLoadc(DFlowNode *self, InstrState *state)
{
   if (InstrState_IsRegisterInUse(state, Register_ACC4))
   {
      QPUOperand acc4;
      QPUOperand_ConstrReg(&acc4, Register_ACC4);

      if (!InstrState_IsReadableReg(state, Register_ACC4))
         return DFlowNode_NOT_SCHEDULABLE;

      return DFlowNode_InsertExtraMov(InstrState_GetOwner(state, Register_ACC4), &acc4, state, DFlowNode_FULLY_RETIRE, QPUResources_PREFER_ACC);
   }
   else
   {
      // Find the loadc child node
      const NodeList           *children = DFlowNode_Children(self);
      NodeList_const_iterator  citer;
      DFlowNode                *loadcNode = NULL;
      for (citer = NodeList_const_begin(children); citer != NodeList_const_end(children); NodeList_const_next(&citer))
      {
         DFlowNode *child = NodeList_const_star(citer);
         if (child->m_flavour == DATAFLOW_FRAG_GET_COL)
         {
            loadcNode = child;
            break;
         }
      }
      assert(loadcNode != NULL);

      if (QPUGenericInstr_SetSignal(&state->m_gi, DFlowNode_GetSignal(loadcNode)))
      {
         DFlowNode_SetReferenced(loadcNode, state, Register_ACC4, 0);
         DFlowNode_SetWrittenAt(state, Register_ACC4, 0);
         if (InstrState_CommitChangesReg(state, loadcNode, Register_ACC4) == DFlowNode_SCHEDULED)
         {
            self->m_delayedLoadc = false;
            return DFlowNode_INSERTED_EXTRA_INSTR;
         }
      }
      return DFlowNode_NOT_SCHEDULABLE;
   }
}

DFlowNode_ScheduleStatus DFlowNode_AddToInstruction(DFlowNode *self, Scheduler *sched, QPUGenericInstr *genericInstr)
{
   DFlowNode *acc5node = NULL;
   InstrState state;
   QPUOperand leftOp, rightOp, operand;

   if (self->m_bypassed)
      return DFlowNode_NOTHING_TO_SCHEDULE;

   // Special case where we had to delay a move from ACC5
   if (Scheduler_NeedsACC5Mov(sched, &acc5node))
   {
      DFlowNode_ScheduleStatus status = DFlowNode_DoDelayedACC5Move(acc5node, sched, genericInstr);
      if (status == DFlowNode_INSERTED_EXTRA_INSTR)
        Scheduler_SetNeedsACC5Mov(sched, NULL, false);

      return status;
   }

   // Check for early exit conditions
   if (!self->m_delayedLoadc && !DFlowNode_LooksSchedulable(self, genericInstr))
      return DFlowNode_NOT_SCHEDULABLE;

   // Take a working copy of the instruction and the register state so we can easily restore the original after a failure
   InstrState_Constr(&state, genericInstr, sched);

   // Have we delayed a loadc that now needs to be processed before we can schedule this node?
   if (self->m_delayedLoadc)
      return DFlowNode_HandleDelayedLoadc(self, &state);

   switch (self->m_flavour)
   {
   // These are constants that will be read by the consuming instruction either as
   // uniforms or as short immediates
   case DATAFLOW_CONST_BOOL :
      DFlowNode_SetResultInt(self, (int8_t)self->m_uniform.m_constBool.m_cbValue);
      return DFlowNode_NOTHING_TO_SCHEDULE;

   case DATAFLOW_CONST_INT :
      return DFlowNode_DoConstInt(self, &state);
   case DATAFLOW_CONST_FLOAT :
      return DFlowNode_DoConstFloat(self, &state);
   case DATAFLOW_CONST_SAMPLER     :
      // These don't generate code directly
      return DFlowNode_NOTHING_TO_SCHEDULE;

   case DATAFLOW_UNIFORM           :
      return DFlowNode_ReadSpecialRegister(self, &state, Register_UNIFORM_READ);

   case DATAFLOW_UNIFORM_OFFSET    :
      assert(0); // Unused?
      return DFlowNode_NOTHING_TO_SCHEDULE;

   // Indexed uniforms
   case DATAFLOW_UNIFORM_ADDRESS:
      return DFlowNode_ReadSpecialRegister(self, &state, Register_UNIFORM_READ);

	case DATAFLOW_ATTRIBUTE         :
      // Ensure we don't try to read vpm data before (or in the same instruction as) the setup
      if (InstrState_IsReadableReg(&state, Register_VPMVCD_RD_SETUP))
         return DFlowNode_ReadSpecialRegister(self, &state, Register_VPM_READ);
      else
         return DFlowNode_NOT_SCHEDULABLE;

	case DATAFLOW_VARYING           :
      return DFlowNode_ReadSpecialRegister(self, &state, Register_VARYING_READ);

   // Standard arithmetic functions for the ALU
	case DATAFLOW_ARITH_NEGATE      :
      assert(0);   // Never get these?
      return DFlowNode_NOTHING_TO_SCHEDULE;

	case DATAFLOW_MUL               :
   case DATAFLOW_V8MULD            :
   case DATAFLOW_V8MIN             :
   case DATAFLOW_V8MAX             :
      {
         QPUResource *output;

         DFlowNode_ScheduleStatus status = DFlowNode_SetupForTwoOperands(self, &leftOp, &rightOp, &state);
         if (status != DFlowNode_NOTHING_TO_SCHEDULE)
            return status;

         if (QPUGenericInstr_SetMul(&state.m_gi, DFlowNode_GetMulOp(self), &leftOp, &rightOp))
         {
            // If child results are no longer needed, free them up
            DFlowNode_RetireOurChildren(self, &state);

            output = DFlowNode_AllocateOutputReg(self, &state, DFlowNode_FOR_RESULT, QPUResources_PREFER_ACC);
            if (output != NULL)
            {
               Register_Enum  outputName = QPUResource_Name(output);

               if (QPUGenericInstr_SetMulOutputRegister(&state.m_gi, outputName))
                  return InstrState_CommitChangesReg(&state, self, outputName);
            }
         }
         return DFlowNode_NOT_SCHEDULABLE;
      }

	case DATAFLOW_ADD               :
   case DATAFLOW_SUB               :
   case DATAFLOW_RSUB              :   // Rev
   case DATAFLOW_BITWISE_AND       :
   case DATAFLOW_BITWISE_OR        :
   case DATAFLOW_BITWISE_XOR       :
   case DATAFLOW_V8ADDS            :
   case DATAFLOW_V8SUBS            :
   case DATAFLOW_INTEGER_ADD       :
   case DATAFLOW_INTRINSIC_MIN     :
   case DATAFLOW_INTRINSIC_MAX     :
   case DATAFLOW_INTRINSIC_MINABS  :
   case DATAFLOW_INTRINSIC_MAXABS  :
   case DATAFLOW_SHIFT_RIGHT       :
   case DATAFLOW_LOGICAL_SHR       :
   case DATAFLOW_LESS_THAN         :
   case DATAFLOW_LESS_THAN_EQUAL   :   // Rev
   case DATAFLOW_GREATER_THAN      :   // Rev
   case DATAFLOW_GREATER_THAN_EQUAL:
   case DATAFLOW_EQUAL             :
   case DATAFLOW_NOT_EQUAL         :
   case DATAFLOW_LOGICAL_AND       :
   case DATAFLOW_LOGICAL_XOR       :
   case DATAFLOW_LOGICAL_OR        :
      {
         DFlowNode_ScheduleStatus status;

         if (khrn_options.glsl_optimizations_on && !self->m_delayedCondition)
         {
            // Optimization for set flags.
            // If we happen to be the condition child of a cond node, we will delay our own execution until our
            // parent cond node is being scheduled. At that point, we will get scheduled, and set flags will be added
            // to our instruction.
            if (NodeList_size(DFlowNode_Parents(self)) == 1 && NodeList_size(DFlowNode_IoParents(self)) == 0)
            {
               DFlowNode       *parent        = NodeList_front(DFlowNode_Parents(self));
               DataflowFlavour  parentFlavour = DFlowNode_Flavour(parent);

               if ((parentFlavour == DATAFLOW_CONDITIONAL && parent->m_args[DFlowNode_ARG_COND] == self) ||
                   ((parentFlavour == DATAFLOW_FRAG_SUBMIT_STENCIL || parentFlavour == DATAFLOW_FRAG_SUBMIT_Z ||
                     parentFlavour == DATAFLOW_FRAG_SUBMIT_MS || parentFlavour == DATAFLOW_FRAG_SUBMIT_ALL) &&
                     parent->m_args[DFlowNode_ARG_DISCARD] == self))
               {
                  // We are a condition, and have only one parent. Don't schedule this node yet
                  self->m_delayedCondition = true;
                  return DFlowNode_NOTHING_TO_SCHEDULE;
               }
            }
         }

         status = DFlowNode_SetupForTwoOperands(self, &leftOp, &rightOp, &state);
         if (status != DFlowNode_NOTHING_TO_SCHEDULE)
            return status;

         if (QPUGenericInstr_SetAdd(&state.m_gi, DFlowNode_GetAddOp(self), &leftOp, &rightOp))
         {
            QPUResource *output;

            // If child results are no longer needed, free them up
            DFlowNode_RetireOurChildren(self, &state);

            if (khrn_options.glsl_optimizations_on && self->m_delayedCondition)
            {
               DFlowNode *parent;

               // If we are a delayed condition, we only want to set flags - we don't need the output
               if (InstrState_IsRegisterInUse(&state, Register_VIRTUAL_FLAGS))
                  return DFlowNode_NOT_SCHEDULABLE;

               parent = NodeList_front(DFlowNode_Parents(self));

               if (DFlowNode_Flavour(parent) == DATAFLOW_CONDITIONAL && parent->m_args[DFlowNode_ARG_COND] == self)
                  Scheduler_SetCurrentFlagsCondition(state.m_scheduler, DFlowNode_GetArg(parent, DFlowNode_ARG_COND));

               QPUGenericInstr_SetFlags(&state.m_gi, true);

               DFlowNode_SetWrittenAt(&state, Register_VIRTUAL_FLAGS, 0);
               DFlowNode_SetReferenced(self, &state, Register_VIRTUAL_FLAGS, 1);  // Force ref count to 1
               output = InstrState_GetResource(&state, Register_NOP);
            }
            else
               output = DFlowNode_AllocateOutputReg(self, &state, DFlowNode_FOR_RESULT, QPUResources_PREFER_ACC);

            if (output != NULL)
            {
               Register_Enum  outputName = QPUResource_Name(output);

               if (QPUGenericInstr_SetAddOutputRegister(&state.m_gi, outputName))
                  return InstrState_CommitChangesReg(&state, self, outputName);
            }
         }
         return DFlowNode_NOT_SCHEDULABLE;
      }

   case DATAFLOW_CONDITIONAL       :
      {
         if (state.m_outputRegistersFree == Register_FILE_NONE)
            return DFlowNode_NOT_SCHEDULABLE;

         if (self->m_schedPass == 0)
         {
            const QPUOperand *ifOp = DFlowNode_GetInput(self, &state, self->m_args[DFlowNode_ARG_TRUE_VALUE]);
            if (ifOp == NULL)
               return DFlowNode_NOT_SCHEDULABLE;

            if (khrn_options.glsl_optimizations_on)
            {
               bool           ifIsReg = QPUOperand_IsRegister(ifOp);
               Register_Enum  ifReg   = QPUOperand_ValueRegister(ifOp);

               // We don't need an extra mov if the ifOp is already in a register and only referenced by us
               if (ifIsReg && Register_IsNormalRegOrAcc(ifReg) &&
                   !Register_IsReadOnly(ifReg) &&
                   DFlowNode_GetUnpackCode(self->m_args[DFlowNode_ARG_TRUE_VALUE]) == VirtualUnpack_NONE &&
                   NodeList_size(DFlowNode_Parents(self->m_args[DFlowNode_ARG_TRUE_VALUE])) == 1 &&
                   NodeList_size(DFlowNode_Parents(self)) == 1 && // TODO : this shouldn't be necessary
                   QPUResource_RefCount(InstrState_GetResource(&state, ifReg)) == 1)
               {
                  // Ensure ref counts are correct
                  InstrState_RetireOp(&state, self->m_args[DFlowNode_ARG_TRUE_VALUE], ifOp);
                  DFlowNode_SetReferenced(self, &state, ifReg, 0);

                  if (InstrState_CommitChangesOp(&state, self, ifOp) == DFlowNode_NOT_SCHEDULABLE)
                     return DFlowNode_NOT_SCHEDULABLE;

                  self->m_schedPass++;
                  return DFlowNode_INSERTED_EXTRA_INSTR;
               }
            }

            if (DFlowNode_InsertExtraMov(self, ifOp, &state, DFlowNode_NORMAL_RETIRE, QPUResources_PREFER_ACC) == DFlowNode_NOT_SCHEDULABLE)
               return DFlowNode_NOT_SCHEDULABLE;

            self->m_schedPass++;

            return DFlowNode_INSERTED_EXTRA_INSTR;
         }
         else if (self->m_schedPass == 1)
         {
            DFlowNode *childCond = self->m_args[DFlowNode_ARG_COND];

            if (khrn_options.glsl_optimizations_on)
            {
               // Has our child condition been delayed
               if (childCond->m_delayedCondition)
               {
                  DFlowNode_ScheduleStatus status;

                  // We must schedule the child cond node now, as we delayed it earlier
                  status = DFlowNode_AddToInstruction(childCond, sched, genericInstr);
                  if (status == DFlowNode_SCHEDULED)
                  {
                     childCond->m_delayedCondition = false;
                     self->m_schedPass++;
                     return DFlowNode_INSERTED_EXTRA_INSTR;
                  }

                  return status;
               }
            }

            // Second pass generates a move from the underlying comparison node to NOP in order to set the flags
            if (!DFlowNode_SetConditionFlags(self, &state, self->m_args[DFlowNode_ARG_COND]))
               return DFlowNode_NOT_SCHEDULABLE;

            // Retire the cond op
            InstrState_RetireOp(&state ,childCond, &childCond->m_result);

            if (InstrState_CommitChanges(&state) == DFlowNode_NOT_SCHEDULABLE)
               return DFlowNode_NOT_SCHEDULABLE;

            self->m_schedPass++;

            return DFlowNode_INSERTED_EXTRA_INSTR;
         }
         else if (self->m_schedPass == 2)
         {
            const QPUOperand        *elseOp;
            QPUGenericInstr_MovUnit unitUsed;

            // Third pass generates a conditional move of the else part to the same output as the if part
            if (!InstrState_IsReadableReg(&state, Register_VIRTUAL_FLAGS))
               return DFlowNode_NOT_SCHEDULABLE;

            elseOp = DFlowNode_GetInput(self, &state, self->m_args[DFlowNode_ARG_FALSE_VALUE]);
            if (elseOp == NULL)
               return DFlowNode_NOT_SCHEDULABLE;

            if (QPUGenericInstr_SetMovOpOpEx(&state.m_gi, elseOp, &self->m_result, QPUGenericInstr_PREFER_ADD, &unitUsed))
            {
               Register_Enum  resultReg;
               bool           ok;

               if (unitUsed == QPUGenericInstr_MUL)
                  QPUGenericInstr_SetMulCondition(&state.m_gi, DFlowNode_GetCondCode(self->m_args[DFlowNode_ARG_COND]));
               else
                  QPUGenericInstr_SetAddCondition(&state.m_gi, DFlowNode_GetCondCode(self->m_args[DFlowNode_ARG_COND]));

               InstrState_RetireOp(&state, self->m_args[DFlowNode_ARG_FALSE_VALUE], elseOp);

               // Free up the virtual flags register
               InstrState_RetireReg(&state, NULL, Register_VIRTUAL_FLAGS);

               resultReg = QPUOperand_ValueRegister(&self->m_result);

               if (unitUsed == QPUGenericInstr_ADD)
                  ok = QPUGenericInstr_SetAddOutputRegister(&state.m_gi, resultReg);
               else
                  ok = QPUGenericInstr_SetMulOutputRegister(&state.m_gi, resultReg);

               // Refresh the delay slot on the result
               DFlowNode_SetWrittenAt(&state, resultReg, 0);
               // Already referenced, so don't increment again

               // Re-use the if operand's output register
               if (ok)
               {
                  if (InstrState_CommitChangesOp(&state, self, &self->m_result) == DFlowNode_NOT_SCHEDULABLE)
                     return DFlowNode_NOT_SCHEDULABLE;

                  self->m_schedPass = 0;
                  return DFlowNode_SCHEDULED;
               }
            }
         }
         return DFlowNode_NOT_SCHEDULABLE;
      }

   case DATAFLOW_FTOI_TRUNC        :
   case DATAFLOW_FTOI_NEAREST      : // TODO: DATAFLOW_FTOI_NEAREST is wrong: QPU op ftoi truncates.
   case DATAFLOW_ITOF              :
   case DATAFLOW_BITWISE_NOT       :

      if (!DFlowNode_SetupForSingleOperand(self, &operand, &state))
         return DFlowNode_NOT_SCHEDULABLE;

      if (QPUGenericInstr_SetAdd(&state.m_gi, DFlowNode_GetAddOp(self), &operand, &operand))
      {
         QPUResource *output;

         // If child results are no longer needed, free them up
         DFlowNode_RetireOurChildren(self, &state);

         output = DFlowNode_AllocateOutputReg(self, &state, DFlowNode_FOR_RESULT, QPUResources_PREFER_ACC);
         if (output != NULL)
         {
            Register_Enum  outputName = QPUResource_Name(output);

            if (QPUGenericInstr_SetAddOutputRegister(&state.m_gi, outputName))
               return InstrState_CommitChangesReg(&state, self, outputName);
         }
      }
      return DFlowNode_NOT_SCHEDULABLE;

   // Intrinsic, hardware supported operators.
	case DATAFLOW_INTRINSIC_RCP   :
   case DATAFLOW_INTRINSIC_RSQRT :
   case DATAFLOW_INTRINSIC_LOG2  :
   case DATAFLOW_INTRINSIC_EXP2  :
      {
         if (khrn_options.glsl_optimizations_on)
         {
            // These operations will write to r4. To avoid dependency loops, we could just insert a mov from r4
            // afterwards, but this is inefficient. What we'll actually do is check whether there is anything
            // in r4 that hasn't been consumed yet. If there is, we'll move it somewhere before overwriting it.
            if (InstrState_IsRegisterInUse(&state, Register_ACC4))
            {
               QPUOperand acc4;

               if (!InstrState_IsReadableReg(&state, Register_ACC4))
                  return DFlowNode_NOT_SCHEDULABLE;

               QPUOperand_ConstrReg(&acc4, Register_ACC4);
               return DFlowNode_InsertExtraMov(InstrState_GetOwner(&state, Register_ACC4), &acc4, &state, DFlowNode_FULLY_RETIRE, QPUResources_PREFER_ACC);
            }
            else
            {
               const QPUOperand *src;
               QPUOperand        dest;

               DFlowNode_RetireOurChildren(self, &state);

               src = DFlowNode_GetInput(self, &state, self->m_args[DFlowNode_ARG_OPERAND]);
               if (src == NULL)
                  return DFlowNode_NOT_SCHEDULABLE;

               if (InstrState_IsRegisterInUse(&state, Register_ACC4))
                  return DFlowNode_NOT_SCHEDULABLE;

               QPUOperand_ConstrReg(&dest, DFlowNode_GetSFUReg(self));

               if (QPUGenericInstr_SetMovOpOp(&state.m_gi, src, &dest))
               {
                  // Can't read (or write) r4 for 2 more slots
                  DFlowNode_SetReferenced(self, &state, Register_ACC4, 0);
                  DFlowNode_SetWrittenAt(&state, Register_ACC4, 2);
                  if (InstrState_CommitChangesReg(&state, self, Register_ACC4) == DFlowNode_SCHEDULED)
                     return DFlowNode_SCHEDULED;
               }
            }

            return DFlowNode_NOT_SCHEDULABLE;
         }
         else
         {
            // Sub-optimal
            if (self->m_schedPass == 0)
            {
               const QPUOperand *src;
               QPUOperand       dest;

               DFlowNode_RetireOurChildren(self, &state);

               src = DFlowNode_GetInput(self, &state, self->m_args[DFlowNode_ARG_OPERAND]);
               if (src == NULL)
                  return DFlowNode_NOT_SCHEDULABLE;

               if (InstrState_IsRegisterInUse(&state, Register_ACC4))
                  return DFlowNode_NOT_SCHEDULABLE;

               QPUOperand_ConstrReg(&dest, DFlowNode_GetSFUReg(self));

               if (QPUGenericInstr_SetMovOpOp(&state.m_gi, src, &dest))
               {
                  // Can't read (or write) r4 for 2 more slots
                  DFlowNode_SetReferenced(self, &state, Register_ACC4, 0);
                  DFlowNode_SetWrittenAt(&state, Register_ACC4, 2);
                  if (InstrState_CommitChangesReg(&state, self, Register_ACC4) == DFlowNode_NOT_SCHEDULABLE)
                     return DFlowNode_NOT_SCHEDULABLE;

                  self->m_schedPass++;
                  return DFlowNode_INSERTED_EXTRA_INSTR;
               }
            }
            else
            {
               QPUOperand acc4;

               if (!InstrState_IsReadableReg(&state, Register_ACC4))
                  return DFlowNode_NOT_SCHEDULABLE;

               QPUOperand_ConstrReg(&acc4, Register_ACC4);

               if (DFlowNode_InsertExtraMov(self, &acc4, &state, DFlowNode_FULLY_RETIRE, QPUResources_PREFER_ACC) != DFlowNode_NOT_SCHEDULABLE)
                  return DFlowNode_SCHEDULED;
            }
            return DFlowNode_NOT_SCHEDULABLE;
         }
      }

   case DATAFLOW_INTRINSIC_CEIL  :
   case DATAFLOW_INTRINSIC_FLOOR :
   case DATAFLOW_INTRINSIC_SIGN  :
      assert(0); // Unused?
      break;

   case DATAFLOW_THREADSWITCH:
      assert(0); // Thread switch. Only used by old scheduler.
	   break;

   // TMU configuration
   case DATAFLOW_TMU_SWAP:
      {
         QPUOperand  dest;
         QPUOperand  src;

         QPUOperand_ConstrReg(&dest, Register_TMU_NOSWAP);
         QPUOperand_ConstrFloat(&src, 1.0f);

         DFlowNode_SetWrittenAt(&state, Register_VIRTUAL_TMUWRITE, 0);

         if (QPUGenericInstr_SetMovOpOp(&state.m_gi, &src, &dest))
         {
            DFlowNode_SetWrittenAt(&state, Register_TMU_NOSWAP, 0);
            DFlowNode_SetReferenced(self, &state, Register_TMU_NOSWAP, 0);
            return InstrState_CommitChanges(&state);
         }
         return DFlowNode_NOT_SCHEDULABLE;
      }

   // Texture lookup operators.
   case DATAFLOW_TEX_SET_DIRECT :
      return DFlowNode_SpecialRegWrite(self, self->m_args[DFlowNode_ARG_PARAM], DFlowNode_GetTextureReg(self), &state, NULL);

   case DATAFLOW_TEX_SET_COORD_S:
   case DATAFLOW_TEX_SET_COORD_T:
	case DATAFLOW_TEX_SET_COORD_R:
	case DATAFLOW_TEX_SET_BIAS   :
	case DATAFLOW_TEX_SET_LOD    :
      return DFlowNode_TextureWrite(self, &state, sched);

   case DATAFLOW_FRAG_GET_COL   :
      {
         uint32_t numParents = NodeList_size(DFlowNode_Parents(self));

         // We sometimes see a frag_get_col with no parent - which makes it pointless. Check for that.
         if (numParents == 0)
            return DFlowNode_NOTHING_TO_SCHEDULE;
         else if (numParents == 1)
         {
            // Optimization to de-prioritize the loadc until it's ready to be consumed
            self->m_parents.m_head->m_node->m_delayedLoadc = true;
            return DFlowNode_NOTHING_TO_SCHEDULE;
         }
         else
         {
            // These operations will write to r4. To avoid dependency loops, we could just insert a mov from r4
            // afterwards, but this is inefficient. What we'll actually do is check whether there is anything
            // in r4 that hasn't been consumed yet. If there is, we'll move it somewhere before overwriting it.
            if (InstrState_IsRegisterInUse(&state, Register_ACC4))
            {
               QPUOperand acc4;
               QPUOperand_ConstrReg(&acc4, Register_ACC4);

               if (!InstrState_IsReadableReg(&state, Register_ACC4))
                  return DFlowNode_NOT_SCHEDULABLE;

               return DFlowNode_InsertExtraMov(InstrState_GetOwner(&state, Register_ACC4), &acc4, &state, DFlowNode_FULLY_RETIRE, QPUResources_PREFER_ACC);
            }
            else
            {
               if (QPUGenericInstr_SetSignal(&state.m_gi, DFlowNode_GetSignal(self)))
               {
                  DFlowNode_SetReferenced(self, &state, Register_ACC4, 0);
                  DFlowNode_SetWrittenAt(&state, Register_ACC4, 0);
                  if (InstrState_CommitChangesReg(&state, self, Register_ACC4) == DFlowNode_SCHEDULED)
                     return DFlowNode_SCHEDULED;
               }
               return DFlowNode_NOT_SCHEDULABLE;
            }
         }
      }
      break;

	case DATAFLOW_TEX_GET_CMP_R  :
      if (khrn_options.glsl_optimizations_on)
      {
         // These operations will write to r4. To avoid dependency loops, we could just insert a mov from r4
         // afterwards, but this is inefficient. What we'll actually do is check whether there is anything
         // in r4 that hasn't been consumed yet. If there is, we'll move it somewhere before overwriting it.
         if (InstrState_IsRegisterInUse(&state, Register_ACC4))
         {
            QPUOperand acc4;
            QPUOperand_ConstrReg(&acc4, Register_ACC4);

            if (!InstrState_IsReadableReg(&state, Register_ACC4))
               return DFlowNode_NOT_SCHEDULABLE;

            return DFlowNode_InsertExtraMov(InstrState_GetOwner(&state, Register_ACC4), &acc4, &state, DFlowNode_FULLY_RETIRE, QPUResources_PREFER_ACC);
         }
         else
         {
            if (QPUGenericInstr_SetSignal(&state.m_gi, DFlowNode_GetSignal(self)))
            {
               DFlowNode_SetReferenced(self, &state, Register_ACC4, 0);
               DFlowNode_SetWrittenAt(&state, Register_ACC4, 0);
               if (InstrState_CommitChangesReg(&state, self, Register_ACC4) == DFlowNode_SCHEDULED)
                  return DFlowNode_SCHEDULED;
            }
            return DFlowNode_NOT_SCHEDULABLE;
         }
      }
      else
      {
         // R4 outputting instructions insert a MOV to avoid dependency loops
         if (self->m_schedPass == 0)
         {
            if (InstrState_IsRegisterInUse(&state, Register_ACC4))
               return DFlowNode_NOT_SCHEDULABLE;

            if (QPUGenericInstr_SetSignal(&state.m_gi, DFlowNode_GetSignal(self)))
            {
               DFlowNode_SetReferenced(self, &state, Register_ACC4, 0);
               DFlowNode_SetWrittenAt(&state, Register_ACC4, 0);
               if (InstrState_CommitChangesReg(&state, self, Register_ACC4) == DFlowNode_NOT_SCHEDULABLE)
                  return DFlowNode_NOT_SCHEDULABLE;

               self->m_schedPass++;
               return DFlowNode_INSERTED_EXTRA_INSTR;
            }
            return DFlowNode_NOT_SCHEDULABLE;
         }
         else
         {
            QPUOperand acc4;

            if (!InstrState_IsReadableReg(&state, Register_ACC4))
               return DFlowNode_NOT_SCHEDULABLE;

            QPUOperand_ConstrReg(&acc4, Register_ACC4);

            if (DFlowNode_InsertExtraMov(self, &acc4, &state, DFlowNode_FULLY_RETIRE, QPUResources_PREFER_ACC) != DFlowNode_NOT_SCHEDULABLE)
               return DFlowNode_SCHEDULED;
         }

         return DFlowNode_NOT_SCHEDULABLE;
      }
      break;

   case DATAFLOW_TEX_GET_CMP_G  :
   case DATAFLOW_TEX_GET_CMP_B  :
   case DATAFLOW_TEX_GET_CMP_A  :
      assert(0); // Unused
      return DFlowNode_NOTHING_TO_SCHEDULE;

   // Fragment coordinate retrieval & varying c
   case DATAFLOW_FRAG_GET_X   :
   case DATAFLOW_FRAG_GET_Y   :
   case DATAFLOW_FRAG_GET_Z   :
   case DATAFLOW_FRAG_GET_W   :
   case DATAFLOW_FRAG_GET_FF  :
      DFlowNode_SetResultReg(self, DFlowNode_GetIOInputReg(self));
      return DFlowNode_NOTHING_TO_SCHEDULE;

   case DATAFLOW_VARYING_C:
      return DFlowNode_NOTHING_TO_SCHEDULE;

   case DATAFLOW_FRAG_GET_PC_X:
   case DATAFLOW_FRAG_GET_PC_Y:
      return DFlowNode_NOTHING_TO_SCHEDULE;   // Unused?

   case DATAFLOW_FRAG_SUBMIT_STENCIL:
   case DATAFLOW_FRAG_SUBMIT_Z      :
   case DATAFLOW_FRAG_SUBMIT_MS     :
   case DATAFLOW_FRAG_SUBMIT_ALL    :
      if (!Scheduler_AllThreadSwitchesDone(sched))
         return DFlowNode_NOT_SCHEDULABLE;

      if (self->m_schedPass == 0)
      {
         if (self->m_args[DFlowNode_ARG_DISCARD] != NULL)
         {
            if (khrn_options.glsl_optimizations_on)
            {
               DFlowNode *discardCond = self->m_args[DFlowNode_ARG_DISCARD];

               // Has our child condition been delayed
               if (discardCond->m_delayedCondition)
               {
                  DFlowNode_ScheduleStatus status;

                  // We must schedule the child cond node now, as we delayed it earlier
                  status = DFlowNode_AddToInstruction(discardCond, sched, genericInstr);
                  if (status == DFlowNode_SCHEDULED)
                  {
                     discardCond->m_delayedCondition = false;
                     self->m_schedPass++;
                     return DFlowNode_INSERTED_EXTRA_INSTR;
                  }

                  return status;
               }
            }

            if (!DFlowNode_SetConditionFlags(self, &state, self->m_args[DFlowNode_ARG_DISCARD]))
               return DFlowNode_NOT_SCHEDULABLE;

            if (InstrState_CommitChanges(&state) == DFlowNode_NOT_SCHEDULABLE)
               return DFlowNode_NOT_SCHEDULABLE;

            self->m_schedPass++;

            return DFlowNode_INSERTED_EXTRA_INSTR;
         }
         else
            return DFlowNode_SpecialRegWrite(self, self->m_args[DFlowNode_ARG_PARAM], DFlowNode_GetIOOutputReg(self), &state, NULL);
      }
      else if (self->m_schedPass == 1)
      {
         DFlowNode_ScheduleStatus status;

         if (!InstrState_IsReadableReg(&state, Register_VIRTUAL_FLAGS))
            return DFlowNode_NOT_SCHEDULABLE;

         // Free up the virtual flags register
         InstrState_RetireReg(&state, NULL, Register_VIRTUAL_FLAGS);

         status = DFlowNode_SpecialRegWrite(self, self->m_args[DFlowNode_ARG_PARAM], DFlowNode_GetIOOutputReg(self), &state, self->m_args[DFlowNode_ARG_DISCARD]);
         if (status == DFlowNode_SCHEDULED)
            self->m_schedPass = 0;

         return status;
      }

   case DATAFLOW_FRAG_SUBMIT_R0     :
   case DATAFLOW_FRAG_SUBMIT_R1     :
   case DATAFLOW_FRAG_SUBMIT_R2     :
   case DATAFLOW_FRAG_SUBMIT_R3     :
      return DFlowNode_NOTHING_TO_SCHEDULE;

   // Vertex result submission.
   case DATAFLOW_VERTEX_SET     :
      // Ensure we don't try to write vpm data before (or in the same instruction as) the setup
      if (InstrState_IsReadableReg(&state, Register_VPMVCD_WR_SETUP))
         return DFlowNode_SpecialRegWrite(self, self->m_args[DFlowNode_ARG_PARAM], Register_VPM_WRITE, &state, NULL);
      else
         return DFlowNode_NOT_SCHEDULABLE;

   case DATAFLOW_VPM_READ_SETUP :
      return DFlowNode_SpecialRegWrite(self, self->m_args[DFlowNode_ARG_PARAM], Register_VPMVCD_RD_SETUP, &state, NULL);

   case DATAFLOW_VPM_WRITE_SETUP:
      return DFlowNode_SpecialRegWrite(self, self->m_args[DFlowNode_ARG_PARAM], Register_VPMVCD_WR_SETUP, &state, NULL);

   // Packing.
   case DATAFLOW_PACK_COL_REPLICATE  :
   case DATAFLOW_PACK_COL_R  :
   case DATAFLOW_PACK_COL_G  :
   case DATAFLOW_PACK_COL_B  :
   case DATAFLOW_PACK_COL_A  :
   case DATAFLOW_PACK_16A    :
   case DATAFLOW_PACK_16B    :
      {
         if (self->m_args[DFlowNode_ARG_BACKGROUND] == NULL)
         {
            QPUResources_Preference  pref;
            QPUResource              *output;
            QPUOperand               dest;

            const QPUOperand *src = DFlowNode_GetInput(self, &state, self->m_args[DFlowNode_ARG_OPERAND]);
            if (src == NULL)
               return DFlowNode_NOT_SCHEDULABLE;

            // If child results are no longer needed, free them up
            DFlowNode_RetireOurChildren(self, &state);

            pref = QPUResources_PREFER_ACC;
            if (self->m_flavour == DATAFLOW_PACK_16A || self->m_flavour == DATAFLOW_PACK_16B)
               pref = QPUResources_FORCE_REGA;

            output = DFlowNode_AllocateOutputReg(self, &state, DFlowNode_FOR_RESULT, pref);
            if (output == NULL)
               return DFlowNode_NOT_SCHEDULABLE;

            QPUOperand_ConstrReg(&dest, QPUResource_Name(output));

            // If the src has an unpack, check that it won't clash with the pack we need. If it does, we'll need an extra move.
            if (!DFlowNode_CheckPackUnpackClash(self, src))
            {
               DFlowNode *unpackNode = InstrState_GetOwner(&state, src->m_reg);
               return DFlowNode_InsertExtraMov(unpackNode, src, &state, DFlowNode_FULLY_RETIRE, QPUResources_PREFER_ACC);
            }

            if (DFlowNode_SetPackedMov(self, &state, src, &dest))
               return InstrState_CommitChangesOp(&state, self, &dest);
         }
         else
         {
            DFlowNode *destNode = self->m_args[DFlowNode_ARG_BACKGROUND];

            const QPUOperand *dest = &destNode->m_result;
            const QPUOperand *src  = DFlowNode_GetInput(self, &state, self->m_args[DFlowNode_ARG_OPERAND]);

            // Packing into a background register. We need to assert that our input (also our output) only has us as a child.
            // There are cases where this isn't true, and a MOV should have been added to the graph.
            assert(self->m_args[DFlowNode_ARG_BACKGROUND]->m_parents.m_size == 1);

            if (src == NULL || dest == NULL)
               return DFlowNode_NOT_SCHEDULABLE;

            // Packing normally writes onto part of an existing register - ensure it can and isn't R4 or R5
            if (!QPUOperand_IsRegister(dest) || !Register_IsNormalRegOrAcc(QPUOperand_ValueRegister(dest)) ||
                                                 Register_IsReadOnly(QPUOperand_ValueRegister(dest)))
            {
               if (!DFlowNode_RecordUniformsAndVaryings(destNode, dest, &state))
                  return DFlowNode_NOT_SCHEDULABLE;

               return DFlowNode_InsertExtraMov(destNode, dest, &state, DFlowNode_FULLY_RETIRE, QPUResources_PREFER_ACC);
            }

            // If the src has an unpack, check that it won't clash with the pack we need. If it does, we'll need an extra move.
            if (!DFlowNode_CheckPackUnpackClash(self, src))
            {
               DFlowNode *unpackNode = InstrState_GetOwner(&state, src->m_reg);
               return DFlowNode_InsertExtraMov(unpackNode, src, &state, DFlowNode_FULLY_RETIRE, QPUResources_PREFER_ACC);
            }

            if (DFlowNode_SetPackedMov(self, &state, src, dest))
            {
               // If child results are no longer needed, free them up
               DFlowNode_RetireOurChildren(self, &state);

               if (QPUOperand_IsRegister(dest))
               {
                  Register_Enum  destReg = QPUOperand_ValueRegister(dest);
                  DFlowNode_SetWrittenAt(&state,  destReg, 0);
                  DFlowNode_SetReferenced(self, &state, destReg, 0);
               }

               return InstrState_CommitChangesOp(&state, self, dest);
            }
         }
      }
      return DFlowNode_NOT_SCHEDULABLE;

   case DATAFLOW_MOV         :
   case DATAFLOW_LOGICAL_NOT : // Uses the bool_rep to do the negation
      {
         QPUOperand           operand;

         const QPUOperand *op = DFlowNode_GetInput(self, &state, self->m_args[DFlowNode_ARG_OPERAND]);
         if (op == NULL)
            return DFlowNode_NOT_SCHEDULABLE;

         QPUOperand_ConstrCopy(&operand, op);

         // It's a mov
         if (khrn_options.glsl_optimizations_on)
         {
            // Mov optimization
            // See if this is a special case where the mov isn't really needed.
            // If the mov nodes child only has one parent, we don't need it
            if (!(QPUOperand_IsRegister(op) && (QPUOperand_ValueRegister(op) == Register_UNIFORM_READ || QPUOperand_ValueRegister(op) == Register_VARYING_READ)))
            {
               DFlowNode *child = NodeList_front(DFlowNode_Children(self));
               if (NodeList_size(DFlowNode_Parents(child)) == 1)
               {
                  // Simply copy the child result
                  return InstrState_CommitChangesOp(&state, self, op);
               }
            }
         }

         if (QPUOperand_IsRegister(&operand))
         {
            QPUResource *output;

            // If child results are no longer needed, free them up
            DFlowNode_RetireOurChildren(self, &state);

            if (state.m_outputRegistersFree == Register_FILE_NONE)
               return DFlowNode_NOT_SCHEDULABLE;

            output = DFlowNode_AllocateOutputReg(self, &state, DFlowNode_FOR_RESULT, QPUResources_PREFER_ACC);
            if (output == NULL)
               return DFlowNode_NOT_SCHEDULABLE;

            if (QPUGenericInstr_SetMovOpRes(&state.m_gi, &operand, output))
               return InstrState_CommitChangesRes(&state, self, output);

            return DFlowNode_NOT_SCHEDULABLE;
         }
         else if (QPUOperand_IsSmallInt(&operand))
            return DFlowNode_DoConstInt(self, &state);
         else if (QPUOperand_IsSmallFloat(&operand))
            return DFlowNode_DoConstFloat(self, &state);
         else
            assert(0);
      }
      return DFlowNode_NOT_SCHEDULABLE;

   case DATAFLOW_UNPACK_COL_R:
   case DATAFLOW_UNPACK_COL_G:
   case DATAFLOW_UNPACK_COL_B:
   case DATAFLOW_UNPACK_COL_A:
   case DATAFLOW_UNPACK_16A  :
   case DATAFLOW_UNPACK_16A_F:
   case DATAFLOW_UNPACK_16B  :
   case DATAFLOW_UNPACK_16B_F:
   case DATAFLOW_UNPACK_8A   :
   case DATAFLOW_UNPACK_8B   :
   case DATAFLOW_UNPACK_8C   :
   case DATAFLOW_UNPACK_8D   :
   case DATAFLOW_UNPACK_8R   :
      {
         QPUOperand           operand;
         VirtualUnpack_Enum   unpackCode;
         bool                 unpackNeedsMov = false;

         const QPUOperand *op = DFlowNode_GetInput(self, &state, self->m_args[DFlowNode_ARG_OPERAND]);
         if (op == NULL)
            return DFlowNode_NOT_SCHEDULABLE;

         QPUOperand_ConstrCopy(&operand, op);

         unpackCode = DFlowNode_GetUnpackCode(self);

         {
            DFlowNode *child = NodeList_front(DFlowNode_Children(self));

            // We need to detect if our child wants to be used both packed and unpacked by the same parent node.
            // Look through our parent's children to see if our child is listed.
            NodeList_const_iterator  iter;
            const NodeList          *parents = DFlowNode_Parents(self);

            for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
            {
               DFlowNode               *parent = NodeList_const_star(iter);
               NodeList_const_iterator  citer;
               const NodeList          *children = DFlowNode_Children(parent);

               for (citer = NodeList_const_begin(children); citer != NodeList_const_end(children); NodeList_const_next(&citer))
               {
                  if (child == NodeList_const_star(citer))
                  {
                     unpackNeedsMov = true;
                     break;
                  }
               }
            }
         }

         if (!unpackNeedsMov)
         {
            // We also need to detect when an unpack's parent has multiple unpack children.
            // In these cases we'll need an extra mov.
            uint32_t unpackChildCount;

            NodeList_const_iterator iter;
            const NodeList *parents = DFlowNode_Parents(self);

            for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
            {
               DFlowNode   *starIter = NodeList_const_star(iter);

               // Packing the result of an unpack is bad news, so don't do it
               switch (DFlowNode_Flavour(starIter))
               {
               case DATAFLOW_PACK_COL_REPLICATE  :
               case DATAFLOW_PACK_COL_R  :
               case DATAFLOW_PACK_COL_G  :
               case DATAFLOW_PACK_COL_B  :
               case DATAFLOW_PACK_COL_A  :
               case DATAFLOW_PACK_16A    :
               case DATAFLOW_PACK_16B    :
                  unpackNeedsMov = true;
                  break;

               default:
                  break;
               }

               unpackChildCount = 0;

               {
                  NodeList_const_iterator iter2;
                  const NodeList *children = DFlowNode_Children(starIter);

                  for (iter2 = NodeList_const_begin(children); iter2 != NodeList_const_end(children); NodeList_const_next(&iter2))
                  {
                     if (DFlowNode_GetUnpackCode(NodeList_const_star(iter2)) != VirtualUnpack_NONE)
                        unpackChildCount++;
                  }
               }

               if (unpackChildCount > 1)
               {
                  unpackNeedsMov = true;
                  break;
               }
            }
         }

         if (!unpackNeedsMov)
         {
            NodeList_const_iterator iter;
            const NodeList *children = DFlowNode_Children(self);

            for (iter = NodeList_const_begin(children); iter != NodeList_const_end(children); NodeList_const_next(&iter))
            {
               DFlowNode   *starIter = NodeList_const_star(iter);

               switch (DFlowNode_Flavour(starIter))
               {
               case DATAFLOW_PACK_COL_REPLICATE  :
               case DATAFLOW_PACK_COL_R  :
               case DATAFLOW_PACK_COL_G  :
               case DATAFLOW_PACK_COL_B  :
               case DATAFLOW_PACK_COL_A  :
               case DATAFLOW_PACK_16A    :
               case DATAFLOW_PACK_16B    :
                  // Nasty case of pack with multiple parents (possibly forcing the reg file) into unpack
                  unpackNeedsMov = (NodeList_size(DFlowNode_Parents(starIter)) > 1);
                  break;
               default:
                  break;
               }
            }
         }

         // Unpack must come from regfile A, so insert a mov there first if needed
         // Can also come from R4 with some restrictions

         if (!QPUOperand_IsRegister(&operand) || Register_GetFile(QPUOperand_ValueRegister(&operand)) != Register_FILE_A)
         {
            if (QPUOperand_ValueRegister(&operand) != Register_ACC4 || !VirtualUnpack_IsR4Compatible(unpackCode))
            {
               // We need to get this in regfile A
               return DFlowNode_InsertExtraMov(self->m_args[DFlowNode_ARG_OPERAND], &operand,
                                               &state, DFlowNode_FULLY_RETIRE, QPUResources_FORCE_REGA);
            }
         }

         if (QPUOperand_IsRegister(&operand))
         {
            if (unpackNeedsMov)
            {
               QPUResource *output;

               // If child results are no longer needed, free them up
               DFlowNode_RetireOurChildren(self, &state);

               if (state.m_outputRegistersFree == Register_FILE_NONE)
                  return DFlowNode_NOT_SCHEDULABLE;

               output = DFlowNode_AllocateOutputReg(self, &state, DFlowNode_FOR_RESULT, QPUResources_PREFER_ACC);
               if (output == NULL)
                  return DFlowNode_NOT_SCHEDULABLE;

               if (!QPUOperand_SetUnpack(&operand, unpackCode))
                  return DFlowNode_NOT_SCHEDULABLE;

               if (QPUGenericInstr_SetMovOpRes(&state.m_gi, &operand, output))
                  return InstrState_CommitChangesRes(&state, self, output);

               return DFlowNode_NOT_SCHEDULABLE;
            }
            else
            {
               NodeList_const_iterator iter;
               int32_t                 count = 0;
               const NodeList          *parents = DFlowNode_Parents(self);

               assert(QPUOperand_IsRegister(&operand));

               // Need to adjust the reference count on the real value to include multiple parented unpacks.
               // In practice this just means adding our parent count - 1 to the underlying ref count.
               // We need to set the register in use for each parent node so that the reference counts are correct
               for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
               {
                  if (DFlowNode_HasUnconsumedReference(NodeList_const_star(iter), state.m_slot))
                     count++;
               }

               QPUResource_SetReferenced(InstrState_GetResource(&state, QPUOperand_ValueRegister(&operand)), NodeList_front(DFlowNode_Children(self)), count - 1);

               // Unpack directly in parent node via downward bypass
               QPUOperand_ConstrType(&operand, QPUOperand_BYPASS, unpackCode);

               return InstrState_CommitChangesOp(&state, self, &operand);
            }
         }
         else if (QPUOperand_IsSmallInt(&operand))
            return DFlowNode_DoConstInt(self, &state);
         else if (QPUOperand_IsSmallFloat(&operand))
            return DFlowNode_DoConstFloat(self, &state);
         else
            assert(0);
      }
      return DFlowNode_NOT_SCHEDULABLE;

   case DATAFLOW_UNPACK_PLACEHOLDER_R:
   case DATAFLOW_UNPACK_PLACEHOLDER_B:
      return DFlowNode_NOTHING_TO_SCHEDULE;

   case DATAFLOW_SCOREBOARD_WAIT:
      if (Scheduler_FirstEmptySlot(sched) < 2)
         return DFlowNode_NOT_SCHEDULABLE;

      if (!Scheduler_AllThreadSwitchesDone(sched))
         return DFlowNode_NOT_SCHEDULABLE;

      // We will likely remove this later
      if (QPUGenericInstr_SetSignal(&state.m_gi, Sig_SBLOCK))
         return InstrState_CommitChanges(&state);
      else
         return DFlowNode_NOT_SCHEDULABLE;

   default:
      assert(0);
   }

   return DFlowNode_NOTHING_TO_SCHEDULE;
}

DFlowNode_ScheduleStatus DFlowNode_TextureWrite(DFlowNode *self, InstrState *state, Scheduler *sched)
{
   const QPUOperand         *operand;
   uint32_t                 unit;
   uint32_t                 uniformType;
   DFlowNode_ScheduleStatus status;

   // Get the operand into a sensible place
   operand = DFlowNode_GetInput(self, state, self->m_args[DFlowNode_ARG_PARAM]);
   if (operand == NULL)
      return DFlowNode_NOT_SCHEDULABLE;

   // If our input is a uniform, we need to insert a move first (uniforms will be implictly read by this operation)
   if (QPUOperand_IsRegister(operand) && QPUOperand_ValueRegister(operand) == Register_UNIFORM_READ)
      return DFlowNode_InsertExtraMov(self->m_args[DFlowNode_ARG_PARAM], operand, state, DFlowNode_FULLY_RETIRE, QPUResources_PREFER_ACC);

   // If this is a provoking write, we might need to do some spill of accumulators
   // and we need to retrofit the thread switch into the N-2 instruction
   //
   if (self->m_threadSwitch && Scheduler_AllowThreadswitch(sched))
   {
      assert(self->m_flavour == DATAFLOW_TEX_SET_COORD_S);

      switch (self->m_schedPass)
      {
      case 0: // Initial step
         // We can't preserve R4, R5 is special and condition flags are not preserved over threads.
         if (InstrState_IsRegisterInUse(state, Register_ACC4) || InstrState_IsRegisterInUse(state, Register_ACC5) || InstrState_IsRegisterInUse(state, Register_VIRTUAL_FLAGS))
            return DFlowNode_NOT_SCHEDULABLE;

         Scheduler_SetActiveNode(sched, self);
         self->m_schedPass = 1;
         return DFlowNode_INSERTED_EXTRA_INSTR;

      case 1: // Spilling registers
         {
            DFlowNode      *param      = self->m_args[DFlowNode_ARG_PARAM];
            Register_Enum  src         = QPUOperand_ValueRegister(&param->m_result);
            uint32_t       regsToMove = 0;
            uint32_t       i;

            for (i = 0; i < 4; ++i)
            {
               Register_Enum r = (Register_Enum)(Register_ACC0 + i);

               if (InstrState_IsRegisterInUse(state, r))
               {
                  regsToMove++;

                  if (InstrState_IsReadableReg(state, r))
                  {
                     // No need to save the input if it is being gobbled by this instruction
                     if (r != src || NodeList_size(DFlowNode_Parents(param)) > 1)
                     {
                        QPUOperand  operand;
                        DFlowNode *node = InstrState_GetOwner(state, r);

                        QPUResources_Preference   force = QPUResources_FORCE_REGB;

                        if (state->m_outputRegistersFree == Register_FILE_A)
                           force = QPUResources_FORCE_REGA;

                        QPUOperand_ConstrReg(&operand, r);

                        return DFlowNode_InsertExtraMov(node, &operand, state, DFlowNode_FULLY_RETIRE, force);
                     }
                     else
                     {
                        regsToMove--;
                     }
                  }
               }
            }

            // Have all registers have been spilled?
            if (regsToMove != 0)
            {
               QPUGenericInstr_SetFull(&state->m_gi);
               InstrState_CommitChanges(state);
               return DFlowNode_INSERTED_EXTRA_INSTR;
            }

            self->m_schedPass = 2;
         }

         return DFlowNode_INSERTED_EXTRA_INSTR;

      case 2:
         {
            // If we can schedule a thread-switch then issue the mov to S
            if (!InstrState_InsertThreadSwitch(state))
            {
               QPUGenericInstr_SetFull(&state->m_gi);
               InstrState_CommitChanges(state);
               return DFlowNode_INSERTED_EXTRA_INSTR;
            }
         }
         // Thread switch invalidates the condition codes, so don't allow later optimizations.
         Scheduler_SetCurrentFlagsCondition(state->m_scheduler, NULL);
         // Fall through to code below
      }
   }

   unit        = self->m_sampler;
   uniformType = InstrState_GetNextTextureUniform(state, unit);
   // Ensure we've not added a uniform in this instruction already
   //assert(state->m_uniforms.size() == state->m_originalUniforms->size());

   //state->m_uniforms.push_back(Uniform(uniformType,
   //                                    m_args[ARG_SAMPLER]->m_uniform.m_constSampler.m_location & ~0x80000000));
   InstrState_AddUniform(state, uniformType, self->m_args[DFlowNode_ARG_SAMPLER]->m_uniform.m_constSampler.m_csLocation & ~0x80000000);

   if (uniformType < BACKEND_UNIFORM_TEX_NOT_USED)
      InstrState_SetNextTextureUniform(state, unit, uniformType + 1);

   if (self->m_flavour == DATAFLOW_TEX_SET_COORD_S)
   {
      // Account for the texture read
      InstrState_SetNextTextureUniform(state, unit, BACKEND_UNIFORM_TEX_PARAM0);
   }

   status = DFlowNode_SpecialRegWrite(self, self->m_args[DFlowNode_ARG_PARAM], DFlowNode_GetTextureReg(self), state, NULL);

   if (self->m_schedPass == 2 && status != DFlowNode_NOT_SCHEDULABLE)
      Scheduler_SetActiveNode(sched, NULL);

   return status;
}

bool DFlowNode_SetConditionFlags(DFlowNode *self, InstrState *state, DFlowNode *conditionNode)
{
   QPUResource             *nop;
   const QPUOperand        *condOp;
   QPUGenericInstr_MovUnit unitUsed;

   // Second pass generates a move from the underlying comparison node to NOP in order to set the flags
   if (InstrState_IsRegisterInUse(state, Register_VIRTUAL_FLAGS))
      return false;

   if (khrn_options.glsl_optimizations_on && conditionNode == Scheduler_CurrentFlagsCondition(state->m_scheduler))
   {
      // The flags were already set by this node and are still valid - make it look like we wrote them again
      DFlowNode_SetWrittenAt(state, Register_VIRTUAL_FLAGS, 0);
      DFlowNode_SetReferenced(self, state, Register_VIRTUAL_FLAGS, 1);  // Force ref count to 1
      return true;
   }

   nop = InstrState_GetResource(state, Register_NOP);

   condOp = DFlowNode_GetInput(self, state, conditionNode);
   if (condOp == NULL)
      return false;

   if (QPUGenericInstr_SetMovOpResEx(&state->m_gi, condOp, nop, QPUGenericInstr_ADD, &unitUsed))
   {
      assert(unitUsed == QPUGenericInstr_ADD);

      // Set the flags, and ensure flags aren't read for 1 cycle
      QPUGenericInstr_SetFlags(&state->m_gi, true);
      Scheduler_SetCurrentFlagsCondition(state->m_scheduler, conditionNode);

      DFlowNode_SetWrittenAt(state, Register_VIRTUAL_FLAGS, 0);
      DFlowNode_SetReferenced(self, state, Register_VIRTUAL_FLAGS, 1);  // Force ref count to 1

      if (QPUGenericInstr_SetAddOutputRegisterRes(&state->m_gi, nop))
         return true;
   }

   return false;
}

bool DFlowNode_SetPackedMov(DFlowNode *self, InstrState *state, const QPUOperand *from, const QPUOperand *to)
{
   switch (self->m_flavour)
   {
   case DATAFLOW_PACK_COL_REPLICATE :
   case DATAFLOW_PACK_COL_R  :
   case DATAFLOW_PACK_COL_G  :
   case DATAFLOW_PACK_COL_B  :
   case DATAFLOW_PACK_COL_A  :
      return QPUGenericInstr_SetPackedMovMul(&state->m_gi, from, to, (Mul_Pack_Enum)(Mul_Pack_8R + (self->m_flavour - DATAFLOW_PACK_COL_REPLICATE)));
   case DATAFLOW_PACK_16A    :
   case DATAFLOW_PACK_16B    :
      return QPUGenericInstr_SetPackedMovRegA(&state->m_gi, from, to, (RegA_Pack_Enum)(RegA_Pack_16A + (self->m_flavour - DATAFLOW_PACK_16A)));

   default:
      UNREACHABLE();
      return false;
   }
}

void DFlowNode_SetResultInt(DFlowNode *self, int8_t val)
{
   QPUOperand_ConstrInt(&self->m_result, val);
}

void DFlowNode_SetResultFloat(DFlowNode *self, float val)
{
   QPUOperand_ConstrFloat(&self->m_result, val);
}

void DFlowNode_SetResultReg(DFlowNode *self, Register_Enum reg)
{
   QPUOperand_ConstrReg(&self->m_result, reg);
}

static bool DFlowNode_RecordUniformsAndVaryings(DFlowNode *node, const QPUOperand *operand, InstrState *state)
{
   // We're about to consume a uniform, so ensure it's in our list
   if (QPUOperand_IsRegister(operand))
   {
      if (QPUOperand_ValueRegister(operand) == Register_UNIFORM_READ)
         InstrState_AddUniformRead(state, node);
      else if (QPUOperand_ValueRegister(operand) == Register_VARYING_READ)
      {
         if (!InstrState_AddVaryingRead(state, node))
            return false;
      }
   }
   return true;
}

// Insert mov for operand of binary operation.
// NOTE: must ONLY be called from SetupForTwoOperands!
DFlowNode_ScheduleStatus SetupForTwoOperandsInsertExtraMov(DFlowNode *self, const QPUOperand *leftOp, const QPUOperand *rightOp, InstrState *state)
{
   QPUOperand              movingOp;
   DFlowNode               *movingNode;
   QPUResources_Preference pref;
   QPUResource             *output;
   QPUOperand              dest;

   assert(InstrState_IsReadableOp(state, leftOp));
   assert(InstrState_IsReadableOp(state, rightOp));

   QPUOperand_ConstrCopy(&movingOp, rightOp);

   movingNode = self->m_args[DFlowNode_ARG_RIGHT];

   // Always favour moving a register
   if (QPUOperand_IsRegister(leftOp))
   {
      movingOp = *leftOp;
      movingNode = self->m_args[DFlowNode_ARG_LEFT];
   }

   // Special case when the node we're moving has parent nodes with bypass (i.e. unpack)
   if (Register_GetFile(QPUOperand_ValueRegister(&movingOp)) == Register_FILE_A)
      pref = QPUResources_ACC_OR_REGB;
   else
      pref = QPUResources_ACC_OR_REGA;

   // Only force an A, if it wasnt one to begin with
   if (pref == QPUResources_ACC_OR_REGA)
   {
      NodeList_const_iterator  iter;
      const NodeList          *parents = DFlowNode_Parents(movingNode);

      for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
      {
         DFlowNode   *starIter = NodeList_const_star(iter);

         if (QPUOperand_IsBypass(&starIter->m_result))
         {
            pref = QPUResources_FORCE_REGA;
            break;
         }
      }
   }

   output = DFlowNode_AllocateOutputReg(movingNode, state, DFlowNode_INTERMEDIATE, pref);
   if (output == NULL)
      return DFlowNode_NOT_SCHEDULABLE;

   QPUOperand_ConstrReg(&dest, QPUResource_Name(output));

   // Special case to avoid moving to ourself (mov x, x)
   if (QPUOperand_ValueRegister(&dest) == QPUOperand_ValueRegister(&movingOp))
   {
      DFlowNode_SetWrittenAt(state, QPUOperand_ValueRegister(&dest), -3);  // We haven't really done a move, so ensure we don't keep the delay

      // Modify the child result we moved
      if (InstrState_CommitChangesOp(state, movingNode, &dest) == DFlowNode_NOT_SCHEDULABLE)
         return DFlowNode_NOT_SCHEDULABLE;

      return DFlowNode_INSERTED_EXTRA_INSTR;
   }

   // Move the reference counts etc.
   if (QPUOperand_IsRegister(&movingOp))
   {
      if (Register_IsNormalRegOrAcc(QPUOperand_ValueRegister(&movingOp)))
         InstrState_MoveRefCount(state, movingNode, &movingOp, &dest);
   }

   if (QPUGenericInstr_SetMovOpOp(&state->m_gi, &movingOp, &dest))
   {
      // If we moved from a uniform or varying, we need to record that
      if (!DFlowNode_RecordUniformsAndVaryings(movingNode, &movingOp, state))
         return DFlowNode_NOT_SCHEDULABLE;

      // Modify the child result we moved
      if (InstrState_CommitChangesOp(state, movingNode, &dest) == DFlowNode_NOT_SCHEDULABLE)
         return DFlowNode_NOT_SCHEDULABLE;

      return DFlowNode_INSERTED_EXTRA_INSTR;
   }

   return DFlowNode_NOT_SCHEDULABLE;
}

DFlowNode_ScheduleStatus DFlowNode_SetupForTwoOperands(DFlowNode *self, QPUOperand *leftOp, QPUOperand *rightOp, InstrState *state)
{
   DFlowNode        *lnode;
   DFlowNode        *rnode;
   const QPUOperand *lo;
   const QPUOperand *ro;
   bool              reverse = false;

   if (state->m_outputRegistersFree == Register_FILE_NONE)
      return DFlowNode_NOT_SCHEDULABLE;

   lnode = self->m_args[DFlowNode_ARG_LEFT];
   rnode = self->m_args[DFlowNode_ARG_RIGHT];
   lo = DFlowNode_PeekInput(self, state, lnode);
   ro = DFlowNode_PeekInput(self, state, rnode);

   if (!lo || !ro)
      return DFlowNode_NOT_SCHEDULABLE;

   switch (self->m_flavour)
   {
      case DATAFLOW_RSUB              :
      case DATAFLOW_LESS_THAN_EQUAL   :
      case DATAFLOW_GREATER_THAN      : reverse = true; break;
      default                         : reverse = false; break;
   }

   if (reverse)
   {
      *leftOp  = *ro;
      *rightOp = *lo;
      lnode = self->m_args[DFlowNode_ARG_RIGHT];
      rnode = self->m_args[DFlowNode_ARG_LEFT];
   }
   else
   {
      *leftOp  = *lo;
      *rightOp = *ro;
      lnode = self->m_args[DFlowNode_ARG_LEFT];
      rnode = self->m_args[DFlowNode_ARG_RIGHT];
   }

   if (!InstrState_IsReadableOp(state, leftOp) || !InstrState_IsReadableOp(state, rightOp))
      return DFlowNode_NOT_SCHEDULABLE;

   if (DFlowNode_ExtraMovNeeded(leftOp, rightOp))
   {
      if (reverse)
         return SetupForTwoOperandsInsertExtraMov(self, rightOp, leftOp, state);
      else
         return SetupForTwoOperandsInsertExtraMov(self, leftOp, rightOp, state);
   }

   if (!DFlowNode_RecordUniformsAndVaryings(lnode, leftOp, state) ||
       !DFlowNode_RecordUniformsAndVaryings(rnode, rightOp, state))
      return DFlowNode_NOT_SCHEDULABLE;

   return DFlowNode_NOTHING_TO_SCHEDULE;
}

bool DFlowNode_SetupForSingleOperand(DFlowNode *self, QPUOperand *operand, InstrState *state)
{
   const QPUOperand *o;

   if (state->m_outputRegistersFree == Register_FILE_NONE)
      return false;

   o = DFlowNode_GetInput(self, state, self->m_args[DFlowNode_ARG_OPERAND]);
   if (o == NULL)
      return false;

   *operand = *o;

   if (!InstrState_IsReadableOp(state, operand))
      return false;

   return true;
}

bool DFlowNode_ExtraMovNeeded(const QPUOperand *leftOp, const QPUOperand *rightOp)
{
   Register_File    leftFile;
   Register_File    rightFile;

   // If the two input registers are identical (and normal registers), we don't need a mov, otherwise we might
   if (QPUOperand_IsRegister(leftOp) && QPUOperand_IsRegister(rightOp) &&
       QPUOperand_ValueRegister(leftOp) == QPUOperand_ValueRegister(rightOp))
   {
      if (Register_IsNormalRegOrAcc(QPUOperand_ValueRegister(leftOp)))
         return false;
      else
         return true;
   }

   leftFile  = Register_GetFile(QPUOperand_ValueRegister(leftOp));
   rightFile = Register_GetFile(QPUOperand_ValueRegister(rightOp));

   if (QPUOperand_IsSmallConst(leftOp))
      leftFile = Register_FILE_B;

   if (QPUOperand_IsSmallConst(rightOp))
      rightFile = Register_FILE_B;

   if (leftFile == rightFile && (leftFile == Register_FILE_A || leftFile == Register_FILE_B))
   {
      if (QPUOperand_IsRegister(leftOp) && QPUOperand_IsRegister(rightOp))
         return true;
      else if (QPUOperand_IsSmallConst(leftOp) && QPUOperand_IsSmallConst(rightOp))
         return true;
      else if (QPUOperand_IsRegister(leftOp) && leftFile == Register_FILE_B && QPUOperand_IsSmallConst(rightOp))
         return true;
      else if (QPUOperand_IsRegister(rightOp) && rightFile == Register_FILE_B && QPUOperand_IsSmallConst(leftOp))
         return true;
   }

   return false;
}

// Insert a mov replacing a node's output
DFlowNode_ScheduleStatus DFlowNode_InsertExtraMov(DFlowNode *self, const QPUOperand *operand, InstrState *state,
                                                  DFlowNode_RetireMode retMode, QPUResources_Preference pref)
{
   NodeList_const_iterator iter;
   const NodeList          *parents;
   QPUResource             *output;
   QPUOperand              dest;

   if (!InstrState_IsReadableOp(state, operand))
      return DFlowNode_NOT_SCHEDULABLE;

   // Special case when the node we're moving has parent nodes with bypass (i.e. unpack)
   parents = DFlowNode_Parents(self);

   for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
   {
      if (QPUOperand_IsBypass(&NodeList_const_star(iter)->m_result))
      {
         pref = QPUResources_FORCE_REGA;
         break;
      }
   }

   output = DFlowNode_AllocateOutputReg(self, state, DFlowNode_FOR_RESULT, pref);
   if (output == NULL)
      return DFlowNode_NOT_SCHEDULABLE;

   QPUOperand_ConstrReg(&dest, QPUResource_Name(output));

   if (QPUOperand_ValueRegister(&dest) == QPUOperand_ValueRegister(operand))
   {
      DFlowNode_SetWrittenAt(state, QPUOperand_ValueRegister(&dest), -3);  // We haven't really done a move, so ensure we don't keep the delay

      // Modify the child result we moved
      if (InstrState_CommitChangesOp(state, self, &dest) == DFlowNode_NOT_SCHEDULABLE)
         return DFlowNode_NOT_SCHEDULABLE;

      return DFlowNode_INSERTED_EXTRA_INSTR;
   }

   // Retire the register that we're moving from
   if (QPUOperand_IsRegister(operand))
   {
      if (retMode == DFlowNode_NORMAL_RETIRE)
         InstrState_RetireOp(state, self, operand);
      else
      {
         // Move the reference counts etc.
         if (QPUOperand_IsRegister(operand))
         {
            if (Register_IsNormalRegOrAcc(QPUOperand_ValueRegister(operand)))
               InstrState_MoveRefCount(state, self, operand, &dest);
         }
      }
   }

   if (QPUGenericInstr_SetMovOpOp(&state->m_gi, operand, &dest))
   {
      // Modify the child result we moved
      if (InstrState_CommitChangesOp(state, self, &dest) == DFlowNode_NOT_SCHEDULABLE)
         return DFlowNode_NOT_SCHEDULABLE;

      return DFlowNode_INSERTED_EXTRA_INSTR;
   }

   return DFlowNode_NOT_SCHEDULABLE;
}

void DFlowNode_RetireOurChildren(DFlowNode *self, InstrState *state)
{
   // If child results are no longer needed, free them up
   NodeList_const_iterator iter;
   const NodeList *children = DFlowNode_Children(self);

   for (iter = NodeList_const_begin(children); iter != NodeList_const_end(children); NodeList_const_next(&iter))
   {
      DFlowNode   *starIter = NodeList_const_star(iter);

      InstrState_RetireOp(state, starIter, &starIter->m_result);
   }
}

bool DFlowNode_CanShortcutOutputReg(DFlowNode *self)
{
   // We can only have one parent for a shortcut output (ioDep parents can cause circular deps)
   if (NodeList_size(DFlowNode_Parents(self)) == 1/* && IoParents().size() == 0*/)
   {
      DFlowNode *parent;

      switch (DFlowNode_Flavour(self))
      {
      case DATAFLOW_CONDITIONAL :
      case DATAFLOW_PACK_COL_R  :
      case DATAFLOW_PACK_COL_G  :
      case DATAFLOW_PACK_COL_B  :
      case DATAFLOW_PACK_COL_A  :
      case DATAFLOW_PACK_16A    :
      case DATAFLOW_PACK_16B    :
         return false;

      default:
         break;
      }

      parent = NodeList_star(NodeList_begin(&self->m_parents));

      if (parent->m_threadSwitch)
         return false;

      switch (DFlowNode_Flavour(parent))
      {
      case DATAFLOW_VPM_READ_SETUP :
      case DATAFLOW_VPM_WRITE_SETUP :
      case DATAFLOW_FRAG_SUBMIT_STENCIL:
      case DATAFLOW_FRAG_SUBMIT_Z:
      case DATAFLOW_FRAG_SUBMIT_MS:
      case DATAFLOW_FRAG_SUBMIT_ALL:
      case DATAFLOW_TEX_SET_COORD_S:
      case DATAFLOW_TEX_SET_COORD_T:
      case DATAFLOW_TEX_SET_COORD_R:
      case DATAFLOW_TEX_SET_BIAS   :
      case DATAFLOW_TEX_SET_LOD    :
      case DATAFLOW_TEX_SET_DIRECT:
      case DATAFLOW_VERTEX_SET :
         return true;

      default:
         return false;
      }
   }
   return false;
}

bool DFlowNode_CanShortcutPack(DFlowNode *self, InstrState *state)
{
   const NodeList   *parents   = DFlowNode_Parents(self);
   const NodeList   *ioParents = DFlowNode_IoParents(self);

   if (NodeList_size(parents) == 1 && NodeList_size(ioParents) == 0)
   {
      DFlowNode *parent = NodeList_star(NodeList_begin(&self->m_parents));

      // Only the multiply based color packs are float based
      if (!DFlowNode_IsMulOperation(self))
         return false;

      // Can't shortcut this kind of pack if the instruction already has an unpack using reg file A
      if (QPUGenericInstr_UnpackUsed(&state->m_gi) && QPUGenericInstr_UnpackSrc(&state->m_gi) == UnpackSource_REGFILE_A)
         return false;

      switch (DFlowNode_Flavour(self))
      {
      // Can't shortcut a pack to a pack, or a conditional
      case DATAFLOW_CONDITIONAL :
      case DATAFLOW_PACK_COL_R  :
      case DATAFLOW_PACK_COL_G  :
      case DATAFLOW_PACK_COL_B  :
      case DATAFLOW_PACK_COL_A  :
      case DATAFLOW_PACK_16A    :
      case DATAFLOW_PACK_16B    :
         return false;

      default:
         break;
      }

      switch (DFlowNode_Flavour(parent))
      {
      case DATAFLOW_PACK_COL_REPLICATE :
      case DATAFLOW_PACK_COL_R  :
      case DATAFLOW_PACK_COL_G  :
      case DATAFLOW_PACK_COL_B  :
      case DATAFLOW_PACK_COL_A  :
      case DATAFLOW_PACK_16A    :
      case DATAFLOW_PACK_16B    :
         return true;

      default:
         break;
      }
   }
   return false;
}

bool DFlowNode_GetSpecificOutputReg(DFlowNode *self, InstrState *state, QPUResource **output)
{
   bool ret = true;
   *output = NULL;

   if (DFlowNode_CanShortcutOutputReg(self))
   {
      DFlowNode              *parent           = NodeList_star(NodeList_begin(&self->m_parents));
      uint32_t                realSiblingCount = 0;
      NodeList_const_iterator iter;
      const NodeList         *children = DFlowNode_Children(parent);

      // Real sibling count

      for (iter = NodeList_const_begin(children); iter != NodeList_const_end(children); NodeList_const_next(&iter))
      {
         DFlowNode   *starIter = NodeList_const_star(iter);

         if (starIter != self && DFlowNode_Flavour(starIter) != DATAFLOW_CONST_SAMPLER)
            realSiblingCount++;
      }

      // Our parent must only have us as a child, and must have no parents (i.e. a special reg destination)
      if (realSiblingCount == 0 && NodeList_size(DFlowNode_Parents(parent)) == 0)
      {
         const NodeList *ioChildren = DFlowNode_IoChildren(parent);

         if (QPUOperand_IsRegister(&self->m_result) && (QPUOperand_ValueRegister(&self->m_result) == Register_ACC4 ||
                                                        QPUOperand_ValueRegister(&self->m_result) == Register_ACC5))
         {
            // Make no attempt to shortcut. We are most likely moving a locked R4 or R5 somewhere.
            return true;
         }

         // Our parent's IO deps must also be satisfied, as we are shortcutting them
         for (iter = NodeList_const_begin(ioChildren); iter != NodeList_const_end(ioChildren); NodeList_const_next(&iter))
         {
            DFlowNode   *starIter = NodeList_const_star(iter);

            if ((starIter != self && DFlowNode_HasUnconsumedReference(starIter, state->m_slot)))
            {
               // There is an unresolved IO dependency.  Return true means give up on short-cut
               // because there could be a dependency loop. Return false means try again later.

               if (DFlowNode_TreeDepth(starIter) < DFlowNode_TreeDepth(self))
                  return true;

               return false;
            }
         }

/* NO LONGER REQUIRED DUE TO AUTO R5 MOVING
         // If any of our children have R5 as a result, we don't shortcut either.
         // These get locked and can prevent things moving on.
         for (iter = Children().begin(); iter != Children().end(); ++iter)
         {
            if ((*iter)->m_result.IsRegister() && (*iter)->m_result.ValueRegister() == Register_ACC5)
               return true;
         }
*/

         // A conditional child will break all of these as it writes and then overwrites a register
         // Can't do that for any FIFO based outputs, obviously
         switch (DFlowNode_Flavour(parent))
         {
         default :
            ret = true;
            break;

         case DATAFLOW_VPM_READ_SETUP:
            if (ret)
            {
               OptionalBypass_Set(&state->m_optionalBypass, parent);
               *output = InstrState_GetResource(state, Register_VPMVCD_RD_SETUP);
            }
            break;

         case DATAFLOW_VPM_WRITE_SETUP:
            if (ret)
            {
               OptionalBypass_Set(&state->m_optionalBypass, parent);
               *output = InstrState_GetResource(state, Register_VPMVCD_WR_SETUP);
            }
            break;

         case DATAFLOW_FRAG_SUBMIT_STENCIL:
         case DATAFLOW_FRAG_SUBMIT_Z:
         case DATAFLOW_FRAG_SUBMIT_MS:
         case DATAFLOW_FRAG_SUBMIT_ALL:
            if (ret)
            {
               OptionalBypass_Set(&state->m_optionalBypass, parent);
               *output = InstrState_GetResource(state, DFlowNode_GetIOOutputReg(parent));
            }
            break;

         case DATAFLOW_TEX_SET_COORD_S:
         case DATAFLOW_TEX_SET_COORD_T:
         case DATAFLOW_TEX_SET_COORD_R:
         case DATAFLOW_TEX_SET_BIAS   :
         case DATAFLOW_TEX_SET_LOD    :
         case DATAFLOW_TEX_SET_DIRECT:
             // Check that TMU swap has happened
            if (!InstrState_IsWritable(state, DFlowNode_GetTextureReg(parent)))
               return ret;

            if (ret)
            {
               // We can't bypass if any of our children (or our parent's children) are uniforms (or might be uniforms once scheduled)
               // TODO : NOT TRUE FOR DATAFLOW_TEX_SET_DIRECT (but currently the instr GetCoding() doesn't know)
               NodeList_const_iterator iter;
               bool anyUnifs = false;
               const NodeList *children = DFlowNode_Children(self);

               for (iter = NodeList_const_begin(children); iter != NodeList_const_end(children); NodeList_const_next(&iter))
               {
                  DFlowNode   *starIter = NodeList_const_star(iter);
                  QPUOperand  *result = &starIter->m_result;
                  DataflowFlavour   flavour = DFlowNode_Flavour(starIter);

                  if ((QPUOperand_IsRegister(result) && QPUOperand_ValueRegister(result) == Register_UNIFORM_READ) ||
                     ((flavour == DATAFLOW_CONST_FLOAT || flavour == DATAFLOW_CONST_INT) &&
                     DFlowNode_HasUnconsumedReference(starIter, state->m_slot)))
                  {
                     anyUnifs = true;
                     break;
                  }
               }

               // Check ourself too
               if ((QPUOperand_IsRegister(&self->m_result) && QPUOperand_ValueRegister(&self->m_result) == Register_UNIFORM_READ) ||
                  ((DFlowNode_Flavour(self) == DATAFLOW_CONST_FLOAT || DFlowNode_Flavour(self) == DATAFLOW_CONST_INT) &&
                  DFlowNode_HasUnconsumedReference(self, state->m_slot)))
               {
                  anyUnifs = true;
               }

               if (!anyUnifs)
               {
                  OptionalBypass_Set(&state->m_optionalBypass, parent);
                  *output = InstrState_GetResource(state, DFlowNode_GetTextureReg(parent));

                  if (DFlowNode_Flavour(parent) != DATAFLOW_TEX_SET_DIRECT)
                  {
                     uint32_t unit        = parent->m_sampler;
                     uint32_t uniformType = InstrState_GetNextTextureUniform(state, unit);
                     //state->m_uniforms.push_back(Uniform(uniformType,
                     //   parent->m_args[ARG_SAMPLER]->m_uniform.m_constSampler.m_location & ~0x80000000));
                     InstrState_AddUniform(state, uniformType, parent->m_args[DFlowNode_ARG_SAMPLER]->m_uniform.m_constSampler.m_csLocation & ~0x80000000);

                     if (uniformType < BACKEND_UNIFORM_TEX_NOT_USED)
                        InstrState_SetNextTextureUniform(state, unit, uniformType + 1);

                     if (DFlowNode_Flavour(parent) == DATAFLOW_TEX_SET_COORD_S)
                        InstrState_SetNextTextureUniform(state, unit, BACKEND_UNIFORM_TEX_PARAM0);

#if 0
                     if (parent->m_threadSwitch)
                     {
                        if (!InstrState_InsertThreadSwitch(state))
                           return false;
                     }
#endif
                  }
               }
            }
            break;

         case DATAFLOW_VERTEX_SET:
            if (ret && InstrState_IsReadableReg(state, Register_VPMVCD_WR_SETUP))
            {
               OptionalBypass_Set(&state->m_optionalBypass ,parent);
               *output = InstrState_GetResource(state, Register_VPM_WRITE);
            }
            break;
         }
      }

      if (ret && *output)
      {
         Register_Enum  outputName = QPUResource_Name(*output);
         // If we're giving a resource, mark it as used with a standard delay slot value
         DFlowNode_SetWrittenAt(state, outputName, 0);
         DFlowNode_SetReferenced(self, state, outputName, 0);
      }
   }
   else if (DFlowNode_CanShortcutPack(self, state))
   {
      DFlowNode               *parent = NodeList_star(NodeList_begin(&self->m_parents));
      NodeList_const_iterator  iter;
      const NodeList          *ioChildren = DFlowNode_IoChildren(parent);
      const NodeList          *children   = DFlowNode_Children(parent);

      // Our parent's children and IO deps must also be satisfied
      for (iter = NodeList_const_begin(ioChildren); iter != NodeList_const_end(ioChildren); NodeList_const_next(&iter))
      {
         DFlowNode   *starIter = NodeList_const_star(iter);

         if ((starIter != self && DFlowNode_HasUnconsumedReference(starIter, state->m_slot)))
         {
            // There is an unresolved IO dependency.  Return true means give up on short-cut
            // because there could be a dependency loop. Return false means try again later.

            if (DFlowNode_TreeDepth(starIter) < DFlowNode_TreeDepth(self))
               return true;

            return false;
         }
      }

      for (iter = NodeList_const_begin(children); iter != NodeList_const_end(children); NodeList_const_next(&iter))
      {
         DFlowNode   *starIter = NodeList_const_star(iter);
         QPUOperand  *result   = &starIter->m_result;

         if ((starIter != self && DFlowNode_HasUnconsumedReference(starIter, state->m_slot)))
            return true;

         if (QPUOperand_IsRegister(result) && Register_IsReadOnly(QPUOperand_ValueRegister(result)))
            return true;
      }

      if (ret)
      {
         if (parent->m_args[DFlowNode_ARG_BACKGROUND] == NULL)
         {
            QPUOperand  outputName;

            // Find an output
            if (DFlowNode_IsMulOperation(self))
            {
               *output = InstrState_FindFreeResource(state, DFlowNode_GetRegFile(self), QPUResources_PREFER_ACC);
               if (!QPUGenericInstr_SetPackMul(&state->m_gi, DFlowNode_GetMulPackCode(parent)))
                  return false;
            }
            else
            {
               *output = InstrState_FindFreeResource(state, DFlowNode_GetRegFile(self), QPUResources_FORCE_REGA);
               if (!QPUGenericInstr_SetPackRegA(&state->m_gi, DFlowNode_GetRegAPackCode(parent)))
                  return false;
            }

            if (*output == NULL)
               return false;  // Not schedulable yet

            // If we're giving a resource, mark it as used with a standard delay slot value
            DFlowNode_SetWrittenAt(state, QPUResource_Name(*output), 0);
            DFlowNode_SetReferenced(self, state, QPUResource_Name(*output), 0);

            OptionalBypass_Set(&state->m_optionalBypass, parent);

            QPUOperand_ConstrReg(&outputName, QPUResource_Name(*output));

            InstrState_AddNodeResult(state, parent, &outputName);
         }
         else
         {
            // Use the existing background register
            QPUOperand     *bgOp = &parent->m_args[DFlowNode_ARG_BACKGROUND]->m_result;
            Register_Enum  outputName;
            QPUOperand     outputOp;

            *output = InstrState_GetResource(state, QPUOperand_ValueRegister(bgOp));

            if (QPUResource_Name(*output) == Register_NOP)
            {
               // Likely trying to shortcut a pack with a background that is this node.
               // Since we don't have an output yet ('cos we're called from AllocateOutputReg), we just return true
               // to ask for a new one.
               *output = NULL;
               return true;
            }

            OptionalBypass_Set(&state->m_optionalBypass, parent);

            if (DFlowNode_IsMulOperation(self))
            {
               if (!QPUGenericInstr_SetPackMul(&state->m_gi, DFlowNode_GetMulPackCode(parent)))
                  return false;
            }
            else
            {
               if (!QPUGenericInstr_SetPackRegA(&state->m_gi, DFlowNode_GetRegAPackCode(parent)))
                  return false;
            }

            InstrState_RetireOp(state, parent->m_args[DFlowNode_ARG_BACKGROUND], &parent->m_args[DFlowNode_ARG_BACKGROUND]->m_result);

            outputName = QPUResource_Name(*output);
            QPUOperand_ConstrReg(&outputOp, outputName);

            InstrState_AddNodeResult(state, parent, &outputOp);
            DFlowNode_SetWrittenAt(state, outputName, 0);
            DFlowNode_SetReferenced(parent, state, outputName, 0);
         }
      }
   }

   return ret;
}

QPUResource *DFlowNode_AllocateOutputReg(DFlowNode *self, InstrState *state, DFlowNode_AllocRegMode mode, QPUResources_Preference p)
{
   QPUResource *output = NULL;

   // If we have no parents, allocate the NOP register as a destination
   if (NodeList_size(DFlowNode_Parents(self)) == 0)
      return InstrState_GetResource(state, Register_NOP);

   if (khrn_options.glsl_optimizations_on)
   {
      // Can we use a shortcut specific output register?
      if (mode == DFlowNode_FOR_RESULT)
         if (!DFlowNode_GetSpecificOutputReg(self, state, &output))
            return NULL; // Not schedulable yet
   }

   if (output == NULL)
   {
      // Prefer registers for longer lived results
      if (p == QPUResources_PREFER_ACC && NodeList_size(&self->m_parents) > 1 && self->m_lifespanGuess > 3)
         p = QPUResources_PREFER_REG;

      // Find an output
      output = InstrState_FindFreeResource(state, DFlowNode_GetRegFile(self), p);
      if (output == NULL)
         return NULL;

      // If we're giving a resource, mark it as used with a standard delay slot value
      DFlowNode_SetWrittenAt(state, QPUResource_Name(output), 0);
      DFlowNode_SetReferenced(self, state, QPUResource_Name(output), 0);
   }

   return output;
}

DFlowNode_ScheduleStatus DFlowNode_SpecialRegWrite(DFlowNode *self, DFlowNode *from, Register_Enum to,
                                                   InstrState *state, DFlowNode *conditional)
{
   QPUOperand              dest;
   const QPUOperand        *src;
   QPUGenericInstr_MovUnit unitUsed;

   if (!InstrState_IsWritable(state, to))
      return DFlowNode_NOT_SCHEDULABLE;

   QPUOperand_ConstrReg(&dest, to);
   src = DFlowNode_GetInput(self, state, from);

   if (src == NULL)
      return DFlowNode_NOT_SCHEDULABLE;

   DFlowNode_RetireOurChildren(self, state);

   if (QPUGenericInstr_SetMovOpOpEx(&state->m_gi, src, &dest, QPUGenericInstr_PREFER_ADD, &unitUsed))
   {
      DFlowNode_SetWrittenAt(state, to, 0);
      DFlowNode_SetReferenced(self, state, to, 0);

      if (conditional != NULL)
      {
         if (unitUsed == QPUGenericInstr_MUL)
            QPUGenericInstr_SetMulCondition(&state->m_gi, DFlowNode_GetCondCode(conditional));
         else
            QPUGenericInstr_SetAddCondition(&state->m_gi, DFlowNode_GetCondCode(conditional));
      }

      return InstrState_CommitChanges(state);
   }

   return DFlowNode_NOT_SCHEDULABLE;
}

const QPUOperand *DFlowNode_PeekInput(DFlowNode *self, InstrState *state, DFlowNode *from)
{
   const QPUOperand *operand = &from->m_result;

   // Handle any downward bypassing (used for unpack)
   if (QPUOperand_IsBypass(operand))
   {
      // Dive into our child, and then add unpack flags
      DFlowNode *child = NodeList_front(DFlowNode_Children(from));

      self->m_tempResult = child->m_result;
      QPUOperand_SetUnpack(&self->m_tempResult, QPUOperand_GetUnpack(operand));

      operand = &self->m_tempResult;
   }

   if (!InstrState_IsReadableOp(state, operand))
      return NULL;

   return operand;
}

const QPUOperand *DFlowNode_GetInput(DFlowNode *self, InstrState *state, DFlowNode *from)
{
   const QPUOperand *operand = &from->m_result;

   // First handle any downward bypassing (used for unpack)
   if (QPUOperand_IsBypass(operand))
   {
      // Dive into our child, and then add unpack flags
      DFlowNode *child = NodeList_front(DFlowNode_Children(from));

      self->m_tempResult = child->m_result;
      QPUOperand_SetUnpack(&self->m_tempResult, QPUOperand_GetUnpack(operand));

      operand = &self->m_tempResult;
   }

   // If we're about to consume a uniform or varying, so ensure it's in our lists
   if (!DFlowNode_RecordUniformsAndVaryings(from, operand, state))
      return NULL;

   if (!InstrState_IsReadableOp(state, operand))
      return NULL;

   return operand;
}

static void DFlowNode_SetWrittenAt(InstrState *state, Register_Enum reg, int32_t extraDelay /*= 0*/)
{
   InstrState_SetWrittenAt(state, reg, extraDelay);
}

void DFlowNode_SetReferenced(DFlowNode *self, InstrState *state, Register_Enum reg, int32_t overrideCount /* = 0 */)
{
   // We need to set the register in use for each parent node so that the reference counts are correct
   NodeList_const_iterator  iter;
   int32_t                  count = overrideCount;
   const NodeList          *parents = DFlowNode_Parents(self);

   if (count == 0)
   {
      for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
      {
         DFlowNode   *starIter = NodeList_const_star(iter);

         if (DFlowNode_HasUnconsumedReference(starIter, state->m_slot))
            count++;
      }
   }

   InstrState_SetReferenced(state, reg, self, count);
}

bool DFlowNode_IsSchedulable(const DFlowNode *self)
{
   // We are schedulable if all our children (& io children) have been scheduled
   NodeList_const_iterator iter;
   const NodeList *children   = DFlowNode_Children(self);
   const NodeList *ioChildren = DFlowNode_IoChildren(self);

   for (iter = NodeList_const_begin(children); iter != NodeList_const_end(children); NodeList_const_next(&iter))
   {
      DFlowNode   *starIter = NodeList_const_star(iter);

      if (starIter->m_slot == DFlowNode_NOT_SCHEDULED)
         return false;
   }

   for (iter = NodeList_const_begin(ioChildren); iter != NodeList_const_end(ioChildren); NodeList_const_next(&iter))
   {
      DFlowNode   *starIter = NodeList_const_star(iter);

      if (starIter->m_slot == DFlowNode_NOT_SCHEDULED)
         return false;
   }

   return true;
}

void DFlowNode_AddExtraIoChild(DFlowNode *self, DFlowNode *node)
{
   DFlowNode_AddToList(&self->m_ioChildren, node);
   DFlowNode_AddToList(&node->m_ioParents, self);
}

void DFlowNode_ReplaceChild(DFlowNode *self, DFlowNode *oldChild, DFlowNode *newChild)
{
   uint32_t cRem   = DFlowNode_RemoveFromList(&self->m_children, oldChild);
   uint32_t iocRem = DFlowNode_RemoveFromList(&self->m_ioChildren, oldChild);
   uint32_t i;

   for (i = 0; i < cRem; i++)
      DFlowNode_AddToList(&self->m_children, newChild);

   for (i = 0; i < iocRem; i++)
      DFlowNode_AddToList(&self->m_ioChildren, newChild);

   // We must also fix up the arguments
   for (i = 0; i < DFlowNode_ARG_COUNT; ++i)
   {
      if (self->m_args[i] == oldChild)
         self->m_args[i] = newChild;
   }
}

void DFlowNode_RemoveParent(DFlowNode *self, DFlowNode *oldParent)
{
   DFlowNode_RemoveFromList(&self->m_parents, oldParent);
}

void DFlowNode_RemoveIoParent(DFlowNode *self, DFlowNode *oldParent)
{
   DFlowNode_RemoveFromList(&self->m_ioParents, oldParent);
}


void DFlowNode_AddParent(DFlowNode *self, DFlowNode *newParent)
{
   DFlowNode_AddToList(&self->m_parents, newParent);
}

void DFlowNode_AddIoParent(DFlowNode *self, DFlowNode *newParent)
{
   DFlowNode_AddToList(&self->m_ioParents, newParent);
}

bool DFlowNode_IsTheSameValueAs(const DFlowNode *self, const DFlowNode *other)
{
   if (self->m_flavour != other->m_flavour)
      return false;

   // IO causes side effects which must be preserved, so don't merge these
   if (NodeList_size(DFlowNode_IoParents(self))  != 0 || NodeList_size(DFlowNode_IoChildren(self))  != 0 ||
       NodeList_size(DFlowNode_IoParents(other)) != 0 || NodeList_size(DFlowNode_IoChildren(other)) != 0)
      return false;

   // Handle constants
   if (NodeList_size(DFlowNode_Children(self)) == 0)
   {
      if (self->m_flavour == DATAFLOW_CONST_FLOAT)
         return self->m_uniform.m_constFloat.m_cfValue == other->m_uniform.m_constFloat.m_cfValue;

      if (self->m_flavour == DATAFLOW_CONST_INT)
         return self->m_uniform.m_constInt.m_ciValue == other->m_uniform.m_constInt.m_ciValue;

      return false;
   }
   else
   {
      bool     argsSame = true;
      uint32_t i;

      for (i = 0; i < DFlowNode_ARG_COUNT; ++i)
         argsSame = argsSame && (self->m_args[i] == other->m_args[i]);

      return argsSame                            &&
             self->m_flavour == other->m_flavour &&
             self->m_boolRep == other->m_boolRep;
   }
}

void DFlowNode_ReplaceWith(DFlowNode *self, DFlowNode *node)
{
   //assert(IoParents().size() == 0 && IoChildren().size() == 0 && node->IoParents().size() == 0 && node->IoChildren().size() == 0);
   NodeList_iterator iter;

   for (iter = NodeList_begin(&self->m_parents); iter != NodeList_end(&self->m_parents); NodeList_next(&iter))
   {
      DFlowNode   *starIter = NodeList_star(iter);

      DFlowNode_AddParent(node, starIter);
      DFlowNode_ReplaceChild(starIter, self, node);
   }

   for (iter = NodeList_begin(&self->m_children); iter != NodeList_end(&self->m_children); NodeList_next(&iter))
   {
      DFlowNode   *starIter = NodeList_star(iter);

      DFlowNode_RemoveParent(starIter, self);
   }

   for (iter = NodeList_begin(&self->m_ioParents); iter != NodeList_end(&self->m_ioParents); NodeList_next(&iter))
   {
      DFlowNode   *starIter = NodeList_star(iter);

      DFlowNode_AddIoParent(node, starIter);
      DFlowNode_ReplaceChild(starIter, self, node);
   }

   for (iter = NodeList_begin(&self->m_ioChildren); iter != NodeList_end(&self->m_ioChildren); NodeList_next(&iter))
   {
      DFlowNode   *starIter = NodeList_star(iter);

      DFlowNode_RemoveIoParent(starIter, self);
      DFlowNode_AddExtraIoChild(node, starIter);
   }
}

void DFlowNode_MarkForReplication(DFlowNode *self, bool tf)
{
   NodeList_iterator iter;

   self->m_replicant     = NULL;
   self->m_wantReplicate = tf;

   for (iter = NodeList_begin(&self->m_children); iter != NodeList_end(&self->m_children); NodeList_next(&iter))
      DFlowNode_MarkForReplication(NodeList_star(iter), tf);
}

bool DFlowNode_IsConstantConditionExpression(const DFlowNode *self)
{
   NodeList_const_iterator iter;

   // Terminal nodes must all be constants or uniforms
   if (NodeList_size(&self->m_children) == 0)
   {
      switch (self->m_flavour)
      {
      case DATAFLOW_UNIFORM :
      case DATAFLOW_CONST_FLOAT :
      case DATAFLOW_CONST_INT :
      case DATAFLOW_CONST_BOOL :
         return true;

      default:
         return false;
      }
   }

   for (iter = NodeList_const_begin(&self->m_children); iter != NodeList_const_end(&self->m_children); NodeList_const_next(&iter))
   {
      if (!DFlowNode_IsConstantConditionExpression(NodeList_const_star(iter)))
         return false;
   }

   return true;
}

uint32_t DFlowNode_RemoveFromList(NodeList *set, DFlowNode *node)
{
   // Needs to return the number of nodes removed
   //uint32_t cnt = 0;
   //DFlowNodeIter  iter;
   //for (iter = set.begin(); iter != set.end(); ++iter)
   //   if ((*iter) == node)
   //      cnt++;

   // Removes all "node" entries (see list::remove)
   return NodeList_remove(set, node);

   //return cnt;
}

bool DFlowNode_ListContains(const NodeList *set, DFlowNode *node)
{
   NodeList_const_iterator iter;

   for (iter = NodeList_const_begin(set); iter != NodeList_const_end(set); NodeList_const_next(&iter))
      if (NodeList_const_star(iter) == node)
         return true;

   return false;
}

bool DFlowNode_HasVaryCChild(const DFlowNode *self)
{
   NodeList_const_iterator iter;

   for (iter = NodeList_const_begin(&self->m_children); iter != NodeList_const_end(&self->m_children); NodeList_const_next(&iter))
   {
      if (DFlowNode_Flavour(NodeList_const_star(iter)) == DATAFLOW_VARYING_C)
         return true;
   }

   return false;
}

int32_t DFlowNode_RealRegistersRead(DFlowNode *self)
{
   int32_t  ret = 1;

   switch (self->m_flavour)
   {
   case DATAFLOW_CONST_BOOL      :
   case DATAFLOW_CONST_INT       :
   case DATAFLOW_CONST_FLOAT     :
   case DATAFLOW_CONST_SAMPLER   :
   case DATAFLOW_UNIFORM         :
   case DATAFLOW_UNIFORM_OFFSET  :
   case DATAFLOW_UNIFORM_ADDRESS :
	//case DATAFLOW_ATTRIBUTE       :
   case DATAFLOW_VARYING         :
   case DATAFLOW_FRAG_GET_X      :
   case DATAFLOW_FRAG_GET_Y      :
   case DATAFLOW_FRAG_GET_Z      :
   case DATAFLOW_FRAG_GET_W      :
   case DATAFLOW_FRAG_GET_FF     :
   case DATAFLOW_VARYING_C       :
      ret = 0;
      break;

   default:
      ret = 1;
      break;
   }

   return ret;
}

int32_t DFlowNode_RealRegistersWritten(DFlowNode *self)
{
   int32_t  ret = 1;

   switch (self->m_flavour)
   {
	case DATAFLOW_INTRINSIC_RCP         :
   case DATAFLOW_INTRINSIC_RSQRT       :
   case DATAFLOW_INTRINSIC_LOG2        :
   case DATAFLOW_INTRINSIC_EXP2        :
   case DATAFLOW_TMU_SWAP              :
   case DATAFLOW_TEX_SET_DIRECT        :
   case DATAFLOW_TEX_SET_COORD_S       :
   case DATAFLOW_TEX_SET_COORD_T       :
	case DATAFLOW_TEX_SET_COORD_R       :
	case DATAFLOW_TEX_SET_BIAS          :
	case DATAFLOW_TEX_SET_LOD           :
   case DATAFLOW_FRAG_SUBMIT_STENCIL   :
   case DATAFLOW_FRAG_SUBMIT_Z         :
   case DATAFLOW_FRAG_SUBMIT_MS        :
   case DATAFLOW_FRAG_SUBMIT_ALL       :
   case DATAFLOW_VERTEX_SET            :
   case DATAFLOW_VPM_READ_SETUP        :
   case DATAFLOW_VPM_WRITE_SETUP       :
      ret = 0;
      break;

   default:
      ret = 1;
      break;
   }

   return ret;
}

// Estimate the net delat on registers as a result of this node
void DFlowNode_CalcRegisterDelta(DFlowNode *self)
{
   NodeList_const_iterator  iter;
   const NodeList          *children = DFlowNode_Children(self);
   const NodeList          *parents  = DFlowNode_Parents(self);

   int32_t  delta = 0;

   for (iter = NodeList_const_begin(children); iter != NodeList_const_end(children); NodeList_const_next(&iter))
   {
      delta = delta - DFlowNode_RealRegistersRead(NodeList_const_star(iter));
   }

   for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
   {
      delta = delta + DFlowNode_RealRegistersWritten(NodeList_const_star(iter));
   }

   self->m_registerDelta = delta;
}

///////////////////////////////////////////////////////////////////////////////
// ResetHelper
///////////////////////////////////////////////////////////////////////////////
void ResetHelper_Constr(ResetHelper *self)
{
   //NodeVector_Constr(&self->m_cppNodes, 64);
   DataflowVector_Constr(&self->m_originalNodes, 64);
}

ResetHelper *ResetHelper_new(void)
{
   ResetHelper *res = (ResetHelper *)bcg_glsl_malloc(sizeof(ResetHelper));
   ResetHelper_Constr(res);

   return res;
}

void ResetHelper_Destr(ResetHelper *self)
{
   //NodeVector_Destr(&self->m_cppNodes);
   DataflowVector_Destr(&self->m_originalNodes);
}

void ResetHelper_delete(ResetHelper *self)
{
   ResetHelper_Destr(self);
   bcg_glsl_free(self);
}

void ResetHelper_Cleanup(ResetHelper *self)
{
   ResetHelper_ResetHelperPtrs(self);
   //ResetHelper_DeleteNodes(self);
}

void ResetHelper_ResetHelperPtrs(ResetHelper *self)
{
   DataflowVector *origNodes = &self->m_originalNodes;
   uint32_t       i;

   for (i = 0; i < DataflowVector_size(origNodes); i++)
      DataflowVector_index(origNodes, i)->bcg_helper = NULL;

   DataflowVector_clear(origNodes);
}

void ResetHelper_AddDataflow(ResetHelper *self, Dataflow *node)
{
   DataflowVector_push_back(&self->m_originalNodes, node);
}

//void ResetHelper_AddDFlowNode(ResetHelper *self, DFlowNode *node)
//{
//   NodeVector_push_back(&self->m_cppNodes, node);
//}

//void ResetHelper_DeleteNodes(ResetHelper *self)
//{
//   NodeVector  *cppNodes = &self->m_cppNodes;
//
//   uint32_t sz = NodeVector_size(cppNodes);
//   uint32_t i;
//
//   for (i = 0; i < sz; i++)
//   {
//      bcg_glsl_free(NodeVector_index(cppNodes, i));
//      *NodeVector_lindex(cppNodes, i) = NULL;
//   }
//
//   NodeVector_clear(cppNodes);
//}
