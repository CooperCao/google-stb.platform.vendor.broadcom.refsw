/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "GatherNodes.h"
#include "Module.h"

namespace bvk {

enum class NodeKind
{
   Instruction,
   Constant,
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
   case spv::Core::OpFunctionCall                   :
   case spv::Core::OpBranch                         :
   case spv::Core::OpBranchConditional              :
   case spv::Core::OpSwitch                         :
   case spv::Core::OpReturn                         :
   case spv::Core::OpReturnValue                    :
   case spv::Core::OpNop                            :
   case spv::Core::OpLoad                           :
   case spv::Core::OpStore                          :
   case spv::Core::OpCopyMemory                     :
   case spv::Core::OpArrayLength                    :
   case spv::Core::OpVectorExtractDynamic           :
   case spv::Core::OpVectorInsertDynamic            :
   case spv::Core::OpVectorShuffle                  :
   case spv::Core::OpCompositeConstruct             :
   case spv::Core::OpCompositeExtract               :
   case spv::Core::OpCompositeInsert                :
   case spv::Core::OpCopyObject                     :
   case spv::Core::OpTranspose                      :
   case spv::Core::OpSampledImage                   :
   case spv::Core::OpImageSampleImplicitLod         :
   case spv::Core::OpImageSampleExplicitLod         :
   case spv::Core::OpImageSampleDrefImplicitLod     :
   case spv::Core::OpImageSampleDrefExplicitLod     :
   case spv::Core::OpImageSampleProjImplicitLod     :
   case spv::Core::OpImageSampleProjExplicitLod     :
   case spv::Core::OpImageSampleProjDrefImplicitLod :
   case spv::Core::OpImageSampleProjDrefExplicitLod :
   case spv::Core::OpImageFetch                     :
   case spv::Core::OpImageGather                    :
   case spv::Core::OpImageDrefGather                :
   case spv::Core::OpImageRead                      :
   case spv::Core::OpImageWrite                     :
   case spv::Core::OpImage                          :
   case spv::Core::OpImageQuerySizeLod              :
   case spv::Core::OpImageQuerySize                 :
   case spv::Core::OpImageQueryLod                  :
   case spv::Core::OpImageQueryLevels               :
   case spv::Core::OpImageQuerySamples              :
   case spv::Core::OpConvertFToU                    :
   case spv::Core::OpConvertFToS                    :
   case spv::Core::OpConvertSToF                    :
   case spv::Core::OpConvertUToF                    :
   case spv::Core::OpUConvert                       :
   case spv::Core::OpSConvert                       :
   case spv::Core::OpFConvert                       :
   case spv::Core::OpQuantizeToF16                  :
   case spv::Core::OpBitcast                        :
   case spv::Core::OpSNegate                        :
   case spv::Core::OpFNegate                        :
   case spv::Core::OpIAdd                           :
   case spv::Core::OpFAdd                           :
   case spv::Core::OpISub                           :
   case spv::Core::OpFSub                           :
   case spv::Core::OpIMul                           :
   case spv::Core::OpFMul                           :
   case spv::Core::OpUDiv                           :
   case spv::Core::OpSDiv                           :
   case spv::Core::OpFDiv                           :
   case spv::Core::OpUMod                           :
   case spv::Core::OpSRem                           :
   case spv::Core::OpSMod                           :
   case spv::Core::OpFRem                           :
   case spv::Core::OpFMod                           :
   case spv::Core::OpVectorTimesScalar              :
   case spv::Core::OpMatrixTimesScalar              :
   case spv::Core::OpVectorTimesMatrix              :
   case spv::Core::OpMatrixTimesVector              :
   case spv::Core::OpMatrixTimesMatrix              :
   case spv::Core::OpOuterProduct                   :
   case spv::Core::OpDot                            :
   case spv::Core::OpIAddCarry                      :
   case spv::Core::OpISubBorrow                     :
   case spv::Core::OpUMulExtended                   :
   case spv::Core::OpSMulExtended                   :
   case spv::Core::OpAny                            :
   case spv::Core::OpAll                            :
   case spv::Core::OpIsNan                          :
   case spv::Core::OpIsInf                          :
   case spv::Core::OpLogicalEqual                   :
   case spv::Core::OpLogicalNotEqual                :
   case spv::Core::OpLogicalOr                      :
   case spv::Core::OpLogicalAnd                     :
   case spv::Core::OpLogicalNot                     :
   case spv::Core::OpSelect                         :
   case spv::Core::OpIEqual                         :
   case spv::Core::OpINotEqual                      :
   case spv::Core::OpUGreaterThan                   :
   case spv::Core::OpSGreaterThan                   :
   case spv::Core::OpUGreaterThanEqual              :
   case spv::Core::OpSGreaterThanEqual              :
   case spv::Core::OpULessThan                      :
   case spv::Core::OpSLessThan                      :
   case spv::Core::OpULessThanEqual                 :
   case spv::Core::OpSLessThanEqual                 :
   case spv::Core::OpFOrdEqual                      :
   case spv::Core::OpFUnordEqual                    :
   case spv::Core::OpFOrdNotEqual                   :
   case spv::Core::OpFUnordNotEqual                 :
   case spv::Core::OpFOrdLessThan                   :
   case spv::Core::OpFUnordLessThan                 :
   case spv::Core::OpFOrdGreaterThan                :
   case spv::Core::OpFUnordGreaterThan              :
   case spv::Core::OpFOrdLessThanEqual              :
   case spv::Core::OpFUnordLessThanEqual            :
   case spv::Core::OpFOrdGreaterThanEqual           :
   case spv::Core::OpFUnordGreaterThanEqual         :
   case spv::Core::OpShiftRightLogical              :
   case spv::Core::OpShiftRightArithmetic           :
   case spv::Core::OpShiftLeftLogical               :
   case spv::Core::OpBitwiseOr                      :
   case spv::Core::OpBitwiseXor                     :
   case spv::Core::OpBitwiseAnd                     :
   case spv::Core::OpNot                            :
   case spv::Core::OpBitFieldInsert                 :
   case spv::Core::OpBitFieldSExtract               :
   case spv::Core::OpBitFieldUExtract               :
   case spv::Core::OpBitReverse                     :
   case spv::Core::OpBitCount                       :
   case spv::Core::OpDPdx                           :
   case spv::Core::OpDPdy                           :
   case spv::Core::OpFwidth                         :
   case spv::Core::OpDPdxFine                       :
   case spv::Core::OpDPdyFine                       :
   case spv::Core::OpFwidthFine                     :
   case spv::Core::OpDPdxCoarse                     :
   case spv::Core::OpDPdyCoarse                     :
   case spv::Core::OpFwidthCoarse                   :
   case spv::Core::OpControlBarrier                 :
   case spv::Core::OpMemoryBarrier                  :
   case spv::Core::OpAtomicLoad                     :
   case spv::Core::OpAtomicStore                    :
   case spv::Core::OpAtomicExchange                 :
   case spv::Core::OpAtomicCompareExchange          :
   case spv::Core::OpAtomicIIncrement               :
   case spv::Core::OpAtomicIDecrement               :
   case spv::Core::OpAtomicIAdd                     :
   case spv::Core::OpAtomicISub                     :
   case spv::Core::OpAtomicSMin                     :
   case spv::Core::OpAtomicUMin                     :
   case spv::Core::OpAtomicSMax                     :
   case spv::Core::OpAtomicUMax                     :
   case spv::Core::OpAtomicAnd                      :
   case spv::Core::OpAtomicOr                       :
   case spv::Core::OpAtomicXor                      :
   case spv::Core::OpPhi                            :
   case spv::Core::OpKill                           :
   case spv::Core::OpUnreachable                    :
   case spv::Core::OpLoopMerge                      :
   case spv::Core::OpGroupNonUniformElect           :
      return NodeKind::Instruction;

   case spv::Core::OpConstant              :
   case spv::Core::OpConstantTrue          :
   case spv::Core::OpConstantFalse         :
   case spv::Core::OpConstantNull          :
   case spv::Core::OpConstantComposite     :
   case spv::Core::OpSpecConstantTrue      :
   case spv::Core::OpSpecConstantFalse     :
   case spv::Core::OpSpecConstant          :
   case spv::Core::OpSpecConstantComposite :
   case spv::Core::OpSpecConstantOp        :
   case spv::Core::OpUndef                 :
      return NodeKind::Constant;

   case spv::Core::OpSourceExtension     :
   case spv::Core::OpString              :
   case spv::Core::OpLine                :
   case spv::Core::OpNoLine              :
   case spv::Core::OpSourceContinued     :
   case spv::Core::OpMemberName          :
   case spv::Core::OpSelectionMerge      :
   case spv::Core::OpFunctionEnd         :
   case spv::Core::OpAccessChain         :
   case spv::Core::OpInBoundsAccessChain :
   case spv::Core::OpImageTexelPointer   :
   case spv::Core::OpModuleProcessed     :
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
      case NodeKind::Instruction : module.AddInstruction(node);  break;
      case NodeKind::Constant    : module.AddConstant(node);     break;
      case NodeKind::Special     : node->Accept(visitor);        break;
      case NodeKind::Ignored     : /* do nothing */              break;
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
void GatherNodes::Visit(const NodeName *node)                {                                          }
void GatherNodes::Visit(const NodeExecutionMode *node)       { m_module.AddExecutionMode(node);         }
void GatherNodes::Visit(const NodeVariable *node)            { m_module.AddVariable(node);              }
void GatherNodes::Visit(const NodeLabel *node)               { m_module.AddLabel(node);                 }
void GatherNodes::Visit(const NodeFunction *node)            { m_module.AddFunction(node);              }
void GatherNodes::Visit(const NodeFunctionParameter *node)   { m_module.AddParameter(node);             }
void GatherNodes::Visit(const NodeMemberDecorate *node)      { m_module.AddMemberDecoration(node);      }
void GatherNodes::Visit(const NodeGroupMemberDecorate *node) { m_module.AddGroupMemberDecoration(node); }

//////////////////////////////////////////////////////////////////////////////
// Types are added to the module's type table
//////////////////////////////////////////////////////////////////////////////
void GatherNodes::Visit(const NodeTypeVoid *node)            { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypeBool *node)            { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypeInt *node)             { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypeFloat *node)           { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypeVector *node)          { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypeMatrix *node)          { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypeImage *node)           { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypeSampler *node)         { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypeSampledImage *node)    { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypeArray *node)           { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypeStruct *node)          { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypePointer *node)         { m_module.AddType(node); }
void GatherNodes::Visit(const NodeTypeFunction *node)        { m_module.AddType(node); }

} // namespace bvk
