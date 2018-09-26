/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Spirv.h"

#include <vector>
#include <list>
#include <stdint.h>
#include <assert.h>
#include <scoped_allocator>

#include "Nodes.h"
#include "Compiler.h"
#include "Allocating.h"
#include "SysMemCmdBlock.h"
#include "ArenaAllocator.h"
#include "PoolAllocator.h"

#include "glsl_symbols.h"
#include "glsl_ir_program.h"
#include "glsl_binary_shader.h"

namespace bvk
{

class ModuleInfo;

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

   const SpvAllocator &GetAllocator() const { return m_allocator; }
   SpvAllocator       &GetAllocator()       { return m_allocator; }

   uint32_t IdBound() const { return m_idBound; }

   void AddCapability(const NodeCapability *capability)   { m_capabilities.push_back(capability); }
   void AddExtension(const NodeExtension *extension)      { m_extensions.push_back(extension);    }
   void AddExtInstImport(const NodeExtInstImport *import) { m_extImports.push_back(import);       }
   void AddDecoration(const NodeDecorate *decorate);
   void AddGroupDecoration(const NodeGroupDecorate *decorate);
   void AddDecorationGroup(const NodeDecorationGroup *group);
   void AddGroupMemberDecoration(const NodeGroupMemberDecorate *decorate);

   void AddParameter(const NodeFunctionParameter *parameter);
   void AddInstruction(const Node *node);
   void AddMemberDecoration(const NodeMemberDecorate *node);

   void AddFunction(const NodeFunction *function)
   {
      m_functions.push_back(function);
   }

   void AddEntryPoint(const NodeEntryPoint *entryPoint)
   {
      m_entryPoints.push_back(entryPoint);
   }

   void AddVariable(const NodeVariable *node)
   {
      m_variables.push_back(node);

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

   void AddGlobal(const Node *node)
   {
      m_globals.push_back(node);
   }

   void FillNodePointer(NodeConstPtr *lhs, uint32_t i)
   {
      NodeConstPtr *rhs = &m_results[i];

      if (*rhs != nullptr)
         *lhs = *rhs;
      else
         m_forwards.push_back(std::make_pair(lhs, rhs));
   }

   void AddLabel(const NodeLabel *node);

   void SetMemoryModel(const NodeMemoryModel *memoryModel)  { m_memoryModel = memoryModel; }
   void SetSource(const NodeSource *source)                 { m_source = source;           }

   const NodeFunction *GetEntryPointFunction(const NodeEntryPoint *entryPoint) const;

   const spv::vector<const Node *> &GetGlobals() const { return m_globals;   }

   const spv::list<const Decoration *> &GetDecorations(const Node *node) const
   {
      return *m_decorations[node->GetResultId()];
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

   const NodeEntryPoint *GetEntryPoint(const char *name, spv::ExecutionModel model,
                                       uint32_t *index = nullptr) const;

   const spv::vector<const Node *> &GetNodes()     const { return m_allNodes;   }
   uint32_t                         GetNumTypes()  const { return m_nTypes;     }

private:
   void AddNode(Node *node);

   const NodeEntryPoint *GetEntryPoint(uint32_t entryPointId, uint32_t *index = nullptr);

   void AllocateArrays(const ModuleInfo &info);

   const uint32_t *ParseHeader(const uint32_t *instr);
   const uint32_t *ParseInstruction(const uint32_t *instr);

private:
   ArenaAllocator<SysMemCmdBlock, void*>  m_arena;
   SpvAllocator                           m_allocator;
   uint32_t                               m_idBound;

   spv::vector<const Node *>              m_allNodes;
   spv::vector<const Node *>              m_results;

   // Patch list for forwards references (used during construction)
   std::list<std::pair<NodeConstPtr *, NodeConstPtr *>>
                                          m_forwards;

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

   spv::vector<spv::list<const Decoration *> *>
                                          m_decorations;    // Decorations applied to nodes
};

} // Namespace bvk
