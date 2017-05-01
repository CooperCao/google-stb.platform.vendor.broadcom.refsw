/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_VISITOR_H__
#define __BSG_VISITOR_H__

#include <memory.h>
#include <stdint.h>
#include "bsg_common.h"
#include "bsg_scene_node.h"

namespace bsg
{

//! The base class for tree visitors - visitors implement the algorithm over the scene graph.
class Visitor
{
public:
   //! Abstract interface for starting a visit traversal at a given scene node
   virtual void Visit(const SceneNode &node) = 0;
};

}

#endif /* __BSG_VISITOR_H__ */
