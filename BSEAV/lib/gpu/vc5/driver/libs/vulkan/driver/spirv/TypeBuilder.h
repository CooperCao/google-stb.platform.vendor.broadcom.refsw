/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "NodeBase.h"
#include "Module.h"

namespace bvk {

class DflowBuilder;

class MemberIter
{
public:
   MemberIter(const DflowBuilder &builder, const spv::vector<NodeIndex> &types);

   SymbolTypeHandle operator[](uint32_t i) const;

   uint32_t size() const { return m_types.size(); }

private:
   const DflowBuilder           &m_builder;
   const spv::vector<NodeIndex> &m_types;
};

} // namespace bvk
