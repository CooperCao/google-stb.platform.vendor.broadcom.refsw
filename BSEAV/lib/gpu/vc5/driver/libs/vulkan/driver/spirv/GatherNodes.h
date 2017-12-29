/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "NodeBase.h"

namespace bvk {

class Module;

//////////////////////////////////////////////////////////////////
// GatherNodes
//
// Adds instructions, types and other specific data to the module.
//////////////////////////////////////////////////////////////////
class GatherNodes: public NodeEmptyVisitor
{
public:
   static void Gather(Module &module);

private:
   GatherNodes(Module &module) :
      m_module(module)
   {}

   // Adds a visit method declaration for Nodes with special behaviour
   void Visit(const NodeCapability *node)          override;
   void Visit(const NodeExtension *node)           override;
   void Visit(const NodeExtInstImport *node)       override;
   void Visit(const NodeMemoryModel *node)         override;
   void Visit(const NodeEntryPoint *node)          override;
   void Visit(const NodeSource *node)              override;
   void Visit(const NodeDecorate *node)            override;
   void Visit(const NodeDecorationGroup *node)     override;
   void Visit(const NodeGroupDecorate *node)       override;
   void Visit(const NodeName *node)                override;
   void Visit(const NodeExecutionMode *node)       override;
   void Visit(const NodeVariable *node)            override;
   void Visit(const NodeLabel *node)               override;
   void Visit(const NodeFunction *node)            override;
   void Visit(const NodeFunctionParameter *node)   override;
   void Visit(const NodeMemberDecorate *node)      override;
   void Visit(const NodeGroupMemberDecorate *node) override;

   void Visit(const NodeTypeVoid *node)         override;
   void Visit(const NodeTypeBool *node)         override;
   void Visit(const NodeTypeInt *node)          override;
   void Visit(const NodeTypeFloat *node)        override;
   void Visit(const NodeTypeVector *node)       override;
   void Visit(const NodeTypeMatrix *node)       override;
   void Visit(const NodeTypeImage *node)        override;
   void Visit(const NodeTypeSampler *node)      override;
   void Visit(const NodeTypeSampledImage *node) override;
   void Visit(const NodeTypeArray *node)        override;
   void Visit(const NodeTypeRuntimeArray *node) override;
   void Visit(const NodeTypeStruct *node)       override;
   void Visit(const NodeTypePointer *node)      override;
   void Visit(const NodeTypeFunction *node)     override;

private:
   Module &m_module;
};

} // namespace bvk
