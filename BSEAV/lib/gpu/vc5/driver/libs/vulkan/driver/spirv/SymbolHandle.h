/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "glsl_common.h"
#include "glsl_symbols.h"
#include "SymbolTypeHandle.h"
#include "ModuleAllocator.h"
#include "Spirv.h"

namespace bvk {

class NodeVariable;
class Module;
class SymbolListHandle;

FormatQualifier ConvertToFormatQualifier(spv::ImageFormat fmt);

////////////////////////////////////////////////////
// SymbolHandle
//
// Wraps the glsl 'C' Symbol with the C++ interface
// needed for the SPIR-V translation
////////////////////////////////////////////////////
class SymbolHandle
{
public:
   SymbolHandle() = default;
   SymbolHandle(const SymbolHandle &rhs) = default;
   explicit SymbolHandle(Symbol *symbol);

   // Named constructors
   static SymbolHandle Variable(const Module &module, ShaderFlavour flavour, const char *name,
                                const NodeVariable *var, SymbolTypeHandle type);

   static SymbolHandle Builtin(const Module &module, const char *name,
                               spv::StorageClass storageClass, SymbolTypeHandle type);

   static SymbolHandle Internal(const Module &module, const char *name, SymbolTypeHandle type);

   static SymbolHandle SharedBlock(const SymbolListHandle &symbols);

   // Test for nullptr
   explicit operator bool() const;

   // Casts
   operator       Symbol *();
   operator const Symbol *() const;

   // Getters
   StorageQualifier       GetStorageQualifier() const;
   InterpolationQualifier GetInterpQualifier()  const;
   MemLayout             *GetMemLayout()        const;
   uint32_t               GetOffset()           const;
   uint32_t               GetBlockSize()        const;
   SymbolTypeHandle       GetType()             const;
   uint32_t               GetNumScalars()       const;
   SymbolFlavour          GetFlavour()          const;
   const char            *GetName()             const;

   // Setters
   void                   SetName(const char *newName);

   // Debug
   void                   DebugPrint() const;

private:
   SymbolHandle(const Module &module);

private:
   Symbol   *m_symbol = nullptr;
};

////////////////////////////////////////////////////////
// SymbolListHandle
//
// Wraps the glsl 'C' SymbolList with a C++ interface
////////////////////////////////////////////////////////
class SymbolListHandle
{
public:
   SymbolListHandle()
   {
      // We don't own this memory
      m_symbolList = glsl_symbol_list_new();
   }

   explicit SymbolListHandle(SymbolList *symbolList) :
      m_symbolList(symbolList)
   {}

   operator       SymbolList *()       { return m_symbolList; }
   operator const SymbolList *() const { return m_symbolList; }

   void push_back(SymbolHandle sym) { glsl_symbol_list_append(m_symbolList, sym); }

   class iterator
   {
   public:
      iterator(SymbolListNode *ptr) : m_ptr(ptr) {}

      iterator &operator++() { m_ptr = m_ptr->next; return *this; }
      bool operator!=(const iterator &other) const { return m_ptr != other.m_ptr; }
      bool operator==(const iterator &other) const { return m_ptr == other.m_ptr; }
      SymbolHandle operator*() { return SymbolHandle(m_ptr->s); }

      SymbolListNode *m_ptr;
   };

   class const_iterator
   {
   public:
      const_iterator(SymbolListNode *ptr) : m_ptr(ptr) {}

      const_iterator &operator++() { m_ptr = m_ptr->next; return *this; }
      bool operator!=(const const_iterator &other) const { return m_ptr != other.m_ptr; }
      bool operator==(const const_iterator &other) const { return m_ptr == other.m_ptr; }
      const SymbolHandle operator*() const { return SymbolHandle(m_ptr->s); }

      SymbolListNode *m_ptr;
   };

   iterator begin() { return iterator(m_symbolList->head);    }
   iterator end()   { return iterator(nullptr); }

   const_iterator begin() const { return const_iterator(m_symbolList->head);    }
   const_iterator end() const   { return const_iterator(nullptr); }

   size_t size() const
   {
      size_t          i;
      SymbolListNode *n;

      for (i = 0, n = m_symbolList->head; n != NULL; n = n->next, i++)
         continue;

      return i;
   }

private:
   SymbolList  *m_symbolList;
};

//
// SymbolHandle inlines
//
inline SymbolHandle::SymbolHandle(Symbol *symbol) :
   m_symbol(symbol)
{}

inline SymbolHandle::operator bool() const
{
   return m_symbol != nullptr;
}

inline SymbolHandle::operator Symbol *()
{
   return m_symbol;
}

inline SymbolHandle::operator const Symbol *() const
{
   return m_symbol;
}

inline SymbolTypeHandle SymbolHandle::GetType() const
{
   return SymbolTypeHandle(m_symbol->type);
}

inline uint32_t SymbolHandle::GetNumScalars() const
{
   return GetType().GetNumScalars();
}

inline SymbolFlavour SymbolHandle::GetFlavour() const
{
   return m_symbol->flavour;
}

inline const char *SymbolHandle::GetName() const
{
   return m_symbol->name;
}

inline void SymbolHandle::SetName(const char *newName)
{
   m_symbol->name = newName;
}

inline StorageQualifier SymbolHandle::GetStorageQualifier() const
{
   assert(m_symbol->flavour == SYMBOL_VAR_INSTANCE);
   return m_symbol->u.var_instance.storage_qual;
}

inline InterpolationQualifier SymbolHandle::GetInterpQualifier() const
{
   assert(m_symbol->flavour == SYMBOL_VAR_INSTANCE);
   return m_symbol->u.var_instance.interp_qual;
}

inline MemLayout *SymbolHandle::GetMemLayout() const
{
   assert(m_symbol->flavour == SYMBOL_VAR_INSTANCE);
   return m_symbol->u.var_instance.block_info.layout;
}

inline uint32_t SymbolHandle::GetOffset() const
{
   assert(m_symbol->flavour == SYMBOL_VAR_INSTANCE);
   return m_symbol->u.var_instance.offset;
}

inline uint32_t SymbolHandle::GetBlockSize() const
{
   assert(m_symbol->flavour == SYMBOL_INTERFACE_BLOCK);
   return m_symbol->u.interface_block.block_data_type->u.block_type.layout->u.struct_layout.size;
}

class SymbolHandleCompare
{
public:
   bool operator()(const SymbolHandle &lhs, const SymbolHandle &rhs) const
   {
      return std::less<const Symbol *>()(static_cast<const Symbol *>(lhs),
                                         static_cast<const Symbol *>(rhs));
   }
};

} // namespace bvk
