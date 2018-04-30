/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <cassert>
#include "Spirv.h"
#include "ModuleAllocator.h"

namespace bvk {

class Extractor;
class NodeTypeStruct;
class Node;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Decoration
//
// Some Node types have a set of variable data in the SPIRV.
// This class contains the variable payload data for Decorations.
// The Extractor class defers to this class to import the data.
///////////////////////////////////////////////////////////////////////////////////////////////////
class Decoration
{
public:
   friend Extractor &operator>>(Extractor &ext, Decoration &deco);

   bool HasLiteral() const;

   spv::Decoration GetKind() const { return m_kind; }

   bool Is(spv::Decoration kind) const
   {
      return m_kind == kind;
   }

   uint32_t GetLiteral() const
   {
      assert(HasLiteral());
      return m_payload;
   }

   spv::BuiltIn GetBuiltIn() const
   {
      assert(m_kind == spv::Decoration::BuiltIn);
      return static_cast<spv::BuiltIn>(m_payload);
   }

   spv::FunctionParameterAttribute GetFuncParamAttr() const
   {
      assert(m_kind == spv::Decoration::FuncParamAttr);
      return static_cast<spv::FunctionParameterAttribute>(m_payload);
   }

   spv::FPRoundingMode GetFPRoundingMode() const
   {
      assert(m_kind == spv::Decoration::FPRoundingMode);
      return static_cast<spv::FPRoundingMode>(m_payload);
   }

   uint32_t GetFPFastMathModeMask() const
   {
      assert(m_kind == spv::Decoration::FPFastMathMode);
      return m_payload;
   }

private:
   spv::Decoration m_kind;
   uint32_t        m_payload = 0;
};

// Extraction operator to build a Decorations object
Extractor &operator>>(Extractor &ext, Decoration &deco);

///////////////////////////////////////////////////////////////////////////////////////////////////
// DecorationQuery
///////////////////////////////////////////////////////////////////////////////////////////////////
class DecorationQuery
{
public:
   using const_iterator = std::list<const Decoration *>::const_iterator;

   DecorationQuery(const Node *node);

   bool     Builtin(spv::BuiltIn *builtin) const;
   bool     Literal(uint32_t *literal, spv::Decoration decoType) const;
   uint32_t RequireLiteral(spv::Decoration decoType) const;
   bool     Has(spv::Decoration decoType) const;

   const_iterator begin() const { return m_decorations->begin(); }
   const_iterator end()   const { return m_decorations->end();   }

private:
   const spv::list<const Decoration *> *m_decorations;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemberDecorationQuery
///////////////////////////////////////////////////////////////////////////////////////////////////
class MemberDecorationQuery
{
public:
   using List = spv::list<std::pair<uint32_t, const Decoration *>>;

   class Iterator
   {
   private:
      void Next()
      {
         for ( ; m_current != m_end && m_current->first != m_index; ++m_current)
            continue;
      }

   public:
      Iterator(const List::const_iterator &current, const List::const_iterator &end, uint32_t index) :
         m_current(current),
         m_end(end),
         m_index(index)
      {
         Next();
      }

      Iterator(const List::const_iterator &end) :
         m_current(end),
         m_end(end),
         m_index(0)
      {}

      Iterator &operator++()
      {
         if (m_current != m_end)
            ++m_current;

         Next();

         return *this;
      }

      bool operator!=(const Iterator &rhs) const
      {
         return m_current != rhs.m_current;
      }

      const Decoration *operator*()  const { return m_current->second; }
      const Decoration *operator->() const { return m_current->second; }

   private:
      List::const_iterator m_current;
      List::const_iterator m_end;
      uint32_t             m_index;
   };

   using const_iterator = Iterator;

   MemberDecorationQuery(const NodeTypeStruct *node, uint32_t index);

   bool     Builtin(spv::BuiltIn *builtin) const;
   bool     Literal(uint32_t *literal, spv::Decoration decoType) const;
   uint32_t RequireLiteral(spv::Decoration decoType) const;
   bool     Has(spv::Decoration decoType) const;

   const_iterator begin() const { return Iterator(m_decorations.begin(), m_decorations.end(), m_index); }
   const_iterator end()   const { return Iterator(m_decorations.end());                                 }

private:
   const List &m_decorations;
   uint32_t    m_index;
};

} // namespace bvk
