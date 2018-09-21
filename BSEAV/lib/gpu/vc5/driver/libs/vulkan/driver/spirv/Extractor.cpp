/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Extractor.h"
#include "Module.h"

namespace bvk {

Extractor::Extractor(Module &module, const uint32_t *instr) :
   m_module   (module),
   m_allocator(module.GetAllocator()),
   m_instr    (instr),
   m_index    (1),        // Start of first argument
   m_wordCount(instr[0] >> spv::con::WordCountShift),
   m_opCode   (static_cast<spv::Core>(instr[0] & spv::con::OpCodeMask))
{
}

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

} // namespace bvk
