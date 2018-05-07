/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Extractor.h"
#include "Module.h"

namespace bvk {

Extractor &Extractor::operator>>(const Node *&node)
{
   m_module.FillNodePointer(&node, m_instr[m_index]);
   m_index++;

   return *this;
}

Extractor &Extractor::operator>>(const NodeType *&node)
{
   m_module.FillNodePointer(reinterpret_cast<const Node **>(&node), m_instr[m_index]);
   m_index++;

   return *this;
}

spv::ModuleAllocator<uint32_t> &Extractor::GetAllocator() const
{
   return m_module.GetArenaAllocator();
}


} // namespace bvk
