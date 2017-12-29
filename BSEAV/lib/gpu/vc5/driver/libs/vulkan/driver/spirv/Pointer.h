/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "DflowScalars.h"
#include "NodeBase.h"
#include "NodeIndex.h"
#include "SymbolHandle.h"
#include "BasicBlock.h"

namespace bvk {

class DFlowBuilder;
class NodeVariable;

//////////////////////////////////////////////////////////////////
// LoadScalars
//////////////////////////////////////////////////////////////////
class LoadScalars : public NodeAssertVisitor
{
public:
   static DflowScalars Load(DflowBuilder &builder, BasicBlockHandle block, const Node *pointer);

   void Visit(const NodeVariable            *node) override;
   void Visit(const NodeAccessChain         *node) override;
   void Visit(const NodeFunctionParameter   *node) override;
   void Visit(const NodeImageTexelPointer   *node) override;
   void Visit(const NodeCopyObject          *node) override;

private:
   LoadScalars(DflowBuilder &builder, BasicBlockHandle block, const NodeType *resultType);

   void LoadVariable(const DflowScalars &scalars, const NodeType *compositeType);
   void LoadInputVariable(const DflowScalars &scalars, const NodeType *compositeType);
   void LoadFromMemory(const NodeVariable *var, const NodeType *compositeType);
   void LoadFromWorkgroup(const NodeVariable *var, const NodeType *compositeType);
   void LoadPushConstant(const NodeVariable *var, const NodeType *compositeType);
   void LoadAccessChain(const Node *chain);

private:
   DflowBuilder           &m_builder;
   BasicBlockHandle        m_block;
   const NodeType         *m_resultType;

   spv::vector<const Node *>  m_accessChain;

   DflowScalars            m_result;
};

//////////////////////////////////////////////////////////////////
// StoreScalars
//////////////////////////////////////////////////////////////////
class StoreScalars : public NodeAssertVisitor
{
public:
   static void Store(DflowBuilder &builder, BasicBlockHandle block, const Node *pointer, const Node *value);
   static void Store(DflowBuilder &builder, BasicBlockHandle block, const Node *pointer, const DflowScalars &value);

   void Visit(const NodeVariable            *node) override;
   void Visit(const NodeAccessChain         *node) override;
   void Visit(const NodeFunctionParameter   *node) override;
   void Visit(const NodeImageTexelPointer   *node) override;
   void Visit(const NodeCopyObject          *node) override;

private:
   StoreScalars(DflowBuilder &builder, BasicBlockHandle block, const NodeType *storeType,
                const DflowScalars &scalars);

   void StoreSymbol(SymbolHandle symbol, const Node *pointer) const;
   void StoreToMemory(const NodeVariable *var) const;
   void StoreToWorkgroup(const NodeVariable *var) const;

   void StoreAccessChain(const Node *chain);

private:
   DflowBuilder           &m_builder;
   BasicBlockHandle        m_block;
   const NodeType         *m_storeType;
   const DflowScalars     &m_scalars;

   spv::vector<const Node *>  m_accessChain;
};

//////////////////////////////////////////////////////////////////
// AtomicAccess
//////////////////////////////////////////////////////////////////
class AtomicAccess : public NodeAssertVisitor
{
public:
   static DflowScalars Access(DflowBuilder &builder, DataflowFlavour atomicOp, BasicBlockHandle block,
                              const Node *pointer, const DflowScalars &value, Dflow comp = Dflow());

   void Visit(const NodeVariable            *node) override;
   void Visit(const NodeAccessChain         *node) override;
   void Visit(const NodeFunctionParameter   *node) override;
   void Visit(const NodeImageTexelPointer   *node) override;
   void Visit(const NodeCopyObject          *node) override;

private:
   AtomicAccess(DflowBuilder &builder, DataflowFlavour atomicOp, BasicBlockHandle block,
                const NodeType *storeType, const DflowScalars &scalars, Dflow comp);

   DflowScalars ReadWriteMemory(const NodeVariable *var) const;
   DflowScalars ReadWriteWorkgroup(const NodeVariable *var) const;
   DflowScalars ReadWriteTexel(const NodeImageTexelPointer *var) const;

   void         StoreAccessChain(const Node *chain);

private:
   DflowBuilder           &m_builder;
   DataflowFlavour         m_atomicOp;
   BasicBlockHandle        m_block;
   const NodeType         *m_storeType;
   DflowScalars            m_vec4;
   spv::vector<const Node *>  m_accessChain;

   DflowScalars            m_result;
};

} // namespace bvk
