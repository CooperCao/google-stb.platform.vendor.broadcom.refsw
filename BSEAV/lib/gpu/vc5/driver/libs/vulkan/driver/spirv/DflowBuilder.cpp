/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#ifdef WIN32
#pragma warning(disable : 4800)  // From autogen'd code
#endif

#include "DflowBuilder.h"
#include "DflowLibrary.h"
#include "DflowMatrix.h"
#include "Nodes.h"
#include "TypeBuilder.h"
#include "Compiler.h"
#include "Module.h"
#include "PrimitiveTypes.h"
#include "Composite.h"
#include "Pointer.h"
#include "Specialization.h"
#include "TextureLookup.h"
#include "Options.h"

#include "DescriptorInfo.h"

#include "libs/platform/v3d_scheduler.h"
#include "glsl_symbols.h"
#include "libs/util/log/log.h"
#include "glsl_dataflow.h"
#include "glsl_fastmem.h"

LOG_DEFAULT_CAT("bvk::comp::DflowBuilder");

// Get rid of Windows namespace pollution
#ifdef WIN32
#undef GetObject
#endif

namespace bvk {

// Some static helper functions
static void FindMatrixInfo(const Node *node, uint32_t *cols, uint32_t *rows)
{
   auto matrixType = node->GetResultType()->As<const NodeTypeMatrix *>();
   auto columnType = matrixType->GetColumnType()->As<const NodeTypeVector *>();

   *cols = matrixType->GetColumnCount();
   *rows = columnType->GetComponentCount();
}

static uint32_t VectorSize(const Node *node)
{
   auto vecType = node->GetResultType()->As<const NodeTypeVector *>();

   return vecType->GetComponentCount();
}

///////////////////////////////////////////////////////////////////////
// ExecutionModes helper class
///////////////////////////////////////////////////////////////////////
void ExecutionModes::Record(const NodeExecutionMode *node)
{
   const spv::vector<uint32_t> &literals = node->GetLiterals();

   switch (node->GetMode())
   {
   case spv::ExecutionMode::LocalSize :
      assert(literals.size() == 3);
      SetWorkgroupSize(literals[0], literals[1], literals[2]);
      break;
   case spv::ExecutionMode::DepthReplacing:
      m_depthReplacing = true;
      break;
   case spv::ExecutionMode::EarlyFragmentTests:
      m_earlyFragmentTests = true;
      break;
   case spv::ExecutionMode::OriginUpperLeft:
      break;
   default:
      // TODO -- handle other modes we're interested in.
      log_warn("Ignoring execution mode %u", static_cast<uint32_t>(node->GetMode()));
      break;
   }
}

///////////////////////////////////////////////////////////////////////
// DflowBuilder proper
///////////////////////////////////////////////////////////////////////
DflowBuilder::DflowBuilder(DescriptorTables *tables,
                           const Module &module, const Specialization &specialization,
                           bool robustBufferAccess, bool multiSampled) :
   m_module(module),
   m_arena(module.GetCallbacks()),
   m_arenaAllocator(&m_arena),
   m_robustBufferAccess(robustBufferAccess),
   m_multiSampled(multiSampled),
   m_basicBlockPool(m_arenaAllocator),
   m_functionStack(*this),
   m_dataflow(module.IdBound(), DflowScalars(*this), m_arenaAllocator),
   m_extraDataflow(m_arenaAllocator),
   m_dataflowBlock(module.IdBound(), BasicBlockHandle(), m_arenaAllocator),
   m_symbolTypes(module.GetNumTypes(), SymbolTypeHandle(), m_arenaAllocator),
   m_descriptorMaps(m_arenaAllocator, tables),
   m_usedSymbols(SymbolHandleCompare(), m_arenaAllocator),
   m_conditionals(m_arenaAllocator),
   m_loopMerge(m_arenaAllocator),
   m_specializations(specialization)
{
}

DflowBuilder::~DflowBuilder()
{
   m_arenaAllocator.LogMemoryUsage("=== DflowBuilder ===");
}

DataflowType DflowBuilder::ResultDataflowType(const Node *node) const
{
   const NodeType *type = node->GetResultType();

   return GetSymbolType(type).ToDataflowType(0);
}

const DflowScalars &DflowBuilder::GetDataflow(BasicBlockHandle sourceBlock, const Node *node)
{
   uint32_t          resultId      = node->GetResultId();
   BasicBlockHandle  dataflowBlock = m_dataflowBlock[resultId];

   // Entry block holds constants (or variables which are loaded)
   if (dataflowBlock.IsSameBlock(m_entryBlock))
      return m_dataflow[resultId];

   // If this is local to the source block, then we can just use the dataflow directly
   if (dataflowBlock.IsSameBlock(sourceBlock))
      return m_dataflow[resultId];

   // If not we will have to create a symbol for it in its home
   // block and then load that in the sourceBlock
   // If the symbol already exists then we can re-use it.
   SymbolHandle   symbol = m_functionStack->GetDataflowSymbol(node);

   if (!symbol)
   {
      symbol = CreateInternal("$$tmp", node->GetResultType());
      m_functionStack->SetDataflowSymbol(node, symbol);

      StoreToSymbol(dataflowBlock, symbol, m_dataflow[resultId]);
   }

   // This gives the dflow a fixed location in memory so we can
   // pass it around by reference/pointer
   m_extraDataflow.push_back(LoadFromSymbol(sourceBlock, symbol));

   return m_extraDataflow.back();
}

const DflowScalars &DflowBuilder::GetDataflow(const Node *at)
{
   return GetDataflow(m_functionStack->GetCurrentBlock(), at);
}

///////////////////////////////////////////////////////////////////////////////
// Types
///////////////////////////////////////////////////////////////////////////////

void DflowBuilder::Visit(const NodeTypeVoid *node)
{
   AddSymbolType(node, SymbolTypeHandle::Void());
}

void DflowBuilder::Visit(const NodeTypeBool *node)
{
   AddSymbolType(node, SymbolTypeHandle::Bool());
}

void DflowBuilder::Visit(const NodeTypeFloat *node)
{
   if (node->GetWidth() != 32)
      log_warn("%u-bit NodeTypeFloat encountered. Treating as 32-bit float.", node->GetWidth());

   AddSymbolType(node, SymbolTypeHandle::Float());
}

void DflowBuilder::Visit(const NodeTypeInt *node)
{
   if (node->GetWidth() != 32)
      log_warn("%u-bit NodeTypeInt encountered. Treating as 32-bit int.", node->GetWidth());

   AddSymbolType(node, node->GetSignedness() ? SymbolTypeHandle::Int() : SymbolTypeHandle::UInt());
}

void DflowBuilder::Visit(const NodeTypeVector *node)
{
   uint32_t compCount = node->GetComponentCount();
   auto     compType  = node->GetComponentType()->As<const NodeType *>();

   AddSymbolType(node, SymbolTypeHandle::Vector(GetSymbolType(compType), compCount));
}

void DflowBuilder::Visit(const NodeTypeFunction *node)
{
   // The underlying IR won't need to know about these
}

void DflowBuilder::Visit(const NodeTypePointer *node)
{
   auto  targetType = node->GetType()->As<const NodeType *>();

   AddSymbolType(node, SymbolTypeHandle::Pointer(m_module, GetSymbolType(targetType)));
}


void DflowBuilder::Visit(const NodeTypeImage *node)
{
   spv::Dim dim     = node->GetDim();
   uint32_t arrayed = node->GetArrayed();
   uint32_t ms      = node->GetMS();

   SymbolTypeHandle sampledType = GetSymbolType(node->GetSampledType()->As<const NodeType *>());

   switch (node->GetSampled())
   {
   case 1 : AddSymbolType(node, SymbolTypeHandle::SampledImage(sampledType, dim, arrayed, ms));
            break;
   case 2 : AddSymbolType(node, SymbolTypeHandle::Image(sampledType, dim, arrayed));
            break;
   default: unreachable();
   }
}

void DflowBuilder::Visit(const NodeTypeSampledImage *node)
{
   auto imageNode = node->GetImageType()->As<const NodeTypeImage *>();

   spv::Dim dim     = imageNode->GetDim();
   uint32_t arrayed = imageNode->GetArrayed();
   uint32_t ms      = imageNode->GetMS();

   SymbolTypeHandle sampledType = GetSymbolType(imageNode->GetSampledType()->As<const NodeType *>());

   AddSymbolType(node, SymbolTypeHandle::CombinedSampledImage(sampledType, dim, arrayed, ms));
}

void DflowBuilder::Visit(const NodeTypeStruct *node)
{
   MemberIter  mi(*this, node->GetMemberstype());

   AddSymbolType(node, SymbolTypeHandle::Struct(m_module, mi));
}

void DflowBuilder::Visit(const NodeTypeMatrix *node)
{
   auto     columnType = node->GetColumnType()->As<const NodeTypeVector *>();
   uint32_t cols       = node->GetColumnCount();
   uint32_t rows       = columnType->GetComponentCount();

   AddSymbolType(node, SymbolTypeHandle::Matrix(cols, rows));
}

void DflowBuilder::Visit(const NodeTypeSampler *node)
{
   AddSymbolType(node, SymbolTypeHandle::Sampler());
}

void DflowBuilder::Visit(const NodeTypeArray *node)
{
   auto     elementType = node->GetElementType()->As<const NodeType *>();
   uint32_t length = RequireConstantInt(node->GetLength());

   AddSymbolType(node, SymbolTypeHandle::Array(m_module, GetSymbolType(elementType), length));
}

void DflowBuilder::Visit(const NodeTypeRuntimeArray *node)
{
   auto elementType = node->GetElementType()->As<const NodeType *>();

   AddSymbolType(node, SymbolTypeHandle::Array(m_module, GetSymbolType(elementType), 0));
}

uint32_t DflowBuilder::StructureOffset(const NodeTypeStruct *type, uint32_t index) const
{
   uint32_t  offset = 0;

   for (uint32_t i = 0; i < index; ++i)
   {
      SymbolTypeHandle symbolType = GetSymbolType(type->GetMemberstype()[i]->As<const NodeType *>());
      offset += symbolType.GetNumScalars();
   }

   return offset;
}

///////////////////////////////////////////////////////////////////////////////
// Misc
///////////////////////////////////////////////////////////////////////////////
void DflowBuilder::Visit(const NodeNop *node)
{
}

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////
void DflowBuilder::Visit(const NodeUndef *node)
{
   const NodeType   *type       = node->GetResultType();
   SymbolTypeHandle  symbolType = GetSymbolType(type);

   DflowScalars result(*this, GetNumScalars(type));
   AddDataflow(node, DflowScalars::Default(*this, symbolType));
}

void DflowBuilder::Visit(const NodeConstantNull *node)
{
   const NodeType   *type       = node->GetResultType();
   SymbolTypeHandle  symbolType = GetSymbolType(type);

   DflowScalars result(*this, GetNumScalars(type));
   AddDataflow(node, DflowScalars::Default(*this, symbolType));
}

void DflowBuilder::AddBoolConstant(const Node *node, bool value)
{
   AddDataflow(node, DflowScalars::ConstantBool(*this, value));
}

void DflowBuilder::AddValueConstant(const Node *node, uint32_t value)
{
   const NodeType *nodeType = node->GetResultType();

   AddDataflow(node, DflowScalars::ConstantValue(*this, GetSymbolType(nodeType), value));
}

// Constant
void DflowBuilder::Visit(const NodeConstant *constant)
{
   AddValueConstant(constant, constant->GetValue());
}

// True
void DflowBuilder::Visit(const NodeConstantTrue *node)
{
   AddBoolConstant(node, true);
}

// False
void DflowBuilder::Visit(const NodeConstantFalse *node)
{
   AddBoolConstant(node, false);
}

void DflowBuilder::AddComposite(const Node *node, const spv::vector<NodeIndex> &constituents)
{
   const NodeType *type = node->GetResultType();

   DflowScalars    result(*this, GetNumScalars(type));

   uint32_t offset = 0;
   for (const NodeIndex &ni : constituents)
   {
      const DflowScalars &slice = GetDataflow(ni);
      result.SetSlice(offset, slice);
      offset += slice.Size();
   }

   // Is this the workgroup size?
   spv::BuiltIn builtIn;
   if (m_module.GetBuiltinDecoration(&builtIn, node))
   {
      if (builtIn == spv::BuiltIn::WorkgroupSize)
      {
         // Overwrite any previously set workgroup size
         assert(result.Size() == 3);
         m_executionModes.SetWorkgroupSize(result[0].GetConstantInt(),
                                           result[1].GetConstantInt(),
                                           result[2].GetConstantInt());
      }
   }

   AddDataflow(node, result);
}

// Composite
void DflowBuilder::Visit(const NodeConstantComposite *node)
{
   AddComposite(node, node->GetConstituents());
}

///////////////////////////////////////////////////////////////////////////////
// Specialization constants
///////////////////////////////////////////////////////////////////////////////
void DflowBuilder::AddBoolSpecConstant(const Node *node, bool def)
{
   uint32_t id;
   bool     value = def;

   if (m_module.GetLiteralDecoration(&id, spv::Decoration::SpecId, node))
      m_specializations.GetBool(&value, id);

   AddBoolConstant(node, value);
}

// Constant
void DflowBuilder::Visit(const NodeSpecConstant *constant)
{
   uint32_t id;
   uint32_t value = constant->GetValue();

   if (m_module.GetLiteralDecoration(&id, spv::Decoration::SpecId, constant))
      m_specializations.GetValue(&value, id);

   AddValueConstant(constant, value);
}

// True
void DflowBuilder::Visit(const NodeSpecConstantTrue *node)
{
   AddBoolSpecConstant(node, true);
}

// False
void DflowBuilder::Visit(const NodeSpecConstantFalse *node)
{
   AddBoolSpecConstant(node, false);
}

// Composite
void DflowBuilder::Visit(const NodeSpecConstantComposite *node)
{
   AddComposite(node, node->GetConstituents());
}

static bool SpecOpIsSpecial(spv::Core op)
{
   bool result = false;

   switch (op)
   {
   case spv::Core::OpVectorShuffle    :
   case spv::Core::OpCompositeExtract :
   case spv::Core::OpCompositeInsert  :
      result = true;
      break;

   default:
      break;
   }

   return result;
}

void DflowBuilder::Visit(const NodeSpecConstantOp *node)
{
   DflowScalars   result;
   auto          &operands  = node->GetOperands();
   spv::Core      operation = node->GetOperation();

   if (!SpecOpIsSpecial(operation))
   {
      const DflowScalars *arg0 = nullptr;
      const DflowScalars *arg1 = nullptr;
      const DflowScalars *arg2 = nullptr;

      switch (operands.size())
      {
      case 3: arg2 = &GetDataflow(operands[2]); /* Fallthrough */
      case 2: arg1 = &GetDataflow(operands[1]); /* Fallthrough */
      case 1: arg0 = &GetDataflow(operands[0]);
      }

      // Constructing dataflow will call glsl_dataflow_simplify implicitly.
      // If everything is constant (as it should be), then the dataflow should
      // be simplified to a constant.
      switch (operation)
      {
      case spv::Core::OpSConvert             : result = *arg0; break;
      case spv::Core::OpFConvert             : result = *arg0; break;

      case spv::Core::OpIAdd                 : result = *arg0 + *arg1; break;
      case spv::Core::OpISub                 : result = *arg0 - *arg1; break;
      case spv::Core::OpIMul                 : result = *arg0 * *arg1; break;

      case spv::Core::OpUDiv                 : result = *arg0 / *arg1;      break;
      case spv::Core::OpUMod                 : result = umod(*arg0, *arg1); break;

      case spv::Core::OpSNegate              : result = -*arg0;                              break;
      case spv::Core::OpSMod                 : result = smod(*arg0, *arg1);                  break;
      case spv::Core::OpSDiv                 : result = arg0->Signed() / arg1->Signed();     break;
      case spv::Core::OpSRem                 : result = rem(arg0->Signed(), arg1->Signed()); break;

      case spv::Core::OpNot                  : result = ~*arg0;            break;
      case spv::Core::OpShiftRightLogical    : result = lsr(*arg0, *arg1); break;
      case spv::Core::OpShiftRightArithmetic : result = asr(*arg0, *arg1); break;
      case spv::Core::OpShiftLeftLogical     : result = *arg0 << *arg1;    break;
      case spv::Core::OpBitwiseOr            : result = *arg0 |  *arg1;    break;
      case spv::Core::OpBitwiseXor           : result = *arg0 ^  *arg1;    break;
      case spv::Core::OpBitwiseAnd           : result = *arg0 &  *arg1;    break;

      case spv::Core::OpLogicalNot           : result = !*arg0;            break;
      case spv::Core::OpLogicalOr            : result = *arg0 || *arg1;    break;
      case spv::Core::OpLogicalAnd           : result = *arg0 && *arg1;    break;
      case spv::Core::OpLogicalEqual         : result = *arg0 == *arg1;    break;
      case spv::Core::OpLogicalNotEqual      : result = *arg0 != *arg1;    break;

      case spv::Core::OpIEqual               : result = intEqual(*arg0, *arg1);    break;
      case spv::Core::OpINotEqual            : result = intNotEqual(*arg0, *arg1); break;

      case spv::Core::OpULessThan            : result = arg0->Unsigned() <  arg1->Unsigned(); break;
      case spv::Core::OpSLessThan            : result = arg0->Signed()   <  arg1->Signed();   break;
      case spv::Core::OpUGreaterThan         : result = arg0->Unsigned() >  arg1->Unsigned(); break;
      case spv::Core::OpSGreaterThan         : result = arg0->Signed()   >  arg1->Signed();   break;
      case spv::Core::OpULessThanEqual       : result = arg0->Unsigned() <= arg1->Unsigned(); break;
      case spv::Core::OpSLessThanEqual       : result = arg0->Signed()   <= arg1->Signed();   break;
      case spv::Core::OpUGreaterThanEqual    : result = arg0->Unsigned() >= arg1->Unsigned(); break;
      case spv::Core::OpSGreaterThanEqual    : result = arg0->Signed()   >= arg1->Signed();   break;

      case spv::Core::OpSelect               : result = cond(*arg0, *arg1, *arg2); break;
      case spv::Core::OpQuantizeToF16        : result = quantizeToF16(*arg0);      break;

      default                                : assert(0); break;
      }

      result = result.As(ResultDataflowType(node));
   }
   else
   {
      switch (operation)
      {
      case spv::Core::OpVectorShuffle:
      {
         const DflowScalars &vec1 = GetDataflow(node->GetVector1());
         const DflowScalars &vec2 = GetDataflow(node->GetVector2());

         result = shuffle(vec1, vec2, node->GetIndices());
         break;
      }

      case spv::Core::OpCompositeExtract:
      {
         const Node     *compositeNode = node->GetComposite();
         const NodeType *compositeType = compositeNode->GetResultType();

         uint32_t     offset     = ScalarOffsetStatic::Calculate(*this, compositeType, node->GetIndices());
         DflowScalars scalars    = GetDataflow(compositeNode);
         uint32_t     numScalars = GetNumScalars(node->GetResultType());

         result = scalars.Slice(offset, numScalars);
         break;
      }

      case spv::Core::OpCompositeInsert:
      {
         const Node     *compositeNode = node->GetComposite();
         const NodeType *compositeType = compositeNode->GetResultType();

         uint32_t       offset  = ScalarOffsetStatic::Calculate(*this, compositeType, node->GetIndices());
         DflowScalars   slice   = GetDataflow(node->GetObject());

         result = GetDataflow(compositeNode);
         result.SetSlice(offset, slice);

         break;
      }

      default:
         assert(0);
         break;
      }
   }

   // Currently the glsl simplifier gets invoked when dataflow is constructed,
   // so at this point, the result should have been simplified to a constant.
   // If this changes, we will need to call the glsl_dataflow_simplify at this point.
   AddDataflow(node, result);
}

///////////////////////////////////////////////////////////////////////////////

void DflowBuilder::Visit(const NodeVectorShuffle *vectorShuffle)
{
   const DflowScalars &vec1 = GetDataflow(vectorShuffle->GetVector1());
   const DflowScalars &vec2 = GetDataflow(vectorShuffle->GetVector2());

   AddDataflow(vectorShuffle, shuffle(vec1, vec2, vectorShuffle->GetComponents()));
}

///////////////////////////////////////////////////////////////////////////////
// Texturing
///////////////////////////////////////////////////////////////////////////////

void DflowBuilder::Visit(const NodeSampledImage *node)
{
   const DflowScalars &image   = GetDataflow(node->GetImage());
   const DflowScalars &sampler = GetDataflow(node->GetSampler());

   DflowScalars   result(*this, { image[0], sampler[0] });

   AddDataflow(node, result);
}

static const ImageOperands *ImplicitImageOperands(const Optional<ImageOperands> &opt)
{
   // ImageOperands are optional for implicit lod sampling, so check and extract
   return opt.IsValid() ? &opt.Get() : nullptr;
}

void DflowBuilder::Visit(const NodeImageSampleImplicitLod *imageSample)
{
   TextureLookup tl(*this, ImplicitImageOperands(imageSample->GetImageOperands()),
                           imageSample->GetCoordinate(), imageSample->GetSampledImage());

   AddDataflow(imageSample, tl.ImplicitLodLookup());
}

void DflowBuilder::Visit(const NodeImageSampleProjImplicitLod *imageSample)
{
   TextureLookup tl(*this, ImplicitImageOperands(imageSample->GetImageOperands()),
                           imageSample->GetCoordinate(), imageSample->GetSampledImage(),
                           /*dref=*/nullptr, /*project=*/true);

   AddDataflow(imageSample, tl.ImplicitLodLookup());
}

void DflowBuilder::Visit(const NodeImageSampleDrefImplicitLod *imageSample)
{
   TextureLookup tl(*this, ImplicitImageOperands(imageSample->GetImageOperands()),
                           imageSample->GetCoordinate(), imageSample->GetSampledImage(),
                           imageSample->GetDref());

   AddDataflow(imageSample, tl.ImplicitLodLookup());
}

void DflowBuilder::Visit(const NodeImageSampleProjDrefImplicitLod *imageSample)
{
   TextureLookup tl(*this, ImplicitImageOperands(imageSample->GetImageOperands()),
                           imageSample->GetCoordinate(), imageSample->GetSampledImage(),
                           imageSample->GetDref(), /*project=*/true);

   AddDataflow(imageSample, tl.ImplicitLodLookup());
}

void DflowBuilder::Visit(const NodeImageSampleExplicitLod *imageSample)
{
   TextureLookup tl(*this, &imageSample->GetImageOperands(),
                            imageSample->GetCoordinate(), imageSample->GetSampledImage());

   AddDataflow(imageSample, tl.ExplicitLodLookup());
}

void DflowBuilder::Visit(const NodeImageSampleProjExplicitLod *imageSample)
{
   TextureLookup tl(*this, &imageSample->GetImageOperands(),
                            imageSample->GetCoordinate(), imageSample->GetSampledImage(),
                            /*dref=*/nullptr, /*project=*/true);

   AddDataflow(imageSample, tl.ExplicitLodLookup());
}

void DflowBuilder::Visit(const NodeImageSampleDrefExplicitLod *imageSample)
{
   TextureLookup tl(*this, &imageSample->GetImageOperands(),
                            imageSample->GetCoordinate(), imageSample->GetSampledImage(),
                            imageSample->GetDref());

   AddDataflow(imageSample, tl.ExplicitLodLookup());
}

void DflowBuilder::Visit(const NodeImageSampleProjDrefExplicitLod *imageSample)
{
   TextureLookup tl(*this, &imageSample->GetImageOperands(),
                            imageSample->GetCoordinate(), imageSample->GetSampledImage(),
                            imageSample->GetDref(), /*project=*/true);

   AddDataflow(imageSample, tl.ExplicitLodLookup());
}

void DflowBuilder::Visit(const NodeImageGather *imageGather)
{
   const ImageOperands  *imageOperands = nullptr;
   uint32_t component = RequireConstantInt(imageGather->GetComponent());

   if (imageGather->GetImageOperands().IsValid())
      imageOperands = &imageGather->GetImageOperands().Get();

   TextureLookup tl(*this, imageOperands, imageGather->GetCoordinate(),
                           imageGather->GetSampledImage());

   AddDataflow(imageGather, tl.GatherLookup(component));
}

void DflowBuilder::Visit(const NodeImageDrefGather *imageGather)
{
   const ImageOperands  *imageOperands = nullptr;

   if (imageGather->GetImageOperands().IsValid())
      imageOperands = &imageGather->GetImageOperands().Get();

   TextureLookup tl(*this, imageOperands, imageGather->GetCoordinate(),
                           imageGather->GetSampledImage(), imageGather->GetDref());

   AddDataflow(imageGather, tl.GatherLookup(0));
}

void DflowBuilder::Visit(const NodeImageQuerySize *node)
{
   const DflowScalars &img        = GetDataflow(node->GetImage());
   uint32_t            numScalars = GetNumScalars(node->GetResultType());
   DflowScalars        tSize      = DflowScalars::TextureSize(*this, numScalars, img[0]);

   AddDataflow(node, tSize);
}

void DflowBuilder::Visit(const NodeImageQuerySizeLod *node)
{
   const DflowScalars  &img        = GetDataflow(node->GetImage());
   const DflowScalars  &lod        = GetDataflow(node->GetLevelofDetail());
   const NodeTypeImage *imageType  = GetImageType(node->GetImage());
   uint32_t             numScalars = GetNumScalars(node->GetResultType());
   DflowScalars         tSize      = DflowScalars::TextureSize(*this, numScalars, img[0]);

   for (uint32_t i = 0; i < numScalars; i++)
   {
      if (!imageType->GetArrayed() || i < numScalars - 1)
         tSize[i] = tSize[i] >> lod[0];
   }

   tSize = max(tSize, DflowScalars::ConstantInt(*this, 1));

   AddDataflow(node, tSize);
}

void DflowBuilder::Visit(const NodeImageQuerySamples *node)
{
   AddDataflow(node, DflowScalars::ConstantInt(*this, 4));
}

void DflowBuilder::Visit(const NodeImageQueryLevels *node)
{
   const DflowScalars &img    = GetDataflow(node->GetImage());
   DflowScalars        levels = DflowScalars::TextureNumLevels(*this, img[0]);

   AddDataflow(node, levels);
}

void DflowBuilder::Visit(const NodeImageQueryLod *node)
{
#if V3D_VER_AT_LEAST(4,2,13,0)
   TextureLookup tl(*this, nullptr, node->GetCoordinate(), node->GetSampledImage(),
                           nullptr, false, false, true);

   AddDataflow(node, tl.LodQuery());
#else
   const DflowScalars  &sampler   = GetDataflow(node->GetSampledImage());
   const NodeTypeImage *imageType = GetImageType(node->GetSampledImage());

   DflowScalars p      = GetDataflow(node->GetCoordinate());
   DflowScalars tSize  = DflowScalars::TextureSize(*this, p.Size(), sampler[0]);
   bool         isCube = imageType->GetDim() == spv::Dim::Cube;
   DflowScalars lod    = calculateLod(tSize, p, dFdx(p), dFdy(p), isCube);

   DflowScalars minLevel = DflowScalars::TextureNumLevels(*this, sampler[0]) -
                           DflowScalars::ConstantInt(*this, 1);

   // Clamp the rounded lod to the actual mip-levels available
   DflowScalars mipLvl = clamp(round(lod), 0.0f, itof(minLevel));

   DflowScalars result(*this, { mipLvl[0], lod[0] });
   AddDataflow(node, result);
#endif
}

const NodeTypeImage *DflowBuilder::GetImageType(const Node *sampledImage)
{
   // Find the imageType. TODO : Visitor?
   auto sampledImageType = sampledImage->GetResultType()->TryAs<const NodeTypeSampledImage *>();

   if (sampledImageType != nullptr)
      return sampledImageType->GetImageType()->As<const NodeTypeImage *>();
   else
      return sampledImage->GetResultType()->As<const NodeTypeImage *>();
}

void DflowBuilder::Visit(const NodeImage *node)
{
   AddDataflow(node, GetDataflow(node->GetSampledImage()));
}

void DflowBuilder::Visit(const NodeImageFetch *imageNode)
{
   TextureLookup tl(*this, ImplicitImageOperands(imageNode->GetImageOperands()),
                           imageNode->GetCoordinate(), imageNode->GetImage(), /*dref=*/nullptr,
                           /*project=*/false, /*fetch=*/true);

   AddDataflow(imageNode, tl.ImageFetch());
}

void DflowBuilder::Visit(const NodeImageRead *imageNode)
{
   TextureLookup tl(*this, ImplicitImageOperands(imageNode->GetImageOperands()),
                           imageNode->GetCoordinate(), imageNode->GetImage(), /*dref=*/nullptr,
                           /*project=*/false, /*fetch=*/true);

   AddDataflow(imageNode, tl.ImageFetch());
}

void DflowBuilder::Visit(const NodeImageWrite *imageNode)
{
   TextureLookup tl(*this, ImplicitImageOperands(imageNode->GetImageOperands()),
                           imageNode->GetCoordinate(), imageNode->GetImage(), /*dref=*/nullptr,
                           /*project=*/false, /*fetch=*/true);

   const DflowScalars &data = GetDataflow(imageNode->GetTexel());

   tl.ImageWrite(data, m_functionStack->GetCurrentBlock());
}

///////////////////////////////////////////////////////////////////////////////
// Control Flow
///////////////////////////////////////////////////////////////////////////////

void DflowBuilder::Visit(const NodeBranchConditional *node)
{
   BasicBlockHandle current = m_functionStack->GetCurrentBlock();

   BasicBlockHandle trueBlock  = m_functionStack->BlockForLabel(node->GetTrueLabel());
   BasicBlockHandle falseBlock = m_functionStack->BlockForLabel(node->GetFalseLabel());

   if (trueBlock.IsSameBlock(current) || falseBlock.IsSameBlock(current))
   {
      BasicBlockHandle newBlock(*this);

      current->SetFallthroughTarget(newBlock);
      m_functionStack->SetCurrentBlock(newBlock);
      current = newBlock;
   }

   current->SetControl(GetDataflow(node->GetCondition())[0],
                       trueBlock, falseBlock);

   if (m_loopMerge.size() > 0)
      m_conditionals.emplace_back(current, m_loopMerge.back());
}

static bool ReachesUncond(BasicBlock *from, BasicBlock *to)
{
   while (from != nullptr)
   {
      // Found it?
      if (from == to)
         return true;

      // Not unconditional?
      if (from->fallthrough_target != nullptr && from->branch_target != nullptr)
         return false;

      from = from->fallthrough_target;
   }

   return false;
}

void DflowBuilder::PatchConditionals() const
{
   for (auto conditional : m_conditionals)
   {
      BasicBlock *cond  = conditional.first->GetBlock();
      BasicBlock *merge = conditional.second->GetBlock();

      // Is this a jump out of the loop?
      if (ReachesUncond(cond->branch_target, merge))
      {
         std::swap(cond->branch_target, cond->fallthrough_target);
         cond->branch_cond = glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, cond->branch_cond);
      }
   }
}

void DflowBuilder::Visit(const NodeBranch *node)
{
   BasicBlockHandle current = m_functionStack->GetCurrentBlock();

   current->SetFallthroughTarget(m_functionStack->BlockForLabel(node->GetTargetLabel()));
}

void DflowBuilder::Visit(const NodeSwitch *node)
{
   auto &cases = node->GetTarget();

   const Node     *selector  = node->GetSelector();

   // Possibly there are no cases
   BasicBlockHandle currentBlock = m_functionStack->GetCurrentBlock();
   SymbolHandle     switchSym    = CreateInternal("$$switch", selector->GetResultType());

   // Set the selector variable
   StoreToSymbol(currentBlock, switchSym, GetDataflow(selector));

   for (uint32_t i = 0; i < cases.size(); ++i)
   {
      int32_t          literal     = static_cast<int32_t>(cases[i].first);
      BasicBlockHandle targetBlock = m_functionStack->BlockForLabel(cases[i].second);

      // Load the selector and compare with literal
      DflowScalars   select = LoadFromSymbol(currentBlock, switchSym);
      DflowScalars   test   = select == literal;

      BasicBlockHandle nextBlock(*this);

      currentBlock->SetControl(test[0], targetBlock, nextBlock);
      currentBlock = nextBlock;
      m_functionStack->SetCurrentBlock(currentBlock);
   }

   // Add the default
   currentBlock->SetFallthroughTarget(m_functionStack->BlockForLabel(node->GetDefault()));
}

void DflowBuilder::Visit(const NodeKill *node)
{
   BasicBlockHandle current = m_functionStack->GetCurrentBlock();

   StoreToSymbol(current, m_discard, DflowScalars::ConstantBool(*this, true));

   current->SetFallthroughTarget(m_functionStack->GetReturnBlock());
}

void DflowBuilder::Visit(const NodePhi *node)
{
   SymbolTypeHandle  symbolType = GetSymbolType(node->GetResultType());
   assert(symbolType.GetNumScalars() != 0);

   SymbolHandle phiSym   = SymbolHandle::Internal(m_module, "$$phi", symbolType);
   DflowScalars defaults = DflowScalars::Default(*this, symbolType);

   StoreToSymbol(m_entryBlock, phiSym, defaults);

   DflowScalars result = LoadFromSymbol(m_functionStack->GetCurrentBlock(), phiSym);

   m_functionStack->AddPhi(node, phiSym);

   AddDataflow(node, result);
}

///////////////////////////////////////////////////////////////////////////////
// Comparison
///////////////////////////////////////////////////////////////////////////////

void DflowBuilder::Visit(const NodeFOrdEqual *node)
{
   const DflowScalars &lhs = GetDataflow(node->GetOperand1());
   const DflowScalars &rhs = GetDataflow(node->GetOperand2());

   // Ordered AND Equal : this matches the IEEE == operation.
   // i.e. any NaNs yield 0
   AddDataflow(node, lhs == rhs);
}

void DflowBuilder::Visit(const NodeFOrdNotEqual *node)
{
   const DflowScalars &a = GetDataflow(node->GetOperand1());
   const DflowScalars &b = GetDataflow(node->GetOperand2());

   // Ordered AND !Equal : yields 0 if either argument is NaN.
   // This doesn't match IEEE != which would yield a 1 in these cases.
   // We will use (a < b || a > b) which gives the correct value in all cases.
   AddDataflow(node, (a < b) || (a > b));
}

/* KERNEL
void DflowBuilder::Visit(const NodeLessOrGreater *node)
{
   const DflowScalars &a = GetDataflow(node->GetX());
   const DflowScalars &b = GetDataflow(node->GetY());
   AddDataflow(node, (a < b) || (a > b));
}
*/

void DflowBuilder::Visit(const NodeFOrdLessThan *node)
{
   const DflowScalars &lhs = GetDataflow(node->GetOperand1());
   const DflowScalars &rhs = GetDataflow(node->GetOperand2());

   // Ordered AND LessThan : this matches the IEEE < operation. i.e. 0 for any NaNs
   AddDataflow(node, lhs < rhs);
}

void DflowBuilder::Visit(const NodeFOrdLessThanEqual *node)
{
   const DflowScalars &lhs = GetDataflow(node->GetOperand1());
   const DflowScalars &rhs = GetDataflow(node->GetOperand2());

   // Ordered AND LessThanOrEqual : this matches the IEEE <= operation. i.e. 0 for any NaNs
   AddDataflow(node, lhs <= rhs);
}

void DflowBuilder::Visit(const NodeFOrdGreaterThan *node)
{
   const DflowScalars &lhs = GetDataflow(node->GetOperand1());
   const DflowScalars &rhs = GetDataflow(node->GetOperand2());

   // Ordered AND GreaterThan : this matches the IEEE > operation. i.e. 0 for any NaNs
   AddDataflow(node, lhs > rhs);
}

void DflowBuilder::Visit(const NodeFOrdGreaterThanEqual *node)
{
   const DflowScalars &lhs = GetDataflow(node->GetOperand1());
   const DflowScalars &rhs = GetDataflow(node->GetOperand2());

   // Ordered AND GreaterThanOrEqual : this matches the IEEE >= operation. i.e. 0 for any NaNs
   AddDataflow(node, lhs >= rhs);
}

void DflowBuilder::Visit(const NodeFUnordEqual *node)
{
   const DflowScalars &a = GetDataflow(node->GetOperand1());
   const DflowScalars &b = GetDataflow(node->GetOperand2());

   // Unordered OR Equal : yields 1 if either argument is NaN.
   // This doesn't match IEEE == which would give a 0 in these cases.
   // We will use !(a < b || a > b) which gives the correct value in all cases.
   AddDataflow(node, !(a < b || a > b));
}

void DflowBuilder::Visit(const NodeFUnordNotEqual *node)
{
   const DflowScalars &lhs = GetDataflow(node->GetOperand1());
   const DflowScalars &rhs = GetDataflow(node->GetOperand2());

   // Unordered OR NotEqual : this matches the negation of the IEEE == operation.
   // i.e. any NaNs yield 1
   AddDataflow(node, !(lhs == rhs)); // Do not replace with lhs != rhs - it's not the same
}

void DflowBuilder::Visit(const NodeFUnordLessThan *node)
{
   const DflowScalars &lhs = GetDataflow(node->GetOperand1());
   const DflowScalars &rhs = GetDataflow(node->GetOperand2());

   // Unordered OR LessThan : this matches the negation of the IEEE >= operation.
   // i.e. any NaNs yield 1
   AddDataflow(node, !(lhs >= rhs)); // Do not replace with lhs < rhs - it's not the same
}

void DflowBuilder::Visit(const NodeFUnordLessThanEqual *node)
{
   const DflowScalars &lhs = GetDataflow(node->GetOperand1());
   const DflowScalars &rhs = GetDataflow(node->GetOperand2());

   // Unordered OR LessThan : this matches the negation of the IEEE > operation.
   // i.e. any NaNs yield 1
   AddDataflow(node, !(lhs > rhs)); // Do not replace with lhs <= rhs - it's not the same
}

void DflowBuilder::Visit(const NodeFUnordGreaterThan *node)
{
   const DflowScalars &lhs = GetDataflow(node->GetOperand1());
   const DflowScalars &rhs = GetDataflow(node->GetOperand2());

   // Unordered OR LessThan : this matches the negation of the IEEE <= operation.
   // i.e. any NaNs yield 1
   AddDataflow(node, !(lhs <= rhs)); // Do not replace with lhs > rhs - it's not the same
}

void DflowBuilder::Visit(const NodeFUnordGreaterThanEqual *node)
{
   const DflowScalars &lhs = GetDataflow(node->GetOperand1());
   const DflowScalars &rhs = GetDataflow(node->GetOperand2());

   // Unordered OR LessThan : this matches the negation of the IEEE < operation.
   // i.e. any NaNs yield 1
   AddDataflow(node, !(lhs < rhs)); // Do not replace with lhs >= rhs - it's not the same
}

/* KERNEL
void DflowBuilder::Visit(const NodeOrdered *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());

#ifdef NAN_CORRECT_OPTIMISATION
   // x==x && y==y
   AddDataflow(node, (x == x) && (y == y));
#else
   AddDataflow(node, !(isnan(x) || isnan(y)));
#endif
}

void DflowBuilder::Visit(const NodeUnordered *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());

#ifdef NAN_CORRECT_OPTIMISATION
   // x or y are NaN (x != x || y != y)
   AddDataflow(node, (x != x) || (y != y));
#else
   AddDataflow(node, isnan(x) || isnan(y));
#endif
}
*/

void DflowBuilder::Visit(const NodeIsNan *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, isnan(x));
}

void DflowBuilder::Visit(const NodeIsInf *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, isinf(x));
}

/* KERNEL
void DflowBuilder::Visit(const NodeIsFinite *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());

   auto xint = reinterpu(fabs(x));
   auto c    = DflowScalars::ConstantUInt(*this, 0x7F800000);
   AddDataflow(node, xint != c);
}

void DflowBuilder::Visit(const NodeIsNormal *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());

   // Normal means exponent is non-zero
   auto xint = reinterpu(x);
   auto c    = DflowScalars::ConstantUInt(*this, 0x7F800000);
   auto zero = DflowScalars::ConstantUInt(*this, 0);

   AddDataflow(node, (xint & c) != zero);
}
*/

void DflowBuilder::Visit(const NodeBitReverse *node)
{
   const DflowScalars &x = GetDataflow(node->GetBase());

   bool isSigned = IsSigned::Get(node->GetResultType());

   // Assumes 32-bit width
   auto c1        = DflowScalars::ConstantUInt(*this, 1);
   auto c2        = DflowScalars::ConstantUInt(*this, 2);
   auto c4        = DflowScalars::ConstantUInt(*this, 4);
   auto c8        = DflowScalars::ConstantUInt(*this, 8);
   auto c16       = DflowScalars::ConstantUInt(*this, 16);
   auto x55555555 = DflowScalars::ConstantUInt(*this, 0x55555555u);
   auto x33333333 = DflowScalars::ConstantUInt(*this, 0x33333333u);
   auto x0F0F0F0F = DflowScalars::ConstantUInt(*this, 0x0F0F0F0Fu);
   auto x00FF00FF = DflowScalars::ConstantUInt(*this, 0x00FF00FFu);

   DflowScalars v = isSigned ? reinterpu(x) : x;

   v = ((v >> c1) & x55555555) | ((v & x55555555) << c1);
   v = ((v >> c2) & x33333333) | ((v & x33333333) << c2);
   v = ((v >> c4) & x0F0F0F0F) | ((v & x0F0F0F0F) << c4);
   v = ((v >> c8) & x00FF00FF) | ((v & x00FF00FF) << c8);

   v = (v >> c16) | (v << c16);

   if (isSigned)
      v = reinterpi(v);

   AddDataflow(node, v);
}

void DflowBuilder::Visit(const NodeVectorTimesScalar *node)
{
   const DflowScalars &v = GetDataflow(node->GetVector());
   const DflowScalars &s = GetDataflow(node->GetScalar());
   assert(s.Size() == 1);

   DflowScalars result(*this, v.Size());

   for (uint32_t i = 0; i < v.Size(); ++i)
      result[i] = v[i] * s[0];

   AddDataflow(node, result);
}

void DflowBuilder::Visit(const NodeDot *node)
{
   const DflowScalars &v1 = GetDataflow(node->GetVector1());
   const DflowScalars &v2 = GetDataflow(node->GetVector2());
   AddDataflow(node, dot(v1, v2));
}

void DflowBuilder::Visit(const NodeShiftLeftLogical *node)
{
   const DflowScalars &b = GetDataflow(node->GetBase());
   const DflowScalars &s = GetDataflow(node->GetShift());
   AddDataflow(node, b << s);
}

void DflowBuilder::Visit(const NodeShiftRightLogical *node)
{
   const DflowScalars &b = GetDataflow(node->GetBase()).Unsigned();
   const DflowScalars &s = GetDataflow(node->GetShift()).Unsigned();

   DflowScalars   result = lsr(b, s);

   AddDataflow(node, result.As(IsSigned::Get(node->GetResultType()) ? DF_INT : DF_UINT));
}

void DflowBuilder::Visit(const NodeShiftRightArithmetic *node)
{
   const DflowScalars &b = GetDataflow(node->GetBase()).Unsigned();
   const DflowScalars &s = GetDataflow(node->GetShift()).Unsigned();

   DflowScalars   result = asr(b, s);

   AddDataflow(node, result.As(IsSigned::Get(node->GetResultType()) ? DF_INT : DF_UINT));
}

void DflowBuilder::Visit(const NodeCompositeConstruct *node)
{
   uint32_t size = GetNumScalars(node->GetResultType());

   DflowScalars   result = DflowScalars(*this, size);
   uint32_t       offset = 0;

   // This is more general than the SPIR-V spec allows
   for (const NodeIndex constituent : node->GetConstituents())
   {
      const DflowScalars &scalars = GetDataflow(constituent);

      for (uint32_t i = 0; i < scalars.Size(); ++i)
         result[offset + i] = scalars[i];

      offset += scalars.Size();
   }

   AddDataflow(node, result);
}

void DflowBuilder::Visit(const NodeCompositeExtract *node)
{
   const Node     *compositeNode = node->GetComposite();
   const NodeType *compositeType = compositeNode->GetResultType();

   uint32_t     offset     = ScalarOffsetStatic::Calculate(*this, compositeType, node->GetIndices());
   DflowScalars scalars    = GetDataflow(compositeNode);
   uint32_t     numScalars = GetNumScalars(node->GetResultType());

   AddDataflow(node, scalars.Slice(offset, numScalars));
}

void DflowBuilder::Visit(const NodeCompositeInsert *node)
{
   const Node     *compositeNode = node->GetComposite();
   const NodeType *compositeType = compositeNode->GetResultType();

   uint32_t       offset  = ScalarOffsetStatic::Calculate(*this, compositeType, node->GetIndices());
   DflowScalars   scalars = GetDataflow(compositeNode);
   DflowScalars   slice   = GetDataflow(node->GetObject());

   scalars.SetSlice(offset, slice);

   AddDataflow(node, scalars);
}

void DflowBuilder::Visit(const NodeVectorExtractDynamic *node)
{
   const Node *index  = node->GetIndex();
   const Node *vector = node->GetVector();

   DflowScalars scalars = ExtractDynamicScalars::Calculate(*this, GetDataflow(vector),
                                                           vector->GetResultType(), index);

   AddDataflow(node, scalars);
}

void DflowBuilder::Visit(const NodeVectorInsertDynamic *node)
{
   const DflowScalars   &index     = GetDataflow(node->GetIndex());
   const DflowScalars   &component = GetDataflow(node->GetComponent());
   const DflowScalars   &vector    = GetDataflow(node->GetVector());

   DflowScalars scalars = InsertDynamicScalars::Calculate(*this, vector, node->GetResultType(),
                                                          index, component);

   AddDataflow(node, scalars);
}

void DflowBuilder::Visit(const NodeSelect *node)
{
   const DflowScalars   &lhs = GetDataflow(node->GetObject1());
   const DflowScalars   &rhs = GetDataflow(node->GetObject2());
   const DflowScalars   &cnd = GetDataflow(node->GetCondition());

   // The cnd, lhs, and rhs should be the same size according to the spec,
   // but deqp somehow generates code with cnd as a bool and lhs and rhs
   // as vectors.  This will work as currently coded.  glslang stand-alone
   // does not do this so it is odd as to how deqp is doing it.
   AddDataflow(node, cond(cnd, lhs, rhs));
}

///////////////////////////////////////////////////////////////////////////////
// Operators
///////////////////////////////////////////////////////////////////////////////

void DflowBuilder::Visit(const NodeBitwiseOr *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());
   AddDataflow(node, op1 | op2);
}

void DflowBuilder::Visit(const NodeBitwiseAnd *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());
   AddDataflow(node, op1 & op2);
}

void DflowBuilder::Visit(const NodeBitwiseXor *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1 ^ op2);
}

void DflowBuilder::Visit(const NodeNot *node)
{
   const DflowScalars &op = GetDataflow(node->GetOperand());

   AddDataflow(node, ~op);
}

void DflowBuilder::Visit(const NodeFNegate *node)
{
   const DflowScalars &op = GetDataflow(node->GetOperand());

   AddDataflow(node, -op);
}

void DflowBuilder::Visit(const NodeSNegate *node)
{
   DataflowType type = ResultDataflowType(node);

   const DflowScalars &op     = GetDataflow(node->GetOperand());
   const DflowScalars  result = -op.Signed();

   AddDataflow(node, result.As(type));
}

void DflowBuilder::Visit(const NodeFAdd *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1 + op2);
}

void DflowBuilder::Visit(const NodeIAdd *node)
{
   DataflowType type = ResultDataflowType(node);

   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1.As(type) + op2.As(type));
}

void DflowBuilder::Visit(const NodeFSub *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1 - op2);
}

void DflowBuilder::Visit(const NodeISub *node)
{
   DataflowType type = ResultDataflowType(node);

   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1.As(type) - op2.As(type));
}

void DflowBuilder::Visit(const NodeFMul *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1 * op2);
}

void DflowBuilder::Visit(const NodeIMul *node)
{
   DataflowType type = ResultDataflowType(node);

   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1.As(type) * op2.As(type));
}

void DflowBuilder::Visit(const NodeFDiv *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1 / op2);
}

void DflowBuilder::Visit(const NodeUDiv *node)
{
   // SPIRV spec states that arguments and result must be unsigned
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1 / op2);
}

void DflowBuilder::Visit(const NodeSDiv *node)
{
   DataflowType type = ResultDataflowType(node);

   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   DflowScalars result = op1.Signed() / op2.Signed();

   AddDataflow(node, result.As(type));
}

void DflowBuilder::Visit(const NodeFRem *node)
{
   const DflowScalars &x = GetDataflow(node->GetOperand1());
   const DflowScalars &y = GetDataflow(node->GetOperand2());

   AddDataflow(node, fsign(x) * mod(fabs(x), fabs(y)));
}

void DflowBuilder::Visit(const NodeSRem *node)
{
   DataflowType type = ResultDataflowType(node);

   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   DflowScalars result = rem(op1.Signed(), op2.Signed());

   AddDataflow(node, result.As(type));
}

void DflowBuilder::Visit(const NodeFMod *node)
{
   const DflowScalars &x = GetDataflow(node->GetOperand1());
   const DflowScalars &y = GetDataflow(node->GetOperand2());

   AddDataflow(node, mod(x, y));
}

void DflowBuilder::Visit(const NodeSMod *node)
{
   DataflowType type = ResultDataflowType(node);

   // Sign must come from the divisor
   const DflowScalars &x = GetDataflow(node->GetOperand1());
   const DflowScalars &y = GetDataflow(node->GetOperand2());

   AddDataflow(node, smod(x, y).As(type));
}

void DflowBuilder::Visit(const NodeUMod *node)
{
   // SPIRV spec states that arguments and result must be unsigned
   const DflowScalars &x = GetDataflow(node->GetOperand1());
   const DflowScalars &y = GetDataflow(node->GetOperand2());

   AddDataflow(node, umod(x, y));
}

void DflowBuilder::Visit(const NodeStdModf *node)
{
   const DflowScalars &x      = GetDataflow(node->GetX());
   const DflowScalars  result = modf(x);

   uint32_t half = result.Size() / 2;

   StoreScalars::Store(*this, m_functionStack->GetCurrentBlock(),
                       node->GetI(), result.Slice(0, half));

   AddDataflow(node, result.Slice(half, half));
}

void DflowBuilder::Visit(const NodeStdModfStruct *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());

   AddDataflow(node, modf(x));
}

void DflowBuilder::Visit(const NodeStdFrexp *node)
{
   const DflowScalars &x      = GetDataflow(node->GetX());
   const DflowScalars  result = frexp(x);

   uint32_t half = result.Size() / 2;

   StoreScalars::Store(*this, m_functionStack->GetCurrentBlock(),
                        node->GetExp(), result.Slice(half, half));

   AddDataflow(node, result.Slice(0, half));
}

void DflowBuilder::Visit(const NodeStdFrexpStruct *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());

   AddDataflow(node, frexp(x));
}

void DflowBuilder::Visit(const NodeStdLdexp *node)
{
   const DflowScalars &x   = GetDataflow(node->GetX());
   const DflowScalars &exp = GetDataflow(node->GetExp());

   AddDataflow(node, ldexp(x, exp));
}

void DflowBuilder::Visit(const NodeLogicalEqual *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1 == op2);
}

void DflowBuilder::Visit(const NodeLogicalNotEqual *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1 != op2);
}

void DflowBuilder::Visit(const NodeLogicalOr *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1 || op2);
}

void DflowBuilder::Visit(const NodeLogicalAnd *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1 && op2);
}

void DflowBuilder::Visit(const NodeLogicalNot *node)
{
   const DflowScalars &op = GetDataflow(node->GetOperand());

   AddDataflow(node, !op);
}

void DflowBuilder::Visit(const NodeBitcast *node)
{
   const Node *operand = node->GetOperand();

   // Not supported yet
   assert(!node->IsPointer());
   assert(!operand->IsPointer());

   const DflowScalars &op = GetDataflow(operand);

   AddDataflow(node, DflowScalars::Reinterpret(ResultDataflowType(node), op));
}

void DflowBuilder::Visit(const NodeIEqual *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, intEqual(op1, op2));
}

void DflowBuilder::Visit(const NodeINotEqual *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, intNotEqual(op1, op2));
}

void DflowBuilder::Visit(const NodeUGreaterThan *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1.Unsigned() > op2.Unsigned());
}

void DflowBuilder::Visit(const NodeUGreaterThanEqual *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1.Unsigned() >= op2.Unsigned());
}

void DflowBuilder::Visit(const NodeULessThan *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1.Unsigned() < op2.Unsigned());
}

void DflowBuilder::Visit(const NodeULessThanEqual *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1.Unsigned() <= op2.Unsigned());
}

void DflowBuilder::Visit(const NodeSGreaterThan *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1.Signed() > op2.Signed());
}

void DflowBuilder::Visit(const NodeSGreaterThanEqual *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1.Signed() >= op2.Signed());
}

void DflowBuilder::Visit(const NodeSLessThan *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1.Signed() < op2.Signed());
}

void DflowBuilder::Visit(const NodeSLessThanEqual *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   AddDataflow(node, op1.Signed() <= op2.Signed());
}

void DflowBuilder::Visit(const NodeIAddCarry *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   DflowScalars r     = op1 + op2;
   DflowScalars carry = cond(r < op1, constU(1, r), constU(0, r));

   AddDataflow(node, DflowScalars::Join(r, carry));
}

void DflowBuilder::Visit(const NodeISubBorrow *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   DflowScalars r      = op1 - op2;
   DflowScalars borrow = cond(op1 >= op2, constU(0, r), constU(1, r));

   AddDataflow(node, DflowScalars::Join(r, borrow));
}

void DflowBuilder::Visit(const NodeUMulExtended *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   DflowScalars lsb;
   DflowScalars msb;

   uMulExtended(op1, op2, lsb, msb);

   AddDataflow(node, DflowScalars::Join(lsb, msb));
}

void DflowBuilder::Visit(const NodeSMulExtended *node)
{
   const DflowScalars &op1 = GetDataflow(node->GetOperand1());
   const DflowScalars &op2 = GetDataflow(node->GetOperand2());

   DflowScalars lsbU;
   DflowScalars msbU;

   uMulExtended(op1.Unsigned(), op2.Unsigned(), lsbU, msbU);

   DflowScalars msb  = msbU.Signed();
   DflowScalars lsb  = lsbU.Signed();
   DflowScalars zero = constI(0, op1);

   msb = cond(op1 < zero, msb - op2, msb);
   msb = cond(op2 < zero, msb - op1, msb);

   AddDataflow(node, DflowScalars::Join(lsb, msb));
}

void DflowBuilder::Visit(const NodeBitFieldSExtract *node)
{
   const DflowScalars &value  = GetDataflow(node->GetBase()).Signed();
   const DflowScalars &offset = GetDataflow(node->GetOffset()).Signed();
   const DflowScalars &count  = GetDataflow(node->GetCount()).Signed();

   DflowScalars zero   = constI(0, value);
   DflowScalars isZero = count == zero;
   DflowScalars shift  = constI(32, value) - count;

   AddDataflow(node, cond(isZero, zero, asr(value << (shift - offset), shift)));
}

void DflowBuilder::Visit(const NodeBitFieldUExtract *node)
{
   const DflowScalars &value  = GetDataflow(node->GetBase()).Unsigned();
   const DflowScalars &offset = GetDataflow(node->GetOffset()).Unsigned();
   const DflowScalars &count  = GetDataflow(node->GetCount()).Unsigned();

   DflowScalars zero   = constU(0, value);
   DflowScalars isZero = count == zero;
   DflowScalars shift  = constU(32, value) - count;

   AddDataflow(node, cond(isZero, zero, lsr(value << (shift - offset), shift)));
}

void DflowBuilder::Visit(const NodeBitFieldInsert *node)
{
   const DflowScalars &value  = GetDataflow(node->GetBase()).Unsigned();
   const DflowScalars &offset = GetDataflow(node->GetOffset()).Unsigned();
   const DflowScalars &count  = GetDataflow(node->GetCount()).Unsigned();
   const DflowScalars &insert = GetDataflow(node->GetInsert()).Unsigned();

   DflowScalars xFFFFFFFF = constU(0xFFFFFFFF, value);
   DflowScalars u32       = constU(32, value);
   DflowScalars zero      = constU(0, value);
   DflowScalars isZero    = count == zero;

   DflowScalars mask   = (xFFFFFFFF >> (u32 - count)) << offset;
   DflowScalars result = cond(isZero, value, ((insert << offset) & mask) + (value & ~mask));

   AddDataflow(node, result);
}

void DflowBuilder::Visit(const NodeBitCount *node)
{
   DflowScalars   value = GetDataflow(node->GetBase()).Unsigned();

   const DflowScalars   u1        = constU(1, value);
   const DflowScalars   u2        = constU(2, value);
   const DflowScalars   u4        = constU(4, value);
   const DflowScalars   u24       = constU(24, value);
   const DflowScalars   x33333333 = constU(0x33333333, value);
   const DflowScalars   x55555555 = constU(0x55555555, value);
   const DflowScalars   x0F0F0F0F = constU(0x0F0F0F0F, value);
   const DflowScalars   x01010101 = constU(0x01010101, value);

   value = value - ((value >> u1) & x55555555);               // 2 bit counters
   value = (value & x33333333) + ((value >> u2) & x33333333); // 4 bit counters
   value = (value + (value >> u4)) & x0F0F0F0F;               // 8 bit counters

   AddDataflow(node, lsr(value * x01010101, u24).Signed());
}

void DflowBuilder::Visit(const NodeQuantizeToF16 *node)
{
   const DflowScalars &value = GetDataflow(node->GetValue());

   AddDataflow(node, quantizeToF16(value));
}

void DflowBuilder::Visit(const NodeTranspose *node)
{
   uint32_t rows, cols;
   // This gets the info on the result i.e. the dst
   // so the src will be the other way round.
   FindMatrixInfo(node, &cols, &rows);

   DflowMatrix src(rows, cols, GetDataflow(node->GetMatrix()));
   DflowMatrix dst(*this, cols, rows);

   for (uint32_t c = 0; c < cols; ++c)
      for (uint32_t r = 0; r < rows; ++r)
         dst(c, r) = src(r, c);

   AddDataflow(node, dst.GetScalars());
}

void DflowBuilder::Visit(const NodeMatrixTimesScalar *node)
{
   uint32_t rows, cols;
   FindMatrixInfo(node, &cols, &rows);

   const DflowScalars &matrix = GetDataflow(node->GetMatrix());
   const DflowScalars &scalar = GetDataflow(node->GetScalar());

   DflowScalars result(*this, cols * rows);

   for (uint32_t i = 0; i < rows * cols; ++i)
      result[i] = matrix[i] * scalar[0];

   AddDataflow(node, result);
}

static inline DflowScalars MatXY_MatZYxMatXZ(const DflowMatrix &lhs, const DflowMatrix &rhs)
{
   const DflowBuilder &alloc = lhs.Builder();

   const uint32_t X = rhs.GetCols();
   const uint32_t Y = lhs.GetRows();
   const uint32_t Z = lhs.GetCols();

   DflowMatrix    result(alloc, X, Y);

   for (uint32_t c = 0; c < X; ++c)
   {
      for (uint32_t r = 0; r < Y; ++r)
      {
         Dflow sum;

         for (uint32_t t = 0; t < Z; ++t)
         {
            Dflow product = lhs(t, r) * rhs(c, t);

            sum = (t == 0) ? product : sum + product;
         }

         result(c, r) = sum;
      }
   }

   return result.GetScalars();
}

void DflowBuilder::Visit(const NodeVectorTimesMatrix *node)
{
   const Node *vecOp = node->GetVector();
   const Node *matOp = node->GetMatrix();

   uint32_t cols, rows;
   FindMatrixInfo(matOp, &cols, &rows);

   DflowMatrix lhs(rows, 1,    GetDataflow(vecOp));
   DflowMatrix rhs(cols, rows, GetDataflow(matOp));

   DflowScalars result = MatXY_MatZYxMatXZ(lhs, rhs);

   AddDataflow(node, result);
}

void DflowBuilder::Visit(const NodeMatrixTimesVector *node)
{
   const Node *vecOp = node->GetVector();
   const Node *matOp = node->GetMatrix();

   uint32_t cols, rows;
   FindMatrixInfo(matOp, &cols, &rows);

   DflowMatrix lhs(cols, rows, GetDataflow(matOp));
   DflowMatrix rhs(1,    cols, GetDataflow(vecOp));

   DflowScalars result = MatXY_MatZYxMatXZ(lhs, rhs);

   AddDataflow(node, result);
}

void DflowBuilder::Visit(const NodeMatrixTimesMatrix *node)
{
   const Node *lMatOp = node->GetLeftMatrix();
   const Node *rMatOp = node->GetRightMatrix();

   uint32_t lCols, lRows, rCols, rRows;
   FindMatrixInfo(lMatOp, &lCols, &lRows);
   FindMatrixInfo(rMatOp, &rCols, &rRows);

   const DflowMatrix lhs(lCols, lRows, GetDataflow(lMatOp));
   const DflowMatrix rhs(rCols, rRows, GetDataflow(rMatOp));

   DflowScalars result = MatXY_MatZYxMatXZ(lhs, rhs);

   AddDataflow(node, result);
}

void DflowBuilder::Visit(const NodeOuterProduct *node)
{
   const Node *vecOp1 = node->GetVector1();
   const Node *vecOp2 = node->GetVector2();

   uint32_t rows = VectorSize(vecOp1);
   uint32_t cols = VectorSize(vecOp2);

   const DflowScalars &lhs = GetDataflow(vecOp1);
   const DflowScalars &rhs = GetDataflow(vecOp2);

   DflowMatrix  result(*this, cols, rows);

   for (uint32_t c = 0; c < cols; ++c)
      for (uint32_t r = 0; r < rows; ++r)
         result(c, r) = lhs[r] * rhs[c];

   AddDataflow(node, result.GetScalars());
}

static inline Dflow Det2x2(const DflowMatrix &m)
{
   return m(0, 0) * m(1, 1) -
          m(0, 1) * m(1, 0);
}

static inline Dflow Det3x3(const DflowMatrix &m)
{
   const DflowBuilder &alloc = m.Builder();

   DflowMatrix minor00(alloc, m(1, 1), m(1, 2),
                              m(2, 1), m(2, 2));
   DflowMatrix minor01(alloc, m(1, 0), m(1, 2),
                              m(2, 0), m(2, 2));
   DflowMatrix minor02(alloc, m(1, 0), m(1, 1),
                              m(2, 0), m(2, 1));

   return m(0, 0) * Det2x2(minor00) -
          m(0, 1) * Det2x2(minor01) +
          m(0, 2) * Det2x2(minor02);
}

static inline Dflow Det4x4(const DflowMatrix &m)
{
   const DflowBuilder &alloc = m.Builder();

   DflowMatrix minor00(alloc, m(1, 1), m(1, 2), m(1, 3),
                              m(2, 1), m(2, 2), m(2, 3),
                              m(3, 1), m(3, 2), m(3, 3));

   DflowMatrix minor01(alloc, m(1, 0), m(1, 2), m(1, 3),
                              m(2, 0), m(2, 2), m(2, 3),
                              m(3, 0), m(3, 2), m(3, 3));

   DflowMatrix minor02(alloc, m(1, 0), m(1, 1), m(1, 3),
                              m(2, 0), m(2, 1), m(2, 3),
                              m(3, 0), m(3, 1), m(3, 3));

   DflowMatrix minor03(alloc, m(1, 0), m(1, 1), m(1, 2),
                              m(2, 0), m(2, 1), m(2, 2),
                              m(3, 0), m(3, 1), m(3, 2));

   return m(0, 0) * Det3x3(minor00) -
          m(0, 1) * Det3x3(minor01) +
          m(0, 2) * Det3x3(minor02) -
          m(0, 3) * Det3x3(minor03);
}

void DflowBuilder::Visit(const NodeStdDeterminant *node)
{
   uint32_t cols, rows;
   FindMatrixInfo(node->GetX(), &cols, &rows);
   assert(rows == cols && rows >= 2 && rows <= 4);

   DflowMatrix m(cols, rows, GetDataflow(node->GetX()));

   Dflow result;

   switch (cols)
   {
   case 2: result = Det2x2(m); break;
   case 3: result = Det3x3(m); break;
   case 4: result = Det4x4(m); break;
   }

   AddDataflow(node, DflowScalars(*this, result));
}

inline DflowScalars InvMat2x2(const DflowMatrix &m)
{
   const DflowBuilder &alloc = m.Builder();

   Dflow scale = Dflow::ConstantFloat(1.0f) / Det2x2(m);

   DflowMatrix  result(alloc,  m(1, 1) * scale, -m(0, 1) * scale,
                              -m(1, 0) * scale,  m(0, 0) * scale);

   return result.GetScalars();
}

inline DflowScalars InvMat3x3(const DflowMatrix &m)
{
   const DflowBuilder &alloc = m.Builder();

   Dflow scale = Dflow::ConstantFloat(1.0f) / Det3x3(m);

   DflowMatrix    minor00(alloc, m(1, 1), m(1, 2),
                                 m(2, 1), m(2, 2));
   DflowMatrix    minor01(alloc, m(1, 0), m(1, 2),
                                 m(2, 0), m(2, 2));
   DflowMatrix    minor02(alloc, m(1, 0), m(1, 1),
                                 m(2, 0), m(2, 1));
   DflowMatrix    minor10(alloc, m(0, 1), m(0, 2),
                                 m(2, 1), m(2, 2));
   DflowMatrix    minor11(alloc, m(0, 0), m(0, 2),
                                 m(2, 0), m(2, 2));
   DflowMatrix    minor12(alloc, m(0, 0), m(0, 1),
                                 m(2, 0), m(2, 1));
   DflowMatrix    minor20(alloc, m(0, 1), m(0, 2),
                                 m(1, 1), m(1, 2));
   DflowMatrix    minor21(alloc, m(0, 0), m(0, 2),
                                 m(1, 0), m(1, 2));
   DflowMatrix    minor22(alloc, m(0, 0), m(0, 1),
                                 m(1, 0), m(1, 1));

   DflowMatrix  result(alloc,  Det2x2(minor00) * scale, -Det2x2(minor10) * scale,  Det2x2(minor20) * scale,
                              -Det2x2(minor01) * scale,  Det2x2(minor11) * scale, -Det2x2(minor21) * scale,
                               Det2x2(minor02) * scale, -Det2x2(minor12) * scale,  Det2x2(minor22) * scale);

   return result.GetScalars();
}

inline DflowScalars InvMat4x4(const DflowMatrix &m)
{
   const DflowBuilder &alloc = m.Builder();

   Dflow scale = Dflow::ConstantFloat(1.0f) / Det4x4(m);

   DflowMatrix    minor00(alloc, m(1, 1), m(1, 2), m(1, 3),
                                 m(2, 1), m(2, 2), m(2, 3),
                                 m(3, 1), m(3, 2), m(3, 3));
   DflowMatrix    minor01(alloc, m(1, 0), m(1, 2), m(1, 3),
                                 m(2, 0), m(2, 2), m(2, 3),
                                 m(3, 0), m(3, 2), m(3, 3));
   DflowMatrix    minor02(alloc, m(1, 0), m(1, 1), m(1, 3),
                                 m(2, 0), m(2, 1), m(2, 3),
                                 m(3, 0), m(3, 1), m(3, 3));
   DflowMatrix    minor03(alloc, m(1, 0), m(1, 1), m(1, 2),
                                 m(2, 0), m(2, 1), m(2, 2),
                                 m(3, 0), m(3, 1), m(3, 2));
   DflowMatrix    minor10(alloc, m(0, 1), m(0, 2), m(0, 3),
                                 m(2, 1), m(2, 2), m(2, 3),
                                 m(3, 1), m(3, 2), m(3, 3));
   DflowMatrix    minor11(alloc, m(0, 0), m(0, 2), m(0, 3),
                                 m(2, 0), m(2, 2), m(2, 3),
                                 m(3, 0), m(3, 2), m(3, 3));
   DflowMatrix    minor12(alloc, m(0, 0), m(0, 1), m(0, 3),
                                 m(2, 0), m(2, 1), m(2, 3),
                                 m(3, 0), m(3, 1), m(3, 3));
   DflowMatrix    minor13(alloc, m(0, 0), m(0, 1), m(0, 2),
                                 m(2, 0), m(2, 1), m(2, 2),
                                 m(3, 0), m(3, 1), m(3, 2));
   DflowMatrix    minor20(alloc, m(0, 1), m(0, 2), m(0, 3),
                                 m(1, 1), m(1, 2), m(1, 3),
                                 m(3, 1), m(3, 2), m(3, 3));
   DflowMatrix    minor21(alloc, m(0, 0), m(0, 2), m(0, 3),
                                 m(1, 0), m(1, 2), m(1, 3),
                                 m(3, 0), m(3, 2), m(3, 3));
   DflowMatrix    minor22(alloc, m(0, 0), m(0, 1), m(0, 3),
                                 m(1, 0), m(1, 1), m(1, 3),
                                 m(3, 0), m(3, 1), m(3, 3));
   DflowMatrix    minor23(alloc, m(0, 0), m(0, 1), m(0, 2),
                                 m(1, 0), m(1, 1), m(1, 2),
                                 m(3, 0), m(3, 1), m(3, 2));
   DflowMatrix    minor30(alloc, m(0, 1), m(0, 2), m(0, 3),
                                 m(1, 1), m(1, 2), m(1, 3),
                                 m(2, 1), m(2, 2), m(2, 3));
   DflowMatrix    minor31(alloc, m(0, 0), m(0, 2), m(0, 3),
                                 m(1, 0), m(1, 2), m(1, 3),
                                 m(2, 0), m(2, 2), m(2, 3));
   DflowMatrix    minor32(alloc, m(0, 0), m(0, 1), m(0, 3),
                                 m(1, 0), m(1, 1), m(1, 3),
                                 m(2, 0), m(2, 1), m(2, 3));
   DflowMatrix    minor33(alloc, m(0, 0), m(0, 1), m(0, 2),
                                 m(1, 0), m(1, 1), m(1, 2),
                                 m(2, 0), m(2, 1), m(2, 2));

   DflowMatrix  result(alloc,  Det3x3(minor00) * scale, -Det3x3(minor10) * scale,  Det3x3(minor20) * scale, -Det3x3(minor30) * scale,
                              -Det3x3(minor01) * scale,  Det3x3(minor11) * scale, -Det3x3(minor21) * scale,  Det3x3(minor31) * scale,
                               Det3x3(minor02) * scale, -Det3x3(minor12) * scale,  Det3x3(minor22) * scale, -Det3x3(minor32) * scale,
                              -Det3x3(minor03) * scale,  Det3x3(minor13) * scale, -Det3x3(minor23) * scale,  Det3x3(minor33) * scale);

   return result.GetScalars();
}

void DflowBuilder::Visit(const NodeStdMatrixInverse *node)
{
   uint32_t cols, rows;
   FindMatrixInfo(node, &cols, &rows);
   assert(rows == cols && rows >= 2 && rows <= 4);

   DflowMatrix m(cols, rows, GetDataflow(node->GetX()));

   switch (cols)
   {
   case 2: AddDataflow(node, InvMat2x2(m)); break;
   case 3: AddDataflow(node, InvMat3x3(m)); break;
   case 4: AddDataflow(node, InvMat4x4(m)); break;
   }
}

void DflowBuilder::Visit(const NodeAny *node)
{
   const DflowScalars &v = GetDataflow(node->GetVector());

   DflowScalars result(*this, 1);

   result[0] = v[0];
   for (uint32_t i = 1; i < v.Size(); ++i)
      result[0] = result[0] || v[i];

   AddDataflow(node, result);
}

void DflowBuilder::Visit(const NodeAll *node)
{
   const DflowScalars &v = GetDataflow(node->GetVector());

   DflowScalars   result(*this, 1);

   result[0] = v[0];
   for (uint32_t i = 1; i < v.Size(); ++i)
      result[0] = result[0] && v[i];

   AddDataflow(node, result);
}

void DflowBuilder::Visit(const NodeConvertUToF *node)
{
   const DflowScalars &x = GetDataflow(node->GetUnsignedValue());
   AddDataflow(node, DflowScalars::UnaryOp(DATAFLOW_UTOF, x));
}

void DflowBuilder::Visit(const NodeConvertSToF *node)
{
   const DflowScalars &x = GetDataflow(node->GetSignedValue());
   AddDataflow(node, DflowScalars::UnaryOp(DATAFLOW_ITOF, x));
}

void DflowBuilder::Visit(const NodeConvertFToU *node)
{
   const DflowScalars &x = GetDataflow(node->GetFloatValue());
   AddDataflow(node, DflowScalars::UnaryOp(DATAFLOW_FTOU, x));
}

void DflowBuilder::Visit(const NodeConvertFToS *node)
{
   // Must round toward zero
   const DflowScalars &x = GetDataflow(node->GetFloatValue());
   AddDataflow(node, DflowScalars::UnaryOp(DATAFLOW_FTOI_TRUNC, x));
}

void DflowBuilder::Visit(const NodeUConvert *node)
{
   const DflowScalars &x = GetDataflow(node->GetUnsignedValue());
   AddDataflow(node, x);   // Nothing to do here
}

void DflowBuilder::Visit(const NodeSConvert *node)
{
   const DflowScalars &x = GetDataflow(node->GetSignedValue());
   AddDataflow(node, x);   // Nothing to do here
}

void DflowBuilder::Visit(const NodeFConvert *node)
{
   const DflowScalars &x = GetDataflow(node->GetFloatValue());
   AddDataflow(node, x);   // Nothing to do here
}

void DflowBuilder::Visit(const NodeStdLog2 *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, log2(x));
}

void DflowBuilder::Visit(const NodeStdLog *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, log(x));
}

void DflowBuilder::Visit(const NodeStdSin *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, sin(x));
}

void DflowBuilder::Visit(const NodeStdCos *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, cos(x));
}

void DflowBuilder::Visit(const NodeStdTan *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, tan(x));
}

void DflowBuilder::Visit(const NodeStdSinh *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, sinh(x));
}

void DflowBuilder::Visit(const NodeStdCosh *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, cosh(x));
}

void DflowBuilder::Visit(const NodeStdTanh *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, tanh(x));
}

void DflowBuilder::Visit(const NodeStdAsin *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, asin(x));
}

void DflowBuilder::Visit(const NodeStdAcos *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, acos(x));
}

void DflowBuilder::Visit(const NodeStdAtan *node)
{
   const DflowScalars &yOverX = GetDataflow(node->GetY_over_x());
   AddDataflow(node, atan(yOverX));
}

void DflowBuilder::Visit(const NodeStdAtan2 *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   AddDataflow(node, atan(y, x));
}

void DflowBuilder::Visit(const NodeStdAsinh *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, asinh(x));
}

void DflowBuilder::Visit(const NodeStdAcosh *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, acosh(x));
}

void DflowBuilder::Visit(const NodeStdAtanh *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, atanh(x));
}

void DflowBuilder::Visit(const NodeStdDegrees *node)
{
   const DflowScalars &r = GetDataflow(node->GetRadians());
   AddDataflow(node, degrees(r));
}

void DflowBuilder::Visit(const NodeStdRadians *node)
{
   const DflowScalars &d = GetDataflow(node->GetDegrees());
   AddDataflow(node, radians(d));
}

void DflowBuilder::Visit(const NodeStdInverseSqrt *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, inversesqrt(x));
}

void DflowBuilder::Visit(const NodeStdSqrt *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, sqrt(x));
}

void DflowBuilder::Visit(const NodeStdPow *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   AddDataflow(node, pow(x, y));
}

void DflowBuilder::Visit(const NodeStdExp *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, exp(x));
}

void DflowBuilder::Visit(const NodeStdExp2 *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, exp2(x));
}

void DflowBuilder::Visit(const NodeStdFAbs *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, fabs(x));
}

void DflowBuilder::Visit(const NodeStdFMax *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   AddDataflow(node, max(x, y));
}

void DflowBuilder::Visit(const NodeStdFMin *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   AddDataflow(node, min(x, y));
}

void DflowBuilder::Visit(const NodeStdFClamp *node)
{
   const DflowScalars &x    = GetDataflow(node->GetX());
   const DflowScalars &minv = GetDataflow(node->GetMinVal());
   const DflowScalars &maxv = GetDataflow(node->GetMaxVal());
   AddDataflow(node, clamp(x, minv, maxv));
}

void DflowBuilder::Visit(const NodeStdFMix *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   const DflowScalars &a = GetDataflow(node->GetA());
   AddDataflow(node, fmix(x, y, a));
}

void DflowBuilder::Visit(const NodeStdIMix *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   const DflowScalars &a = GetDataflow(node->GetA());
   AddDataflow(node, imix(x, y, a));
}

void DflowBuilder::Visit(const NodeStdSAbs *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, iabs(x));
}

void DflowBuilder::Visit(const NodeStdSMax *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   AddDataflow(node, max(x, y));
}

void DflowBuilder::Visit(const NodeStdSMin *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   AddDataflow(node, min(x, y));
}

void DflowBuilder::Visit(const NodeStdSClamp *node)
{
   const DflowScalars &x    = GetDataflow(node->GetX());
   const DflowScalars &minv = GetDataflow(node->GetMinVal());
   const DflowScalars &maxv = GetDataflow(node->GetMaxVal());
   AddDataflow(node, clamp(x, minv, maxv));
}

void DflowBuilder::Visit(const NodeStdUMax *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   AddDataflow(node, max(x, y));
}

void DflowBuilder::Visit(const NodeStdUMin *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   AddDataflow(node, min(x, y));
}

void DflowBuilder::Visit(const NodeStdUClamp *node)
{
   const DflowScalars &x    = GetDataflow(node->GetX());
   const DflowScalars &minv = GetDataflow(node->GetMinVal());
   const DflowScalars &maxv = GetDataflow(node->GetMaxVal());
   AddDataflow(node, clamp(x, minv, maxv));
}

void DflowBuilder::Visit(const NodeStdNMax *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   AddDataflow(node, nmax(x, y));
}

void DflowBuilder::Visit(const NodeStdNMin *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   AddDataflow(node, nmin(x, y));
}

void DflowBuilder::Visit(const NodeStdNClamp *node)
{
   const DflowScalars &x    = GetDataflow(node->GetX());
   const DflowScalars &minv = GetDataflow(node->GetMinVal());
   const DflowScalars &maxv = GetDataflow(node->GetMaxVal());
   AddDataflow(node, nclamp(x, minv, maxv));
}

void DflowBuilder::Visit(const NodeStdStep *node)
{
   const DflowScalars &x    = GetDataflow(node->GetX());
   const DflowScalars &edge = GetDataflow(node->GetEdge());

   AddDataflow(node, step(edge, x));
}

void DflowBuilder::Visit(const NodeStdSmoothStep *node)
{
   const DflowScalars &x     = GetDataflow(node->GetX());
   const DflowScalars &edge0 = GetDataflow(node->GetEdge0());
   const DflowScalars &edge1 = GetDataflow(node->GetEdge1());
   AddDataflow(node, smoothstep(edge0, edge1, x));
}

void DflowBuilder::Visit(const NodeStdTrunc *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, trunc(x));
}

void DflowBuilder::Visit(const NodeStdRound *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, round(x));  // Same as roundEven
}

void DflowBuilder::Visit(const NodeStdRoundEven *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, round(x));  // Same as round
}

void DflowBuilder::Visit(const NodeStdCeil *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, ceil(x));
}

void DflowBuilder::Visit(const NodeStdFloor *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, floor(x));
}

void DflowBuilder::Visit(const NodeStdFract *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, fract(x));
}

void DflowBuilder::Visit(const NodeStdFSign *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, fsign(x));
}

void DflowBuilder::Visit(const NodeStdSSign *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, ssign(x));
}

void DflowBuilder::Visit(const NodeStdFindUMsb *node)
{
   const DflowScalars &x = GetDataflow(node->GetValue());
   AddDataflow(node, findUMSB(x));
}

void DflowBuilder::Visit(const NodeStdFindSMsb *node)
{
   const DflowScalars &x = GetDataflow(node->GetValue());
   AddDataflow(node, findSMSB(x));
}

void DflowBuilder::Visit(const NodeStdFindILsb *node)
{
   const DflowScalars &x = GetDataflow(node->GetValue());
   AddDataflow(node, findLSB(x));
}

void DflowBuilder::Visit(const NodeStdFma *node)
{
   const DflowScalars &a = GetDataflow(node->GetA());
   const DflowScalars &b = GetDataflow(node->GetB());
   const DflowScalars &c = GetDataflow(node->GetC());
   AddDataflow(node, fma(a, b, c));
}

void DflowBuilder::Visit(const NodeStdLength *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, length(x));
}

void DflowBuilder::Visit(const NodeStdDistance *node)
{
   const DflowScalars &p0 = GetDataflow(node->GetP0());
   const DflowScalars &p1 = GetDataflow(node->GetP1());
   AddDataflow(node, distance(p0, p1));
}

void DflowBuilder::Visit(const NodeStdCross *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   const DflowScalars &y = GetDataflow(node->GetY());
   AddDataflow(node, cross(x, y));
}

void DflowBuilder::Visit(const NodeStdNormalize *node)
{
   const DflowScalars &x = GetDataflow(node->GetX());
   AddDataflow(node, normalize(x));
}

void DflowBuilder::Visit(const NodeStdFaceForward *node)
{
   const DflowScalars &N    = GetDataflow(node->GetN());
   const DflowScalars &I    = GetDataflow(node->GetI());
   const DflowScalars &Nref = GetDataflow(node->GetNref());
   AddDataflow(node, faceforward(N, I, Nref));
}

void DflowBuilder::Visit(const NodeStdReflect *node)
{
   const DflowScalars &I = GetDataflow(node->GetI());
   const DflowScalars &N = GetDataflow(node->GetN());
   AddDataflow(node, reflect(I, N));
}

void DflowBuilder::Visit(const NodeStdRefract *node)
{
   const DflowScalars &I   = GetDataflow(node->GetI());
   const DflowScalars &N   = GetDataflow(node->GetN());
   const DflowScalars &eta = GetDataflow(node->GetEta());
   AddDataflow(node, refract(I, N, eta));
}

void DflowBuilder::Visit(const NodeStdPackHalf2x16 *node)
{
   const DflowScalars &v = GetDataflow(node->GetV());
   AddDataflow(node, packHalf2x16(v));
}

void DflowBuilder::Visit(const NodeStdPackSnorm4x8 *node)
{
   const DflowScalars &v = GetDataflow(node->GetV());
   AddDataflow(node, packSnorm4x8(v));
}

void DflowBuilder::Visit(const NodeStdPackUnorm4x8 *node)
{
   const DflowScalars &v = GetDataflow(node->GetV());
   AddDataflow(node, packUnorm4x8(v));
}

void DflowBuilder::Visit(const NodeStdPackSnorm2x16 *node)
{
   const DflowScalars &v = GetDataflow(node->GetV());
   AddDataflow(node, packSnorm2x16(v));
}

void DflowBuilder::Visit(const NodeStdPackUnorm2x16 *node)
{
   const DflowScalars &v = GetDataflow(node->GetV());
   AddDataflow(node, packUnorm2x16(v));
}

void DflowBuilder::Visit(const NodeStdUnpackHalf2x16 *node)
{
   const DflowScalars &v = GetDataflow(node->GetV());
   AddDataflow(node, unpackHalf2x16(v));
}

void DflowBuilder::Visit(const NodeStdUnpackSnorm4x8 *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, unpackSnorm4x8(p));
}

void DflowBuilder::Visit(const NodeStdUnpackUnorm4x8 *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, unpackUnorm4x8(p));
}

void DflowBuilder::Visit(const NodeStdUnpackSnorm2x16 *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, unpackSnorm2x16(p));
}

void DflowBuilder::Visit(const NodeStdUnpackUnorm2x16 *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, unpackUnorm2x16(p));
}

void DflowBuilder::Visit(const NodeStdInterpolateAtCentroid *node)
{
   DflowScalars p   = LoadScalars::Load(*this, m_functionStack->GetCurrentBlock(), node->GetInterpolant());

   auto sym = GetVariableSymbol(node->GetInterpolant()->As<const NodeVariable*>());
   assert(sym.GetFlavour() == SYMBOL_VAR_INSTANCE);

   DflowScalars res = getValueAtOffset(p, getCentroidOffset(*this), sym.GetInterpQualifier());

   AddDataflow(node, res);
}

void DflowBuilder::Visit(const NodeStdInterpolateAtSample *node)
{
   DflowScalars p = LoadScalars::Load(*this, m_functionStack->GetCurrentBlock(), node->GetInterpolant());

   // If not multi-sampling, this needs to ignore the given offset and just use pixel-centre instead
   if (m_multiSampled)
   {
      // Adjust for sample
      const DflowScalars &sample = GetDataflow(node->GetSample());

      auto sym = GetVariableSymbol(node->GetInterpolant()->As<const NodeVariable*>());
      assert(sym.GetFlavour() == SYMBOL_VAR_INSTANCE);

      DflowScalars res = getValueAtOffset(p, getSampleOffset(p.GetBuilder(), sample[0]),
                                          sym.GetInterpQualifier());
      AddDataflow(node, res);
   }
   else
      AddDataflow(node, p);
}

void DflowBuilder::Visit(const NodeStdInterpolateAtOffset *node)
{
   DflowScalars p = LoadScalars::Load(*this, m_functionStack->GetCurrentBlock(), node->GetInterpolant());

   auto sym = GetVariableSymbol(node->GetInterpolant()->As<const NodeVariable*>());
   assert(sym.GetFlavour() == SYMBOL_VAR_INSTANCE);

   const DflowScalars &offset = GetDataflow(node->GetOffset());
   DflowScalars        res    = getValueAtOffset(p, offset, sym.GetInterpQualifier());

   AddDataflow(node, res);
}

void DflowBuilder::Visit(const NodeDPdx *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, dFdx(p));
}

void DflowBuilder::Visit(const NodeDPdxFine *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, dFdx(p));
}

void DflowBuilder::Visit(const NodeDPdxCoarse *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, dFdx(p));
}

void DflowBuilder::Visit(const NodeDPdy *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, dFdy(p));
}

void DflowBuilder::Visit(const NodeDPdyFine *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, dFdy(p));
}

void DflowBuilder::Visit(const NodeDPdyCoarse *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, dFdy(p));
}

void DflowBuilder::Visit(const NodeFwidth *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, fwidth(p));
}

void DflowBuilder::Visit(const NodeFwidthFine *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, fwidth(p));
}

void DflowBuilder::Visit(const NodeFwidthCoarse *node)
{
   const DflowScalars &p = GetDataflow(node->GetP());
   AddDataflow(node, fwidth(p));
}

void DflowBuilder::Visit(const NodeCopyObject *node)
{
   AddDataflow(node, GetDataflow(node->GetOperand()));
}

void DflowBuilder::Visit(const NodeArrayLength *node)
{
   auto  var        = node->GetStructure()->As<const NodeVariable *>();
   auto  ptrType    = var->GetResultType()->As<const NodeTypePointer *>();
   auto  structType = ptrType->GetType()->As<const NodeTypeStruct *>();

   uint32_t arrayMemberIx = node->GetArraymember();
   uint32_t offset        = 0;

   m_module.GetLiteralMemberDecoration(&offset, spv::Decoration::Offset, structType, arrayMemberIx);

   auto arrayType = structType->GetMemberstype()[arrayMemberIx]->As<const NodeTypeRuntimeArray *>();
   auto elemType  = arrayType->GetElementType()->As<const NodeType *>();

   Dflow elemSize   = Dflow::ConstantUInt(GetNumScalars(elemType) * 4);

   uint32_t descriptorSet = m_module.RequireLiteralDecoration(spv::Decoration::DescriptorSet, var);
   uint32_t binding       = m_module.RequireLiteralDecoration(spv::Decoration::Binding, var);
   bool ssbo              = IsSSBO::Test(GetModule(), var);

   DescriptorInfo dInfo(descriptorSet, binding, 0);
   uint32_t descTableIndex = GetDescriptorMaps().FindBufferEntry(ssbo, dInfo);

   Dflow arrayBytes = Dflow::BufferSize(DF_UINT, offset,
                                        descTableIndex, ssbo ? DATAFLOW_STORAGE_BUFFER : DATAFLOW_UNIFORM_BUFFER);

   DflowScalars result(*this, arrayBytes / elemSize);
   AddDataflow(node, result);
}

void DflowBuilder::Visit(const NodeLabel *node)
{
   if (m_loopMerge.size() == 0)
      return;

   if (m_loopMerge.back().IsSameBlock(m_functionStack->GetCurrentBlock()))
      m_loopMerge.pop_back();
}

void DflowBuilder::Visit(const NodeLoopMerge *node)
{
   m_loopMerge.push_back(m_functionStack->BlockForLabel(node->GetMergeBlock()));
}

void DflowBuilder::Visit(const NodeControlBarrier *node)
{
   BasicBlockHandle currentBlock = m_functionStack->GetCurrentBlock();

   if (m_flavour != SHADER_COMPUTE || glsl_wg_size_requires_barriers(m_executionModes.GetWorkgroupSize().data()))
   {
      currentBlock->SetBarrier();

      // The barrier marks the end of the block so we need to start
      // a new one.
      BasicBlockHandle newBlock(*this);
      currentBlock->SetFallthroughTarget(newBlock);
      currentBlock->SetRedirect(newBlock);

      m_functionStack->SetCurrentBlock(newBlock);
   }

   // TODO: ignoring scope -- does it mean anything for us?
   // There's no dataflow associated with this instruction
   // We do not need to take any notice of the memory semantics
}

void DflowBuilder::Visit(const NodeMemoryBarrier *node)
{
   // Nothing to do.  We have a coherent view of memory.
}

// Create dataflow for loading specified symbol in the specified block
DflowScalars DflowBuilder::LoadFromSymbol(BasicBlockHandle block, SymbolHandle symbol)
{
   // Record that the symbol is statically used
   m_usedSymbols.insert(symbol);

   Dataflow **scalarValues = block->GetScalars(symbol);

   uint32_t numScalars = symbol.GetType().GetNumScalars();

   if (scalarValues != nullptr)
      return DflowScalars(*this, numScalars, scalarValues);

   scalarValues = DflowScalars::Load(*this, symbol).Data();

   // Record the load array
   block->PutLoad(symbol, scalarValues);

   // Creates a new array with the same dataflows
   DflowScalars result(*this, numScalars, scalarValues);

   // Record the dataflow array
   block->PutScalars(symbol, result.Data());

   return result;
}

// Store dataflow for given symbol in the given block
void DflowBuilder::StoreToSymbol(BasicBlockHandle block, SymbolHandle symbol, const DflowScalars &data)
{
   // Record that the symbol is statically used, if it's an output
   if (symbol.GetFlavour() == SYMBOL_VAR_INSTANCE && symbol.GetStorageQualifier() == STORAGE_OUT)
      m_usedSymbols.insert(symbol);

   Dataflow **scalarValues = block->GetScalars(symbol);

   // Have we seen this symbol?
   if (scalarValues == nullptr)
   {
      // Create new empty array
      scalarValues = DflowScalars(*this, data.Size()).Data();

      // Record the dataflow array
      block->PutScalars(symbol, scalarValues);
   }

   // Record results into the symbol's dataflow
   for (uint32_t i = 0; i < data.Size(); ++i)
      scalarValues[i] = data[i];
}

// Store a dataflow to a slice for given symbol in current block
void DflowBuilder::StoreToSymbol(BasicBlockHandle block, SymbolHandle symbol,
                                 const DflowScalars &data, uint32_t offset)
{
   // Record that the symbol is statically used, if it's an output
   if (symbol.GetFlavour() == SYMBOL_VAR_INSTANCE && symbol.GetStorageQualifier() == STORAGE_OUT)
      m_usedSymbols.insert(symbol);

   Dataflow **scalarValues = block->GetScalars(symbol);

   if (scalarValues == nullptr)
   {
      DflowScalars load = DflowScalars::Load(*this, symbol);

      // Record dataflow array in load map
      block->PutLoad(symbol, load.Data());

      // Record a copy in the scalar map
      scalarValues = DflowScalars(*this, load.Size(), load.Data()).Data();
      block->PutScalars(symbol, scalarValues);
   }

   // Write slice into the symbol's dataflow
   for (uint32_t i = 0; i < data.Size(); ++i)
      scalarValues[offset + i] = data[i];
}

void DflowBuilder::Visit(const NodeLoad *load)
{
   DflowScalars   result = LoadScalars::Load(*this, m_functionStack->GetCurrentBlock(),
                                              load->GetPointer());

   AddDataflow(load, result);
}

void DflowBuilder::Visit(const NodeStore *store)
{
   StoreScalars::Store(*this, m_functionStack->GetCurrentBlock(),
                        store->GetPointer(), store->GetObject());
}

void DflowBuilder::Visit(const NodeCopyMemory *node)
{
   // We don't support the Addresses capability, so this should
   // just copy from one target to another
   BasicBlockHandle currentBlock = m_functionStack->GetCurrentBlock();

   DflowScalars   data = LoadScalars::Load(*this, currentBlock, node->GetSource());

   StoreScalars::Store(*this, currentBlock, node->GetTarget(), data);
}

void DflowBuilder::BuildAtomic(const Node *node, DataflowFlavour flavour,
                               const Node *pointer, const DflowScalars &value)
{
   BasicBlockHandle current = m_functionStack->GetCurrentBlock();
   DflowScalars     result  = AtomicAccess::Access(*this, flavour, current, pointer, value);

   AddDataflow(node, result);
}

template <class N>
void DflowBuilder::BuildAtomic(const N *node, DataflowFlavour flavour)
{
   const DflowScalars &value   = GetDataflow(node->GetValue());
   BasicBlockHandle    current = m_functionStack->GetCurrentBlock();

   DflowScalars        result  = AtomicAccess::Access(*this, flavour, current,
                                                       node->GetPointer(), value);
   AddDataflow(node, result);
}

void DflowBuilder::Visit(const NodeAtomicLoad *node)
{
   DflowScalars zero = DflowScalars::ConstantUInt(*this, 0);

   // This could be a float.  This also does a store which is inefficient.
   BuildAtomic(node, DATAFLOW_ATOMIC_OR, node->GetPointer(), zero);
}

void DflowBuilder::Visit(const NodeAtomicStore *node)
{
   const DflowScalars &value   = GetDataflow(node->GetValue());
   BasicBlockHandle    current = m_functionStack->GetCurrentBlock();

   // Result is not used.
   AtomicAccess::Access(*this, DATAFLOW_ADDRESS_STORE, current,
                        node->GetPointer(), value);
}

void DflowBuilder::Visit(const NodeAtomicExchange *node)
{
   BuildAtomic(node, DATAFLOW_ATOMIC_XCHG);
}

void DflowBuilder::Visit(const NodeAtomicCompareExchange *node)
{
   const DflowScalars &value   = GetDataflow(node->GetValue());
   const DflowScalars &comp    = GetDataflow(node->GetComparator());
   BasicBlockHandle    current = m_functionStack->GetCurrentBlock();

   DflowScalars        result  = AtomicAccess::Access(*this, DATAFLOW_ATOMIC_CMPXCHG, current,
                                                       node->GetPointer(), value, comp[0]);
   AddDataflow(node, result);
}

void DflowBuilder::Visit(const NodeAtomicIIncrement *node)
{
   DflowScalars one = DflowScalars::ConstantUInt(*this, 1);

   BuildAtomic(node, DATAFLOW_ATOMIC_ADD, node->GetPointer(), one);
}

void DflowBuilder::Visit(const NodeAtomicIDecrement *node)
{
   DflowScalars one = DflowScalars::ConstantUInt(*this, 1);

   BuildAtomic(node, DATAFLOW_ATOMIC_SUB, node->GetPointer(), one);
}

void DflowBuilder::Visit(const NodeAtomicIAdd *node)
{
   BuildAtomic(node, DATAFLOW_ATOMIC_ADD);
}

void DflowBuilder::Visit(const NodeAtomicISub *node)
{
   BuildAtomic(node, DATAFLOW_ATOMIC_SUB);
}

void DflowBuilder::Visit(const NodeAtomicSMin *node)
{
   BuildAtomic(node, DATAFLOW_ATOMIC_MIN);
}

void DflowBuilder::Visit(const NodeAtomicUMin *node)
{
   BuildAtomic(node, DATAFLOW_ATOMIC_MIN);
}

void DflowBuilder::Visit(const NodeAtomicSMax *node)
{
   BuildAtomic(node, DATAFLOW_ATOMIC_MAX);
}

void DflowBuilder::Visit(const NodeAtomicUMax *node)
{
   BuildAtomic(node, DATAFLOW_ATOMIC_MAX);
}

void DflowBuilder::Visit(const NodeAtomicAnd *node)
{
   BuildAtomic(node, DATAFLOW_ATOMIC_AND);
}

void DflowBuilder::Visit(const NodeAtomicOr *node)
{
   BuildAtomic(node, DATAFLOW_ATOMIC_OR);
}

void DflowBuilder::Visit(const NodeAtomicXor *node)
{
   BuildAtomic(node, DATAFLOW_ATOMIC_XOR);
}

// Test whether a chain of indices are all constants
bool DflowBuilder::IsConstant(const spv::vector<const Node *> &indices)
{
   for (const Node *ix : indices)
      if (!ConstantInt(ix, nullptr))
         return false;

   return true;
}

// Allocate ids to the symbol and record in the symbol id map
void DflowBuilder::AssignIds(SymbolHandle symbol, uint32_t *current)
{
   uint32_t count = symbol.GetType().GetNumScalars();

   int *ids = static_cast<int *>(malloc_fast(count * sizeof(int)));

   for (uint32_t i = 0; i < count; ++i)
   {
      ids[i] = *current;
      *current += 1;
   }

   m_symbolIdMap.Put(symbol, ids);
}

void DflowBuilder::RecordInputSymbol(SymbolHandle symbol)
{
   m_inputSymbols.push_back(symbol);
   if (strncmp(symbol.GetName(), "gl_", 3))
      AssignIds(symbol, &m_curInputRow);
}

void DflowBuilder::RecordOutputSymbol(SymbolHandle symbol)
{
   m_outputSymbols.push_back(symbol);
   AssignIds(symbol, &m_curOutputRow);
}

void DflowBuilder::RecordUniformSymbol(SymbolHandle symbol, int *ids)
{
   m_uniformSymbols.push_back(symbol);
   m_symbolIdMap.Put(symbol, ids);
}

void DflowBuilder::RenameBuiltinSymbol(const Node *var, SymbolHandle symbol, bool *seenPointSize)
{
   // Set the expected name for builtins
   spv::BuiltIn   builtin;
   if (m_module.GetBuiltinDecoration(&builtin, var))
   {
      switch(builtin)
      {
      case spv::BuiltIn::Position   : symbol.SetName("gl_Position");   break;
      case spv::BuiltIn::PointSize  : symbol.SetName("gl_PointSize");  break;
      case spv::BuiltIn::FragDepth  : symbol.SetName("gl_FragDepth");  break;
      case spv::BuiltIn::SampleMask : symbol.SetName("gl_SampleMask"); break;
      default                   : break;
      }

      if (builtin == spv::BuiltIn::PointSize)
         *seenPointSize = true;
   }
}

void DflowBuilder::SetupInterface(const NodeEntryPoint *entryPoint, ShaderFlavour flavour)
{
   BasicBlockHandle exitBlock     = m_functionStack->GetCurrentBlock();
   bool             seenPointSize = false;

   for (const NodeIndex index : entryPoint->GetInterface())
   {
      auto         var    = index->As<const NodeVariable *>();
      SymbolHandle symbol = GetVariableSymbol(var);

      RenameBuiltinSymbol(var, symbol, &seenPointSize);

      if (flavour == SHADER_VERTEX && symbol == m_glPerVertex)
      {
         auto ptr       = var->GetResultType()->As<const NodeTypePointer *>();
         auto structure = ptr->GetType()->As<const NodeTypeStruct *>();

         // Are there any member decorations for gl_Position and gl_PointSize?
         for (uint32_t i = 0; i < structure->GetMemberstype().size(); i++)
         {
            spv::vector<uint32_t> indices(1, i, m_arenaAllocator);
            uint32_t offset = ScalarOffsetStatic::Calculate(*this, structure, indices);

            spv::BuiltIn   builtin;
            if (m_module.GetBuiltinMemberDecoration(&builtin, structure, i))
            {

               if (builtin == spv::BuiltIn::Position)
               {
                  SymbolTypeHandle  floatType = SymbolTypeHandle::Float();

                  auto glPosition = SymbolHandle::Builtin(m_module, "gl_Position",
                                                          spv::StorageClass::Output, SymbolTypeHandle::Vector(floatType, 4));
                  const DflowScalars scalars  = LoadFromSymbol(exitBlock, m_glPerVertex);
                  const DflowScalars position = scalars.Slice(offset, 4);

                  StoreToSymbol(exitBlock, glPosition, position);
                  RecordOutputSymbol(glPosition);
               }
               else if (builtin == spv::BuiltIn::PointSize)
               {
                  SymbolTypeHandle  floatType = SymbolTypeHandle::Float();

                  auto glPointSize = SymbolHandle::Builtin(m_module, "gl_PointSize",
                                                           spv::StorageClass::Output, floatType);
                  const DflowScalars scalars   = LoadFromSymbol(exitBlock, m_glPerVertex);
                  const DflowScalars pointSize = scalars.Slice(offset, 1);

                  StoreToSymbol(exitBlock, glPointSize, pointSize);
                  RecordOutputSymbol(glPointSize);

                  seenPointSize = true;
               }
            }
         }
      }
   }

   if (flavour == SHADER_VERTEX && !seenPointSize)
   {
      // We must always have a pointsize, so if it hasn't already turned up,
      // create a fresh one
      auto glPointSize = SymbolHandle::Builtin(m_module, "gl_PointSize",
                                               spv::StorageClass::Output, SymbolTypeHandle::Float());
      auto pointSize   = DflowScalars::ConstantFloat(*this, 1.0f);
      StoreToSymbol(exitBlock, glPointSize, pointSize);
      RecordOutputSymbol(glPointSize);
   }
}

DflowScalars DflowBuilder::CreateBuiltinInputDataflow(spv::BuiltIn builtin)
{
   DflowScalars   result(*this);

   switch (builtin)
   {
   ////////////////////////////////////////////////////////////////////////////
   // Graphics
   ////////////////////////////////////////////////////////////////////////////
   case spv::BuiltIn::Position      : // Special handling elsewhere
   case spv::BuiltIn::PointSize     : // Special handling elsewhere
      break;

   // These are all part of the Shader capability group which we must support
   case spv::BuiltIn::VertexId      :
   case spv::BuiltIn::VertexIndex   :
      result = DflowScalars::NullaryOp(*this, DATAFLOW_GET_VERTEX_ID);
      break;

   case spv::BuiltIn::InstanceId    :
   case spv::BuiltIn::InstanceIndex :
      result = DflowScalars::NullaryOp(*this, DATAFLOW_GET_INSTANCE_ID);
      // On v4 h/w, the instance id doesn't include the baseInstance, so add it back in
      result = result + DflowScalars::NullaryOp(*this, DATAFLOW_GET_BASE_INSTANCE);
      break;

   case spv::BuiltIn::FragCoord     :
      result = DflowScalars::NullaryOp(*this, { DATAFLOW_FRAG_GET_X,
                                                DATAFLOW_FRAG_GET_Y,
                                                DATAFLOW_FRAG_GET_Z,
                                                DATAFLOW_FRAG_GET_W });
      break;

   case spv::BuiltIn::PointCoord :
      result = DflowScalars::NullaryOp(*this, { DATAFLOW_GET_POINT_COORD_X,
                                                DATAFLOW_GET_POINT_COORD_Y });
      result[1] = Dflow::ConstantFloat(1.0f) - result[1];
      break;

   case spv::BuiltIn::FrontFacing :
      result = DflowScalars::NullaryOp(*this, DATAFLOW_FRAG_GET_FF);
      break;

   case spv::BuiltIn::HelperInvocation :
      result = DflowScalars::NullaryOp(*this, DATAFLOW_IS_HELPER);
      break;

   // Part of Geometry and Tessellation
   case spv::BuiltIn::PrimitiveId :
      // TODO
      assert(0);
      break;

   case spv::BuiltIn::InvocationId :
      result = DflowScalars::NullaryOp(*this, DATAFLOW_GET_INVOCATION_ID);
      break;

   // Geometry
   case spv::BuiltIn::Layer :
      // TODO
      assert(0);
      break;

   // Tessellation
   case spv::BuiltIn::TessLevelOuter :
   case spv::BuiltIn::TessLevelInner :
   case spv::BuiltIn::TessCoord      :
   case spv::BuiltIn::PatchVertices  :
      // TODO
      assert(0);
      break;

   // SampleRateShading
   case spv::BuiltIn::SampleId :
      result = DflowScalars::NullaryOp(*this, DATAFLOW_SAMPLE_ID);
      break;

   case spv::BuiltIn::SamplePosition :
      result = DflowScalars::NullaryOp(*this, { DATAFLOW_SAMPLE_POS_X, DATAFLOW_SAMPLE_POS_Y });
      break;

   case spv::BuiltIn::SampleMask :
      result = DflowScalars::NullaryOp(*this, DATAFLOW_SAMPLE_MASK);
      break;

   ////////////////////////////////////////////////////////////////////////////
   // Compute
   ////////////////////////////////////////////////////////////////////////////
   case spv::BuiltIn::NumWorkgroups :
      result = DflowScalars::NullaryOp(*this, { DATAFLOW_GET_NUMWORKGROUPS_X,
                                                DATAFLOW_GET_NUMWORKGROUPS_Y,
                                                DATAFLOW_GET_NUMWORKGROUPS_Z });
      break;

   case spv::BuiltIn::WorkgroupSize        :
      // Workgroupsize should be a constant, not a variable and should be specified
      // either in the execution mode, or overidden by a constant decorated with
      // WorkgroupSize
      assert(0);
      break;

   case spv::BuiltIn::WorkgroupId          :
   case spv::BuiltIn::LocalInvocationId    :
   case spv::BuiltIn::GlobalInvocationId   :
      // These symbols are set after the dflow has been constructed, so just put
      // some default values in here
      result = DflowScalars::ConstantUInt(*this, 0, 3);
      break;

   case spv::BuiltIn::LocalInvocationIndex :
      // This symbol is set after the dflow has been constructed, so just put
      // a default value in here
      result = DflowScalars::ConstantUInt(*this, 0, 1);
      break;

   case spv::BuiltIn::ClipDistance              : // We don't support ClipDistance
   case spv::BuiltIn::CullDistance              : // We don't support CullDistance
   case spv::BuiltIn::ViewportIndex             : // We don't support MultiViewport
   case spv::BuiltIn::FragDepth                 : // This is an output, not input
   case spv::BuiltIn::WorkDim                   : // These are OpenCL kernel (not supported)
   case spv::BuiltIn::GlobalSize                :
   case spv::BuiltIn::EnqueuedWorkgroupSize     :
   case spv::BuiltIn::GlobalOffset              :
   case spv::BuiltIn::GlobalLinearId            :
   case spv::BuiltIn::SubgroupSize              :
   case spv::BuiltIn::SubgroupMaxSize           :
   case spv::BuiltIn::NumSubgroups              :
   case spv::BuiltIn::NumEnqueuedSubgroups      :
   case spv::BuiltIn::SubgroupId                :
   case spv::BuiltIn::SubgroupLocalInvocationId :
   default                                  :
      unreachable();
      break;
   }

   return result;
}

DflowScalars DflowBuilder::CreateVariableDataflow(const NodeVariable *var, SymbolTypeHandle type)
{
   DflowScalars      result(*this);
   spv::StorageClass stClass = var->GetStorageClass();

   const Optional<NodeIndex> &init = var->GetInitializer();
   DflowScalars               initScalars(*this);

   if (init.IsValid())
      initScalars = GetDataflow(init.Get());

   switch (stClass)
   {
   case spv::StorageClass::Input:
      {
         // Input variables from API or previous stage
         spv::BuiltIn builtin;
         if (m_module.GetBuiltinDecoration(&builtin, var))
            result = CreateBuiltinInputDataflow(builtin);
         else
            result = DflowScalars::In(*this, type, m_curInputRow);

         if (result.Size() > 0)
            RecordInputSymbol(GetVariableSymbol(var));
      }
      break;

   case spv::StorageClass::Output:
      result = DflowScalars::Initialize(type, initScalars);
      RecordOutputSymbol(GetVariableSymbol(var));
      break;

   case spv::StorageClass::Private:
      result = DflowScalars::Initialize(type, initScalars);
      break;

   case spv::StorageClass::UniformConstant:
      {
         SymbolHandle symbol = GetVariableSymbol(var);
         int *ids = static_cast<int *>(malloc_fast(symbol.GetNumScalars() * sizeof(int)));

         uint32_t descriptorSet = m_module.RequireLiteralDecoration(spv::Decoration::DescriptorSet, var);
         uint32_t binding       = m_module.RequireLiteralDecoration(spv::Decoration::Binding, var);
         result = DflowScalars::ImageSampler(*this, type, descriptorSet, binding, ids);

         RecordUniformSymbol(GetVariableSymbol(var), ids);
      }
      break;

   case spv::StorageClass::Uniform:
   case spv::StorageClass::StorageBuffer:
      // UBOs & SSBOs
      break;

   case spv::StorageClass::Function:
      // Function local variables
      result = DflowScalars::Initialize(type, initScalars);
      break;

   case spv::StorageClass::PushConstant:
      break;

   case spv::StorageClass::Workgroup:
      m_sharedSymbols.push_back(GetVariableSymbol(var));
      break;

   case spv::StorageClass::Image:
      break;

   case spv::StorageClass::CrossWorkgroup:
   case spv::StorageClass::AtomicCounter:
   case spv::StorageClass::Generic:
   default:
      assert(0);
      break;
   }

   return result;
}

static bool IsGLPervertexStruct(const NodeTypeStruct *node)
{
   auto &memberDecorations = node->GetData()->GetMemberDecorations();

   for (auto &memberDecoration : memberDecorations)
   {
      const Decoration *decoration = memberDecoration.second;

      if (decoration->Is(spv::Decoration::BuiltIn))
      {
         spv::BuiltIn bi = decoration->GetBuiltIn();

         if (bi == spv::BuiltIn::Position || bi == spv::BuiltIn::PointSize)
            return true;
      }
   }

   return false;
}

SymbolTypeHandle DflowBuilder::GetSymbolType(const NodeVariable *var) const
{
   return GetSymbolType(var->TypeOfTarget());
}

SymbolHandle DflowBuilder::AddSymbol(const NodeVariable *var)
{
   SymbolHandle      symbol;
   const NodeType   *nodeType = var->TypeOfTarget();
   SymbolTypeHandle  type     = GetSymbolType(var);

   if (type.GetFlavour() == SYMBOL_STRUCT_TYPE)
   {
      auto  structType = nodeType->As<const NodeTypeStruct *>();

      if (IsGLPervertexStruct(structType))
      {
         symbol = SymbolHandle::Variable(m_module, m_flavour, "$$gl_PerVertex", var, type);
         // This symbol needs special treatment as the linker is looking for
         // gl_Position and gl_PointSize, so record it for later (see SetupInterface)
         m_glPerVertex = symbol;
      }
   }

   if (m_flavour == SHADER_COMPUTE)
   {
      // Special compute builtins are handled post facto (see Compiler::Compile)
      spv::BuiltIn  builtin;
      if (m_module.GetBuiltinDecoration(&builtin, var))
      {
         switch (builtin)
         {
         case spv::BuiltIn::WorkgroupId          : symbol = m_glWorkGroupID;          break;
         case spv::BuiltIn::LocalInvocationId    : symbol = m_glLocalInvocationID;    break;
         case spv::BuiltIn::GlobalInvocationId   : symbol = m_glGlobalInvocationID;   break;
         case spv::BuiltIn::LocalInvocationIndex : symbol = m_glLocalInvocationIndex; break;
         default : break;
         }
      }
   }

   // If it's not a special symbol, create a variable symbol for it
   if (symbol == nullptr)
      symbol = SymbolHandle::Variable(m_module, m_flavour, m_module.GetName(var)->c_str(), var, type);

   SetVariableSymbol(var, symbol);

   return symbol;
}

void DflowBuilder::Visit(const NodeVariable *var)
{
   SymbolHandle symbol  = AddSymbol(var);
   DflowScalars scalars = CreateVariableDataflow(var, symbol.GetType());

   // No scalars implies e.g. it is a buffer with no internal
   // representation, only external memory.
   if (scalars.Size() > 0)
      // The back-end requires that all symbols are defined in the start block
      StoreToSymbol(m_entryBlock, symbol, scalars);
}

bool DflowBuilder::HasStaticUse(const SymbolHandle sym) const
{
   return m_usedSymbols.find(sym) != m_usedSymbols.end();
}

void DflowBuilder::Visit(const NodeFunctionCall *functionCall)
{
   auto function = functionCall->GetFunction()->As<const NodeFunction *>();

   CallFunction(function, functionCall);
}

void DflowBuilder::CallFunction(const NodeFunction *function, const NodeFunctionCall *functionCall)
{
   m_functionStack.EnterFunction(function, functionCall);

   // If this is "main", then record the return block for use by NodeUnreachable
   if (functionCall == nullptr)
      m_exitBlock = m_functionStack->GetReturnBlock();

   // Get the blocks for the function (each block begins with a label)
   const spv::vector<const NodeLabel *> &blocks = function->GetData()->GetBlocks();

   // Then fill in their instructions
   for (const NodeLabel *label : blocks)
   {
      auto &instructions = label->GetData()->GetInstructions();

      for (const Node *instruction : instructions)
         instruction->Accept(*this);

      m_functionStack->NextBlock();
   }

   m_functionStack.ExitFunction();
}

void DflowBuilder::Visit(const NodeUnreachable *)
{
   // Must be the last instruction in the block.
   BasicBlockHandle current = m_functionStack->GetCurrentBlock();
   current->SetFallthroughTarget(m_exitBlock);
}

void DflowBuilder::Visit(const NodeReturn *)
{
   // Must be the last instruction in the *block* (guaranteed by SPIRV spec).
   // Jump to return block
   m_functionStack.Return(nullptr);
}

void DflowBuilder::Visit(const NodeReturnValue *node)
{
   const Node *result = node->GetValue();
   // Must be the last instruction in the *block* (guaranteed by SPIRV spec).
   /// Jump to return block having set the return variable value
   const DflowScalars &returnValue = GetDataflow(result);
   m_functionStack.Return(&returnValue);
}

bool DflowBuilder::ConstantInt(const Node *node, uint32_t *value)
{
   const DflowScalars &scalars = GetDataflow(node);

   if (scalars.Size() == 1)
   {
      const Dflow &dflow = scalars[0];

      if (!dflow.IsNull() && dflow.GetFlavour() == DATAFLOW_CONST)
      {
         DataflowType type = dflow.GetType();

         if (type == DF_INT || type == DF_UINT)
         {
            if (value != nullptr)
               *value = dflow.GetConstantInt();
            return true;
         }
      }
   }

   if (value != nullptr)
      *value = 0;

   return false;
}

uint32_t DflowBuilder::RequireConstantInt(const Node *node)
{
   uint32_t ret;
   bool ok = ConstantInt(node, &ret);
   assert(ok);
   return ret;
}

static spv::ExecutionModel ConvertExecutionModel(ShaderFlavour flavour)
{
   switch (flavour)
   {
   case SHADER_VERTEX          : return spv::ExecutionModel::Vertex;
   case SHADER_TESS_CONTROL    : return spv::ExecutionModel::TessellationControl;
   case SHADER_TESS_EVALUATION : return spv::ExecutionModel::TessellationEvaluation;
   case SHADER_GEOMETRY        : return spv::ExecutionModel::Geometry;
   case SHADER_FRAGMENT        : return spv::ExecutionModel::Fragment;
   case SHADER_COMPUTE         : return spv::ExecutionModel::GLCompute;
   default                     : unreachable();
   }
}

SymbolHandle DflowBuilder::CreateInternal(const char *name, const NodeType *type)
{
   SymbolTypeHandle  symbolType = GetSymbolType(type);
   SymbolHandle      symbol;

   if (symbolType.GetNumScalars() != 0)
   {
      symbol = SymbolHandle::Internal(m_module, name, symbolType);

      DflowScalars defaults = DflowScalars::Default(*this, symbolType);

      StoreToSymbol(m_entryBlock, symbol, defaults);
   }

   return symbol;
}

SymbolHandle DflowBuilder::GetParameterSymbol(const NodeFunctionParameter *node) const
{
   return m_functionStack->GetParameterSymbol(node);
}

void DflowBuilder::RecordExecutionModes(const NodeEntryPoint *entry)
{
   auto &modes = entry->GetData()->GetModes();

   for (const NodeExecutionMode *mode : modes)
      m_executionModes.Record(mode);
}

void DflowBuilder::InitCompute()
{
   m_workgroup = SymbolHandle::SharedBlock(GetSharedSymbols());

   glsl_generate_compute_variables(
      m_glLocalInvocationIndex, m_glLocalInvocationID, m_glWorkGroupID,
      m_glGlobalInvocationID, m_entryBlock->GetBlock(), m_executionModes.GetWorkgroupSize().data(),
      m_workgroup, /*TODO multicore=*/false, Options::autoclifEnabled,
      v3d_scheduler_get_compute_shared_mem_size_per_core());
}

Dflow DflowBuilder::WorkgroupAddress()
{
   DflowScalars   addr = LoadFromSymbol(m_functionStack->GetCurrentBlock(), m_workgroup);
   return addr[0];
}

void DflowBuilder::Build(const char *name, ShaderFlavour flavour)
{
   const NodeEntryPoint *entry = m_module.GetEntryPoint(name, ConvertExecutionModel(flavour));
   if (entry == nullptr)
      throw std::runtime_error(std::string("Failed to find entry point '") + name + "'");

   RecordExecutionModes(entry);

   const NodeFunction *function = m_module.GetEntryPointFunction(entry);
   if (function == nullptr)
      throw std::runtime_error(std::string("Failed to find entry point function for '") + name + "'");

   m_entryBlock = m_functionStack->GetCurrentBlock();
   m_flavour    = flavour;

   // Create special symbols
   if (flavour == SHADER_FRAGMENT)
   {
      // The linker looks for the $$discard variable
      m_discard = SymbolHandle::Builtin(m_module, "$$discard", spv::StorageClass::Output, SymbolTypeHandle::Bool());
      StoreToSymbol(m_entryBlock, m_discard, DflowScalars::ConstantBool(*this, false));
      RecordOutputSymbol(m_discard);
   }

   if (flavour == SHADER_COMPUTE)
   {
      SymbolTypeHandle  uintType  = SymbolTypeHandle::UInt();
      SymbolTypeHandle  uvec3Type = SymbolTypeHandle::Vector(uintType, 3);

      // The values associated with these variables are added post-facto
      m_glWorkGroupID          = SymbolHandle::Builtin(m_module, "gl_WorkGroupID",          spv::StorageClass::Input, uvec3Type);
      m_glLocalInvocationID    = SymbolHandle::Builtin(m_module, "gl_LocalInvocationID",    spv::StorageClass::Input, uvec3Type);
      m_glGlobalInvocationID   = SymbolHandle::Builtin(m_module, "gl_GlobalInvocationID",   spv::StorageClass::Input, uvec3Type);
      m_glLocalInvocationIndex = SymbolHandle::Builtin(m_module, "gl_LocalInvocationIndex", spv::StorageClass::Input, uintType);

      SymbolHandle comp_vary = SymbolHandle::Builtin(m_module, "$$comp_vary", spv::StorageClass::Input, uvec3Type);
      RecordInputSymbol(comp_vary);
   }

   // Create types and constants and global variables
   for (const Node *global : m_module.GetGlobals())
      global->Accept(*this);

   if (flavour == SHADER_COMPUTE)
      InitCompute();

   // Enter "main"
   CallFunction(function, nullptr);

   // The back-end does not currently handle "breaks" on the true-branch of a conditional
   // so this swaps these round to false branches and flips the condition.
   PatchConditionals();

   // Set up symbols for the interfaces
   SetupInterface(entry, flavour);

   DebugPrint();

   // Everything has built ok, so keep the blocks that will
   // be used downstream
   m_basicBlockPool.RetainReachableBlocks(m_entryBlock->GetBlock());
}

void DflowBuilder::DebugPrint() const
{
   if (log_trace_enabled())
   {
      m_module.DebugPrint();

      log_trace("============== DATAFLOW");
      for (auto &df : m_dataflow)
         df.DebugPrint();

      log_trace("============== INPUT SYMBOLS");
      for (auto sym : m_inputSymbols)
         SymbolHandle(sym).DebugPrint();

      log_trace("============== OUTPUT SYMBOLS");
      for (auto sym : m_outputSymbols)
         SymbolHandle(sym).DebugPrint();

      log_trace("============== UNIFORM SYMBOLS");
      for (auto sym : m_uniformSymbols)
         SymbolHandle(sym).DebugPrint();

      log_trace("============== SHARED SYMBOLS");
      for (auto sym : m_sharedSymbols)
         SymbolHandle(sym).DebugPrint();
   }
}

} // namespace bvk
