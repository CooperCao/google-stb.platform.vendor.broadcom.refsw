/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "DflowScalars.h"
#include "NodeBase.h"
#include "Compiler.h"
#include "FunctionContext.h"
#include "SymbolHandle.h"
#include "DescriptorMap.h"

#include "glsl_basic_block.h"

#include "DescriptorInfo.h"

#include "Map.h"

#include <vector>
#include <bitset>

namespace bvk {

class Module;
class Node;
class Specialization;

class ExecutionModes
{
public:
   ExecutionModes() :
      m_workgroupSize({1, 1, 1})
   {
   }

   void Record(const NodeExecutionMode *node);

   void SetWorkgroupSize(uint32_t x, uint32_t y, uint32_t z)
   {
      m_workgroupSize[0] = x;
      m_workgroupSize[1] = y;
      m_workgroupSize[2] = z;
   }

   const std::array<uint32_t, 3> &GetWorkgroupSize() const { return m_workgroupSize; }

   bool IsPixelCenterInteger()  const { return m_pixelCenterInteger; }
   bool IsOriginUpperLeft()     const { return m_originUpperLeft;    }
   bool IsDepthReplacing()      const { return m_depthReplacing;     }
   bool HasEarlyFragTests()     const { return m_earlyFragmentTests; }

private:
   std::array<uint32_t, 3> m_workgroupSize;
   bool                    m_pixelCenterInteger = false;
   bool                    m_originUpperLeft = true;
   bool                    m_depthReplacing = false;
   bool                    m_earlyFragmentTests = false;
};

///////////////////////////////////////////////////////////////////////////////
// DflowBuilder
//
// Main class responsible for converting SPIRV instruction stream into
// dataflow suitable for the GLSL compiler.
//
///////////////////////////////////////////////////////////////////////////////

class DflowBuilder : public NodeAssertVisitor, public NonCopyable
{
public:
   struct InterfaceSymbol
   {
      InterfaceSymbol(const DflowScalars &scalars, SymbolHandle sym) :
         scalars(scalars), symbol(sym)
      {}

      DflowScalars   scalars;
      SymbolHandle   symbol;
   };

   // Constructor
   // "module" holds the SPIRV node tree
   DflowBuilder(DescriptorTables *tables,
                const Module &module, const Specialization &specializations,
                bool robustBufferAccess, bool multiSampled);

   ~DflowBuilder();

   // Main entry point
   void Build(const char *name, ShaderFlavour flavour);

   DescriptorMaps &GetDescriptorMaps() { return m_descriptorMaps; }
   const Module   &GetModule() const   { return m_module;         }

   void Visit(const NodeTypeVoid *) override;
   void Visit(const NodeTypeBool *) override;
   void Visit(const NodeTypeInt *) override;
   void Visit(const NodeTypeFloat *) override;
   void Visit(const NodeTypeVector *) override;
   void Visit(const NodeTypeMatrix *) override;
   void Visit(const NodeTypeImage *) override;
   void Visit(const NodeTypeSampler *) override;
   void Visit(const NodeTypeSampledImage *) override;
   void Visit(const NodeTypeArray *) override;
   void Visit(const NodeTypeRuntimeArray *) override;
   void Visit(const NodeTypeStruct *) override;
   void Visit(const NodeTypePointer *) override;
   void Visit(const NodeTypeFunction *) override;


   // Methods to construct dataflow for each node type
   void Visit(const NodeVariable *var) override;

   // Pointer dereferencing
   void Visit(const NodeLoad *node) override;
   void Visit(const NodeStore *node) override;
   void Visit(const NodeCopyMemory *node) override;

   // Atomics
   void Visit(const NodeAtomicLoad *node) override;
   void Visit(const NodeAtomicStore *node) override;
   void Visit(const NodeAtomicExchange *node) override;
   void Visit(const NodeAtomicCompareExchange *node) override;
   void Visit(const NodeAtomicIIncrement *node) override;
   void Visit(const NodeAtomicIDecrement *node) override;
   void Visit(const NodeAtomicIAdd *node) override;
   void Visit(const NodeAtomicISub *node) override;
   void Visit(const NodeAtomicSMin *node) override;
   void Visit(const NodeAtomicUMin *node) override;
   void Visit(const NodeAtomicSMax *node) override;
   void Visit(const NodeAtomicUMax *node) override;
   void Visit(const NodeAtomicAnd *node) override;
   void Visit(const NodeAtomicOr *node) override;
   void Visit(const NodeAtomicXor *node) override;

   void Visit(const NodeSampledImage *node) override;

   void Visit(const NodeImageSampleImplicitLod *node) override;
   void Visit(const NodeImageSampleProjImplicitLod *node) override;
   void Visit(const NodeImageSampleDrefImplicitLod *node) override;
   void Visit(const NodeImageSampleProjDrefImplicitLod *node) override;

   void Visit(const NodeImageSampleExplicitLod *node) override;
   void Visit(const NodeImageSampleProjExplicitLod *node) override;
   void Visit(const NodeImageSampleDrefExplicitLod *node) override;
   void Visit(const NodeImageSampleProjDrefExplicitLod *node) override;
   void Visit(const NodeImageGather *node) override;
   void Visit(const NodeImageDrefGather *imageGather) override;

   void Visit(const NodeImageQuerySize *node) override;
   void Visit(const NodeImageQuerySizeLod *node) override;
   void Visit(const NodeImageQuerySamples *node) override;
   void Visit(const NodeImageQueryLevels *node) override;
   void Visit(const NodeImageQueryLod *node) override;

   void Visit(const NodeImage *node) override;
   void Visit(const NodeImageFetch *node) override;
   void Visit(const NodeImageRead *node) override;
   void Visit(const NodeImageWrite *node) override;

   void Visit(const NodeFOrdEqual *node) override;
   void Visit(const NodeFOrdNotEqual *node) override;
   void Visit(const NodeFOrdLessThan *node) override;
   void Visit(const NodeFOrdLessThanEqual *node) override;
   void Visit(const NodeFOrdGreaterThan *node) override;
   void Visit(const NodeFOrdGreaterThanEqual *node) override;

   void Visit(const NodeFUnordEqual *node) override;
   void Visit(const NodeFUnordNotEqual *node) override;
   void Visit(const NodeFUnordLessThan *node) override;
   void Visit(const NodeFUnordLessThanEqual *node) override;
   void Visit(const NodeFUnordGreaterThan *node) override;
   void Visit(const NodeFUnordGreaterThanEqual *node) override;

   void Visit(const NodeBranchConditional *node) override;
   void Visit(const NodeBranch *node) override;
   void Visit(const NodeSwitch *node) override;
   void Visit(const NodeKill *node) override;
   void Visit(const NodePhi *node) override;

   void Visit(const NodeIsNan *node) override;
   void Visit(const NodeIsInf *node) override;

   void Visit(const NodeDot *node) override;
   void Visit(const NodeStdLength *node) override;
   void Visit(const NodeStdDistance *node) override;
   void Visit(const NodeStdCross *node) override;
   void Visit(const NodeStdNormalize *node) override;
   void Visit(const NodeStdFaceForward *node) override;
   void Visit(const NodeStdReflect *node) override;
   void Visit(const NodeStdRefract *node) override;

   void Visit(const NodeTranspose *node) override;
   void Visit(const NodeVectorTimesScalar *node) override;
   void Visit(const NodeMatrixTimesScalar *node) override;
   void Visit(const NodeVectorTimesMatrix *node) override;
   void Visit(const NodeMatrixTimesVector *node) override;
   void Visit(const NodeMatrixTimesMatrix *node) override;
   void Visit(const NodeOuterProduct *node) override;
   void Visit(const NodeStdDeterminant *node) override;
   void Visit(const NodeStdMatrixInverse *node) override;

   void Visit(const NodeAny *node) override;
   void Visit(const NodeAll *node) override;

   void Visit(const NodeShiftLeftLogical *node) override;
   void Visit(const NodeShiftRightLogical *node) override;
   void Visit(const NodeShiftRightArithmetic *node) override;
   void Visit(const NodeBitReverse *node) override;

   void Visit(const NodeConvertUToF *node) override;
   void Visit(const NodeConvertSToF *node) override;

   void Visit(const NodeConvertFToU *node) override;
   void Visit(const NodeConvertFToS *node) override;

   void Visit(const NodeUConvert *node) override;
   void Visit(const NodeSConvert *node) override;
   void Visit(const NodeFConvert *node) override;

   void Visit(const NodeConstant *node) override;
   void Visit(const NodeConstantTrue *node) override;
   void Visit(const NodeConstantFalse *node) override;
   void Visit(const NodeConstantComposite *node) override;
   void Visit(const NodeSpecConstant *node) override;
   void Visit(const NodeSpecConstantTrue *node) override;
   void Visit(const NodeSpecConstantFalse *node) override;
   void Visit(const NodeSpecConstantComposite *node) override;
   void Visit(const NodeConstantNull *node) override;
   void Visit(const NodeSpecConstantOp *node) override;

   void Visit(const NodeUndef *node) override;
   void Visit(const NodeNop *node) override;

   void Visit(const NodeCompositeConstruct *node) override;
   void Visit(const NodeCompositeExtract *node) override;
   void Visit(const NodeCompositeInsert *node) override;
   void Visit(const NodeVectorExtractDynamic *node) override;
   void Visit(const NodeVectorInsertDynamic *node) override;
   void Visit(const NodeVectorShuffle *node) override;

   void Visit(const NodeSelect *node) override;

   void Visit(const NodeBitwiseOr *node) override;
   void Visit(const NodeBitwiseAnd *node) override;
   void Visit(const NodeBitwiseXor *node) override;
   void Visit(const NodeNot *node) override;

   void Visit(const NodeFNegate *node) override;
   void Visit(const NodeSNegate *node) override;
   void Visit(const NodeFAdd *node) override;
   void Visit(const NodeIAdd *node) override;

   void Visit(const NodeFSub *node) override;
   void Visit(const NodeISub *node) override;

   void Visit(const NodeFMul *node) override;
   void Visit(const NodeIMul *node) override;

   void Visit(const NodeFDiv *node) override;
   void Visit(const NodeUDiv *node) override;
   void Visit(const NodeSDiv *node) override;

   void Visit(const NodeFRem *node) override;
   void Visit(const NodeSRem *node) override;

   void Visit(const NodeFMod *node) override;
   void Visit(const NodeSMod *node) override;
   void Visit(const NodeUMod *node) override;

   void Visit(const NodeStdModf *node) override;
   void Visit(const NodeStdModfStruct *node) override;
   void Visit(const NodeStdFrexp *node) override;
   void Visit(const NodeStdFrexpStruct *node) override;
   void Visit(const NodeStdLdexp *node) override;

   void Visit(const NodeLogicalEqual *node) override;
   void Visit(const NodeLogicalNotEqual *node) override;
   void Visit(const NodeLogicalOr *node) override;
   void Visit(const NodeLogicalAnd *node) override;
   void Visit(const NodeLogicalNot *node) override;
   void Visit(const NodeBitcast *node) override;

   void Visit(const NodeIEqual *node) override;
   void Visit(const NodeINotEqual *node) override;

   void Visit(const NodeUGreaterThan *node) override;
   void Visit(const NodeUGreaterThanEqual *node) override;
   void Visit(const NodeULessThan *node) override;
   void Visit(const NodeULessThanEqual *node) override;

   void Visit(const NodeSGreaterThan *node) override;
   void Visit(const NodeSGreaterThanEqual *node) override;
   void Visit(const NodeSLessThan *node) override;
   void Visit(const NodeSLessThanEqual *node) override;

   void Visit(const NodeIAddCarry *node) override;
   void Visit(const NodeISubBorrow *node) override;
   void Visit(const NodeUMulExtended *node) override;
   void Visit(const NodeSMulExtended *node) override;
   void Visit(const NodeBitFieldSExtract *node) override;
   void Visit(const NodeBitFieldUExtract *node) override;
   void Visit(const NodeBitFieldInsert *node) override;
   void Visit(const NodeBitCount *node) override;

   void Visit(const NodeQuantizeToF16 *node) override;

   void Visit(const NodeStdSqrt *node) override;
   void Visit(const NodeStdInverseSqrt *node) override;

   void Visit(const NodeStdSin *node) override;
   void Visit(const NodeStdCos *node) override;
   void Visit(const NodeStdTan *node) override;

   void Visit(const NodeStdSinh *node) override;
   void Visit(const NodeStdCosh *node) override;
   void Visit(const NodeStdTanh *node) override;

   void Visit(const NodeStdAsin *node) override;
   void Visit(const NodeStdAcos *node) override;
   void Visit(const NodeStdAtan *node) override;
   void Visit(const NodeStdAtan2 *node) override;

   void Visit(const NodeStdAsinh *node) override;
   void Visit(const NodeStdAcosh *node) override;
   void Visit(const NodeStdAtanh *node) override;

   void Visit(const NodeStdDegrees *node) override;
   void Visit(const NodeStdRadians *node) override;

   void Visit(const NodeStdFAbs *node) override;
   void Visit(const NodeStdFMax *node) override;
   void Visit(const NodeStdFMin *node) override;
   void Visit(const NodeStdFClamp *node) override;
   void Visit(const NodeStdFMix *node) override;
   void Visit(const NodeStdIMix *node) override;

   void Visit(const NodeStdFSign *node) override;
   void Visit(const NodeStdSSign *node) override;
   void Visit(const NodeStdFma *node) override;

   void Visit(const NodeStdStep *node) override;
   void Visit(const NodeStdSmoothStep *node) override;
   void Visit(const NodeStdTrunc *node) override;
   void Visit(const NodeStdRound *node) override;
   void Visit(const NodeStdRoundEven *node) override;

   void Visit(const NodeStdSAbs *node) override;
   void Visit(const NodeStdSMax *node) override;
   void Visit(const NodeStdSMin *node) override;
   void Visit(const NodeStdSClamp *node) override;

   void Visit(const NodeStdUMax *node) override;
   void Visit(const NodeStdUMin *node) override;
   void Visit(const NodeStdUClamp *node) override;

   void Visit(const NodeStdNMax *node) override;
   void Visit(const NodeStdNMin *node) override;
   void Visit(const NodeStdNClamp *node) override;

   void Visit(const NodeStdLog2 *node) override;
   void Visit(const NodeStdLog *node) override;
   void Visit(const NodeStdPow *node) override;
   void Visit(const NodeStdExp *node) override;
   void Visit(const NodeStdExp2 *node) override;

   void Visit(const NodeStdCeil *node) override;
   void Visit(const NodeStdFloor *node) override;
   void Visit(const NodeStdFract *node) override;

   void Visit(const NodeStdFindUMsb *node) override;
   void Visit(const NodeStdFindSMsb *node) override;
   void Visit(const NodeStdFindILsb *node) override;

   void Visit(const NodeStdPackHalf2x16 *node) override;
   void Visit(const NodeStdPackSnorm4x8 *node) override;
   void Visit(const NodeStdPackUnorm4x8 *node) override;
   void Visit(const NodeStdPackSnorm2x16 *node) override;
   void Visit(const NodeStdPackUnorm2x16 *node) override;

   void Visit(const NodeStdUnpackHalf2x16 *node) override;
   void Visit(const NodeStdUnpackSnorm4x8 *node) override;
   void Visit(const NodeStdUnpackUnorm4x8 *node) override;
   void Visit(const NodeStdUnpackSnorm2x16 *node) override;
   void Visit(const NodeStdUnpackUnorm2x16 *node) override;

   void Visit(const NodeStdInterpolateAtCentroid *node) override;
   void Visit(const NodeStdInterpolateAtSample *node) override;
   void Visit(const NodeStdInterpolateAtOffset *node) override;

   void Visit(const NodeFunctionCall *node) override;
   void Visit(const NodeReturn       *node) override;
   void Visit(const NodeReturnValue  *node) override;
   void Visit(const NodeUnreachable  *node) override;

   void Visit(const NodeDPdx *node) override;
   void Visit(const NodeDPdxFine *node) override;
   void Visit(const NodeDPdxCoarse *node) override;
   void Visit(const NodeDPdy *node) override;
   void Visit(const NodeDPdyFine *node) override;
   void Visit(const NodeDPdyCoarse *node) override;
   void Visit(const NodeFwidth *node) override;
   void Visit(const NodeFwidthFine *node) override;
   void Visit(const NodeFwidthCoarse *node) override;

   void Visit(const NodeCopyObject *node) override;
   void Visit(const NodeArrayLength *node) override;

   // Structured control flow
   void Visit(const NodeLabel *node) override;
   void Visit(const NodeLoopMerge *node) override;

   // Compute ops
   void Visit(const NodeControlBarrier *node);
   void Visit(const NodeMemoryBarrier *node);

   // Compiler interface
   BasicBlockHandle GetEntryBlock()     const { return m_entryBlock;     }
   SymbolListHandle GetOutputSymbols()  const { return m_outputSymbols;  }
   SymbolListHandle GetInputSymbols()   const { return m_inputSymbols;   }
   SymbolListHandle GetUniformSymbols() const { return m_uniformSymbols; }
   SymbolListHandle GetSharedSymbols()  const { return m_sharedSymbols;  }
   const SymbolHandle GetWorkgroup()    const { return m_workgroup;      }

   // Equivalent to "new T(args)" but will use the arena allocator
   template <typename T, class... Types>
   T *New(Types&&... args) const
   {
      return spv::ModuleAllocator<T>(m_arenaAllocator).New(std::forward<Types>(args)...);
   }

   template <typename T>
   T *NewArray(uint32_t numElems) const
   {
      return spv::ModuleAllocator<T>(m_arenaAllocator).NewArray(numElems);
   }

   const spv::ModuleAllocator<uint32_t> &GetArenaAllocator() const
   {
      return m_arenaAllocator;
   }

   // Dataflows are stored in the m_dataflow vector indexed by the
   // resultId of the corresponding SPIRV instruction.
   // These methods add and retrieve entries

   // We don't want a version that return a non-const
   const DflowScalars &GetDataflow(const Node *at); // Gets from current block
   const DflowScalars &GetDataflow(BasicBlockHandle block, const Node *at);

   void AddDataflow(const Node *at, const DflowScalars &dataflow)
   {
      uint32_t ix = at->GetResultId();

      m_dataflow[ix]      = dataflow;
      m_dataflowBlock[ix] = m_functionStack->GetCurrentBlock();
   }

   SymbolTypeHandle GetSymbolType(const NodeType *node) const
   {
      return m_symbolTypes[node->GetTypeId()];
   }

   uint32_t GetNumScalars(const NodeType *node) const
   {
      return GetSymbolType(node).GetNumScalars();
   }

   SymbolHandle GetVariableSymbol(const NodeVariable *var)
   {
      if (var->GetStorageClass() == spv::StorageClass::Function)
         return m_functionStack->GetSymbol(var);

      return m_functionStack.GlobalContext()->GetSymbol(var);
   }

   void SetVariableSymbol(const NodeVariable *var, SymbolHandle symbol)
   {
      if (var->GetStorageClass() == spv::StorageClass::Function)
         m_functionStack->SetSymbol(var, symbol);
      else
         m_functionStack.GlobalContext()->SetSymbol(var, symbol);
   }

   // Load/store helpers
   void         StoreToSymbol(BasicBlockHandle block, SymbolHandle symbol, const DflowScalars &data);
   void         StoreToSymbol(BasicBlockHandle block, SymbolHandle symbol, const DflowScalars &data, uint32_t offset);
   DflowScalars LoadFromSymbol(BasicBlockHandle block, SymbolHandle symbol);

   bool         IsConstant(const spv::vector<const Node *> &indices);
   bool         HasStaticUse(const SymbolHandle sym) const;
   Map         *GetSymbolIdMap() const { return m_symbolIdMap.GetMap(); }

   // Create a symbol and initialize it to default values in the entry block
   SymbolHandle CreateInternal(const char *name, const NodeType *type);
   SymbolHandle GetParameterSymbol(const NodeFunctionParameter *node) const;

   static const NodeTypeImage *GetImageType(const Node *sampledImage);

   const ExecutionModes &GetExecutionModes() const { return m_executionModes; }
   void InitCompute();

   bool RobustBufferAccess() const { return m_robustBufferAccess; }
   bool ConstantInt(const Node *node, uint32_t *value);

   uint32_t     StructureOffset(const NodeTypeStruct *type, uint32_t index) const;
   DataflowType ComponentType(const Node *node);
   Dflow        WorkgroupAddress();

   const BasicBlockPool &GetBasicBlockPool() const { return m_basicBlockPool; }

private:
   DataflowType ResultDataflowType(const Node *node) const;

   void RenameBuiltinSymbol(const Node *var, SymbolHandle symbol, bool *seenPointSize);
   void SetupInterface(const NodeEntryPoint *entryPoint, ShaderFlavour flavour);

   // Helper functions
   DflowScalars CreateBuiltinInputDataflow(spv::BuiltIn builtin);
   DflowScalars CreateVariableDataflow(const NodeVariable *var, SymbolTypeHandle type);

   void  RecordInputSymbol(SymbolHandle symbol);
   void  RecordOutputSymbol(SymbolHandle symbol);
   void  RecordUniformSymbol(SymbolHandle symbol, int *ids);

   SymbolHandle AddSymbol(const NodeVariable *var);

   void         CallFunction(const NodeFunction *function, const NodeFunctionCall *functionCall);

   // Debug
   void         DebugPrint() const;

   void AddBoolConstant(const Node *node, bool value);
   void AddValueConstant(const Node *node, uint32_t value);
   void AddBoolSpecConstant(const Node *node, bool value);
   void AddComposite(const Node *node,const spv::vector<NodeIndex> &constituents);

   void AddSymbolType(const NodeType *node, SymbolTypeHandle type)
   {
      m_symbolTypes[node->GetTypeId()] = type;
   }

   void RecordExecutionModes(const NodeEntryPoint *entry);

   BasicBlockHandle BlockForLabel(const Node *label);
   void BuildAtomic(const Node *node, DataflowFlavour, const Node *pointer, const DflowScalars &value);
   template <class N> void BuildAtomic(const N *node, DataflowFlavour df);

   // Allocate ids to the symbol and record in the symbol id map
   void AssignIds(SymbolHandle symbol, uint32_t *current);

   void PatchConditionals() const;

private:
   const Module                             &m_module;
   ArenaAllocator<SysMemCmdBlock, void*>     m_arena;
   spv::ModuleAllocator<uint32_t>            m_arenaAllocator;
   ShaderFlavour                             m_flavour;
   bool                                      m_robustBufferAccess;
   bool                                      m_multiSampled;

   BasicBlockPool                m_basicBlockPool; // Record of all the basic blocks for clean-up

   FunctionStack                 m_functionStack;

   spv::vector<DflowScalars>     m_dataflow;       // Dataflow values for each SPIR-V result id (not all entries will be valid)
   spv::list<DflowScalars>       m_extraDataflow;  // Dataflow that has to retain its address but isn't in the m_dataflow array
   spv::vector<BasicBlockHandle> m_dataflowBlock;  // Records which block a particular dataflow belongs to
   spv::vector<SymbolTypeHandle> m_symbolTypes;    // Types indexed by the type's internal id.

   DescriptorMaps                m_descriptorMaps; // Tables of unique (set, binding, element) tuples for UBOs, SSBOs etc.

   SymbolListHandle              m_inputSymbols;
   SymbolListHandle              m_outputSymbols;
   SymbolListHandle              m_uniformSymbols;
   SymbolListHandle              m_sharedSymbols;

   spv::set<SymbolHandle, SymbolHandleCompare>
                                 m_usedSymbols;    // Contains all symbols that are statically used

   // Current linkable rows for each storage class
   uint32_t                      m_curInputRow   = 0;
   uint32_t                      m_curOutputRow  = 0;

   BasicBlockHandle              m_entryBlock;
   BasicBlockHandle              m_exitBlock;
   ExecutionModes                m_executionModes;

   // Some data used to patch up conditions in loops
   spv::vector<std::pair<BasicBlockHandle, BasicBlockHandle>>  m_conditionals;
   spv::vector<BasicBlockHandle>                               m_loopMerge;

   // Built-ins that require special handling
   SymbolHandle                  m_discard;
   SymbolHandle                  m_glPerVertex;

   // Symbol map used by compiler to map symbols to their ids
   PtrMap<Symbol, int>           m_symbolIdMap;

   // Specialization constant data
   const Specialization             &m_specializations;
   std::bitset<V3D_MAX_ATTR_ARRAYS>  m_attributeRBSwaps;  // All zero by default

   // Compute data TODO -- bundle in a class?
   SymbolHandle                  m_glWorkGroupID;
   SymbolHandle                  m_glLocalInvocationID;
   SymbolHandle                  m_glGlobalInvocationID;
   SymbolHandle                  m_glLocalInvocationIndex;
   SymbolHandle                  m_workgroup;
};

} // namespace bvk
