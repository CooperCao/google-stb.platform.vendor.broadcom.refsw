/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Nodes.h"

namespace bvk
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Node
///////////////////////////////////////////////////////////////////////////////////////////////////

bool Node::IsPointer() const
{
   assert(m_resultType != nullptr);
   return m_resultType->GetOpCode() == spv::Core::OpTypePointer;
}

}