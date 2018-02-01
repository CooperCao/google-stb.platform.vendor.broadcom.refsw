/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Spirv.h"

#include <vector>
#include <list>
#include <map>
#include <stdint.h>
#include <assert.h>
#include <scoped_allocator>

#include "Nodes.h"
#include "Compiler.h"
#include "DflowBuilder.h"
#include "NodeIndex.h"
#include "Allocating.h"
#include "SysMemCmdBlock.h"
#include "ArenaAllocator.h"
#include "ModuleAllocator.h"

#include "glsl_symbols.h"
#include "glsl_ir_program.h"
#include "glsl_binary_shader.h"

namespace bvk
{

class ModuleInfo;

// TODO move to separate file?
class DecorationVisitor
{
public:
   DecorationVisitor(const Module &module) :
      m_module(module)
   {}

   virtual ~DecorationVisitor() {}
   virtual void Visit(const Decoration &d) = 0;

   void Foreach(const Node *node);

private:
   const Module &m_module;
};

///////////////////////////////////////////////////////////////////////////////
// Module
//
// Responsible for initial parse of the SPIRV binary.
// Holds our own internal representation of the SPIRV graph (as Nodes)
///////////////////////////////////////////////////////////////////////////////
class Module : public Allocating
{
public:
   Module(const VkAllocationCallbacks *cbs, const uint32_t *spirvCode, uint32_t sizeInBytes);
   ~Module();

   const spv::ModuleAllocator<uint32_t> &GetArenaAllocator() const { return m_arenaAllocator; }
   spv::ModuleAllocator<uint32_t> &GetArenaAllocator() { return m_arenaAllocator; }

   uint32_t IdBound() const { return m_idBound; }

   void AddCapability(const NodeCapability *capability)   { m_capabilities.push_back(capability); }
   void AddExtension(const NodeExtension *extension)      { m_extensions.push_back(extension);    }
   void AddExtInstImport(const NodeExtInstImport *import) { m_extImports.push_back(import);       }
   void AddDecoration(const NodeDecorate *decorate);
   void AddGroupDecoration(const NodeGroupDecorate *decorate);
   void AddDecorationGroup(const NodeDecorationGroup *group);
   void AddGroupMemberDecoration(const NodeGroupMemberDecorate *decorate);

   void AddExecutionMode(const NodeExecutionMode *mode);
   void AddParameter(const NodeFunctionParameter *parameter);
   void AddInstruction(const Node *node);
   void AddMemberDecoration(const NodeMemberDecorate *node);

   uint32_t AddFunction(const NodeFunction *function);
   uint32_t AddEntryPoint(const NodeEntryPoint *entryPoint);

   void AddVariable(const NodeVariable *node)
   {
      uint32_t id = m_variables.size();
      m_variables.push_back(node);
      node->GetData()->SetId(id);

      if (node->GetStorageClass() != spv::StorageClass::Function)
         m_globals.push_back(node);
      else
         AddInstruction(node);
   }

   void AddType(const NodeType *type)
   {
      uint32_t id = m_nTypes++;
      type->SetTypeId(id);

      m_globals.push_back(type);
   }

   void AddConstant(const Node *node)
   {
      m_globals.push_back(node);
   }

   void AddLabel(const NodeLabel *node);
   void SetName(const NodeName *node);

   void SetMemoryModel(const NodeMemoryModel *memoryModel)  { m_memoryModel = memoryModel; }
   void SetSource(const NodeSource *source)                 { m_source = source;           }

   const Node **GetNodePtr(uint32_t i)                      { return &m_results[i];        }
   const Node  *GetNode(uint32_t i)                         { return m_results[i];         }

   const NodeFunction *GetEntryPointFunction(const NodeEntryPoint *entryPoint) const;

   const spv::vector<const Node *> &GetGlobals() const { return m_globals;   }

   const spv::string  *GetName(const Node *node) const { return m_names[node->GetResultId()]; }

   const std::list<const Decoration *> &GetDecorations(const Node *node) const
   {
      return m_decorations[node->GetResultId()];
   }

   uint32_t GetNumEntryPoints() const { return m_entryPoints.size(); }

   const spv::string &GetEntryPointName(uint32_t epIndex) const
   {
      assert(epIndex < m_entryPoints.size());
      return m_entryPoints[epIndex]->GetName();
   }

   spv::ExecutionModel GetEntryPointModel(uint32_t epIndex) const
   {
      assert(epIndex < m_entryPoints.size());
      return m_entryPoints[epIndex]->GetExecutionModel();
   }

   void DebugPrint() const;

   bool GetVarLocation(int *loc, const NodeVariable *var) const;

   // TODO: maybe hive off some of this decoration stuff to another object
   bool     GetBuiltinDecoration(spv::BuiltIn *builtin, const Node *node) const;
   bool     GetBuiltinMemberDecoration(spv::BuiltIn *builtin, const NodeTypeStruct *node, uint32_t memberIndex) const;
   bool     GetLiteralDecoration(uint32_t *literal, spv::Decoration decoType, const Node *node) const;
   uint32_t RequireLiteralDecoration(spv::Decoration decoType, const Node *node) const;
   bool     HasDecoration(spv::Decoration decoType, const Node *node) const;
   bool     HasMemberDecoration(spv::Decoration decoType, const NodeTypeStruct *node, uint32_t memberIndex) const;
   bool     GetLiteralMemberDecoration(uint32_t *literal, spv::Decoration decoType,
                                       const NodeTypeStruct *node, uint32_t memberIndex) const;
   uint32_t RequireLiteralMemberDecoration(spv::Decoration decoType, const NodeTypeStruct *node, uint32_t memberIndex) const;

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

   const NodeEntryPoint *GetEntryPoint(const char *name, spv::ExecutionModel model,
                                       uint32_t *index = nullptr) const;

   uint32_t                         GetNumBlocks() const { return m_blockCount; }
   const spv::vector<const Node *> &GetNodes()     const { return m_allNodes;   }
   uint32_t                         GetNumTypes()  const { return m_nTypes;     }

private:
   void AddNode(Node *node);

   const NodeEntryPoint *GetEntryPoint(uint32_t entryPointId, uint32_t *index = nullptr);

   void PopulateNames();
   void AllocateArrays(const ModuleInfo &info);

   const uint32_t *ParseHeader(const uint32_t *instr);
   const uint32_t *ParseInstruction(const uint32_t *instr);

private:
   ArenaAllocator<SysMemCmdBlock, void*>  m_arena;
   spv::ModuleAllocator<uint32_t>         m_arenaAllocator;
   uint32_t                               m_idBound;

   spv::vector<const Node *>              m_allNodes;
   spv::vector<const Node *>              m_results;

   // Type specific data
   spv::list<const NodeCapability *>      m_capabilities;
   spv::list<const NodeExtension *>       m_extensions;
   spv::list<const NodeExtInstImport *>   m_extImports;
   const NodeMemoryModel                 *m_memoryModel;
   const NodeSource                      *m_source;

   spv::vector<const NodeVariable *>      m_variables;      // List of all variables local and global
   spv::vector<const Node *>              m_globals;        // List of global constants, types and variables

   uint32_t                               m_nTypes = 0;

   spv::vector<const NodeFunction *>      m_functions;
   spv::vector<const NodeEntryPoint *>    m_entryPoints;    // List of entry points

   uint32_t                               m_blockCount = 0;

   spv::vector<const spv::string *>       m_names;          // Names applied to nodes

   // TODO : inner container won't be using the allocator
   spv::vector<std::list<const Decoration *>>
                                          m_decorations;    // Decorations applied to nodes
};

} // Namespace bvk
