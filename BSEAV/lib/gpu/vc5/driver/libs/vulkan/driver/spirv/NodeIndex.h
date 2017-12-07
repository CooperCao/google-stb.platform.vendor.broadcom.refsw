/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <cstdint>
#include <assert.h>
#include <type_traits>

namespace bvk {

class Node;

/////////////////////////////////////////////////
// NodeIndex
//
// Holds a pointer to a node-pointer that will be
// populated once all the nodes are imported.
// Mostly there are no forwards references, but
// in some cases there are.  This redirection
// hides any potential forwards references.
//
// The node-pointers are held in Module::m_results
//////////////////////////////////////////////////
class NodeIndex
{
private:
   const Node *GetNode() const { return m_node == nullptr ? nullptr : *m_node; }

public:
   void operator=(const Node **node)   { m_node = node;                 }

   operator const Node *() const       { return GetNode();              }

   explicit operator bool() const      { return GetNode() != nullptr;   }

   const Node *operator->() const      { return GetNode();              }

private:
   const Node  **m_node = nullptr;
};

} // namespace bvk
