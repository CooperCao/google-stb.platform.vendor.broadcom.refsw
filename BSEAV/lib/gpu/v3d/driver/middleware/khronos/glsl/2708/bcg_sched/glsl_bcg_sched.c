/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/glsl_dataflow.h"
#include "middleware/khronos/glsl/2708/glsl_allocator_4.h"
#include "interface/khronos/common/khrn_options.h"

#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_print_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_depth_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_validate_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_sanitize_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_optimize_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_analyze_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_flatten_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_reghint_visitor.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_dflow_containers.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_qpu_instr.h"
#include "middleware/khronos/glsl/2708/bcg_sched/glsl_bcg_sched.h"

#ifdef WIN32
#define snprintf sprintf_s
#endif

static int graphviz_file_num = 0;

extern uint32_t xxx_shader;

///////////////////////////////////////////////////////////////////////////////
// GIVector
///////////////////////////////////////////////////////////////////////////////

void GIVector_Constr(GIVector *self, uint32_t capacity)
{
   self->m_end  = 0;
   VectorBase_Constr(&self->m_vector, sizeof(QPUGenericInstr) * capacity);
}

void GIVector_push_back(GIVector *self, const QPUGenericInstr *node)
{
   if (VectorBase_Extend(&self->m_vector, sizeof(QPUGenericInstr)))
   {
      QPUGenericInstr *resultList = (QPUGenericInstr *)self->m_vector.data;
      uint32_t         i          = self->m_end;

      self->m_end++;
      resultList[i] = *node;
   }
}

void GIVector_push_front(GIVector *self, const QPUGenericInstr *node)
{
   // NOTE: THIS IS EXPENSIVE!!
   if (VectorBase_Extend(&self->m_vector, sizeof(QPUGenericInstr)))
   {
      QPUGenericInstr *resultList = (QPUGenericInstr *)self->m_vector.data;
      self->m_end++;

      memmove(resultList + 1, resultList, self->m_vector.size - sizeof(QPUGenericInstr));

      resultList[0] = *node;
   }
}

///////////////////////////////////////////////////////////////////////////////
// UniformVector
///////////////////////////////////////////////////////////////////////////////

void UniformVector_Constr(UniformVector *self, uint32_t capacity)
{
   self->m_end  = 0;
   VectorBase_Constr(&self->m_vector, sizeof(Uniform) * capacity);
}

void UniformVector_push_back(UniformVector *self, const Uniform *node)
{
   if (VectorBase_Extend(&self->m_vector, sizeof(Uniform)))
   {
      Uniform     *resultList = (Uniform *)self->m_vector.data;
      uint32_t     i          = self->m_end;

      self->m_end++;
      resultList[i] = *node;
   }
}

///////////////////////////////////////////////////////////////////////////////
// VaryingVector
///////////////////////////////////////////////////////////////////////////////

void VaryingVector_Constr(VaryingVector *self, uint32_t capacity)
{
   self->m_end  = 0;
   VectorBase_Constr(&self->m_vector, sizeof(int32_t) * capacity);
}

void VaryingVector_push_back(VaryingVector *self, const int32_t *node)
{
   if (VectorBase_Extend(&self->m_vector, sizeof(int32_t)))
   {
      int32_t     *resultList = (int32_t *)self->m_vector.data;
      uint32_t     i          = self->m_end;

      self->m_end++;
      resultList[i] = *node;
   }
}

///////////////////////////////////////////////////////////////////////////////
// GraphOrder
///////////////////////////////////////////////////////////////////////////////

typedef struct GraphOrder_s
{
   uint32_t       m_count;
} GraphOrder;

static void GraphOrder_VisitOrdered(GraphOrder *self, DFlowNode *root);
static void GraphOrder_VisitSet(GraphOrder *self, NodeList_const_iterator begin, NodeList_const_iterator end);

void GraphOrder_Constr(GraphOrder *self)
{
   self->m_count = 0;
}

void GraphOrder_Destr(GraphOrder *self)
{
   UNUSED(self);
}

void GraphOrder_Visit(GraphOrder *self, DFlowNode *node)
{
   DFlowNode_lastVisitorId++;
   GraphOrder_VisitOrdered(self, node);
}

static bool BushyCmp(const DFlowNode *l, const DFlowNode *r)
{
   int32_t left  = DFlowNode_GetBushiness(l);
   int32_t right = DFlowNode_GetBushiness(r);

   return left < right;
}

void GraphOrder_VisitSet(GraphOrder *self, NodeList_const_iterator begin, NodeList_const_iterator end)
{
   NodeList_const_iterator iter;

   NodeList tmp;
   NodeList_Constr(&tmp);

   for (iter = begin; iter != end; NodeList_const_next(&iter))
      NodeList_insert(&tmp, NodeList_const_star(iter), BushyCmp);

   for (iter = NodeList_begin(&tmp); iter != NodeList_end(&tmp); NodeList_const_next(&iter))
   {
      if (NodeList_const_star(iter)->m_visitorId != DFlowNode_lastVisitorId)
         GraphOrder_VisitOrdered(self, NodeList_const_star(iter));
   }

   NodeList_Destr(&tmp);
}

void GraphOrder_VisitOrdered(GraphOrder *self, DFlowNode *root)
{
   GraphOrder_VisitSet(self, NodeList_const_begin(DFlowNode_IoChildren(root)), NodeList_const_end(DFlowNode_IoChildren(root)));
   GraphOrder_VisitSet(self, NodeList_const_begin(DFlowNode_Children(root)),   NodeList_const_end(DFlowNode_Children(root)));

   root->m_visitorId = DFlowNode_lastVisitorId;
   DFlowNode_SetOrder(root, self->m_count);
   self->m_count++;
}

static void DumpGraph(DFlowNode *root)
{
   if (khrn_options.glsl_debug_on)
   {
      DFlowRecursionOptimizer opt;  // We use a new recursion optimizer for printing to prevent any misrepresentation
      DFlowPrintVisitor       print;

      char filename[MAX_OPTION_PATHLEN + 20];
      snprintf(filename, sizeof(filename), "%s/graph_cpp_%d.txt", khrn_options.graphviz_folder, graphviz_file_num++);

      DFlowRecursionOptimizer_Constr(&opt);
      DFlowPrintVisitor_Constr(&print, &opt, filename);

      DFlowPrintVisitor_Visit(&print, root);

      DFlowPrintVisitor_Destr(&print);
      DFlowRecursionOptimizer_Destr(&opt);
   }
}

void bcg_reset_graphviz(void)
{
   graphviz_file_num = 0;
}

static bool IsDependent(DFlowNode *node, DFlowRecursionOptimizer *opt)
{
   bool  isDependent;

   DFlowDependentReadVisitor  dependCheck;
   DFlowDependentReadVisitor_Constr(&dependCheck, opt);
   DFlowDependentReadVisitor_Visit(&dependCheck, node);

   isDependent = DFlowDependentReadVisitor_IsDependent(&dependCheck);

   DFlowDependentReadVisitor_Destr(&dependCheck);

   return isDependent;
}

static uint32_t SetupTextureDependencies(DFlowRecursionOptimizer *opt, const DFlowAnalyzeVisitor *analyzer, bool allowThread, ResetHelper *rh)
{
   #define MAX_NUM_TMUS 2

   const uint32_t MAX_TMU_READ  = allowThread ? 2 : 4;
   const uint32_t MAX_TMU_WRITE = allowThread ? 4 : 8;
   const bool     TMU_NO_QUEUE  = false;               // Set to true to issue one texture fetch at a time

   uint32_t numThreadSwitches    = 0;
   uint32_t firstIssued          = 0;     // Index of the first of a batch of texture fetches.
   uint32_t writes[MAX_NUM_TMUS] = { 0 }; // Number of words written to TMU req FIFO, if this would overflow must start reading back.
   uint32_t reads[MAX_NUM_TMUS]  = { 0 }; // Number of fetches issued -- max outstanding is 4.

   bool graphChanged = false;

   uint32_t i, j;

   const TextureSetupVector   *setups = DFlowAnalyzeVisitor_TextureSetups(analyzer);

   // Add a node to do the TMU swap control
   if (TextureSetupVector_size(setups) > 1 && khrn_workarounds.TMUS_PER_SLICE > 1)
   {
      DFlowNode   *tmuSwap  = DFlowNode_Flavour_new(DATAFLOW_TMU_SWAP, rh);

      const TextureSetup *first = TextureSetupVector_const_index(setups, 0);

      if (TextureSetup_GetS(first))
         DFlowNode_AddExtraIoChild(TextureSetup_GetS(first), tmuSwap);
      if (TextureSetup_GetT(first))
         DFlowNode_AddExtraIoChild(TextureSetup_GetT(first), tmuSwap);
      if (TextureSetup_GetR(first))
         DFlowNode_AddExtraIoChild(TextureSetup_GetR(first), tmuSwap);
      if (TextureSetup_GetB(first))
         DFlowNode_AddExtraIoChild(TextureSetup_GetB(first), tmuSwap);

      graphChanged = true;
   }

   for (i = 0; i < TextureSetupVector_size(setups); i++)
   {
      const TextureSetup *t = TextureSetupVector_const_index(setups, i);
      bool last             = i == TextureSetupVector_size(setups) - 1;
      uint32_t nextSize     = 0;
      uint32_t nextWrites   = 0;
      uint32_t nextReads    = 0;

      // This flag is used to indicate that the node has been put into the I/O
      // order for textures.  If a subsequent TMU operation depends on this one, then
      // we must let this one complete (i.e. issue the loads and if necessary thread switch)
      DFlowNode_FlagTMUOrdered(TextureSetup_GetRequest(t));

      // Was there a previous request?
      if (i > 0)
      {
         // Get previous request
         const TextureSetup *p = TextureSetupVector_const_index(setups, i - 1);

         // Link requests in order
         DFlowNode_AddExtraIoChild(TextureSetup_GetRequest(t), TextureSetup_GetRequest(p));

         // All the TMU writes depend on finishing the previous batch (i.e. previous S must have been scheduled)
         if (TextureSetup_GetS(t))
            DFlowNode_AddExtraIoChild(TextureSetup_GetS(t), TextureSetup_GetS(p));
         if (TextureSetup_GetT(t))
            DFlowNode_AddExtraIoChild(TextureSetup_GetT(t), TextureSetup_GetS(p));
         if (TextureSetup_GetR(t))
            DFlowNode_AddExtraIoChild(TextureSetup_GetR(t), TextureSetup_GetS(p));
         if (TextureSetup_GetB(t))
            DFlowNode_AddExtraIoChild(TextureSetup_GetB(t), TextureSetup_GetS(p));

         graphChanged = true;
      }

      reads[TextureSetup_GetUnit(t)]  += 1;
      writes[TextureSetup_GetUnit(t)] += TextureSetup_GetWriteCount(t);

      nextSize   = 0;
      nextWrites = 0;
      nextReads  = 0;

      if (!last)
      {
         const TextureSetup   *nextSetup = TextureSetupVector_const_index(setups, i + 1);
         uint32_t             nextUnit   = TextureSetup_GetUnit(nextSetup);

         nextSize   = TextureSetup_GetWriteCount(nextSetup);
         nextWrites = writes[nextUnit];
         nextReads  = reads[nextUnit];
      }

      // Need to read back textures if:
      // 1) This is the last texture request
      // 2) The next texture request would overflow the request FIFO
      // 3) We have reached the maximum number of texture responses (2 or 4)
      // 4) The next texture request needs the results from an already issued request
      // 5) Flag override
      if (last || (nextWrites + nextSize) > MAX_TMU_WRITE || nextReads == MAX_TMU_READ || IsDependent(TextureSetup_GetS(TextureSetupVector_const_index(setups, i + 1)), opt) || TMU_NO_QUEUE)
      {
         // Link the last write to the first read
         if (firstIssued != i)
            DFlowNode_AddExtraIoChild(TextureSetup_GetRequest(TextureSetupVector_const_index(setups, firstIssued)), TextureSetup_GetS(t));

         // Link the next write to the last read
         if (!last)
         {
            DFlowNode            *treq = TextureSetup_GetRequest(t);
            const TextureSetup   *next = TextureSetupVector_const_index(setups, i + 1);

            if (TextureSetup_GetS(next) != 0)
               DFlowNode_AddExtraIoChild(TextureSetup_GetS(next), treq);

            if (TextureSetup_GetT(next) != 0)
               DFlowNode_AddExtraIoChild(TextureSetup_GetT(next), treq);

            if (TextureSetup_GetR(next) != 0)
               DFlowNode_AddExtraIoChild(TextureSetup_GetR(next), treq);

            if (TextureSetup_GetB(next) != 0)
               DFlowNode_AddExtraIoChild(TextureSetup_GetB(next), treq);
         }

         // This node will provoke a thread switch (if its enabled)
         if (allowThread)
         {
            DFlowNode_FlagForThreadSwitch(TextureSetup_GetS(t));
            numThreadSwitches++;
         }

         // Get ready for the next batch
         firstIssued = i + 1;

         for (j = 0; j < MAX_NUM_TMUS; ++j)
         {
            reads[j]  = 0;
            writes[j] = 0;
         }

         graphChanged = true;
      }
   }

   if (graphChanged)
      DFlowRecursionOptimizer_Invalidate(opt); // We've changed the graph, we must invalidate the recursion optimizer

   return numThreadSwitches;
}

void bcg_schedule_cleanup(void **resetHelper)
{
   ResetHelper *rh = (ResetHelper*)(*resetHelper);
   ResetHelper_Cleanup(rh);
   ResetHelper_delete(rh);
   *resetHelper = NULL;
}

static void DepthVisit(DFlowNode *rootNode, DFlowRecursionOptimizer *recursionOpt)
{
   DFlowDepthVisitor depth;
   DFlowDepthVisitor_Constr(&depth, recursionOpt);
   DFlowDepthVisitor_Visit(&depth, rootNode);
   DFlowDepthVisitor_Destr(&depth);
}

static void SanitizeVisit(DFlowNode *rootNode, DFlowRecursionOptimizer *recursionOpt)
{
   DFlowSanitizeVisitor    sanitizer;
   DFlowSanitizeVisitor_Constr(&sanitizer, recursionOpt);
   DFlowSanitizeVisitor_Visit(&sanitizer, rootNode);
   DFlowSanitizeVisitor_Destr(&sanitizer);
}

static void ValidateVisit(DFlowNode *rootNode, DFlowRecursionOptimizer *recursionOpt)
{
   DFlowValidateVisitor    validator;
   DFlowValidateVisitor_Constr(&validator, recursionOpt);
   DFlowValidateVisitor_Visit(&validator, rootNode);
   DFlowValidateVisitor_Destr(&validator);
}

static void RegHintVisit(DFlowNode *rootNode, DFlowRecursionOptimizer *recursionOpt)
{
   DFlowRegHintVisitor  regHint;
   DFlowRegHintVisitor_Constr(&regHint, recursionOpt);
   DFlowRegHintVisitor_Visit(&regHint, rootNode);
   DFlowRegHintVisitor_Destr(&regHint);
}

/*
static void SubtreeDetectionVisit(DFlowNode *rootNode, DFlowRecursionOptimizer *recursionOpt)
{
   DFlowSubtreeDetectionVisitor  subtree;
   DFlowSubtreeDetectionVisitor_Constr(&subtree, recursionOpt);
   DFlowSubtreeDetectionVisitor_Visit(&subtree, rootNode);
   DFlowSubtreeDetectionVisitor_Destr(&subtree);
}
*/

static void LastGaspVisit(DFlowNode *rootNode, DFlowRecursionOptimizer *recursionOpt)
{
   DFlowBushinessVisitor   bushy;
   GraphOrder              graphOrder;

   DFlowBushinessVisitor_Constr(&bushy, recursionOpt);
   DFlowBushinessVisitor_Visit(&bushy, rootNode);
   DFlowBushinessVisitor_Destr(&bushy);

   GraphOrder_Constr(&graphOrder);
   GraphOrder_Visit(&graphOrder, rootNode);
   GraphOrder_Destr(&graphOrder);
}

static bool BuildOutput(Scheduler *scheduler, uint32_t type,
                        MEM_HANDLE_T *mh_code, MEM_HANDLE_T *mh_uniform_map,
                        uint32_t *vary_count, uint32_t *vary_map)
{
   MEM_HANDLE_T   hcode;
   MEM_HANDLE_T   huniform_map;
   bool           ret = true;

   // Build the output
   hcode = mem_alloc_ex(Scheduler_CodeByteSize(scheduler), 8, MEM_FLAG_DIRECT, "shader code", MEM_COMPACT_DISCARD);
   huniform_map = mem_alloc_ex(Scheduler_UniformsByteSize(scheduler), 4, MEM_FLAG_NONE, "uniform map", MEM_COMPACT_DISCARD);

   if (hcode == MEM_HANDLE_INVALID || huniform_map == MEM_HANDLE_INVALID)
   {
      if (hcode != MEM_HANDLE_INVALID)
         mem_release(hcode);
      if (huniform_map != MEM_HANDLE_INVALID)
         mem_release(huniform_map);

      ret = false;
   }
   else
   {
      MEM_ASSIGN(*mh_code, hcode);
      MEM_ASSIGN(*mh_uniform_map, huniform_map);

      Scheduler_WriteCode(scheduler, mem_lock(hcode, NULL), Scheduler_CodeByteSize(scheduler));
      mem_unlock(hcode);
      mem_release(hcode);

      Scheduler_WriteUniforms(scheduler, mem_lock(huniform_map, NULL), Scheduler_UniformsByteSize(scheduler));
      mem_unlock(huniform_map);
      mem_release(huniform_map);

      if (type & GLSL_BACKEND_TYPE_FRAGMENT)
      {
         *vary_count = VaryingVector_size(Scheduler_Varyings(scheduler));
         Scheduler_WriteVaryings(scheduler, vary_map, Scheduler_VaryingsByteSize(scheduler));
      }

      Scheduler_TransmitOutput(scheduler, type);
      Scheduler_TransmitInfo(scheduler, type);
   }

   return ret;
}

#ifndef WIN32
__attribute__((noinline))  // Compiler bug (pg)
#endif
static void Optimize(DFlowNode *rootNode, DFlowRecursionOptimizer *recursionOpt)
{
   uint32_t count   = 0;
   uint32_t changes = 0;

   // Optimize the graph
   DFlowOptimizeVisitor          optimizer;
   DFlowOptimizeCandidateVisitor candidate;

   DFlowOptimizeVisitor_Constr(&optimizer, recursionOpt);
   DFlowOptimizeVisitor_Visit(&optimizer, rootNode);

   DFlowOptimizeCandidateVisitor_Constr(&candidate, recursionOpt);
   DFlowOptimizeCandidateVisitor_Visit(&candidate, rootNode);

   do
   {
      DFlowCombiner     combiner;
      DFlowSimplifier   simplifier;

      count = 0;

      // Merge identical nodes
      DFlowCombiner_Constr(&combiner, &candidate);
      DFlowCombiner_Visit(&combiner);

      count += DFlowCombiner_GetNumCombines(&combiner);

      DFlowCombiner_Destr(&combiner);

      // Evaluate constant expressions like A x 1
      DFlowSimplifier_Constr(&simplifier, &candidate);
      DFlowSimplifier_Visit(&simplifier);

      count   += DFlowSimplifier_GetNumRemoved(&simplifier);

      DFlowSimplifier_Destr(&simplifier);

      changes += count;

      // Swap old and new candidates
      DFlowOptimizeCandidateVisitor_Next(&candidate);

      // Repeat until a fixed point is reached
   } while (count != 0);

   //DFlowSubtreeDetectionVisitor detector(&recursionOpt);
   //detector.Visit(rootNode);
   //DFlowSanitizeVisitor sanitizer(&recursionOpt);
   //sanitizer.Visit(rootNode);

   // If any changes have been made then Sanitize the graph again as bits
   // of the graph can become orphaned
   if (changes != 0)
   {
      DFlowRecursionOptimizer_Invalidate(recursionOpt);
      SanitizeVisit(rootNode, recursionOpt);
   }

   DFlowOptimizeCandidateVisitor_Destr(&candidate);
   DFlowOptimizeVisitor_Destr(&optimizer);
}

static ResetHelper *InitResetHelper(void **resetHelper)
{
   ResetHelper *rh = (ResetHelper* )(*resetHelper);

   if (rh == NULL)
   {
      // First time in, the helper pointers will be NULL, so make a ResetHelper to record the nodes for next time
      rh = ResetHelper_new();
      *resetHelper = (void*)rh;
   }
   else
   {
      // Second time in, so clear the bcg_helper pointers
      ResetHelper_ResetHelperPtrs(rh);
   }

   return rh;
}

bool bcg_schedule(Dataflow *root, uint32_t type, bool *allow_thread, Scheduler_Strategy strategy, MEM_HANDLE_T *mh_code,
                  MEM_HANDLE_T *mh_uniform_map, uint32_t *vary_map, uint32_t *vary_count, void **resetHelper)
{
   bool                    ret = true;
   bool                    isFragment = type == GLSL_BACKEND_TYPE_FRAGMENT;
   ResetHelper             *rh;
   DFlowNode               *rootNode;
   DFlowRecursionOptimizer recursionOpt;
   DFlowAnalyzeVisitor     analyzer;
   uint32_t                numThreadSwitch;
   Scheduler               scheduler;

   assert(allow_thread != NULL);

   if (khrn_options.glsl_debug_on)
   {
      printf("\nScheduling %s shader from program %d\n",
         type == GLSL_BACKEND_TYPE_FRAGMENT ? "fragment" : (type == GLSL_BACKEND_TYPE_VERTEX ? "vertex" : "coord"),
         xxx_shader);

      if (strategy == Scheduler_ALT_SORT)
         printf("********** ALT SORT ***********\n");

      if (strategy == Scheduler_LAST_GASP)
         printf("********** LAST GASP ***********\n");
   }

   //uint32_t graphManipulationStart = vcos_getmicrosecs();

   // Make sure all bcg_helper (pointers to BCG tree) fields are NULL
   rh = InitResetHelper(resetHelper);

   // Build the tree
   rootNode = DFlowNode_Dataflow_new(root, rh);

   // Make a graph recursion optimizer
   DFlowRecursionOptimizer_Constr(&recursionOpt);

   // This will only dump when selected via environment variable or ShaderTool
   DumpGraph(rootNode);

   // Sanitize the graph and build parent lists
   SanitizeVisit(rootNode, &recursionOpt);

   DumpGraph(rootNode);

   if (khrn_options.glsl_optimizations_on)
      Optimize(rootNode, &recursionOpt);

   DumpGraph(rootNode);

   // Calculate node depths
   DepthVisit(rootNode, &recursionOpt);

   if (strategy != Scheduler_LAST_GASP)
      RegHintVisit(rootNode, &recursionOpt);

   // Check that the graph looks valid
   ValidateVisit(rootNode, &recursionOpt);

   // Analyze
   DFlowAnalyzeVisitor_Constr(&analyzer, &recursionOpt, rh);
   DFlowAnalyzeVisitor_Visit(&analyzer, rootNode);

   if (khrn_options.glsl_debug_on)
   {
      printf("Total number of levels = %d\n", DFlowAnalyzeVisitor_NumLevels(&analyzer));
      printf("Max nodes at any level = %d\n", DFlowAnalyzeVisitor_MaxNodesOnOneLevel(&analyzer));
   }

   // If there are no texture fetches then we mustn't be threaded.
   if (TextureSetupVector_size(DFlowAnalyzeVisitor_TextureSetups(&analyzer)) == 0)
      *allow_thread = false;

   // We need to patch up ioDependencies for texture setups to prevent them interleaving
   numThreadSwitch = SetupTextureDependencies(&recursionOpt, &analyzer, *allow_thread, rh);

   // Recalculate node depths
   DepthVisit(rootNode, &recursionOpt);

   // Check that the graph looks valid again
   ValidateVisit(rootNode, &recursionOpt);

   if (strategy != Scheduler_DEFAULT)
      LastGaspVisit(rootNode, &recursionOpt);

   // Used to detect subtrees for future uber-shader branch optimization
   //SubtreeDetectionVisit(rootNode, &recursionOpt);

   //if (khrn_options.glsl_debug_on)
   //   printf("Graph manipulation took %d\n", vcos_getmicrosecs() - graphManipulationStart);

   DumpGraph(rootNode);

   // Try to schedule
   Scheduler_Constr(&scheduler, isFragment, *allow_thread, strategy, numThreadSwitch);
   ret = Scheduler_Schedule(&scheduler, rootNode, &recursionOpt);

   DumpGraph(rootNode);

   if (ret)
      ret = BuildOutput(&scheduler, type, mh_code, mh_uniform_map, vary_count, vary_map);

   //ResetHelper_DeleteNodes(rh);   // Delete our allocated nodes
   Scheduler_Destr(&scheduler);
   DFlowRecursionOptimizer_Destr(&recursionOpt);
   DFlowAnalyzeVisitor_Destr(&analyzer);

   return ret;
}

void (*line_capture_fragment)(char * str);
void (*line_capture_vertex)(char * str);
void (*line_capture_coord)(char * str);
void (*capture_sched_info)(char * errstr, uint32_t type, bool fragThreaded);

void Scheduler_Constr(Scheduler *self, bool isFragment, bool allowThread, Scheduler_Strategy strategy, uint32_t maxThreadSwitches)
{
   self->m_isFragmentShader      = isFragment;
   self->m_allowThread           = allowThread;
   self->m_strategy              = strategy;
   self->m_maxThreadSwitches     = maxThreadSwitches;
   self->m_numThreadSwitches     = 0;
   self->m_activeNode            = NULL;
   self->m_currentFlagsCondition = NULL;
   self->m_needsACC5Move         = false;
   self->m_ACC5MoveNode          = NULL;

   QPUResources_Constr(&self->m_resources, isFragment, allowThread);
   GIVector_Constr(&self->m_schedule, 64);
   UniformVector_Constr(&self->m_uniforms, 64);
   VaryingVector_Constr(&self->m_varyings, 64);
   NodeList_Constr(&self->m_schedulable);
   NodeList_Constr(&self->m_notSchedulable);

   self->m_nextTextureUniform[0] = BACKEND_UNIFORM_TEX_PARAM0;
   self->m_nextTextureUniform[1] = BACKEND_UNIFORM_TEX_PARAM0;
}

void Scheduler_Destr(Scheduler *self)
{
   uint32_t i;

   NodeList_Destr(&self->m_notSchedulable);
   NodeList_Destr(&self->m_schedulable);
   VaryingVector_Destr(&self->m_varyings);
   UniformVector_Destr(&self->m_uniforms);

   for (i = 0; i < GIVector_size(&self->m_schedule); ++i)
      QPUGenericInstr_Destr(GIVector_lindex(&self->m_schedule, i));

   GIVector_Destr(&self->m_schedule);
   QPUResources_Destr(&self->m_resources);
}

void Scheduler_TransmitOutput(Scheduler *self, uint32_t type)
{
   if (line_capture_fragment != NULL || line_capture_vertex != NULL)
   {
      uint32_t          curUniform = 0;
      GIVector_iterator iter;

      for (iter = GIVector_begin(&self->m_schedule); iter != GIVector_end(&self->m_schedule); GIVector_next(&iter))
      {
         QPUGenericInstr   *gi    = GIVector_star(iter);
         QPUInstr          *instr = QPUGenericInstr_GetInstruction(gi);
         char               buff[DISASM_BUFFER_SIZE + 10];

         buff[0] = '\0';
         strcat(buff, "[0,0] ");

         if (UniformVector_size(&self->m_uniforms) == 0)
            strcat(buff, QPUInstr_Disassemble(instr));
         else
            strcat(buff, QPUInstr_DisassembleEx(instr, UniformVector_size(&self->m_uniforms), UniformVector_lindex(&self->m_uniforms, 0), &curUniform));

         if  (line_capture_fragment && (type == GLSL_BACKEND_TYPE_FRAGMENT))
            line_capture_fragment(buff);
         else if (line_capture_vertex && (type == GLSL_BACKEND_TYPE_VERTEX))
            line_capture_vertex(buff);
         else if (line_capture_coord && (type == GLSL_BACKEND_TYPE_COORD))
            line_capture_coord(buff);
      }
   }
}

void Scheduler_TransmitInfo(Scheduler *self, uint32_t type)
{
   bool stillReferenced = false;
   bool underflow = false;

   QPUResources_CheckRefCounts(&self->m_resources, &stillReferenced, &underflow);

   if (capture_sched_info)
   {
      char errbuf[512];
      errbuf[0] = '\0';

      if (stillReferenced)
         strcat(errbuf, "Err: registers still referenced. ");
      if (underflow)
         strcat(errbuf, "Err: register underflow.");

      capture_sched_info(errbuf, type, self->m_isFragmentShader && self->m_allowThread);
   }
}

static void Scheduler_AddInstr(Scheduler *self, QPUGenericInstr *instr)
{
   GIVector_push_back(&self->m_schedule, instr);
}

static void Scheduler_AddInstrAtHead(Scheduler *self, QPUGenericInstr *instr)
{
   GIVector_push_front(&self->m_schedule, instr);
}

static int32_t NodeValue(const DFlowNode *node)
{
   int32_t value = 0;

   //int32_t  l_off = l->IoParents().size() * 1000;
   //l_off += l->GetRegisterDelta() * -100000;

   //value -= DFlowNode_Flavour(node) == DATAFLOW_TEX_GET_CMP_R ? 1000 : 0;
   value += DFlowNode_Flavour(node) == DATAFLOW_TMU_SWAP ? 1000 : 0;
   value += DFlowNode_HasVaryCChild(node) ? 100 : 0;
   value += DFlowNode_TreeDepth(node) + DFlowNode_LifespanGuess(node);

   return value;
}

static bool DFlowCmp(const DFlowNode *l, const DFlowNode *r)
{
   return NodeValue(l) > NodeValue(r);
}

static bool LastGaspDFlowCmp(const DFlowNode *l, const DFlowNode *r)
{
   int32_t leftCost  = DFlowNode_GetOrder(l);
   int32_t rightCost = DFlowNode_GetOrder(r);

   return leftCost < rightCost;
}

static bool NodeCmp(const DFlowNode *l, const DFlowNode *r)
{
   return DFlowNode_SortOrder(l) < DFlowNode_SortOrder(r);
}

static void Scheduler_NodeCompleted(Scheduler *self, DFlowNode *doneNode)
{
   // This node is done, see if its parents are now schedulable
   NodeList_const_iterator  iter;
   const NodeList          *parents   = DFlowNode_Parents(doneNode);
   const NodeList          *ioParents = DFlowNode_IoParents(doneNode);

   for (iter = NodeList_const_begin(parents); iter != NodeList_const_end(parents); NodeList_const_next(&iter))
   {
      DFlowNode   *parent = NodeList_const_star(iter);

      assert(DFlowNode_Slot(parent) == DFlowNode_NOT_SCHEDULED);

      if (DFlowNode_IsSchedulable(parent))
      {
         NodeList_insert(&self->m_schedulable, parent, NodeCmp);
         NodeList_remove(&self->m_notSchedulable, parent);
      }
   }

   for (iter = NodeList_const_begin(ioParents); iter != NodeList_const_end(ioParents); NodeList_const_next(&iter))
   {
      DFlowNode   *ioParent = NodeList_const_star(iter);
      assert(DFlowNode_Slot(ioParent) == DFlowNode_NOT_SCHEDULED);

      if (DFlowNode_IsSchedulable(ioParent))
      {
         NodeList_insert(&self->m_schedulable, ioParent, NodeCmp);
         NodeList_remove(&self->m_notSchedulable, ioParent);
      }
   }

   NodeList_remove(&self->m_schedulable, doneNode);
}

static bool AllowedWriteInLastThreeInstrs(Register_Enum reg)
{
   switch (reg)
   {
   case Register_RA14            :
   case Register_RB14            :
   case Register_TMU0_B          :
   case Register_TMU1_B          :
   case Register_TMU0_R          :
   case Register_TMU1_R          :
   case Register_TMU0_T          :
   case Register_TMU1_T          :
   case Register_TMU0_S          :
   case Register_TMU1_S          :
   case Register_VPM_READ        :
   case Register_VPM_WRITE       :
   case Register_VPM_LD_BUSY     :
   case Register_VPM_ST_BUSY     :
   case Register_VPMVCD_RD_SETUP :
   case Register_VPMVCD_WR_SETUP :
   case Register_VPM_LD_WAIT     :
   case Register_VPM_ST_WAIT     :
   case Register_VPM_LD_ADDR     :
   case Register_VPM_ST_ADDR     :
      return false;
   case Register_TLB_Z           :
      return !(khrn_workarounds.HW2806 ||
               khrn_workarounds.GFXH1670);
   case Register_TLB_COLOUR_MS   :
   case Register_TLB_COLOUR_ALL  :
   case Register_TLB_ALPHA_MASK:
      return !khrn_workarounds.GFXH1670;
   default:
      return true;
   }
}

static bool WritesTileColor(const QPUGenericInstr *instr)
{
   const QPUInstr *base = QPUGenericInstr_GetInstructionConst(instr);

   return QPUInstr_WritesTo(base, Register_TLB_COLOUR_ALL) || QPUInstr_WritesTo(base, Register_TLB_COLOUR_MS);
}

static bool IsValidLastInstr(const QPUGenericInstr *instr)
{
   const QPUInstr *base = QPUGenericInstr_GetInstructionConst(instr);

   Register_Enum addOut = QPUInstr_GetAddOutRegister(base);
   Register_Enum mulOut = QPUInstr_GetMulOutRegister(base);

   // Final instruction cannot write TLB_Z
   if (QPUInstr_WritesTo(base, Register_TLB_Z))
      return false;

   // If final instr is TLB_C write, it must have a spare signal slot
   if (WritesTileColor(instr) && QPUInstr_GetSignal(base) != Sig_NONE)
      return false;

   // Last 3 instructions must not read varyings or uniforms or any access to VPM, VDR or VDW.
   if (!AllowedWriteInLastThreeInstrs(addOut) || !AllowedWriteInLastThreeInstrs(mulOut))
      return false;
   if (QPUInstr_HasInvalidReadsForProgramEnd(base))
      return false;

   return true;
}

static bool IsValidLastButOneInstr(const QPUGenericInstr *instr)
{
   const QPUInstr *base = QPUGenericInstr_GetInstructionConst(instr);

   Register_Enum addOut = QPUInstr_GetAddOutRegister(base);
   Register_Enum mulOut = QPUInstr_GetMulOutRegister(base);

   // Last 3 instructions must not read varyings or uniforms or any access to VPM, VDR or VDW.
   if (!AllowedWriteInLastThreeInstrs(addOut) || !AllowedWriteInLastThreeInstrs(mulOut))
      return false;
   if (QPUInstr_HasInvalidReadsForProgramEnd(base))
      return false;

   return true;
}

static bool IsValidLastButTwoInstr(const QPUGenericInstr *instr)
{
   const QPUInstr *base = QPUGenericInstr_GetInstructionConst(instr);

   Register_Enum addOut = QPUInstr_GetAddOutRegister(base);
   Register_Enum mulOut = QPUInstr_GetMulOutRegister(base);

   // ThreadEnd cannot write A or B reg files
   if (Register_IsNormalReg(addOut) || Register_IsNormalReg(mulOut))
      return false;

   // Last 3 instructions must not read varyings or uniforms or any access to VPM, VDR or VDW.
   if (!AllowedWriteInLastThreeInstrs(addOut) || !AllowedWriteInLastThreeInstrs(mulOut))
      return false;
   if (QPUInstr_HasInvalidReadsForProgramEnd(base))
      return false;

   // Need a signal for the thrend.
   if (QPUInstr_GetType(base) != QPUInstr_UNKNOWN)
      if (QPUInstr_GetType(base) != QPUInstr_ALU || QPUInstr_GetSignal(base) != Sig_NONE)
         return false;

   return true;
}

static bool IsValidLastButThreeInstr(const QPUGenericInstr *instr)
{
   const QPUInstr *base = QPUGenericInstr_GetInstructionConst(instr);

   if (khrn_workarounds.HW2806 && QPUInstr_WritesTo(base, Register_TLB_Z))
   {
      // Can't write TLB_Z in last four instructions
      return false;
   }

   return true;
}

static void Scheduler_AddProgramEnd(Scheduler *self)
{
   // Check current instruction for the myriad of special cases where program end can't be issued.
   // Last 3 instructions must not read varyings or uniforms or any access to VPM, VDR or VDW.
   // ThreadEnd cannot write A or B reg files
   // ThreadEnd and its 2 delay slots cannot access RA14 or RB14
   // Final instruction cannot write TLBZ
   bool ok = false;

   QPUGenericInstr *last         = NULL;
   QPUGenericInstr *lastButOne   = NULL;
   QPUGenericInstr *lastButTwo   = NULL;
   QPUGenericInstr *lastButThree = NULL;

   do
   {
      GIVector *schedule  = &self->m_schedule;
      uint32_t  schedSize = GIVector_size(schedule);

      if (schedSize > 0)
         last = GIVector_lindex(schedule, schedSize - 1);
      if (schedSize > 1)
         lastButOne = GIVector_lindex(schedule, schedSize - 2);
      if (schedSize > 2)
         lastButTwo = GIVector_lindex(schedule, schedSize - 3);
      if (schedSize > 3)
         lastButThree = GIVector_lindex(schedule, schedSize - 4);

      ok = last && IsValidLastInstr(last);
      ok = ok && lastButOne && IsValidLastButOneInstr(lastButOne);
      ok = ok && lastButTwo && IsValidLastButTwoInstr(lastButTwo);

      if (lastButThree)
         ok = ok && IsValidLastButThreeInstr(lastButThree);

      if (self->m_isFragmentShader)
      {
         // Also need one of the last two instructions to have a signal free for sbdone
         if (ok && (QPUInstr_GetType(QPUGenericInstr_GetInstruction(last)) == QPUInstr_ALU && QPUInstr_GetSignal(QPUGenericInstr_GetInstruction(last)) == Sig_NONE))
            ok = true;
         else if (ok && (QPUInstr_GetType(QPUGenericInstr_GetInstruction(lastButOne)) == QPUInstr_ALU && QPUInstr_GetSignal(QPUGenericInstr_GetInstruction(lastButOne)) == Sig_NONE))
            ok = true;
         else
            ok = false;
      }

      if (!ok)
      {
         QPUGenericInstr nop;
         QPUGenericInstr_ConstrType(&nop, QPUInstr_ALU);
         Scheduler_AddInstr(self, &nop);
      }
   }
   while (!ok);

   assert(QPUInstr_GetSignal(QPUGenericInstr_GetInstruction(lastButTwo)) == Sig_NONE);

   QPUGenericInstr_SetSignal(lastButTwo, Sig_END);

   if (self->m_isFragmentShader)
   {
      bool     needsSBWait = false;
      bool     needsExplicitSBWait = khrn_workarounds.SBWAIT;
      int32_t  bestSBWaitSlot = -1;
      int32_t  implicitSBWaitSlot = -1;
      int32_t  i;
      uint32_t numSlots;
      bool     doneSBwait;

      // Now we need to push the SBWAIT down as far as possible, or remove it completely in some cases
      for (i = 0; i < (int32_t)GIVector_size(&self->m_schedule); i++)
      {
         QPUGenericInstr *gi    = GIVector_lindex(&self->m_schedule, i);
         QPUInstr        *instr = QPUGenericInstr_GetInstruction(gi);

         // If there is an SBWAIT in the code, remove it, but remember where it was
         if (QPUGenericInstr_SigUsed(gi) && QPUInstr_GetSignal(instr) == Sig_SBLOCK)
         {
            bestSBWaitSlot = i;
            QPUGenericInstr_ClearSignal(gi);
         }

         // See if an sbwait is required
         if (QPUInstr_HasImplicitSBWait(instr))
         {
            needsSBWait = true;
            implicitSBWaitSlot = i;
            break;
         }
         else if (!QPUGenericInstr_SigUsed(gi) && QPUGenericInstr_GetType(gi) == QPUInstr_ALU && i < (int32_t)GIVector_size(&self->m_schedule) - 3)
            bestSBWaitSlot = i;
      }

      // No implicit waits in last 3 instructions
      numSlots = GIVector_size(&self->m_schedule);
      if (numSlots - implicitSBWaitSlot <= 3)
         needsExplicitSBWait = true;

      // Degenerate case when there is nothing in the code for scoreboard
      if (implicitSBWaitSlot == -1 && !needsSBWait)
      {
         needsExplicitSBWait = true;
         needsSBWait = true;
      }

      doneSBwait = true;
      do
      {
         if (needsSBWait && needsExplicitSBWait && bestSBWaitSlot != -1)
         {
            doneSBwait = false;

            // Must have an explicit SBWAIT
            if (bestSBWaitSlot == 0 || numSlots - bestSBWaitSlot <= 3)
            {
               // Cannot insert sbwait in first slot or last 3 slots. Insert a nop at the HEAD of the schedule and try again
               QPUGenericInstr nop;
               QPUGenericInstr_ConstrType(&nop, QPUInstr_ALU);
               Scheduler_AddInstrAtHead(self, &nop);

               numSlots = GIVector_size(&self->m_schedule);

               if (numSlots > 0)
                  last = GIVector_lindex(&self->m_schedule, numSlots - 1);
               if (numSlots > 1)
                  lastButOne = GIVector_lindex(&self->m_schedule, numSlots - 2);

               if (bestSBWaitSlot == 0)
                  bestSBWaitSlot = 1;
               else if (numSlots - bestSBWaitSlot <= 3)
                  bestSBWaitSlot = 0;
            }
            else
            {
               QPUGenericInstr_SetSignal(GIVector_lindex(&self->m_schedule, bestSBWaitSlot), Sig_SBLOCK);
               doneSBwait = true;
            }
         }

         implicitSBWaitSlot += 1;
      }
      while (!doneSBwait);

      // Add the sbdone if we need it
      if (needsSBWait)
      {
         if (WritesTileColor(last) || QPUInstr_GetSignal(QPUGenericInstr_GetInstruction(lastButOne)) != Sig_NONE)
         {
            assert(QPUInstr_GetSignal(QPUGenericInstr_GetInstruction(last)) == Sig_NONE);
            vcos_verify(QPUGenericInstr_SetSignal(last, Sig_SBUNLOCK));
         }
         else
         {
            assert(QPUInstr_GetSignal(QPUGenericInstr_GetInstruction(lastButOne)) == Sig_NONE);
            vcos_verify(QPUGenericInstr_SetSignal(lastButOne, Sig_SBUNLOCK));
         }
      }
   }
}

static void Flatten(DFlowNode *rootNode, DFlowRecursionOptimizer *opt, NodeList *linearNodes)
{
   DFlowFlattenVisitor        flattener;
   DFlowFlattenVisitor_Constr(&flattener, opt, linearNodes);
   DFlowFlattenVisitor_Visit(&flattener, rootNode);
   DFlowFlattenVisitor_Destr(&flattener);
}

#ifndef WIN32
__attribute__((noinline))  // Compiler bug (shown with pg)
#endif
bool Scheduler_Schedule(Scheduler *self, DFlowNode *root, DFlowRecursionOptimizer *opt)
{
   bool                       ret = true;
   NodeList                   linearNodes;
   NodeList_iterator          liter;
   int32_t                    order = 0;
   uint32_t                   nopCount = 0;
   DFlowNode_ScheduleStatus   status = DFlowNode_SCHEDULED;

   if (khrn_options.glsl_debug_on)
      printf("Thread switch is %s\n", Scheduler_AllowThreadswitch(self) ? "on" : "off");

   // Create a linear tromp through the code
   NodeList_Constr(&linearNodes);
   Flatten(root, opt, &linearNodes);

   //printf("Before -----------------------------------\n");
   //m_resources.DebugDump();

   // Sort the linear nodes based on our ordering criteria
   if (self->m_strategy == Scheduler_DEFAULT)
      NodeList_sort(&linearNodes, DFlowCmp);
   else
      NodeList_sort(&linearNodes, LastGaspDFlowCmp);

   // Split linear nodes into schedulable and not-schedulable lists.
   // Also number the nodes in sorted order to make maintaining the schedulable list easier.
   for (liter = NodeList_begin(&linearNodes); liter != NodeList_end(&linearNodes); NodeList_next(&liter))
   {
      DFlowNode   *node = NodeList_star(liter);

      DFlowNode_SetSortOrder(node, order);

      if (DFlowNode_NumChildren(node) == 0)
         NodeList_push_back(&self->m_schedulable, node);
      else
         NodeList_push_back(&self->m_notSchedulable, node);

      order++;
   }

   while (NodeList_size(&self->m_schedulable) > 0 || NodeList_size(&self->m_notSchedulable) > 0)
   {
      NodeList_iterator iter;

      QPUGenericInstr instr;
      QPUGenericInstr_Constr(&instr);

      status = DFlowNode_SCHEDULED;

      // All the time we can add more to the current instruction, do so
      while (status == DFlowNode_SCHEDULED || status == DFlowNode_INSERTED_EXTRA_INSTR || status == DFlowNode_NOTHING_TO_SCHEDULE)
      {
         status = DFlowNode_NOT_SCHEDULABLE;

         for (iter = NodeList_begin(&self->m_schedulable); iter != NodeList_end(&self->m_schedulable); NodeList_next(&iter))
         {
            DFlowNode   *node = NodeList_star(iter);

            // Try to avoid increasing the number of registers used when dual issuing
            if (Scheduler_LastGasp(self) && (QPUGenericInstr_AdderUsed(&instr) || QPUGenericInstr_MulUsed(&instr)))
            {
               if (DFlowNode_GetRegisterDelta(node) > 0)
                  continue;
            }

            // Active node causes scheduler to persist with the specified instruction until
            // it is finished.  Used for texture write to S on a thread switch
            if (self->m_activeNode == NULL || node == self->m_activeNode)
            {
               status = DFlowNode_AddToInstruction(node, self, &instr);

               if (status == DFlowNode_INSERTED_EXTRA_INSTR)
               {
                  if (!DFlowNode_IncrementIssueCount(node))
                  {
                     ret = false;
                     goto quit;
                  }
               }


               if (status == DFlowNode_SCHEDULED || status == DFlowNode_NOTHING_TO_SCHEDULE)
               {
                  if (status == DFlowNode_SCHEDULED)
                     DFlowNode_SetSlot(node, Scheduler_FirstEmptySlot(self));
                  else // DFlowNode_NOTHING_TO_SCHEDULE
                     DFlowNode_SetSlot(node, -1);   // Available right away

                  // This node is done, see if its parents are now schedulable and then remove it from the list
                  Scheduler_NodeCompleted(self, node);
                  break;
               }
               else if (status == DFlowNode_INSERTED_EXTRA_INSTR)
                  break;   // Restart loop
            }
         }

         if (QPUGenericInstr_IsFull(&instr))
            break;

         if (khrn_options.glsl_single_issue)
         {
            // Force one instr per slot
            if (status == DFlowNode_SCHEDULED || status == DFlowNode_INSERTED_EXTRA_INSTR)
               break;
         }
      }

      // Instruction is as full as it's going to get, so add to output
      Scheduler_AddInstr(self, &instr);

      //const char *dis = QPUInstr_Disassemble(QPUGenericInstr_GetInstruction(&instr));
      //printf("%s\n", dis);

      if (QPUGenericInstr_GetType(&instr) == QPUInstr_UNKNOWN)
         nopCount++;
      else
         nopCount = 0;

      /*
      if (nopCount == 5)
      {

         if (m_resources.IsFull(AllowThreadswitch()))
         {
            printf("*****************************************************************\n");
            printf("Out of registers\n");
            printf("*****************************************************************\n");
         }
         else
            nopCount = nopCount; // Put breakpoint here
      }
      */

      if (nopCount > 8)
      {
/*
         if (!m_resources.IsFull(AllowThreadswitch()))
         {
            printf("*****************************************************************\n");
            printf("Infinite nop loop\n");
            printf("*****************************************************************\n");
         }
*/

         ret = false;
         goto quit;
      }
   }

   // We've scheduled all the nodes now, so add the program end code
   Scheduler_AddProgramEnd(self);

   //printf("After -----------------------------------\n");
   //m_resources.DebugDump();

   if (khrn_options.glsl_debug_on)
   {
      GIVector_iterator iter;
      for (iter = GIVector_begin(&self->m_schedule); iter != GIVector_end(&self->m_schedule); GIVector_next(&iter))
      {
         QPUGenericInstr   *gi = GIVector_star(iter);
         const char        *dis = QPUInstr_Disassemble(QPUGenericInstr_GetInstruction(gi));

         printf("%s\n", dis);
         fflush(stdout);
      }
   }

quit:
   NodeList_Destr(&linearNodes);
   return ret;
}

int32_t Scheduler_FirstEmptySlot(const Scheduler *self)
{
   return GIVector_size(&self->m_schedule);
}

void Scheduler_WriteCode(Scheduler *self, void *code, size_t bufSize)
{
   uint32_t          *c;
   GIVector_iterator iter;

   if (bufSize < Scheduler_CodeByteSize(self))
   {
      assert(0);
      return;
   }

   c = (uint32_t*)code;

   for (iter = GIVector_begin(&self->m_schedule); iter != GIVector_end(&self->m_schedule); GIVector_next(&iter))
   {
      QPUGenericInstr  *gi    = GIVector_star(iter);
      uint64_t          iCode = QPUInstr_GetCoding(QPUGenericInstr_GetInstruction(gi));
      uint32_t         *pCode = (uint32_t *)&iCode;

#ifdef BIG_ENDIAN_CPU
      c[1] = pCode[0];
      c[0] = pCode[1];
#else
      c[0] = pCode[0];
      c[1] = pCode[1];
#endif
      c += 2;
   }
}

void Scheduler_InsertThreadSwitch(Scheduler *self)
{
   QPUGenericInstr *instr;

   uint32_t schedSize = GIVector_size(&self->m_schedule);
   assert(schedSize >= 2);

   instr = GIVector_lindex(&self->m_schedule, schedSize - 2);

   self->m_numThreadSwitches += 1;

   QPUGenericInstr_SetSignal(instr, self->m_numThreadSwitches == self->m_maxThreadSwitches ? Sig_LTHRSW : Sig_THRSW);
}

bool Scheduler_CanInsertThreadSwitch(const Scheduler *self)
{
   uint32_t        schedSize = GIVector_size(&self->m_schedule);
   uint32_t        last;
   QPUGenericInstr instr;

   if (schedSize < 2)
      return false;

   last = schedSize - 1;

   // Intervening instructions mustn't signal
   if (QPUGenericInstr_SigUsed(GIVector_const_lindex(&self->m_schedule, last)))
      return false;

   if (QPUGenericInstr_SigUsed(GIVector_const_lindex(&self->m_schedule, last - 1)))
      return false;

   // Copy the instruction for a trial run
   QPUGenericInstr_ConstrCopy(&instr, GIVector_const_lindex(&self->m_schedule, last - 1));

   // Just try to set a signal for thread switch (doesn't matter which)
   return QPUGenericInstr_SetSignal(&instr, Sig_THRSW);
}

bool Scheduler_AllThreadSwitchesDone(const Scheduler *self)
{
   assert(self->m_numThreadSwitches <= self->m_maxThreadSwitches);

   return self->m_numThreadSwitches == self->m_maxThreadSwitches;
}

void Scheduler_WriteUniforms(Scheduler *self, void *uniforms, size_t bufSize)
{
   uint32_t unifByteSize = Scheduler_UniformsByteSize(self);
   uint32_t i;

   if (bufSize < unifByteSize)
   {
      assert(0);
      return;
   }

   if (unifByteSize > 0)
      memcpy(uniforms, UniformVector_lindex(&self->m_uniforms, 0), unifByteSize);

   if (khrn_options.glsl_debug_on)
   {
      printf("===== uniforms:\n");
      for (i = 0; i < UniformVector_size(&self->m_uniforms); i++)
      {
         Uniform  *uniform = UniformVector_lindex(&self->m_uniforms, i);

         printf("%2d) %d %08x\n", i, uniform->m_type, uniform->m_value);
      }
   }
}

void Scheduler_WriteVaryings(Scheduler *self, void *varyings, size_t bufSize)
{
   uint32_t varyingsByteSize = Scheduler_VaryingsByteSize(self);

   if (bufSize < varyingsByteSize || bufSize > 32 * 4)
   {
      assert(0);
      return;
   }

   if (varyingsByteSize > 0)
   {
      memcpy(varyings, VaryingVector_lindex(&self->m_varyings, 0), varyingsByteSize);

      if (khrn_options.glsl_debug_on)
      {
         uint32_t i;

         printf("================= Varyings byte size = %d\n", varyingsByteSize);
         for (i = 0; i < varyingsByteSize / 4; i++)
            printf("v%2d = %x\n", i, VaryingVector_index(&self->m_varyings, i));
      }
   }
}
