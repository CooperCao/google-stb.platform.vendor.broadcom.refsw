/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "TypeBuilder.h"
#include "DflowBuilder.h"

namespace bvk {

MemberIter::MemberIter(const DflowBuilder &builder, const spv::vector<NodeIndex> &types) :
   m_builder(builder),
   m_types(types)
{}

SymbolTypeHandle MemberIter::operator[](uint32_t i) const
{
   return m_builder.GetSymbolType(m_types[i]->As<const NodeType *>());
}

} // namespace bvk
