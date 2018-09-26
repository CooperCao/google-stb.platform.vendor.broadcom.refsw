/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "GatherNodes.h"
#include "Module.h"

namespace bvk {

enum class NodeKind
{
   Instruction,
   Global,
   Type,
   Special,
   Ignored
};

// Work out the kind of an instruction.
// We could auto-generate this information
static NodeKind Kind(const Node *node)
{
   if (node->GetInstructionSet() == spv::InstructionSet::GLSL)
      return NodeKind::Instruction;

   switch (node->GetOpCode())
   {
   using namespace spv;

   case Core::OpFunctionCall                   :
   case Core::OpBranch                         :
   case Core::OpBranchConditional              :
   case Core::OpSwitch                         :
   case Core::OpReturn                         :
   case Core::OpReturnValue                    :
   case Core::OpNop                            :
   case Core::OpLoad                           :
   case Core::OpStore                          :
   case Core::OpCopyMemory                     :
   case Core::OpArrayLength                    :
   case Core::OpVectorExtractDynamic           :
   case Core::OpVectorInsertDynamic            :
   case Core::OpVectorShuffle                  :
   case Core::OpCompositeConstruct             :
   case Core::OpCompositeExtract               :
   case Core::OpCompositeInsert                :
   case Core::OpCopyObject                     :
   case Core::OpTranspose                      :
   case Core::OpSampledImage                   :
   case Core::OpImageSampleImplicitLod         :
   case Core::OpImageSampleExplicitLod         :
   case Core::OpImageSampleDrefImplicitLod     :
   case Core::OpImageSampleDrefExplicitLod     :
   case Core::OpImageSampleProjImplicitLod     :
   case Core::OpImageSampleProjExplicitLod     :
   case Core::OpImageSampleProjDrefImplicitLod :
   case Core::OpImageSampleProjDrefExplicitLod :
   case Core::OpImageFetch                     :
   case Core::OpImageGather                    :
   case Core::OpImageDrefGather                :
   case Core::OpImageRead                      :
   case Core::OpImageWrite                     :
   case Core::OpImage                          :
   case Core::OpImageQuerySizeLod              :
   case Core::OpImageQuerySize                 :
   case Core::OpImageQueryLod                  :
   case Core::OpImageQueryLevels               :
   case Core::OpImageQuerySamples              :
   case Core::OpConvertFToU                    :
   case Core::OpConvertFToS                    :
   case Core::OpConvertSToF                    :
   case Core::OpConvertUToF                    :
   case Core::OpUConvert                       :
   case Core::OpSConvert                       :
   case Core::OpFConvert                       :
   case Core::OpQuantizeToF16                  :
   case Core::OpBitcast                        :
   case Core::OpSNegate                        :
   case Core::OpFNegate                        :
   case Core::OpIAdd                           :
   case Core::OpFAdd                           :
   case Core::OpISub                           :
   case Core::OpFSub                           :
   case Core::OpIMul                           :
   case Core::OpFMul                           :
   case Core::OpUDiv                           :
   case Core::OpSDiv                           :
   case Core::OpFDiv                           :
   case Core::OpUMod                           :
   case Core::OpSRem                           :
   case Core::OpSMod                           :
   case Core::OpFRem                           :
   case Core::OpFMod                           :
   case Core::OpVectorTimesScalar              :
   case Core::OpMatrixTimesScalar              :
   case Core::OpVectorTimesMatrix              :
   case Core::OpMatrixTimesVector              :
   case Core::OpMatrixTimesMatrix              :
   case Core::OpOuterProduct                   :
   case Core::OpDot                            :
   case Core::OpIAddCarry                      :
   case Core::OpISubBorrow                     :
   case Core::OpUMulExtended                   :
   case Core::OpSMulExtended                   :
   case Core::OpAny                            :
   case Core::OpAll                            :
   case Core::OpIsNan                          :
   case Core::OpIsInf                          :
   case Core::OpLogicalEqual                   :
   case Core::OpLogicalNotEqual                :
   case Core::OpLogicalOr                      :
   case Core::OpLogicalAnd                     :
   case Core::OpLogicalNot                     :
   case Core::OpSelect                         :
   case Core::OpIEqual                         :
   case Core::OpINotEqual                      :
   case Core::OpUGreaterThan                   :
   case Core::OpSGreaterThan                   :
   case Core::OpUGreaterThanEqual              :
   case Core::OpSGreaterThanEqual              :
   case Core::OpULessThan                      :
   case Core::OpSLessThan                      :
   case Core::OpULessThanEqual                 :
   case Core::OpSLessThanEqual                 :
   case Core::OpFOrdEqual                      :
   case Core::OpFUnordEqual                    :
   case Core::OpFOrdNotEqual                   :
   case Core::OpFUnordNotEqual                 :
   case Core::OpFOrdLessThan                   :
   case Core::OpFUnordLessThan                 :
   case Core::OpFOrdGreaterThan                :
   case Core::OpFUnordGreaterThan              :
   case Core::OpFOrdLessThanEqual              :
   case Core::OpFUnordLessThanEqual            :
   case Core::OpFOrdGreaterThanEqual           :
   case Core::OpFUnordGreaterThanEqual         :
   case Core::OpShiftRightLogical              :
   case Core::OpShiftRightArithmetic           :
   case Core::OpShiftLeftLogical               :
   case Core::OpBitwiseOr                      :
   case Core::OpBitwiseXor                     :
   case Core::OpBitwiseAnd                     :
   case Core::OpNot                            :
   case Core::OpBitFieldInsert                 :
   case Core::OpBitFieldSExtract               :
   case Core::OpBitFieldUExtract               :
   case Core::OpBitReverse                     :
   case Core::OpBitCount                       :
   case Core::OpDPdx                           :
   case Core::OpDPdy                           :
   case Core::OpFwidth                         :
   case Core::OpDPdxFine                       :
   case Core::OpDPdyFine                       :
   case Core::OpFwidthFine                     :
   case Core::OpDPdxCoarse                     :
   case Core::OpDPdyCoarse                     :
   case Core::OpFwidthCoarse                   :
   case Core::OpControlBarrier                 :
   case Core::OpMemoryBarrier                  :
   case Core::OpAtomicLoad                     :
   case Core::OpAtomicStore                    :
   case Core::OpAtomicExchange                 :
   case Core::OpAtomicCompareExchange          :
   case Core::OpAtomicIIncrement               :
   case Core::OpAtomicIDecrement               :
   case Core::OpAtomicIAdd                     :
   case Core::OpAtomicISub                     :
   case Core::OpAtomicSMin                     :
   case Core::OpAtomicUMin                     :
   case Core::OpAtomicSMax                     :
   case Core::OpAtomicUMax                     :
   case Core::OpAtomicAnd                      :
   case Core::OpAtomicOr                       :
   case Core::OpAtomicXor                      :
   case Core::OpPhi                            :
   case Core::OpKill                           :
   case Core::OpUnreachable                    :
   case Core::OpLoopMerge                      :
   case Core::OpSelectionMerge                 :
   case Core::OpGroupNonUniformElect           :
      return NodeKind::Instruction;

   case Core::OpTypeVoid         :
   case Core::OpTypeBool         :
   case Core::OpTypeInt          :
   case Core::OpTypeFloat        :
   case Core::OpTypeVector       :
   case Core::OpTypeMatrix       :
   case Core::OpTypeImage        :
   case Core::OpTypeSampler      :
   case Core::OpTypeSampledImage :
   case Core::OpTypeArray        :
   case Core::OpTypeStruct       :
   case Core::OpTypePointer      :
   case Core::OpTypeFunction     :
      return NodeKind::Type;

   case Core::OpConstant              :
   case Core::OpConstantTrue          :
   case Core::OpConstantFalse         :
   case Core::OpConstantNull          :
   case Core::OpConstantComposite     :
   case Core::OpSpecConstantTrue      :
   case Core::OpSpecConstantFalse     :
   case Core::OpSpecConstant          :
   case Core::OpSpecConstantComposite :
   case Core::OpSpecConstantOp        :
   case Core::OpUndef                 :
   case Core::OpExecutionMode         :
   case Core::OpExecutionModeId       :
      return NodeKind::Global;

   case Core::OpSourceExtension     :
   case Core::OpString              :
   case Core::OpLine                :
   case Core::OpNoLine              :
   case Core::OpSourceContinued     :
   case Core::OpMemberName          :
   case Core::OpFunctionEnd         :
   case Core::OpAccessChain         :
   case Core::OpInBoundsAccessChain :
   case Core::OpImageTexelPointer   :
   case Core::OpModuleProcessed     :
   case Core::OpName                :
      return NodeKind::Ignored;

   default:
      return NodeKind::Special;
   }
}

void GatherNodes::Gather(Module &module)
{
   GatherNodes visitor(module);

   for (const Node *node : module.GetNodes())
   {
      switch (Kind(node))
      {
      case NodeKind::Instruction : module.AddInstruction(node);                  break;
      case NodeKind::Type        : module.AddType(node->As<const NodeType *>()); break;
      case NodeKind::Global      : module.AddGlobal(node);                       break;
      case NodeKind::Special     : node->Accept(visitor);                        break;
      case NodeKind::Ignored     : /* do nothing */                              break;
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// Nodes with special behaviours
///////////////////////////////////////////////////////////////////////////////
void GatherNodes::Visit(const NodeCapability *node)          { m_module.AddCapability(node);            }
void GatherNodes::Visit(const NodeExtension *node)           { m_module.AddExtension(node);             }
void GatherNodes::Visit(const NodeExtInstImport *node)       { m_module.AddExtInstImport(node);         }
void GatherNodes::Visit(const NodeMemoryModel *node)         { m_module.SetMemoryModel(node);           }
void GatherNodes::Visit(const NodeEntryPoint *node)          { m_module.AddEntryPoint(node);            }
void GatherNodes::Visit(const NodeSource *node)              { m_module.SetSource(node);                }
void GatherNodes::Visit(const NodeDecorate *node)            { m_module.AddDecoration(node);            }
void GatherNodes::Visit(const NodeDecorationGroup *node)     { m_module.AddDecorationGroup(node);       }
void GatherNodes::Visit(const NodeGroupDecorate *node)       { m_module.AddGroupDecoration(node);       }
void GatherNodes::Visit(const NodeVariable *node)            { m_module.AddVariable(node);              }
void GatherNodes::Visit(const NodeLabel *node)               { m_module.AddLabel(node);                 }
void GatherNodes::Visit(const NodeFunction *node)            { m_module.AddFunction(node);              }
void GatherNodes::Visit(const NodeFunctionParameter *node)   { m_module.AddParameter(node);             }
void GatherNodes::Visit(const NodeMemberDecorate *node)      { m_module.AddMemberDecoration(node);      }
void GatherNodes::Visit(const NodeGroupMemberDecorate *node) { m_module.AddGroupMemberDecoration(node); }

} // namespace bvk
