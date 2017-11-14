/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "NodeData.h"
#include "Nodes.h"

namespace bvk {

///////////////////////////////////////////////////////////////////////////////
// DataTypeStruct
///////////////////////////////////////////////////////////////////////////////
void DataTypeStruct::SetMemberDecoration(uint32_t member, const Decoration *decoration)
{
   m_memberDecorations.emplace_back(member, decoration);
}

///////////////////////////////////////////////////////////////////////////////
// DataFunction
///////////////////////////////////////////////////////////////////////////////
void DataFunction::AddInstruction(const Node *instruction)
{
   m_blocks.back()->GetData()->AddInstruction(instruction);
}

} // namespace bvk
