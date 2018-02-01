/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Module.h"
#include "Nodes.h"
#include "GatherNodes.h"
#include "TypeBuilder.h"
#include "Extractor.h"
#include "Options.h"

#include "libs/util/log/log.h"

LOG_DEFAULT_CAT("bvk::comp::Module");

namespace bvk
{

static uint32_t SwapBytes(uint32_t from)
{
   return ((from & 0xff000000) >> 24) |
          ((from & 0x00ff0000) >> 8 ) |
          ((from & 0x0000ff00) << 8 ) |
          ((from & 0x000000ff) << 24);
}

std::vector<uint32_t> SwapArray(uint32_t sizeInWords, const uint32_t *ptr)
{
   std::vector<uint32_t> result(sizeInWords);

   for (uint32_t &i : result)
      i = SwapBytes(*ptr++);

   return result;
}

void DecorationVisitor::Foreach(const Node *node)
{
   const std::list<const Decoration *> &decoNodes = m_module.GetDecorations(node);

   for (const Decoration *decoration : decoNodes)
      Visit(*decoration);
}

///////////////////////////////////////////////////////////////////////////////
// ModuleInfo
//
// Gathers counts of the various operation types in a SPIRV file
///////////////////////////////////////////////////////////////////////////////
class ModuleInfo
{
public:
   ModuleInfo(const uint32_t *spirv, const uint32_t *end);

   uint32_t m_nodeCount       = 0;
   uint32_t m_maxId           = 0;
   uint32_t m_capabilityCount = 0;
   uint32_t m_extensionCount  = 0;
   uint32_t m_extImportCount  = 0;
   uint32_t m_constantCount   = 0;
   uint32_t m_typeCount       = 0;
   uint32_t m_variableCount   = 0;
   uint32_t m_globalCount     = 0;
   uint32_t m_functionCount   = 0;
   uint32_t m_entryPointCount = 0;

   bool     m_seenFunction    = false;

private:
   void  AddResultId(uint32_t id);
   void  AddOp(spv::Core op);
};

#include "NodeParse.auto.inc"

void ModuleInfo::AddResultId(uint32_t id)
{
   m_maxId = std::max(m_maxId, id);
}

void ModuleInfo::AddOp(spv::Core op)
{
   m_nodeCount++;

   switch (op)
   {
   case spv::Core::OpCapability :
      m_capabilityCount++;
      break;

   case spv::Core::OpExtension :
      m_extensionCount++;
      break;

   case spv::Core::OpExtInst :
      m_extImportCount++;
      break;

   case spv::Core::OpVariable :
      m_variableCount++;
      if (!m_seenFunction)
         m_globalCount++;
      break;

   case spv::Core::OpFunction :
      m_functionCount++;
      m_seenFunction = true;
      break;

   case spv::Core::OpEntryPoint :
      m_entryPointCount++;
      break;

   // Constants
   case spv::Core::OpConstant :
   case spv::Core::OpConstantTrue :
   case spv::Core::OpConstantFalse :
   case spv::Core::OpConstantNull :
   case spv::Core::OpConstantComposite :
   case spv::Core::OpSpecConstantTrue :
   case spv::Core::OpSpecConstantFalse :
   case spv::Core::OpSpecConstant :
   case spv::Core::OpSpecConstantComposite :
      m_constantCount++;
      m_globalCount++;
      break;

   // Types
   case spv::Core::OpTypeVoid :
   case spv::Core::OpTypeBool :
   case spv::Core::OpTypeInt :
   case spv::Core::OpTypeFloat :
   case spv::Core::OpTypeVector :
   case spv::Core::OpTypeMatrix :
   case spv::Core::OpTypeImage :
   case spv::Core::OpTypeSampler :
   case spv::Core::OpTypeSampledImage :
   case spv::Core::OpTypeArray :
   case spv::Core::OpTypeRuntimeArray :
   case spv::Core::OpTypeStruct :
   case spv::Core::OpTypePointer :
   case spv::Core::OpTypeFunction :
      m_typeCount++;
      m_globalCount++;
      break;

   default:
      break;
   }
}

ModuleInfo::ModuleInfo(const uint32_t *spirv, const uint32_t *end)
{
   // Skip header
   spirv += spv::con::HeaderWordCount;

   while (spirv < end)
   {
      uint32_t  instr = spirv[0];
      uint16_t  wc    = instr >> spv::con::WordCountShift;
      spv::Core op    = static_cast<spv::Core>(instr & spv::con::OpCodeMask);
      uint32_t  index = IndexOfResultId(op);

      if (index > 0)
         AddResultId(spirv[index]);

      AddOp(op);

      spirv += wc;
   }
}

///////////////////////////////////////////////////////////////////////////////
// Module
///////////////////////////////////////////////////////////////////////////////
ShaderFlavour ConvertModel(spv::ExecutionModel model)
{
   switch (model)
   {
   case spv::ExecutionModel::Vertex:                   return SHADER_VERTEX;
   case spv::ExecutionModel::TessellationControl:      return SHADER_TESS_CONTROL;
   case spv::ExecutionModel::TessellationEvaluation:   return SHADER_TESS_EVALUATION;
   case spv::ExecutionModel::Geometry:                 return SHADER_GEOMETRY;
   case spv::ExecutionModel::Fragment:                 return SHADER_FRAGMENT;
   case spv::ExecutionModel::GLCompute:                return SHADER_COMPUTE;
   case spv::ExecutionModel::Kernel:
   default:
      unreachable();
      return SHADER_FLAVOUR_COUNT;
   }
}

static void DumpSPIRV(const uint32_t *spirvCode, uint32_t sizeInBytes)
{
   static uint32_t fileCount = 0;
   char            name[128];

   sprintf(name, "shader_%04u.spv", fileCount);
   std::cerr << "Saving " << name << std::endl;

   FILE *fp = fopen(name, "wb");
   if (fp != nullptr)
   {
      fwrite(reinterpret_cast<const uint8_t*>(spirvCode), 1, sizeInBytes, fp);
      fclose(fp);
   }

   fileCount++;
}

Module::Module(const VkAllocationCallbacks *cbs, const uint32_t *code, uint32_t sizeInBytes) :
   Allocating(cbs),
   m_arena(cbs),
   m_arenaAllocator(&m_arena),
   m_allNodes(m_arenaAllocator),
   m_results(m_arenaAllocator),
   m_capabilities(m_arenaAllocator),
   m_extensions(m_arenaAllocator),
   m_extImports(m_arenaAllocator),
   m_variables(m_arenaAllocator),
   m_globals(m_arenaAllocator),
   m_functions(m_arenaAllocator),
   m_entryPoints(m_arenaAllocator),
   m_names(m_arenaAllocator),
   m_decorations(m_arenaAllocator)
{
   assert((sizeInBytes & 3) == 0); // Must be multiple of 4

   uint32_t  sizeInWords  = sizeInBytes / 4;
   uint32_t  magic        = *code;
   bool      littleEndian = magic == spv::con::MagicNumber;
   bool      bigEndian    = SwapBytes(magic) == spv::con::MagicNumber;

   assert(littleEndian || bigEndian);

   std::vector<uint32_t>  swappedCode;

   if (bigEndian)
      swappedCode = SwapArray(sizeInWords, code);

   const uint32_t *spirvCode = littleEndian ? code : swappedCode.data();

   if (Options::dumpSPIRV)
      DumpSPIRV(spirvCode, sizeInBytes);

   const uint32_t *end = spirvCode + sizeInWords;
   const uint32_t *ptr = spirvCode;

   // Preparse to gather size information
   ModuleInfo  info(ptr, end);
   AllocateArrays(info);

   // Main parse
   ptr = ParseHeader(ptr);
   while (ptr < end)
      ptr = ParseInstruction(ptr);

   // Fill out the lists of vars, types, functions etc. held in this module
   GatherNodes::Gather(*this);

   // SPIR-V files don't have to name any symbols. Make sure we have names where needed.
   PopulateNames();
}

Module::~Module()
{
   m_arenaAllocator.LogMemoryUsage("=== Module ===");
}

const uint32_t *Module::ParseHeader(const uint32_t *instr)
{
   m_idBound = instr[3];

   return &instr[spv::con::HeaderWordCount];
}

const uint32_t *Module::ParseInstruction(const uint32_t *instr)
{
   Extractor  extr(*this, instr);
   Node      *node = nullptr;

   #include "NodeFactory.auto.inc"

   if (node != nullptr)
      AddNode(node);
   else
   {
      // Didn't find an auto-gen match for the op, make a dummy one
      log_warn("Found an unimplemented or unsupported instruction (%u)\n", static_cast<uint32_t>(extr.GetOpCode()));
      AddNode(New<NodeDummy>(extr));
   }

   return instr + extr.GetWordCount();
}

void Module::AllocateArrays(const ModuleInfo &info)
{
   uint32_t size = info.m_maxId + 1;

   m_allNodes.reserve(info.m_nodeCount);
   m_results.resize(size);
   m_names.resize(size);
   m_decorations.resize(size);
   m_variables.reserve(info.m_variableCount);
   m_globals.reserve(info.m_globalCount),
   m_functions.reserve(info.m_functionCount);
   m_entryPoints.reserve(info.m_entryPointCount);
}

void Module::PopulateNames()
{
   for (const NodeVariable *v : m_variables)
   {
      uint32_t nodeId = v->GetResultId();

      if (m_names[nodeId] == nullptr)
      {
         char     name[24];

         if (GetBuiltinDecoration(nullptr, v))
            sprintf(name, "gl_%%%u", nodeId);   // Backend needs to see these as builtins
         else
            sprintf(name, "%%%u", nodeId);

         m_names[nodeId] = New<spv::string>(name, m_arenaAllocator);
      }
   }
}

void Module::AddLabel(const NodeLabel *node)
{
   // Assign an id to the label
   node->GetData()->SetBlockId(m_blockCount++);

   // Add the label to the current function
   m_functions.back()->GetData()->StartBlock(node);

   AddInstruction(node);
}

uint32_t Module::AddFunction(const NodeFunction *function)
{
   uint32_t id = m_functions.size();
   m_functions.push_back(function);
   function->GetData()->SetId(id);
   return id;
}

void Module::AddParameter(const NodeFunctionParameter *parameter)
{
   const NodeFunction *function = m_functions.back();

   function->GetData()->AddParameter(parameter);
}

void Module::AddInstruction(const Node *node)
{
   const NodeFunction *function = m_functions.back();

   function->GetData()->AddInstruction(node);
}

void Module::AddNode(Node *node)
{
   uint32_t id = node->GetResultId();

   if (id != 0)
      m_results[id] = node;

   m_allNodes.push_back(node);
}

uint32_t Module::AddEntryPoint(const NodeEntryPoint *entryPoint)
{
   uint32_t id = m_entryPoints.size();
   m_entryPoints.push_back(entryPoint);
   entryPoint->GetData()->SetId(id);
   return id;
}

const NodeEntryPoint *Module::GetEntryPoint(const char *name, spv::ExecutionModel model, uint32_t *index) const
{
   uint32_t ix = 0;

   for (const NodeEntryPoint *ep : m_entryPoints)
   {
      if (!strcmp(ep->GetName().c_str(), name) && ep->GetExecutionModel() == model)
      {
         if (index != nullptr)
            *index = ix;

         return ep;
      }

      ++ix;
   }

   return nullptr;
}

const NodeEntryPoint *Module::GetEntryPoint(uint32_t entryPointId, uint32_t *index)
{
   uint32_t ix = 0;

   for (const NodeEntryPoint *ep : m_entryPoints)
   {
      if (ep->GetEntryPoint()->GetResultId() == entryPointId)
      {
         if (index != nullptr)
            *index = ix;

         return ep;
      }

      ++ix;
   }

   return nullptr;
}

void Module::AddExecutionMode(const NodeExecutionMode *mode) // uint32_t entryPointId, const NodeExecutionMode *mode)
{
   const NodeEntryPoint *entryPoint = GetEntryPoint(mode->GetEntryPoint()->GetResultId());

   entryPoint->GetData()->AddMode(mode);
}

void Module::SetName(const NodeName *node)
{
   m_names[node->GetTarget()->GetResultId()] = &node->GetName();
}

const NodeFunction *Module::GetEntryPointFunction(const NodeEntryPoint *entryPoint) const
{
   return m_results[entryPoint->GetEntryPoint()->GetResultId()]->As<const NodeFunction *>();
}

bool Module::GetVarLocation(int *loc, const NodeVariable *var) const
{
   auto &decorations = GetDecorations(var);

   for (const Decoration *decoration : decorations)
   {
      if (decoration->Is(spv::Decoration::Location))
      {
         *loc = static_cast<int>(decoration->GetLiteral());
         return true;
      }
   }

   return false;
}

bool Module::GetBuiltinDecoration(spv::BuiltIn *builtin, const Node *node) const
{
   auto &decorations = GetDecorations(node);

   for (const Decoration *decoration : decorations)
   {
      if (decoration->Is(spv::Decoration::BuiltIn))
      {
         if (builtin != nullptr)
            *builtin = decoration->GetBuiltIn();
         return true;
      }
   }

   return false;
}

bool Module::GetBuiltinMemberDecoration(spv::BuiltIn *builtin, const NodeTypeStruct *node,
                                        uint32_t memberIndex) const
{
   auto &memberDecorations = node->GetData()->GetMemberDecorations();

   for (auto &memberDecoration : memberDecorations)
   {
      const Decoration *decoration = memberDecoration.second;

      if (memberDecoration.first == memberIndex && decoration->Is(spv::Decoration::BuiltIn))
      {
         if (builtin != nullptr)
            *builtin = decoration->GetBuiltIn();
         return true;
      }
   }

   return false;
}

bool Module::GetLiteralDecoration(uint32_t *literal, spv::Decoration decoType, const Node *node) const
{
   auto &decorations = GetDecorations(node);

   for (const Decoration *decoration : decorations)
   {
      if (decoration->Is(decoType))
      {
         *literal = decoration->GetLiteral();
         return true;
      }
   }

   return false;
}

uint32_t Module::RequireLiteralDecoration(spv::Decoration dec, const Node *n) const
{
   uint32_t l = 0;      /* Value never used */
   bool ok = GetLiteralDecoration(&l, dec, n);
   assert(ok);
   return l;
}

void Module::AddDecoration(const NodeDecorate *node)
{
   m_decorations[node->GetTarget()->GetResultId()].push_back(&node->GetDecoration());
}

void Module::AddGroupDecoration(const NodeGroupDecorate *node)
{
   const Node *decorationGroup = node->GetDecorationGroup();
   auto &targets               = node->GetTargets();

   for (const NodeIndex &target : targets)
   {
      uint32_t  targetId    = target->GetResultId();
      auto     &decorations = GetDecorations(decorationGroup);

      for (const Decoration *decoration : decorations)
         m_decorations[targetId].push_back(decoration);
   }
}

void Module::AddDecorationGroup(const NodeDecorationGroup *group)
{
   // Decoration groups are targets for decorations that preceed them
   // We don't need to do anything
}

void Module::AddMemberDecoration(const NodeMemberDecorate *node)
{
   auto structure = node->GetStructureType()->As<const NodeTypeStruct *>();

   structure->GetData()->SetMemberDecoration(node->GetMember(), &node->GetDecoration());
}

void Module::AddGroupMemberDecoration(const NodeGroupMemberDecorate *decorate)
{
   const NodeIndex &decorationGroup = decorate->GetDecorationGroup();
   auto &targets                    = decorate->GetTargets();

   for (auto &target : targets)
   {
      NodeIndex targetIx = target.first;
      uint32_t  memberIx = target.second;

      auto  structure   = targetIx->As<const NodeTypeStruct *>();
      auto &decorations = GetDecorations(decorationGroup);

      for (const Decoration *decoration : decorations)
         structure->GetData()->SetMemberDecoration(memberIx, decoration);
   }
}

bool Module::GetLiteralMemberDecoration(uint32_t *literal, spv::Decoration decoType,
                                        const NodeTypeStruct *node, uint32_t memberIndex) const
{
   auto &memberDecorations = node->GetData()->GetMemberDecorations();

   for (auto &memberDecoration : memberDecorations)
   {
      const Decoration *decoration = memberDecoration.second;
      if (memberDecoration.first == memberIndex && decoration->Is(decoType))
      {
         *literal = decoration->GetLiteral();
         return true;
      }
   }

   return false;
}

uint32_t Module::RequireLiteralMemberDecoration(spv::Decoration decoType,
                                                const NodeTypeStruct *node, uint32_t memberIndex) const
{
   uint32_t ret = 0; // Value never used
   bool ok = GetLiteralMemberDecoration(&ret, decoType, node, memberIndex);
   assert(ok);
   return ret;
}

bool Module::HasDecoration(spv::Decoration decoType, const Node *node) const
{
   for (const Decoration *decoration : GetDecorations(node))
   {
      if (decoration->Is(decoType))
         return true;
   }

   return false;
}

bool Module::HasMemberDecoration(spv::Decoration decoType, const NodeTypeStruct *node,
                                 uint32_t memberIndex) const
{
   auto &memberDecorations = node->GetData()->GetMemberDecorations();

   for (auto &memberDecoration : memberDecorations)
   {
      uint32_t          index      = memberDecoration.first;
      const Decoration *decoration = memberDecoration.second;

      if (index == memberIndex && decoration->Is(decoType))
         return true;
   }

   return false;
}

void Module::DebugPrint() const
{
}

} // namespace bvk
