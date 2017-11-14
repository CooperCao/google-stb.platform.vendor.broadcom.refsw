/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "ModuleAllocator.h"
#include "NodeIndex.h"
#include "SymbolHandle.h"
#include "BasicBlock.h"

namespace bvk
{

class Node;
class NodeLabel;
class NodePhi;
class NodeFunction;
class NodeFunctionCall;
class NodeFunctionParameter;
class DflowBuilder;
class DflowScalars;

// FunctionContext
//
// Holds the list of blocks associated with labels in the current
// function.  Keeps the notion of "current block" which is the
// block into which instructions should be added.
class FunctionContext
{
public:
   // Base context from which to call "main"
   FunctionContext(DflowBuilder &builder);

   // General contexts
   FunctionContext(const NodeFunction *function, const NodeFunctionCall *call, DflowBuilder &builder);

   // Gets the first block (stored as the block list can be modified)
   BasicBlockHandle GetStartBlock() const
   {
      return m_startBlock;
   }

   // Gets the current block
   BasicBlockHandle GetCurrentBlock() const
   {
      return m_blocks[m_currentIndex];
   }

   void SetCurrentBlock(BasicBlockHandle block)
   {
      m_blocks[m_currentIndex] = block;
   }

   // Move to the next block.
   // Current and next block will moved along
   void NextBlock()
   {
      m_currentIndex += 1;
   }

   SymbolHandle GetReturnSymbol() const
   {
      return m_returnSymbol;
   }

   BasicBlockHandle GetReturnBlock() const
   {
      return m_returnBlock;
   }

   const NodeFunctionCall *GetFunctionCall() const
   {
      return m_functionCall;
   }

   void AddParameterSymbol(const NodeFunctionParameter *node, SymbolHandle symbol)
   {
      m_params[node] = symbol;
   }

   SymbolHandle GetParameterSymbol(const NodeFunctionParameter *node) const
   {
      return m_params.at(node);
   }

   void AddPhi(const NodePhi *node, SymbolHandle sym)
   {
      m_phis.emplace_back(node, sym);
   }

   BasicBlockHandle BlockForLabel(const Node *label)
   {
      return m_blockMap[label];
   }

   void SetSymbol(const NodeVariable *var, SymbolHandle symbol)
   {
      m_symbols[var] = symbol;
   }

   SymbolHandle GetSymbol(const NodeVariable *var)
   {
      return m_symbols[var];
   }

   void SetDataflowSymbol(const Node *node, SymbolHandle symbol)
   {
      m_dataflowSymbols[node] = symbol;
   }

   SymbolHandle GetDataflowSymbol(const Node *node)
   {
      auto iter = m_dataflowSymbols.find(node);

      if (iter == m_dataflowSymbols.end())
         return SymbolHandle();

      return (*iter).second;
   }

   void ResolvePhis(DflowBuilder &builder);

private:
   FunctionContext(DflowBuilder &builder, nullptr_t);

private:
   // Basic blocks for this function -- one per label.
   spv::vector<BasicBlockHandle>             m_blocks;
   spv::map<const Node *, BasicBlockHandle>  m_blockMap;
   BasicBlockHandle                          m_startBlock;

   // Parameter map
   spv::map<const NodeFunctionParameter *, SymbolHandle>  m_params;

   // Variable map
   spv::map<const NodeVariable *, SymbolHandle> m_symbols;
   spv::map<const Node *,         SymbolHandle> m_dataflowSymbols;

   // Current block.  Instructions will be added into this block.
   uint32_t                       m_currentIndex = 0;

   BasicBlockHandle               m_returnBlock;
   SymbolHandle                   m_returnSymbol;

   // Record all the phi nodes processing at the end of the function
   spv::list<std::pair<const NodePhi *, SymbolHandle>>   m_phis;

   // Record of the calling node
   const NodeFunctionCall        *m_functionCall = nullptr;
};

// FunctionStack
//
// A stack of function contexts to track function calls during unrolling.
// Use -> to access methods on top-of-stack
class FunctionStack
{
public:
   FunctionStack(DflowBuilder &builder);

   // Push a new function context onto the stack
   void EnterFunction(const NodeFunction *function, const NodeFunctionCall *call);
   void ExitFunction();

   // Record return value and link to return address
   void Return(const DflowScalars *value);

   // Redirect onto top of stack
   FunctionContext *operator->()
   {
      assert(m_stack.size() > 0);
      return &m_stack.back();
   }

   // Redirect onto top of stack
   const FunctionContext *operator->() const
   {
      assert(m_stack.size() > 0);
      return &m_stack.back();
   }

   FunctionContext *GlobalContext()
   {
      return &m_stack.front();
   }

   const FunctionContext *GlobalContext() const
   {
      return &m_stack.front();
   }

private:
   spv::list<FunctionContext>  m_stack;
   DflowBuilder               &m_builder;
};

}