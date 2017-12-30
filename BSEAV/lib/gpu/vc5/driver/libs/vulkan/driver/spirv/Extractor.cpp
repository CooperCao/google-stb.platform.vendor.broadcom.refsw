/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Extractor.h"
#include "Module.h"

namespace bvk {

Extractor &Extractor::operator>>(NodeIndex &index)
{
   uint32_t i = m_instr[m_index];
   index = m_module.GetNodePtr(i);
   m_index++;
   return *this;
}

Extractor &Extractor::operator>>(const NodeType *&type)
{
   uint32_t i = m_instr[m_index];
   type = (*m_module.GetNodePtr(i))->As<const NodeType *>();
   m_index++;
   return *this;
}

spv::ModuleAllocator<uint32_t> &Extractor::GetAllocator() const
{
   return m_module.GetArenaAllocator();
}


} // namespace bvk
