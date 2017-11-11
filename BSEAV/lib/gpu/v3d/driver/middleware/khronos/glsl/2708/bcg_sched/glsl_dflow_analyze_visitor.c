/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/glsl/glsl_common.h"
#include "interface/khronos/common/khrn_options.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_analyze_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// TextureSetupVector
///////////////////////////////////////////////////////////////////////////////
void TextureSetupVector_Constr(TextureSetupVector *self, uint32_t capacity)
{
   self->m_end  = 0;
   VectorBase_Constr(&self->m_vector, sizeof(TextureSetup) * capacity);
}


void TextureSetupVector_push_back(TextureSetupVector *self, const TextureSetup *node)
{
   if (VectorBase_Extend(&self->m_vector, sizeof(TextureSetup)))
   {
      TextureSetup *resultList = (TextureSetup *)self->m_vector.data;
      uint32_t      i          = self->m_end;

      self->m_end++;
      resultList[i] = *node;
   }
}

void TextureSetupVector_sort(TextureSetupVector *self, TextureSetup_pred pred)
{
   qsort(TextureSetupVector_begin(self),
         TextureSetupVector_size(self),
         sizeof(TextureSetup),
         (int(*)(const void *, const void *))pred);
}

///////////////////////////////////////////////////////////////////////////////
// DFlowAnalyzeVisitor
///////////////////////////////////////////////////////////////////////////////

static int compareTS(const TextureSetup *i, const TextureSetup *j)
{
   uint32_t iDepth = DFlowNode_TreeDepth(TextureSetup_GetRequest(i));
   uint32_t jDepth = DFlowNode_TreeDepth(TextureSetup_GetRequest(j));

   if (iDepth == jDepth)
      return (TextureSetup_GetRequest(i)->m_uniqueId < TextureSetup_GetRequest(j)->m_uniqueId) ? 1 : -1;
   else
      return (iDepth < jDepth) ? 1 : -1;
}

static void GatherTextureSetupData(DFlowNode *node, TextureSetup *setupGroup)
{
   switch (DFlowNode_Flavour(node))
   {
   case DATAFLOW_TEX_GET_CMP_R   : TextureSetup_SetRequest(setupGroup, node); break;
   case DATAFLOW_TEX_SET_DIRECT  :
   case DATAFLOW_TEX_SET_COORD_S : TextureSetup_SetS(setupGroup, node); break;
   case DATAFLOW_TEX_SET_COORD_T : TextureSetup_SetT(setupGroup, node); break;
   case DATAFLOW_TEX_SET_COORD_R : TextureSetup_SetR(setupGroup, node); break;
   case DATAFLOW_TEX_SET_BIAS    :
   case DATAFLOW_TEX_SET_LOD     : TextureSetup_SetB(setupGroup, node); break;

   default:
      break;
   }

   {
      const NodeList          *iokids = DFlowNode_IoChildren(node);
      NodeList_const_iterator  iter;

      for (iter = NodeList_const_begin(iokids); iter != NodeList_const_end(iokids); NodeList_const_next(&iter))
         GatherTextureSetupData(NodeList_const_star(iter), setupGroup);   // Recurse down
   }
}

static void Analyze_Destroy(void *me)
{
   DFlowAnalyzeVisitor  *self = (DFlowAnalyzeVisitor *)me;

   TextureSetupVector_Destr(&self->m_textureSetups);
   NodesAtLevel_Destr(&self->m_nodesAtLevel);

   NodeList_Destr(&self->m_extraVpmWriteDeps);
   NodeList_Destr(&self->m_extraMovNodeList);
}

static uint32_t maxi(uint32_t a, uint32_t b)
{
   return a > b ? a : b;
}

static void Analyze_Accept(void *me, DFlowNode *node)
{
   DFlowAnalyzeVisitor  *self = (DFlowAnalyzeVisitor *)me;
   // We'll also work out a number of heuristics during this pass:
   // 1) The number of nodes at each tree level
   // 2) A cumulative count of all children recursively beneath this one
   // 3) How often each node is referenced in the graph
   // 4) Texture setup groups
   // 5) Register delta
   // 6) Lifespan
   const NodeList          *parents = DFlowNode_Parents(node);
   NodeList_const_iterator  piter;

   // Estimate the register delta (0 if no extra registers needed, -ve if we decrease the amount of registers used, +ve otherwise)
   //node->m_registerDelta = node->Parents().size() - node->Children().size();
   DFlowNode_CalcRegisterDelta(node);

   // Estimate the lifespan of this node's result (in tree depths)
   for (piter = NodeList_const_begin(parents); piter != NodeList_const_end(parents); NodeList_const_next(&piter))
   {
      int32_t lifespan = DFlowNode_TreeDepth(node) - DFlowNode_TreeDepth(NodeList_const_star(piter));
      if (lifespan > node->m_lifespanGuess)
         node->m_lifespanGuess = lifespan;
   }

   if (DFlowNode_TreeDepth(node) >= (int32_t)NodesAtLevel_size(&self->m_nodesAtLevel))
   {
      NodesAtLevel_resize(&self->m_nodesAtLevel, DFlowNode_TreeDepth(node) + 1);
   }

   NodesAtLevel_inc(&self->m_nodesAtLevel, DFlowNode_TreeDepth(node));
   self->m_maxNodesOnOneLevel = maxi(self->m_maxNodesOnOneLevel, NodesAtLevel_index(&self->m_nodesAtLevel, DFlowNode_TreeDepth(node)));

   {
      const NodeList          *kids = DFlowNode_Children(node);
      const NodeList          *iokids = DFlowNode_IoChildren(node);
      NodeList_const_iterator  iter;

      for (iter = NodeList_const_begin(kids); iter != NodeList_const_end(kids); NodeList_const_next(&iter))
      {
         DFlowNode   *starIter = NodeList_const_star(iter);

         node->m_numRecursiveChildren += starIter->m_numRecursiveChildren + 1;
         starIter->m_numReferences++;
      }

      for (iter = NodeList_const_begin(iokids); iter != NodeList_const_end(iokids); NodeList_const_next(&iter))
      {
         DFlowNode   *starIter = NodeList_const_star(iter);

         node->m_numRecursiveChildren += starIter->m_numRecursiveChildren + 1;
         starIter->m_numReferences++;
      }
   }

   // Texture setup groups
   if (DFlowNode_Flavour(node) == DATAFLOW_TEX_GET_CMP_R)
   {
      TextureSetup   setupGroup;
      TextureSetup_Constr(&setupGroup);

      GatherTextureSetupData(node, &setupGroup);

      TextureSetupVector_push_back(&self->m_textureSetups, &setupGroup);

      TextureSetup_Destr(&setupGroup);
   }
   else if (DFlowNode_Flavour(node) == DATAFLOW_VARYING)
   {
      int32_t  row = node->m_uniform.m_linkableValue.m_lvRow;

      // A point coordinate
      if (row < 2)
         self->m_pointCoord[row] = node;
      else
         self->m_varyings[row - 32] = node;
   }

   // This optimization prevents accumulators being tied up by pending VPM writes due to VPM write bypassing.
   // It adds an IO dep to make the shortcut operation be delayed until the VPM write is schedulable.
   if (DFlowNode_CanShortcutOutputReg(node))
   {
      DFlowNode *parent;

      assert(NodeList_size(DFlowNode_Parents(node)) == 1);

      parent = node->m_parents.m_head->m_node;

      if (NodeList_size(DFlowNode_IoChildren(node)) == 0 && DFlowNode_Flavour(parent) == DATAFLOW_VERTEX_SET &&
          NodeList_size(DFlowNode_IoChildren(parent)) > 0)
         NodeList_push_back(&self->m_extraVpmWriteDeps, node);
   }

   // Identify the case where we have a node trying to use a varying and its varyc at the same time.
   // This happens during the ES1 primtest for example.
   if (node->m_flavour == DATAFLOW_VARYING_C)
   {
      DFlowNode         *parent, *vary;
      NodeList_iterator iter;

      assert(NodeList_size(DFlowNode_Parents(node)) == 1);
      assert(NodeList_size(DFlowNode_IoChildren(node)) == 1);

      parent = node->m_parents.m_head->m_node;
      vary = node->m_ioChildren.m_head->m_node;

      // Does the vary contain parent in it's parent list?
      for (iter = NodeList_begin(&vary->m_parents); iter != NodeList_end(&vary->m_parents); NodeList_next(&iter))
      {
         if (NodeList_star(iter) == parent)
            NodeList_push_back(&self->m_extraMovNodeList, vary);
      }
   }
}

void DFlowAnalyzeVisitor_Constr(DFlowAnalyzeVisitor *self, DFlowRecursionOptimizer *opt, ResetHelper *rh)
{
   uint32_t i;

   DFlowVisitor_Constr(self, Analyze_Destroy, Analyze_Accept, opt);

   self->m_maxNodesOnOneLevel = 0;
   self->m_resetHelper = rh;

   NodesAtLevel_Constr(&self->m_nodesAtLevel, 64);
   TextureSetupVector_Constr(&self->m_textureSetups, 16);

   self->m_pointCoord[0] = NULL;
   self->m_pointCoord[1] = NULL;

   for (i = 0; i < 32; ++i)
      self->m_varyings[i] = NULL;

   NodeList_Constr(&self->m_extraVpmWriteDeps);
   NodeList_Constr(&self->m_extraMovNodeList);
}

static DFlowNode *GetVaryingParent(DFlowNode *node)
{
   const NodeList *list = DFlowNode_Parents(node);

   assert(NodeList_size(list) == 1);

   return NodeList_front(list);
}

void DFlowAnalyzeVisitor_Visit(DFlowAnalyzeVisitor *self, DFlowNode *node)
{
   // Assign texture units
   uint32_t unit    = 0;
   uint32_t numTMUs = khrn_workarounds.TMUS_PER_SLICE;
   NodeList_iterator iter;
   bool              changesMade = false;

   DFlowVisitor_VisitBottomUp(self, node);

   TextureSetupVector_sort(&self->m_textureSetups, compareTS);

   // No point using 2 TMUs when only one texture fetch -- code will just be bigger (TMURS write) for no gain.
   if (TextureSetupVector_size(&self->m_textureSetups) < 2)
      numTMUs = 1;

   {
      TextureSetupVector_iterator   i;

      for (i = TextureSetupVector_begin(&self->m_textureSetups); i != TextureSetupVector_end(&self->m_textureSetups); TextureSetupVector_next(&i))
      {
         TextureSetup_SetUnit(TextureSetupVector_star(i), unit);

         unit += 1;
         if (unit == numTMUs)
            unit = 0;
      }
   }

   if (self->m_pointCoord[0] != NULL && self->m_pointCoord[1] != NULL)
   {
      uint32_t i;

      DFlowNode   *parentPointCoord0 = GetVaryingParent(self->m_pointCoord[0]);
      DFlowNode   *parentPointCoord1 = GetVaryingParent(self->m_pointCoord[1]);

      DFlowNode_AddExtraIoChild(parentPointCoord1, parentPointCoord0);
      changesMade = true;

      for (i = 0; i < 32; ++i)
      {
         if (self->m_varyings[i] != NULL)
         {
            DFlowNode   *parentVarying = GetVaryingParent(self->m_varyings[i]);

            DFlowNode_AddExtraIoChild(parentVarying, parentPointCoord1);
         }
      }
   }

   for (iter = NodeList_begin(&self->m_extraVpmWriteDeps); iter != NodeList_end(&self->m_extraVpmWriteDeps); NodeList_next(&iter))
   {
      DFlowNode *node = NodeList_star(iter);
      DFlowNode *parent;

      assert(NodeList_size(DFlowNode_Parents(node)) == 1);

      parent = node->m_parents.m_head->m_node;

      assert(NodeList_size(DFlowNode_IoChildren(parent)) > 0);

      DFlowNode_AddExtraIoChild(node, parent->m_ioChildren.m_head->m_node);
      changesMade = true;
   }

   for (iter = NodeList_begin(&self->m_extraMovNodeList); iter != NodeList_end(&self->m_extraMovNodeList); NodeList_next(&iter))
   {
      DFlowNode *vary = NodeList_star(iter);
      DFlowNode *parent;
      DFlowNode *mov;

      assert(NodeList_size(DFlowNode_Parents(vary)) == 1);

      parent = vary->m_parents.m_head->m_node;

      mov = DFlowNode_Flavour_new(DATAFLOW_MOV, self->m_resetHelper);
      DFlowNode_AddChildNode(mov, DFlowNode_ARG_OPERAND, vary);

      DFlowNode_ReplaceChild(parent, vary, mov);
      DFlowNode_RemoveParent(vary, parent);
      DFlowNode_AddParent(mov, parent);

      changesMade = true;
   }

   if (changesMade)
      DFlowRecursionOptimizer_Invalidate(self->m_base.m_opt);
}

///////////////////////////////////////////////////////////////////////////////
// DFlowDependentReadVisitor
///////////////////////////////////////////////////////////////////////////////
static void DependentRead_Destroy(void *me)
{
   UNUSED(me);
}

static void DependentRead_Accept(void *me, DFlowNode *node)
{
   DFlowDependentReadVisitor *self = (DFlowDependentReadVisitor *)me;

   if (DFlowNode_IsTMUOrdered(node))
      self->m_isDependent = true;
}

// Search a sub-tree for an already ordered IO node
// Can happen for dependent texture reads.
void DFlowDependentReadVisitor_Constr(DFlowDependentReadVisitor *self, DFlowRecursionOptimizer *opt)
{
   DFlowVisitor_Constr(self, DependentRead_Destroy, DependentRead_Accept, opt);
   self->m_isDependent = false;
}

void DFlowDependentReadVisitor_Visit(DFlowDependentReadVisitor *self, DFlowNode *node)
{
   DFlowVisitor_VisitTopDown(self, node);
}

///////////////////////////////////////////////////////////////////////////////
// DFlowCombineVisitor
// Common up identical nodes
///////////////////////////////////////////////////////////////////////////////

// Private
static bool MergeIdentical(DFlowCombiner *self, DFlowNode *original, DFlowNode *candidate)
{
   bool same = DFlowNode_IsTheSameValueAs(candidate, original);

   if (same)
   {
      DFlowNode_ReplaceWith(candidate, original);
      DFlowOptimizeCandidateVisitor_AddParents(self->m_candidates, original);
      DFlowOptimizeCandidateVisitor_Remove(self->m_candidates, candidate);
      self->m_numCombines++;
   }

   return same;
}

static void MergeCallback(NodeVectorMap_T *map, uint32_t key, NodeVector *bag, void *me)
{
   DFlowCombiner *self = (DFlowCombiner *)me;

   uint32_t    bagSize = NodeVector_size(bag);

   UNUSED(map);
   UNUSED(key);

   if (bagSize > 1)
   {
      uint32_t             j_ix;
      NodeVector_iterator  j;

      bool     *done = (bool *)bcg_glsl_malloc(sizeof(bool) * bagSize);

      memset(done, 0, sizeof(bool) *bagSize);

      j_ix = 0;

      for (j = NodeVector_begin(bag); j != NodeVector_end(bag); ++j_ix, NodeVector_next(&j))
      {
         if (!done[j_ix])
         {
            uint32_t             k_ix;
            NodeVector_iterator  k;

            done[j_ix] = true;

            k = j;

            NodeVector_next(&k);

            k_ix = j_ix + 1;

            for (; k != NodeVector_end(bag); ++k_ix, NodeVector_next(&k))
            {
               if (!done[k_ix])
               {
                  done[k_ix] = MergeIdentical(self, NodeVector_star(j), NodeVector_star(k));
               }
            }
         }
      }

      bcg_glsl_free(done);
   }
}

static void MergeIdenticalNodes(DFlowCombiner *self)
{
   // For each bag, try to merge identical nodes
   NodeVectorMap_iterate(&self->m_nodeMap, MergeCallback, self);
}

// Public
void DFlowCombiner_Constr(DFlowCombiner *self, DFlowOptimizeCandidateVisitor *candidates)
{
   NodeVectorMap_Constr(&self->m_nodeMap, 128);
   self->m_candidates  = candidates;
   self->m_numCombines = 0;
}

static void DeleteCallback(NodeVectorMap_T *map, uint32_t key, NodeVector *bag, void *me)
{
   UNUSED(map);
   UNUSED(key);
   UNUSED(me);

   if (bag != NULL)
      NodeVector_delete(bag);
}

void DFlowCombiner_Destr(DFlowCombiner *self)
{
   NodeVectorMap_iterate(&self->m_nodeMap, DeleteCallback, NULL);
   NodeVectorMap_Destr(&self->m_nodeMap);
}

void DFlowCombiner_Visit(DFlowCombiner *self)
{
   uint32_t    i;
   NodeVector  flat;
   NodeVector_Constr(&flat, DFlowOptimizeCandidateVisitor_size(self->m_candidates));

   DFlowOptimizeCandidateVisitor_Flatten(self->m_candidates, &flat);

   for (i = 0; i < NodeVector_size(&flat); ++i)
      DFlowCombiner_Accept(self, NodeVector_index(&flat, i));

   MergeIdenticalNodes(self);

   if (khrn_options.glsl_debug_on)
   {
      if (self->m_numCombines > 0)
      {
         printf("Merged %d identical operations\n", self->m_numCombines);
      }
   }

   NodeVector_Destr(&flat);
}

void DFlowCombiner_Accept(DFlowCombiner *self, DFlowNode *node)
{
   // Create a map containing nodes that look similar with a view to amalgamating them
   // Calculate a reasonably unique hash for this node based on its arguments
   if (NodeList_size(DFlowNode_IoChildren(node)) == 0 && NodeList_size(DFlowNode_IoParents(node)) == 0)
   {
      uint32_t hash  = 0;
      uint32_t args = 0;
      uint32_t i;
      bool     isConst = false;

      for (i = 0; i < DFlowNode_ARG_COUNT; ++i)
      {
         const DFlowNode *arg = DFlowNode_GetArg(node, i);
         if (arg != NULL)
         {
            args += 1;
            hash = (hash << 8) + arg->m_uniqueId;
         }
      }

      hash += DFlowNode_Flavour(node);

      if (DFlowNode_Flavour(node) == DATAFLOW_CONST_FLOAT || DFlowNode_Flavour(node) == DATAFLOW_CONST_INT)
      {
         hash += node->m_uniform.m_constFloat.m_cfValue;
         isConst = true;
      }

      if (args != 0 || isConst) // Otherwise zero means a terminal node which should probably not be merged (e.g. varyings etc.)
      {
         NodeVector  *entry = NodeVectorMap_lookup(&self->m_nodeMap, hash);

         if (entry == NULL)
         {
            entry = NodeVector_new(8);

            NodeVectorMap_insert(&self->m_nodeMap, hash, entry);
         }

         NodeVector_push_back(entry, node);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// DFlowSimplifier
///////////////////////////////////////////////////////////////////////////////

void SimplificationVector_Constr(SimplificationVector *self, uint32_t capacity)
{
   self->m_end  = 0;
   VectorBase_Constr(&self->m_vector, sizeof(Simplification) * capacity);
}


void SimplificationVector_push_back(SimplificationVector *self, const Simplification *node)
{
   if (VectorBase_Extend(&self->m_vector, sizeof(Simplification)))
   {
      Simplification *resultList = (Simplification *)self->m_vector.data;
      uint32_t      i            = self->m_end;

      self->m_end++;
      resultList[i] = *node;
   }
}

// Simplifier
//
// Look for simplifiable expressions e.g. X * 0, X * 1, X + 0,  A ? B : B
//
void DFlowSimplifier_Constr(DFlowSimplifier *self, DFlowOptimizeCandidateVisitor *candidates)
{
   SimplificationVector_Constr(&self->m_simplifications, 16);
   self->m_candidates = candidates;
}

void DFlowSimplifier_Destr(DFlowSimplifier *self)
{
   SimplificationVector_Destr(&self->m_simplifications);
}

void DFlowSimplifier_Accept(DFlowSimplifier *self, DFlowNode *node)
{
   Simplifier_Type  type = Simplifier_NONE;

   switch (DFlowNode_Flavour(node))
   {
   case DATAFLOW_MUL:
      {
         const DFlowNode   *left  = DFlowNode_GetArg(node, DFlowNode_ARG_LEFT);
         const DFlowNode   *right = DFlowNode_GetArg(node, DFlowNode_ARG_RIGHT);
         DataflowFlavour   leftFlavour  = DFlowNode_Flavour(left);
         DataflowFlavour   rightFlavour = DFlowNode_Flavour(right);

         if (leftFlavour == DATAFLOW_CONST_INT && DFlowNode_GetConst_const(left)->m_constFloat.m_cfValue == 1)
            type = Simplifier_ONE_TIMES_X;
         else if (rightFlavour == DATAFLOW_CONST_INT && DFlowNode_GetConst_const(right)->m_constFloat.m_cfValue == 1)
            type = Simplifier_X_TIMES_ONE;
         else if (leftFlavour == DATAFLOW_CONST_FLOAT && DFlowNode_GetConst_const(left)->m_constFloat.m_cfValue == CONST_FLOAT_ONE)
            type = Simplifier_ONE_TIMES_X;
         else if (rightFlavour == DATAFLOW_CONST_FLOAT && DFlowNode_GetConst_const(right)->m_constFloat.m_cfValue == CONST_FLOAT_ONE)
            type = Simplifier_X_TIMES_ONE;
         else if (leftFlavour == DATAFLOW_CONST_INT && DFlowNode_GetConst_const(left)->m_constFloat.m_cfValue == 0)
            type = Simplifier_ZERO_TIMES_X;
         else if (rightFlavour == DATAFLOW_CONST_INT && DFlowNode_GetConst_const(right)->m_constFloat.m_cfValue == 0)
            type = Simplifier_X_TIMES_ZERO;
         else if (leftFlavour == DATAFLOW_CONST_FLOAT && DFlowNode_GetConst_const(left)->m_constFloat.m_cfValue == CONST_FLOAT_ZERO)
            type = Simplifier_ZERO_TIMES_X;
         else if (rightFlavour == DATAFLOW_CONST_FLOAT && DFlowNode_GetConst_const(right)->m_constFloat.m_cfValue == CONST_FLOAT_ZERO)
            type = Simplifier_X_TIMES_ZERO;
      }
      break;

   case DATAFLOW_SUB:
   case DATAFLOW_V8SUBS:
      {
         const DFlowNode   *right = DFlowNode_GetArg(node, DFlowNode_ARG_RIGHT);
         DataflowFlavour   rightFlavour = DFlowNode_Flavour(right);

         if (rightFlavour == DATAFLOW_CONST_INT && DFlowNode_GetConst_const(right)->m_constFloat.m_cfValue == 0)
            type = Simplifier_X_PLUS_ZERO;
         else if (rightFlavour == DATAFLOW_CONST_FLOAT && DFlowNode_GetConst_const(right)->m_constFloat.m_cfValue == CONST_FLOAT_ZERO)
            type = Simplifier_X_PLUS_ZERO;
      }
      break;

   case DATAFLOW_ADD:
   case DATAFLOW_INTEGER_ADD:
   case DATAFLOW_V8ADDS:
      {
         const DFlowNode   *left  = DFlowNode_GetArg(node, DFlowNode_ARG_LEFT);
         const DFlowNode   *right = DFlowNode_GetArg(node, DFlowNode_ARG_RIGHT);

         DataflowFlavour   leftFlavour  = DFlowNode_Flavour(left);
         DataflowFlavour   rightFlavour = DFlowNode_Flavour(right);

         if (leftFlavour == DATAFLOW_CONST_INT && DFlowNode_GetConst_const(left)->m_constFloat.m_cfValue == 0)
             type = Simplifier_ZERO_PLUS_X;
         else if (rightFlavour == DATAFLOW_CONST_INT && DFlowNode_GetConst_const(right)->m_constFloat.m_cfValue == 0)
             type = Simplifier_X_PLUS_ZERO;
         else if (leftFlavour == DATAFLOW_CONST_FLOAT && DFlowNode_GetConst_const(left)->m_constFloat.m_cfValue == CONST_FLOAT_ZERO)
             type = Simplifier_ZERO_PLUS_X;
         else if (rightFlavour == DATAFLOW_CONST_FLOAT && DFlowNode_GetConst_const(right)->m_constFloat.m_cfValue == CONST_FLOAT_ZERO)
             type = Simplifier_X_PLUS_ZERO;
      }
      break;

   case DATAFLOW_CONDITIONAL:
      {
         const DFlowNode   *true_node  = DFlowNode_GetArg(node, DFlowNode_ARG_TRUE_VALUE);
         const DFlowNode   *false_node = DFlowNode_GetArg(node, DFlowNode_ARG_FALSE_VALUE);

         // Do the if and else parts point to the same node?  Note that if constants are merged
         // this will also catch the case when the nodes are constants
         if (true_node == false_node)
            type = Simplifier_IF_X_THEN_Y_ELSE_Y;
      }
      break;

   default:
      // Nothing to optimize
      break;
   }

   if (type != Simplifier_NONE)
   {
      Simplification simp;
      simp.m_type = type;
      simp.m_node = node;
      SimplificationVector_push_back(&self->m_simplifications, &simp);
   }
}

static void DFlowSimplifier_Simplify(const DFlowSimplifier *self)
{
   uint32_t i;
   uint32_t sz = SimplificationVector_size(&self->m_simplifications);

   for (i = 0; i < sz; ++i)
   {
      const Simplification *simp = SimplificationVector_const_index(&self->m_simplifications, i);
      DFlowNode   *node          = simp->m_node;
      DFlowNode   *replacement   = NULL;

      switch (simp->m_type)
      {
      // Keep right hand argument
      case Simplifier_X_TIMES_ZERO       :
      case Simplifier_ONE_TIMES_X        :
      case Simplifier_ZERO_PLUS_X        :
         replacement = DFlowNode_GetArg(node, DFlowNode_ARG_RIGHT);
         break;

      // Keep left hand argument
      case Simplifier_X_TIMES_ONE        :
      case Simplifier_ZERO_TIMES_X       :
      case Simplifier_X_PLUS_ZERO        :
         replacement = DFlowNode_GetArg(node, DFlowNode_ARG_LEFT);
         break;

      case Simplifier_IF_X_THEN_Y_ELSE_Y :
         replacement = DFlowNode_GetArg(node, DFlowNode_ARG_TRUE_VALUE);
         break;

      case Simplifier_NONE               :
      default                            :
         assert(0);
         break;
      }

      DFlowNode_ReplaceWith(node, replacement);
      DFlowOptimizeCandidateVisitor_Remove(self->m_candidates, node);
      DFlowOptimizeCandidateVisitor_AddParents(self->m_candidates, replacement);
   }

   if (khrn_options.glsl_debug_on)
      printf("Removed %d benign operations\n", SimplificationVector_size(&self->m_simplifications));
}

void DFlowSimplifier_Visit(DFlowSimplifier *self)
{
   uint32_t           i;
   NodeVector         flat;
   NodeVector_Constr(&flat, DFlowOptimizeCandidateVisitor_size(self->m_candidates));

   DFlowOptimizeCandidateVisitor_Flatten(self->m_candidates, &flat);

   for (i = 0; i < NodeVector_size(&flat); ++i)
      DFlowSimplifier_Accept(self, NodeVector_index(&flat, i));

   DFlowSimplifier_Simplify(self);

   NodeVector_Destr(&flat);
}

///////////////////////////////////////////////////////////////////////////////
// DFlowBushinessVisitor
///////////////////////////////////////////////////////////////////////////////


static void Bushiness_Delete(void *me)
{
   UNUSED(me);
}

static int32_t CostOfFlavour(DataflowFlavour flavour)
{
   int32_t  cost = 0;

   switch (flavour)
   {
   case DATAFLOW_UNIFORM_OFFSET:
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
   case DATAFLOW_CONDITIONAL:
   case DATAFLOW_TEX_GET_CMP_R:
   case DATAFLOW_TEX_GET_CMP_G:
   case DATAFLOW_TEX_GET_CMP_B:
   case DATAFLOW_TEX_GET_CMP_A:
   case DATAFLOW_PACK_COL_REPLICATE:
   case DATAFLOW_PACK_COL_R:
   case DATAFLOW_PACK_COL_G:
   case DATAFLOW_PACK_COL_B:
   case DATAFLOW_PACK_COL_A:
   case DATAFLOW_PACK_16A:
   case DATAFLOW_PACK_16B:
   case DATAFLOW_FRAG_GET_X:
   case DATAFLOW_FRAG_GET_Y:
   case DATAFLOW_FRAG_GET_Z:
   case DATAFLOW_FRAG_GET_W:
   case DATAFLOW_FRAG_GET_PC_X:
   case DATAFLOW_FRAG_GET_PC_Y:
   case DATAFLOW_FRAG_GET_FF:
   case DATAFLOW_FRAG_GET_COL:
      cost = 1;
      break;

   case DATAFLOW_TEX_SET_COORD_S:
   case DATAFLOW_TEX_SET_COORD_T:
   case DATAFLOW_TEX_SET_COORD_R:
   case DATAFLOW_TEX_SET_LOD:
   case DATAFLOW_TEX_SET_BIAS:
   case DATAFLOW_TEX_SET_DIRECT:
   case DATAFLOW_FRAG_SUBMIT_STENCIL:
   case DATAFLOW_FRAG_SUBMIT_Z:
   case DATAFLOW_FRAG_SUBMIT_MS:
   case DATAFLOW_FRAG_SUBMIT_ALL:
   case DATAFLOW_FRAG_SUBMIT_R0:
   case DATAFLOW_FRAG_SUBMIT_R1:
   case DATAFLOW_FRAG_SUBMIT_R2:
   case DATAFLOW_FRAG_SUBMIT_R3:
   case DATAFLOW_TMU_SWAP:
   case DATAFLOW_VERTEX_SET:
   case DATAFLOW_VPM_READ_SETUP:
   case DATAFLOW_VPM_WRITE_SETUP:
   case DATAFLOW_CONST_BOOL:
   case DATAFLOW_CONST_FLOAT:
   case DATAFLOW_CONST_SAMPLER:
   case DATAFLOW_UNIFORM:
   case DATAFLOW_ATTRIBUTE:
   case DATAFLOW_VARYING:
   case DATAFLOW_CONST_INT:
   case DATAFLOW_UNIFORM_ADDRESS:
   case DATAFLOW_VARYING_C:
   case DATAFLOW_SCOREBOARD_WAIT:
   case DATAFLOW_THREADSWITCH:
      cost = 0;
      break;

   default:
      UNREACHABLE();
      break;
   }

   return cost;
}

static void Bushiness_Accept(void *me, DFlowNode *node)
{
   UNUSED(me);
   int32_t                 cost = 0;
   NodeList_const_iterator iter;

   cost = CostOfFlavour(DFlowNode_Flavour(node));

   //for (iter = NodeList_const_begin(DFlowNode_Children(node)); iter != NodeList_const_end(DFlowNode_Children(node)); NodeList_const_next(&iter))
   //{
   //   cost = cost + DFlowNode_GetBushiness(NodeList_const_star(iter));
   //}
   //DFlowNode_SetBushiness(node, cost + NodeList_size(DFlowNode_Parents(node)) - 1);

   for (iter = NodeList_const_begin(DFlowNode_Children(node)); iter != NodeList_const_end(DFlowNode_Children(node)); NodeList_const_next(&iter))
   {
      if (NodeList_size(DFlowNode_Parents(NodeList_const_star(iter))) <= 1)
         cost = cost - 1;
   }

   DFlowNode_SetBushiness(node, cost);
}

void DFlowBushinessVisitor_Constr(DFlowBushinessVisitor *self, DFlowRecursionOptimizer *opt)
{
   DFlowVisitor_Constr(self, Bushiness_Delete, Bushiness_Accept, opt);
}

void DFlowBushinessVisitor_Visit(DFlowBushinessVisitor *self, DFlowNode *root)
{
   DFlowVisitor_VisitBottomUp(self, root);
}


///////////////////////////////////////////////////////////////////////////////
// DFlowSubtreeDetectionVisitor
///////////////////////////////////////////////////////////////////////////////

#if 0

static void SubtreeDetect_Destroy(void *me)
{
   DFlowSubtreeDetectionVisitor  *self = (DFlowSubtreeDetectionVisitor *)me;

   NodeVector_Destr(&self->m_nodeList);
}

void SubtreeDetect_Accept(void *me, DFlowNode *node)
{
   DFlowSubtreeDetectionVisitor  *self = (DFlowSubtreeDetectionVisitor *)me;

   if (DFlowNode_Flavour(node) == DATAFLOW_CONDITIONAL)
   {
      if (DFlowNode_IsConstantConditionExpression(DFlowNode_GetArg(node, DFlowNode_ARG_COND)))
      {
         if (0)
         {
            DFlowNode_GetArg(node, DFlowNode_ARG_TRUE_VALUE)->m_subtreeRoot = true;
            DFlowNode_GetArg(node, DFlowNode_ARG_FALSE_VALUE)->m_subtreeRoot = true;
         }

         printf("BRANCH CANDIDATE FOUND : (%d, %d) sub-nodes\n",
                DFlowNode_GetArg(node, DFlowNode_ARG_TRUE_VALUE)->m_numRecursiveChildren,
                DFlowNode_GetArg(node, DFlowNode_ARG_FALSE_VALUE)->m_numRecursiveChildren);

         if (0)
         {
            DFlowNode *duplicate;

            // Duplicate the if and else branches in their entirety
            // First mark the nodes we want to replicate
            DFlowNode_MarkForReplication(DFlowNode_GetArg(node, DFlowNode_ARG_TRUE_VALUE), true);
            duplicate = DFlowNode_DuplicateTree(DFlowNode_GetArg(node, DFlowNode_ARG_TRUE_VALUE));
            DFlowNode_RemoveParent(duplicate, node);   // The duplication will have set the parent correctly,
            // but ReplaceWith is about to add it again, so remove it
            DFlowNode_ReplaceWith(DFlowNode_GetArg(node, DFlowNode_ARG_TRUE_VALUE), duplicate);
            DFlowNode_MarkForReplication(DFlowNode_GetArg(node, DFlowNode_ARG_TRUE_VALUE), false);

            // And the same for the else branch
            DFlowNode_MarkForReplication(DFlowNode_GetArg(node, DFlowNode_ARG_FALSE_VALUE), true);
            duplicate = DFlowNode_DuplicateTree(DFlowNode_GetArg(node, DFlowNode_ARG_FALSE_VALUE));
            DFlowNode_RemoveParent(duplicate, node);   // The duplication will have set the parent correctly,
            // but ReplaceWith is about to add it again, so remove it
            DFlowNode_ReplaceWith(DFlowNode_GetArg(node, DFlowNode_ARG_FALSE_VALUE), duplicate);
            DFlowNode_MarkForReplication(DFlowNode_GetArg(node, DFlowNode_ARG_FALSE_VALUE), false);
         }
      }
   }
}

void DFlowSubtreeDetectionVisitor_Constr(DFlowSubtreeDetectionVisitor *self, DFlowRecursionOptimizer *opt)
{
   DFlowVisitor_Constr(self, SubtreeDetect_Destroy, SubtreeDetect_Accept, opt);

   NodeVector_Constr(&self->m_nodeList, 128);
}

void DFlowSubtreeDetectionVisitor_Visit(DFlowSubtreeDetectionVisitor *self, DFlowNode *node)
{
   DFlowVisitor_VisitTopDown(self, node);

   // TODO
   if (0)
      DFlowVisitor_InvalidateRecursionOptimizer(self); // We've changed the graph, we must invalidate the recursion optimizer
}

#endif

///////////////////////////////////////////////////////////////////////////////
// DFlowOptimizeCandidate
///////////////////////////////////////////////////////////////////////////////

static void OptimizeCandidate_Destroy(void *me)
{
   DFlowOptimizeCandidateVisitor *self = (DFlowOptimizeCandidateVisitor *)me;

   NodeSet_Destr(&self->m_candidates[0]);
   NodeSet_Destr(&self->m_candidates[1]);
}

static void OptimizeCandidate_Accept(void *me, DFlowNode *node)
{
   DFlowOptimizeCandidateVisitor *self = (DFlowOptimizeCandidateVisitor *)me;

   // Add all the nodes to the candidate set
   NodeSet_insert(&self->m_candidates[self->m_current], node);
}

// Optimization candidate sets
void DFlowOptimizeCandidateVisitor_Constr(DFlowOptimizeCandidateVisitor *self, DFlowRecursionOptimizer *opt)
{
   DFlowVisitor_Constr(self, OptimizeCandidate_Destroy, OptimizeCandidate_Accept, opt);

   NodeSet_Constr(&self->m_candidates[0]);
   NodeSet_Constr(&self->m_candidates[1]);

   self->m_current = 0;
   self->m_next    = 1;
}


void DFlowOptimizeCandidateVisitor_Visit(DFlowOptimizeCandidateVisitor *self, DFlowNode *node)
{
   DFlowVisitor_VisitBottomUp(self, node);
}

void DFlowOptimizeCandidateVisitor_Remove(DFlowOptimizeCandidateVisitor *self, DFlowNode *node)
{
   NodeSet_erase(&self->m_candidates[self->m_current], node);
   NodeSet_erase(&self->m_candidates[self->m_next], node);
}

void DFlowOptimizeCandidateVisitor_AddParents(DFlowOptimizeCandidateVisitor *self, DFlowNode *node)
{
   // Changes to this node can potentially affect parents of the new node, so add them to the candidate list
   NodeList_const_iterator iter;

   for (iter = NodeList_const_begin(DFlowNode_Parents(node)); iter != NodeList_const_end(DFlowNode_Parents(node)); NodeList_const_next(&iter))
      NodeSet_insert(&self->m_candidates[self->m_next], NodeList_const_star(iter));
}

void DFlowOptimizeCandidateVisitor_Next(DFlowOptimizeCandidateVisitor *self)
{
   // Swap current and next
   self->m_current = (self->m_current + 1) % 2;
   self->m_next    = (self->m_next + 1)    % 2;

   // Clear the next list
   NodeSet_clear(&self->m_candidates[self->m_next]);
}

void DFlowOptimizeCandidateVisitor_Flatten(DFlowOptimizeCandidateVisitor *self, NodeVector *flat)
{
   NodeSet_Flatten(&self->m_candidates[self->m_current], flat);
}
