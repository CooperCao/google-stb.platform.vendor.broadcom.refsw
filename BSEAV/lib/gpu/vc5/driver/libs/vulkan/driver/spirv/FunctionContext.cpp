/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "FunctionContext.h"
#include "BasicBlock.h"
#include "Nodes.h"
#include "DflowBuilder.h"
#include "Pointer.h"
#include "Module.h"

namespace bvk
{

///////////////////////////////////////////////////////////////////////////////
// FunctionContext
//
// Stores information required for each function invocation
///////////////////////////////////////////////////////////////////////////////

// Private helper constructor
// The nullptr_t is just there to disambiguate this function
FunctionContext::FunctionContext(const SpvAllocator &allocator) :
   m_blocks(allocator),
   m_blockMap(std::less<const Node *>(), allocator),                 // gcc doesn't like not
   m_params(std::less<const NodeFunctionParameter *>(), allocator),  // being given the std::less
   m_dataflowSymbols(std::less<const Node *>(), allocator),          // so we have to be explicit
   m_phis(allocator)
{
}

// Create a default context with a single block -- for the entry point ("main") to be called from
FunctionContext::FunctionContext(DflowBuilder &builder) :
   FunctionContext(builder.GetAllocator())
{
   m_startBlock = BasicBlockHandle(builder.GetBasicBlockPool());
   m_blocks.push_back(m_startBlock);
}

// Create a function context from the function and the (optional) function call
// Note when the function call is nullptr, this would be an entry point
FunctionContext::FunctionContext(const NodeFunction *function, const NodeFunctionCall *call,
                                 DflowBuilder &builder) :
   FunctionContext(builder.GetAllocator())
{
   m_returnBlock  = BasicBlockHandle(builder.GetBasicBlockPool());
   m_functionCall = call;

   const spv::vector<const NodeLabel *> &blocks = function->GetData()->GetBlocks();

   // Create the blocks first to create association between the labels
   // and their corresponding blocks.
   m_blocks.reserve(blocks.size());

   for (const NodeLabel *label : blocks)
   {
      m_blocks.push_back(BasicBlockHandle(builder.GetBasicBlockPool()));
      // Record label's block
      m_blockMap[label] = m_blocks.back();
   }

   m_startBlock   = m_blocks[0];
   m_returnSymbol = builder.CreateInternal("$$return", function->GetResultType());
}

void FunctionContext::ResolvePhis(DflowBuilder &builder)
{
   for (auto &nodeSym : m_phis)
   {
      const SymbolHandle   &symbol = nodeSym.second;
      const NodePhi        *phi    = nodeSym.first;

      for (auto &var : phi->GetVariable())
      {
         BasicBlockHandle sourceBlock = BlockForLabel(var.second);
         BasicBlockHandle lastBlock   = sourceBlock->RedirectedBlock();
         DflowScalars     df          = builder.GetDataflow(lastBlock, var.first);

         builder.StoreToSymbol(lastBlock, symbol, df);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// FunctionStack
//
// Compile-time stack for function in-lining
///////////////////////////////////////////////////////////////////////////////
FunctionStack::FunctionStack(DflowBuilder &builder) :
   m_stack(builder.GetAllocator()),
   m_builder(builder)
{
   // Creates an empty block from which "main" can be called.
   m_stack.emplace_back(builder);
}

void FunctionStack::EnterFunction(const NodeFunction *function, const NodeFunctionCall *functionCall)
{
   BasicBlockHandle callingBlock = m_stack.back().GetCurrentBlock();

   std::map<const NodeFunctionParameter *, SymbolHandle> paramMap;

   if (functionCall != nullptr)
   {
      auto &actualParams = functionCall->GetArguments();

      // Instantiate the parameters
      auto &params = function->GetData()->GetParameters();

      uint32_t i = 0;
      for (const NodeFunctionParameter *param : params)
      {
         const Node            *valueNode   = actualParams[i++];
         const NodeTypePointer *pointerType = param->GetResultType()->TryAs<const NodeTypePointer *>();

         if (valueNode->IsPointer())
         {
            if (pointerType != nullptr)
            {
               paramMap[param] = m_builder.GetVariableSymbol(valueNode->As<const NodeVariable *>());
            }
            else
            {
               DflowScalars scalars = LoadScalars::Load(m_builder, callingBlock, valueNode);
               m_builder.AddDataflow(param, scalars);
            }
         }
         else
         {
            DflowScalars scalars = m_builder.GetDataflow(valueNode);

            if (pointerType != nullptr)
            {
               auto           paramType   = pointerType->GetType()->As<const NodeType *>();
               SymbolHandle   paramSymbol = m_builder.CreateInternal("$$param", paramType);

               paramMap[param] = paramSymbol;

               m_builder.StoreToSymbol(callingBlock, paramSymbol, scalars);
            }
            else
            {
               m_builder.AddDataflow(param, scalars);
            }
         }
      }
   }

   // Push new context for the function -- creates new start and return blocks
   m_stack.emplace_back(function, functionCall, m_builder);
   FunctionContext &newContext = m_stack.back();

   // Branch to the function
   callingBlock->SetFallthroughTarget(newContext.GetStartBlock());
   callingBlock->SetRedirect(newContext.GetReturnBlock());

   for (auto &p : paramMap)
      newContext.AddParameterSymbol(p.first, p.second);
}

void FunctionStack::ExitFunction()
{
   FunctionContext &functionContext = m_stack.back();

   functionContext.ResolvePhis(m_builder);

   SymbolHandle            returnSymbol = functionContext.GetReturnSymbol();
   BasicBlockHandle        returnBlock  = functionContext.GetReturnBlock();
   const NodeFunctionCall *functionCall = functionContext.GetFunctionCall();

   m_stack.pop_back();

   // Block with call has been split, so replace the current block with the return block
   m_stack.back().SetCurrentBlock(returnBlock);

   if (returnSymbol != nullptr)
   {
      DflowScalars result = m_builder.LoadFromSymbol(returnBlock, returnSymbol);

      m_builder.AddDataflow(functionCall, result);
   }
}

void FunctionStack::Return(const DflowScalars *value)
{
   const FunctionContext &context      = m_stack.back();
   BasicBlockHandle       currentBlock = context.GetCurrentBlock();

   // Is this a return with value?
   if (value != nullptr)
   {
      assert(context.GetReturnSymbol() != nullptr);

      m_builder.StoreToSymbol(currentBlock, context.GetReturnSymbol(), *value);
   }

   // Branch to return block
   currentBlock->SetFallthroughTarget(context.GetReturnBlock());
}

}
